///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2014 John Greb <hexameron@spam.no>                              //
// Copyright (C) 2015-2022 Edouard Griffiths, F4EXB <f4exb06@gmail.com>          //
// Copyright (C) 2020 Kacper Michajłow <kasper93@gmail.com>                      //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <complex.h>

#include <QTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QThread>

#include "SWGChannelSettings.h"
#include "SWGWorkspaceInfo.h"
#include "SWGRDFDemodSettings.h"
#include "SWGChannelReport.h"
#include "SWGRDFDemodReport.h"

#include "device/deviceapi.h"
#include "dsp/dspcommands.h"
#include "dsp/devicesamplemimo.h"
#include "device/deviceapi.h"
#include "settings/serializable.h"
#include "util/db.h"
#include "maincore.h"

#include "rdfdemod.h"

MESSAGE_CLASS_DEFINITION(RDFDemod::MsgConfigureRDFDemod, Message)

const char* const RDFDemod::m_channelIdURI = "sdrangel.channel.rdfdemod";
const char* const RDFDemod::m_channelId = "RDFDemod";
const int RDFDemod::m_udpBlockSize = 512;

RDFDemod::RDFDemod(DeviceAPI* deviceAPI) :
        ChannelAPI(m_channelIdURI, ChannelAPI::StreamSingleSink),
        m_deviceAPI(deviceAPI),
        m_thread(nullptr),
        m_basebandSink(nullptr),
        m_running(false),
        m_basebandSampleRate(0)
{
	setObjectName(m_channelId);
	applySettings(m_settings, true);

    m_deviceAPI->addChannelSink(this);
    m_deviceAPI->addChannelSinkAPI(this);

    m_networkManager = new QNetworkAccessManager();
    QObject::connect(
        m_networkManager,
        &QNetworkAccessManager::finished,
        this,
        &RDFDemod::networkManagerFinished
    );
    QObject::connect(
        this,
        &ChannelAPI::indexInDeviceSetChanged,
        this,
        &RDFDemod::handleIndexInDeviceSetChanged
    );
}

RDFDemod::~RDFDemod()
{
    qDebug() << "========================================";
    qDebug() << "RDFDemod::~RDFDemod()";

    // Stop DSP processing before destroying the RDF demodulator.
    stop();

    if (m_deviceAPI)
    {
        m_deviceAPI->removeChannelSinkAPI(this);
        m_deviceAPI->removeChannelSink(this);
    }

    if (m_networkManager)
    {
        m_networkManager->disconnect();
        delete m_networkManager;
        m_networkManager = nullptr;
    }

    qDebug() << "RDFDemod destroyed";
    qDebug() << "========================================";
}

void RDFDemod::setDeviceAPI(DeviceAPI *deviceAPI)
{
    if (deviceAPI != m_deviceAPI)
    {
        m_deviceAPI->removeChannelSinkAPI(this);
        m_deviceAPI->removeChannelSink(this);
        m_deviceAPI = deviceAPI;
        m_deviceAPI->addChannelSink(this);
        m_deviceAPI->addChannelSinkAPI(this);
    }
}

uint32_t RDFDemod::getNumberOfDeviceStreams() const
{
    return m_deviceAPI->getNbSourceStreams();
}

void RDFDemod::feed(const SampleVector::const_iterator& begin, const SampleVector::const_iterator& end, bool firstOfBurst)
{
    (void) firstOfBurst;

    if (m_running) {
        m_basebandSink->feed(begin, end);
    }
}

void RDFDemod::start()
{
    if (m_running) {
        return;
    }

    qDebug() << "RDFDemod::start";

    m_thread = new QThread(this);
    m_basebandSink = new RDFDemodBaseband();
    m_basebandSink->setChannel(this);
    m_basebandSink->moveToThread(m_thread);

    QObject::connect(m_thread, &QThread::finished, m_basebandSink, &QObject::deleteLater);
    QObject::connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

    if (m_basebandSampleRate != 0) {
        m_basebandSink->setBasebandSampleRate(m_basebandSampleRate);
    }

    m_basebandSink->reset();
    m_thread->start();

    RDFDemodBaseband::MsgConfigureRDFDemodBaseband *msg = RDFDemodBaseband::MsgConfigureRDFDemodBaseband::create(m_settings, true);
    m_basebandSink->getInputMessageQueue()->push(msg);

    m_running = true;
}

void RDFDemod::stop()
{
    qDebug() << "========================================";
    qDebug() << "RDFDemod::stop()";
    qDebug() << "m_running =" << m_running;
    qDebug() << "m_thread =" << m_thread;

    if (!m_running)
    {
        qDebug() << "RDFDemod::stop(): already stopped";
        qDebug() << "========================================";
        return;
    }

    m_running = false;

    if (m_thread)
    {
        qDebug() << "Stopping RDF thread...";

        m_thread->quit();
        m_thread->wait();

        qDebug() << "RDF thread stopped";
    }

    qDebug() << "========================================";
}

bool RDFDemod::handleMessage(const Message& cmd)
{
    if (MsgConfigureRDFDemod::match(cmd))
    {
        MsgConfigureRDFDemod& cfg = (MsgConfigureRDFDemod&) cmd;
        qDebug("RDFDemod::handleMessage: MsgConfigureRDFDemod");

        applySettings(cfg.getSettings(), cfg.getForce());

        return true;
    }
    else if (DSPSignalNotification::match(cmd))
    {
        DSPSignalNotification& notif = (DSPSignalNotification&) cmd;
        m_basebandSampleRate = notif.getSampleRate();
        qDebug() << "RDFDemod::handleMessage: DSPSignalNotification";

        // Forward to the sink
        if (m_running)
        {
            DSPSignalNotification* rep = new DSPSignalNotification(notif); // make a copy
            m_basebandSink->getInputMessageQueue()->push(rep);
        }

        // Forwatd to GUI if any
        if (getMessageQueueToGUI()) {
            getMessageQueueToGUI()->push(new DSPSignalNotification(notif));
        }

        return true;
    }
    else if (MainCore::MsgChannelDemodQuery::match(cmd))
    {
        qDebug() << "RDFDemod::handleMessage: MsgChannelDemodQuery";
        sendSampleRateToDemodAnalyzer();

        return true;
    }
	else
	{
		return false;
	}
}

void RDFDemod::setCenterFrequency(qint64 frequency)
{
    RDFDemodSettings settings = m_settings;
    settings.m_inputFrequencyOffset = frequency;
    applySettings(settings, false);

    if (m_guiMessageQueue) // forward to GUI if any
    {
        MsgConfigureRDFDemod *msgToGUI = MsgConfigureRDFDemod::create(settings, false);
        m_guiMessageQueue->push(msgToGUI);
    }
}

void RDFDemod::applySettings(const RDFDemodSettings& settings, bool force)
{
    qDebug() << "RDFDemod::applySettings:"
            << " m_inputFrequencyOffset: " << settings.m_inputFrequencyOffset
            << " m_rfBandwidth: " << settings.m_rfBandwidth
            << " m_afBandwidth: " << settings.m_afBandwidth
            << " m_volume: " << settings.m_volume
            << " m_squelch: " << settings.m_squelch
            << " m_audioDeviceName: " << settings.m_audioDeviceName
            << " m_audioMute: " << settings.m_audioMute
            << " m_streamIndex: " << settings.m_streamIndex
            << " m_useReverseAPI: " << settings.m_useReverseAPI
            << " m_reverseAPIAddress: " << settings.m_reverseAPIAddress
            << " m_reverseAPIPort: " << settings.m_reverseAPIPort
            << " m_reverseAPIDeviceIndex: " << settings.m_reverseAPIDeviceIndex
            << " m_reverseAPIChannelIndex: " << settings.m_reverseAPIChannelIndex
            << " force: " << force;

    QList<QString> reverseAPIKeys;

    if((settings.m_inputFrequencyOffset != m_settings.m_inputFrequencyOffset) || force) {
        reverseAPIKeys.append("inputFrequencyOffset");
    }
    if((settings.m_rfBandwidth != m_settings.m_rfBandwidth) || force) {
        reverseAPIKeys.append("rfBandwidth");
    }
    if((settings.m_afBandwidth != m_settings.m_afBandwidth) || force) {
        reverseAPIKeys.append("afBandwidth");
    }
    if((settings.m_volume != m_settings.m_volume) || force) {
        reverseAPIKeys.append("volume");
    }
    if((settings.m_squelch != m_settings.m_squelch) || force) {
        reverseAPIKeys.append("squelch");
    }
    if((settings.m_audioMute != m_settings.m_audioMute) || force) {
        reverseAPIKeys.append("audioMute");
    }
    if((settings.m_audioDeviceName != m_settings.m_audioDeviceName) || force) {
        reverseAPIKeys.append("audioDeviceName");
    }
    if((settings.m_title != m_settings.m_title) || force) {
        reverseAPIKeys.append("title");
    }
    if((settings.m_rgbColor != m_settings.m_rgbColor) || force) {
        reverseAPIKeys.append("rgbColor");
    }

    if (m_settings.m_streamIndex != settings.m_streamIndex)
    {
        if (m_deviceAPI->getSampleMIMO()) // change of stream is possible for MIMO devices only
        {
            m_deviceAPI->removeChannelSinkAPI(this);
            m_deviceAPI->removeChannelSink(this, m_settings.m_streamIndex);
            m_deviceAPI->addChannelSink(this, settings.m_streamIndex);
            m_deviceAPI->addChannelSinkAPI(this);
            m_settings.m_streamIndex = settings.m_streamIndex; // make sure ChannelAPI::getStreamIndex() is consistent
            emit streamIndexChanged(settings.m_streamIndex);
        }

        reverseAPIKeys.append("streamIndex");
    }

    if (m_running)
    {
        RDFDemodBaseband::MsgConfigureRDFDemodBaseband *msg = RDFDemodBaseband::MsgConfigureRDFDemodBaseband::create(settings, force);
        m_basebandSink->getInputMessageQueue()->push(msg);
    }

    if (settings.m_useReverseAPI)
    {
        bool fullUpdate = ((m_settings.m_useReverseAPI != settings.m_useReverseAPI) && settings.m_useReverseAPI) ||
                (m_settings.m_reverseAPIAddress != settings.m_reverseAPIAddress) ||
                (m_settings.m_reverseAPIPort != settings.m_reverseAPIPort) ||
                (m_settings.m_reverseAPIDeviceIndex != settings.m_reverseAPIDeviceIndex) ||
                (m_settings.m_reverseAPIChannelIndex != settings.m_reverseAPIChannelIndex);
        webapiReverseSendSettings(reverseAPIKeys, settings, fullUpdate || force);
    }

    QList<ObjectPipe*> pipes;
    MainCore::instance()->getMessagePipes().getMessagePipes(this, "settings", pipes);

    if (pipes.size() > 0) {
        sendChannelSettings(pipes, reverseAPIKeys, settings, force);
    }

    m_settings = settings;
}

QByteArray RDFDemod::serialize() const
{
    return m_settings.serialize();
}

bool RDFDemod::deserialize(const QByteArray& data)
{
    if (m_settings.deserialize(data))
    {
        MsgConfigureRDFDemod *msg = MsgConfigureRDFDemod::create(m_settings, true);
        m_inputMessageQueue.push(msg);
        return true;
    }
    else
    {
        m_settings.resetToDefaults();
        MsgConfigureRDFDemod *msg = MsgConfigureRDFDemod::create(m_settings, true);
        m_inputMessageQueue.push(msg);
        return false;
    }
}

void RDFDemod::sendSampleRateToDemodAnalyzer()
{
    QList<ObjectPipe*> pipes;
    MainCore::instance()->getMessagePipes().getMessagePipes(this, "reportdemod", pipes);

    if (pipes.size() > 0)
    {
        for (const auto& pipe: pipes)
        {
            MessageQueue* messageQueue = qobject_cast<MessageQueue*>(pipe->m_element);
            MainCore::MsgChannelDemodReport *msg = MainCore::MsgChannelDemodReport::create(
                this,
                getAudioSampleRate()
            );
            messageQueue->push(msg);

        }
    }
}

int RDFDemod::webapiSettingsGet(
        SWGSDRangel::SWGChannelSettings& response,
        QString& errorMessage)
{
    (void) errorMessage;
    response.setRdfDemodSettings(new SWGSDRangel::SWGRDFDemodSettings());
    response.getRdfDemodSettings()->init();
    webapiFormatChannelSettings(response, m_settings);
    return 200;
}

int RDFDemod::webapiWorkspaceGet(
        SWGSDRangel::SWGWorkspaceInfo& response,
        QString& errorMessage)
{
    (void) errorMessage;
    response.setIndex(m_settings.m_workspaceIndex);
    return 200;
}

int RDFDemod::webapiSettingsPutPatch(
        bool force,
        const QStringList& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings& response,
        QString& errorMessage)
{
    (void) errorMessage;
    RDFDemodSettings settings = m_settings;
    webapiUpdateChannelSettings(settings, channelSettingsKeys, response);

    MsgConfigureRDFDemod *msg = MsgConfigureRDFDemod::create(settings, force);
    m_inputMessageQueue.push(msg);

    qDebug("RDFDemod::webapiSettingsPutPatch: forward to GUI: %p", m_guiMessageQueue);
    if (m_guiMessageQueue) // forward to GUI if any
    {
        MsgConfigureRDFDemod *msgToGUI = MsgConfigureRDFDemod::create(settings, force);
        m_guiMessageQueue->push(msgToGUI);
    }

    webapiFormatChannelSettings(response, settings);

    return 200;
}

void RDFDemod::webapiUpdateChannelSettings(
        RDFDemodSettings& settings,
        const QStringList& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings& response)
{
    if (channelSettingsKeys.contains("inputFrequencyOffset")) {
        settings.m_inputFrequencyOffset = response.getRdfDemodSettings()->getInputFrequencyOffset();
    }
    if (channelSettingsKeys.contains("rfBandwidth")) {
        settings.m_rfBandwidth = response.getRdfDemodSettings()->getRfBandwidth();
    }
    if (channelSettingsKeys.contains("afBandwidth")) {
        settings.m_afBandwidth = response.getRdfDemodSettings()->getAfBandwidth();
    }
    if (channelSettingsKeys.contains("volume")) {
        settings.m_volume = response.getRdfDemodSettings()->getVolume();
    }
    if (channelSettingsKeys.contains("squelch")) {
        settings.m_squelch = response.getRdfDemodSettings()->getSquelch();
    }
    if (channelSettingsKeys.contains("audioMute")) {
        settings.m_audioMute = response.getRdfDemodSettings()->getAudioMute() != 0;
    }
    if (channelSettingsKeys.contains("rgbColor")) {
        settings.m_rgbColor = response.getRdfDemodSettings()->getRgbColor();
    }
    if (channelSettingsKeys.contains("title")) {
        settings.m_title = *response.getRdfDemodSettings()->getTitle();
    }
    if (channelSettingsKeys.contains("audioDeviceName")) {
        settings.m_audioDeviceName = *response.getRdfDemodSettings()->getAudioDeviceName();
    }
    if (channelSettingsKeys.contains("streamIndex")) {
        settings.m_streamIndex = response.getRdfDemodSettings()->getStreamIndex();
    }
    if (channelSettingsKeys.contains("useReverseAPI")) {
        settings.m_useReverseAPI = response.getRdfDemodSettings()->getUseReverseApi() != 0;
    }
    if (channelSettingsKeys.contains("reverseAPIAddress")) {
        settings.m_reverseAPIAddress = *response.getRdfDemodSettings()->getReverseApiAddress();
    }
    if (channelSettingsKeys.contains("reverseAPIPort")) {
        settings.m_reverseAPIPort = response.getRdfDemodSettings()->getReverseApiPort();
    }
    if (channelSettingsKeys.contains("reverseAPIDeviceIndex")) {
        settings.m_reverseAPIDeviceIndex = response.getRdfDemodSettings()->getReverseApiDeviceIndex();
    }
    if (channelSettingsKeys.contains("reverseAPIChannelIndex")) {
        settings.m_reverseAPIChannelIndex = response.getRdfDemodSettings()->getReverseApiChannelIndex();
    }
    if (settings.m_channelMarker && channelSettingsKeys.contains("channelMarker")) {
        settings.m_channelMarker->updateFrom(channelSettingsKeys, response.getRdfDemodSettings()->getChannelMarker());
    }
    if (settings.m_rollupState && channelSettingsKeys.contains("rollupState")) {
        settings.m_rollupState->updateFrom(channelSettingsKeys, response.getRdfDemodSettings()->getRollupState());
    }
}

int RDFDemod::webapiReportGet(
        SWGSDRangel::SWGChannelReport& response,
        QString& errorMessage)
{
    (void) errorMessage;
    response.setRdfDemodReport(new SWGSDRangel::SWGRDFDemodReport());
    response.getRdfDemodReport()->init();
    webapiFormatChannelReport(response);
    return 200;
}

void RDFDemod::webapiFormatChannelSettings(SWGSDRangel::SWGChannelSettings& response, const RDFDemodSettings& settings)
{
    response.getRdfDemodSettings()->setInputFrequencyOffset(settings.m_inputFrequencyOffset);
    response.getRdfDemodSettings()->setRfBandwidth(settings.m_rfBandwidth);
    response.getRdfDemodSettings()->setAfBandwidth(settings.m_afBandwidth);
    response.getRdfDemodSettings()->setVolume(settings.m_volume);
    response.getRdfDemodSettings()->setSquelch(settings.m_squelch);
    response.getRdfDemodSettings()->setAudioMute(settings.m_audioMute ? 1 : 0);
    response.getRdfDemodSettings()->setRgbColor(settings.m_rgbColor);

    if (response.getRdfDemodSettings()->getTitle()) {
        *response.getRdfDemodSettings()->getTitle() = settings.m_title;
    } else {
        response.getRdfDemodSettings()->setTitle(new QString(settings.m_title));
    }

    if (response.getRdfDemodSettings()->getAudioDeviceName()) {
        *response.getRdfDemodSettings()->getAudioDeviceName() = settings.m_audioDeviceName;
    } else {
        response.getRdfDemodSettings()->setAudioDeviceName(new QString(settings.m_audioDeviceName));
    }

    response.getRdfDemodSettings()->setStreamIndex(settings.m_streamIndex);
    response.getRdfDemodSettings()->setUseReverseApi(settings.m_useReverseAPI ? 1 : 0);

    if (response.getRdfDemodSettings()->getReverseApiAddress()) {
        *response.getRdfDemodSettings()->getReverseApiAddress() = settings.m_reverseAPIAddress;
    } else {
        response.getRdfDemodSettings()->setReverseApiAddress(new QString(settings.m_reverseAPIAddress));
    }

    response.getRdfDemodSettings()->setReverseApiPort(settings.m_reverseAPIPort);
    response.getRdfDemodSettings()->setReverseApiDeviceIndex(settings.m_reverseAPIDeviceIndex);
    response.getRdfDemodSettings()->setReverseApiChannelIndex(settings.m_reverseAPIChannelIndex);

    if (settings.m_channelMarker)
    {
        if (response.getRdfDemodSettings()->getChannelMarker())
        {
            settings.m_channelMarker->formatTo(response.getRdfDemodSettings()->getChannelMarker());
        }
        else
        {
            SWGSDRangel::SWGChannelMarker *swgChannelMarker = new SWGSDRangel::SWGChannelMarker();
            settings.m_channelMarker->formatTo(swgChannelMarker);
            response.getRdfDemodSettings()->setChannelMarker(swgChannelMarker);
        }
    }

    if (settings.m_rollupState)
    {
        if (response.getRdfDemodSettings()->getRollupState())
        {
            settings.m_rollupState->formatTo(response.getRdfDemodSettings()->getRollupState());
        }
        else
        {
            SWGSDRangel::SWGRollupState *swgRollupState = new SWGSDRangel::SWGRollupState();
            settings.m_rollupState->formatTo(swgRollupState);
            response.getRdfDemodSettings()->setRollupState(swgRollupState);
        }
    }
}

void RDFDemod::webapiFormatChannelReport(SWGSDRangel::SWGChannelReport& response)
{
    if (!m_running) {
        return;
    }

    double magsqAvg, magsqPeak;
    int nbMagsqSamples;
    getMagSqLevels(magsqAvg, magsqPeak, nbMagsqSamples);

    response.getRdfDemodReport()->setChannelPowerDb(CalcDb::dbPower(magsqAvg));
    response.getRdfDemodReport()->setSquelch(m_basebandSink->getSquelchState() > 0 ? 1 : 0);
    response.getRdfDemodReport()->setAudioSampleRate(m_basebandSink->getAudioSampleRate());
    response.getRdfDemodReport()->setChannelSampleRate(m_basebandSink->getChannelSampleRate());
}

void RDFDemod::webapiReverseSendSettings(QList<QString>& channelSettingsKeys, const RDFDemodSettings& settings, bool force)
{
    SWGSDRangel::SWGChannelSettings *swgChannelSettings = new SWGSDRangel::SWGChannelSettings();
    webapiFormatChannelSettings(channelSettingsKeys, swgChannelSettings, settings, force);

    QString channelSettingsURL = QString("http://%1:%2/sdrangel/deviceset/%3/channel/%4/settings")
            .arg(settings.m_reverseAPIAddress)
            .arg(settings.m_reverseAPIPort)
            .arg(settings.m_reverseAPIDeviceIndex)
            .arg(settings.m_reverseAPIChannelIndex);
    m_networkRequest.setUrl(QUrl(channelSettingsURL));
    m_networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QBuffer *buffer = new QBuffer();
    buffer->open((QBuffer::ReadWrite));
    buffer->write(swgChannelSettings->asJson().toUtf8());
    buffer->seek(0);

    // Always use PATCH to avoid passing reverse API settings
    QNetworkReply *reply = m_networkManager->sendCustomRequest(m_networkRequest, "PATCH", buffer);
    buffer->setParent(reply);

    delete swgChannelSettings;
}

void RDFDemod::sendChannelSettings(
    const QList<ObjectPipe*>& pipes,
    QList<QString>& channelSettingsKeys,
    const RDFDemodSettings& settings,
    bool force)
{
    for (const auto& pipe : pipes)
    {
        MessageQueue *messageQueue = qobject_cast<MessageQueue*>(pipe->m_element);

        if (messageQueue)
        {
            SWGSDRangel::SWGChannelSettings *swgChannelSettings = new SWGSDRangel::SWGChannelSettings();
            webapiFormatChannelSettings(channelSettingsKeys, swgChannelSettings, settings, force);
            MainCore::MsgChannelSettings *msg = MainCore::MsgChannelSettings::create(
                this,
                channelSettingsKeys,
                swgChannelSettings,
                force
            );
            messageQueue->push(msg);
        }
    }
}

void RDFDemod::webapiFormatChannelSettings(
        QList<QString>& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings *swgChannelSettings,
        const RDFDemodSettings& settings,
        bool force
)
{
    swgChannelSettings->setDirection(0); // Single sink (Rx)
    swgChannelSettings->setOriginatorChannelIndex(getIndexInDeviceSet());
    swgChannelSettings->setOriginatorDeviceSetIndex(getDeviceSetIndex());
    swgChannelSettings->setChannelType(new QString(m_channelId));
    swgChannelSettings->setRdfDemodSettings(new SWGSDRangel::SWGRDFDemodSettings());
    SWGSDRangel::SWGRDFDemodSettings *swgRDFDemodSettings = swgChannelSettings->getRdfDemodSettings();

    // transfer data that has been modified. When force is on transfer all data except reverse API data

    if (channelSettingsKeys.contains("inputFrequencyOffset") || force) {
        swgRDFDemodSettings->setInputFrequencyOffset(settings.m_inputFrequencyOffset);
    }
    if (channelSettingsKeys.contains("rfBandwidth") || force) {
        swgRDFDemodSettings->setRfBandwidth(settings.m_rfBandwidth);
    }
    if (channelSettingsKeys.contains("afBandwidth") || force) {
        swgRDFDemodSettings->setAfBandwidth(settings.m_afBandwidth);
    }
    if (channelSettingsKeys.contains("volume") || force) {
        swgRDFDemodSettings->setVolume(settings.m_volume);
    }
    if (channelSettingsKeys.contains("squelch") || force) {
        swgRDFDemodSettings->setSquelch(settings.m_squelch);
    }
    if (channelSettingsKeys.contains("audioMute") || force) {
        swgRDFDemodSettings->setAudioMute(settings.m_audioMute ? 1 : 0);
    }
    if (channelSettingsKeys.contains("rgbColor") || force) {
        swgRDFDemodSettings->setRgbColor(settings.m_rgbColor);
    }
    if (channelSettingsKeys.contains("title") || force) {
        swgRDFDemodSettings->setTitle(new QString(settings.m_title));
    }
    if (channelSettingsKeys.contains("audioDeviceName") || force) {
        swgRDFDemodSettings->setAudioDeviceName(new QString(settings.m_audioDeviceName));
    }
    if (channelSettingsKeys.contains("streamIndex") || force) {
        swgRDFDemodSettings->setStreamIndex(settings.m_streamIndex);
    }

    if (settings.m_channelMarker && (channelSettingsKeys.contains("channelMarker") || force))
    {
        SWGSDRangel::SWGChannelMarker *swgChannelMarker = new SWGSDRangel::SWGChannelMarker();
        settings.m_channelMarker->formatTo(swgChannelMarker);
        swgRDFDemodSettings->setChannelMarker(swgChannelMarker);
    }

    if (settings.m_rollupState && (channelSettingsKeys.contains("rollupState") || force))
    {
        SWGSDRangel::SWGRollupState *swgRollupState = new SWGSDRangel::SWGRollupState();
        settings.m_rollupState->formatTo(swgRollupState);
        swgRDFDemodSettings->setRollupState(swgRollupState);
    }
}

void RDFDemod::networkManagerFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError replyError = reply->error();

    if (replyError)
    {
        qWarning() << "RDFDemod::networkManagerFinished:"
                << " error(" << (int) replyError
                << "): " << replyError
                << ": " << reply->errorString();
    }
    else
    {
        QString answer = reply->readAll();
        answer.chop(1); // remove last \n
        qDebug("RDFDemod::networkManagerFinished: reply:\n%s", answer.toStdString().c_str());
    }

    reply->deleteLater();
}

void RDFDemod::handleIndexInDeviceSetChanged(int index)
{
    if (!m_running || (index < 0)) {
        return;
    }

    QString fifoLabel = QString("%1 [%2:%3]")
        .arg(m_channelId)
        .arg(m_deviceAPI->getDeviceSetIndex())
        .arg(index);
    m_basebandSink->setFifoLabel(fifoLabel);
    m_basebandSink->setAudioFifoLabel(fifoLabel);
}
