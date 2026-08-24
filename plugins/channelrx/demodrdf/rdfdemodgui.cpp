///////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany     //
// written by Christian Daniel                                                       //
// Copyright (C) 2014 John Greb <hexameron@spam.no>                                  //
// Copyright (C) 2015-2020, 2022 Edouard Griffiths, F4EXB <f4exb06@gmail.com>        //
// Copyright (C) 2021-2023 Jon Beniston, M7RCE <jon@beniston.com>                    //
//                                                                                   //
// This program is free software; you can redistribute it and/or modify              //
// it under the terms of the GNU General Public License as published by              //
// the Free Software Foundation as version 3 of the License, or                      //
// (at your option) any later version.                                               //
//                                                                                   //
// This program is distributed in the hope that it will be useful,                   //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                    //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                      //
// GNU General Public License V3 for more details.                                   //
//                                                                                   //
// You should have received a copy of the GNU General Public License                 //
// along with this program. If not, see <http://www.gnu.org/licenses/>.              //
///////////////////////////////////////////////////////////////////////////////////////
#include "rdfdemodgui.h"

#include "device/deviceuiset.h"

#include <QPointer>
#include <QDockWidget>
#include <QMainWindow>
#include <QDebug>
#include <QLocale>

#include "ui_rdfdemodgui.h"
#include "dsp/dspengine.h"
#include "dsp/dspcommands.h"
#include "plugin/pluginapi.h"
#include "util/db.h"
#include "gui/basicchannelsettingsdialog.h"
#include "gui/crightclickenabler.h"
#include "gui/dialogpositioner.h"
#include "gui/audioselectdialog.h"
#include "maincore.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkProxy>

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

#include "rdfdemod.h"

#include "gui/featureadddialog.h"

RDFDemodGUI* RDFDemodGUI::create(PluginAPI* pluginAPI, DeviceUISet *deviceUISet, BasebandSampleSink *rxChannel)
{
	RDFDemodGUI* gui = new RDFDemodGUI(pluginAPI, deviceUISet, rxChannel);
	return gui;
}

void RDFDemodGUI::destroy()
{
	delete this;
}

void RDFDemodGUI::resetToDefaults()
{
    m_settings.resetToDefaults();
    displaySettings();
    applySettings();
}

QByteArray RDFDemodGUI::serialize() const
{
    return m_settings.serialize();
}

bool RDFDemodGUI::deserialize(const QByteArray& data)
{
    if(m_settings.deserialize(data)) {
        displaySettings();
        applySettings(true);
        return true;
    } else {
        resetToDefaults();
        return false;
    }
}

bool RDFDemodGUI::handleMessage(const Message& message)
{
    if (RDFDemod::MsgConfigureRDFDemod::match(message))
    {
        qDebug("RDFDemodGUI::handleMessage: RDFDemod::MsgConfigureRDFDemod");
        const RDFDemod::MsgConfigureRDFDemod& cfg = (RDFDemod::MsgConfigureRDFDemod&) message;
        m_settings = cfg.getSettings();
        blockApplySettings(true);
        displaySettings();
        blockApplySettings(false);
        return true;
    }
    else if (DSPSignalNotification::match(message))
    {
        const DSPSignalNotification& notif = (const DSPSignalNotification&)message;
        m_deviceCenterFrequency = notif.getCenterFrequency();
        m_basebandSampleRate = notif.getSampleRate();

        // Hapus tanda komentar di dua baris ini
        ui->deltaFrequency->setValueRange(false, 8, -m_basebandSampleRate / 2, m_basebandSampleRate / 2);
        ui->deltaFrequencyLabel->setToolTip(tr("Range %1 %L2 Hz").arg(QChar(0xB1)).arg(m_basebandSampleRate / 2));

        updateAbsoluteCenterFrequency();
        return true;
    }
    else
    {
        return false;
    }
}

void RDFDemodGUI::handleInputMessages()
{
    Message* message;

    while ((message = getInputMessageQueue()->pop()) != 0)
    {
        if (handleMessage(*message))
        {
            delete message;
        }
    }
}

void RDFDemodGUI::channelMarkerChangedByCursor()
{
    ui->deltaFrequency->setValue(m_channelMarker.getCenterFrequency());
    m_settings.m_inputFrequencyOffset = m_channelMarker.getCenterFrequency();
    applySettings();

    /*ui->deltaFrequency->setValue(m_deviceCenterFrequency + m_channelMarker.getCenterFrequency());
    m_settings.m_inputFrequencyOffset = m_channelMarker.getCenterFrequency();
    applySettings();*/
}

void RDFDemodGUI::channelMarkerHighlightedByCursor()
{
    setHighlighted(m_channelMarker.getHighlighted());
}

void RDFDemodGUI::on_deltaFrequency_changed(qint64 value)
{
    m_channelMarker.setCenterFrequency(value);
    m_settings.m_inputFrequencyOffset = m_channelMarker.getCenterFrequency();
    updateAbsoluteCenterFrequency();
    applySettings();

    //qint64 offset = value - m_deviceCenterFrequency;
    //m_channelMarker.setCenterFrequency(offset);
    //m_settings.m_inputFrequencyOffset = m_channelMarker.getCenterFrequency();
    //updateAbsoluteCenterFrequency();
    //applySettings();
}

void RDFDemodGUI::on_rfBW_changed(quint64 value)
{
    m_channelMarker.setBandwidth(value);
    m_settings.m_rfBandwidth = value;
    applySettings();
}

void RDFDemodGUI::on_afBW_valueChanged(int value)
{
    ui->afBWText->setText(QString("%1 kHz").arg(value));
    m_settings.m_afBandwidth = value * 1000.0;
	applySettings();
}

void RDFDemodGUI::on_volume_valueChanged(int value)
{
    ui->volumeText->setText(QString("%1").arg(value / 10.0, 0, 'f', 1));
    m_settings.m_volume = value / 10.0;
	applySettings();
}

void RDFDemodGUI::on_squelch_valueChanged(int value)
{
	ui->squelchText->setText(QString("%1 dB").arg(value));
    m_settings.m_squelch = value;
	applySettings();
}

void RDFDemodGUI::on_audioMute_toggled(bool checked)
{
    m_settings.m_audioMute = checked;
    applySettings();
}

void RDFDemodGUI::on_DF()
{
    // Center frequency final = Device Center Frequency + Freq DF
    qint64 cfHz = m_deviceCenterFrequency + m_settings.m_inputFrequencyOffset;
    double cfMHz = static_cast<double>(cfHz) / 1000000.0;

    qDebug() << "DF Clicked :: Device CF(Hz) =" << m_deviceCenterFrequency;
    qDebug() << "DF Clicked :: Freq DF Offset(Hz) =" << m_settings.m_inputFrequencyOffset;
    qDebug() << "DF Clicked :: Final Center Frequency(Hz) =" << cfHz;
    qDebug() << "DF Clicked :: Final Center Frequency(MHz) =" << QString::number(cfMHz, 'f', 4);

    //QUrl url("http://192.168.1.10:8080/api/daq/center-freq");
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile file(path + "/ip_device.txt");

    QString ip = "192.168.1.10";

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString savedIp = in.readLine().trimmed();

        if (!savedIp.isEmpty())
        {
            ip = savedIp;
        }

        file.close();
    }

    QString urlString = QString("http://%1:8080/api/daq/center-freq").arg(ip);

    qDebug() << "Using API URL:" << urlString;

    QUrl url(urlString);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("accept", "*/*");

    QJsonObject json;
    json["daq_center_freq"] = cfMHz;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    qDebug() << "POST Payload:" << data;

    QNetworkReply* reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [reply]() {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray response = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "HTTP Status:" << statusCode;
            qDebug() << "API Error:" << reply->errorString();
            qDebug() << "API Body:" << response;
        }
        else {
            qDebug() << "HTTP Status:" << statusCode;
            qDebug() << "API Response:" << response;
        }

        reply->deleteLater();
        });
}

void RDFDemodGUI::onWidgetRolled(QWidget* widget, bool rollDown)
{
    (void) widget;
    (void) rollDown;

    getRollupContents()->saveState(m_rollupState);
    applySettings();
}

void RDFDemodGUI::onMenuDialogCalled(const QPoint &p)
{
    if (m_contextMenuType == ContextMenuChannelSettings)
    {
        BasicChannelSettingsDialog dialog(&m_channelMarker, this);
        dialog.setUseReverseAPI(m_settings.m_useReverseAPI);
        dialog.setReverseAPIAddress(m_settings.m_reverseAPIAddress);
        dialog.setReverseAPIPort(m_settings.m_reverseAPIPort);
        dialog.setReverseAPIDeviceIndex(m_settings.m_reverseAPIDeviceIndex);
        dialog.setReverseAPIChannelIndex(m_settings.m_reverseAPIChannelIndex);
        dialog.setDefaultTitle(m_displayedName);

        if (m_deviceUISet->m_deviceMIMOEngine)
        {
            dialog.setNumberOfStreams(m_rdfDemod->getNumberOfDeviceStreams());
            dialog.setStreamIndex(m_settings.m_streamIndex);
        }

        dialog.move(p);
        new DialogPositioner(&dialog, false);
        dialog.exec();

        m_settings.m_rgbColor = m_channelMarker.getColor().rgb();
        m_settings.m_title = m_channelMarker.getTitle();
        m_settings.m_useReverseAPI = dialog.useReverseAPI();
        m_settings.m_reverseAPIAddress = dialog.getReverseAPIAddress();
        m_settings.m_reverseAPIPort = dialog.getReverseAPIPort();
        m_settings.m_reverseAPIDeviceIndex = dialog.getReverseAPIDeviceIndex();
        m_settings.m_reverseAPIChannelIndex = dialog.getReverseAPIChannelIndex();

        setWindowTitle(m_settings.m_title);
        setTitle(m_channelMarker.getTitle());
        setTitleColor(m_settings.m_rgbColor);

        if (m_deviceUISet->m_deviceMIMOEngine)
        {
            m_settings.m_streamIndex = dialog.getSelectedStreamIndex();
            m_channelMarker.clearStreamIndexes();
            m_channelMarker.addStreamIndex(m_settings.m_streamIndex);
            updateIndexLabel();
        }

        applySettings();
    }

    resetContextMenuType();
}

RDFDemodGUI::RDFDemodGUI(PluginAPI* pluginAPI, DeviceUISet *deviceUISet, BasebandSampleSink *rxChannel, QWidget* parent) :
	ChannelGUI(parent),
	ui(new Ui::RDFDemodGUI),
	m_pluginAPI(pluginAPI),
	m_deviceUISet(deviceUISet),
	m_channelMarker(this),
    m_deviceCenterFrequency(0),
    m_basebandSampleRate(1),
	m_basicSettingsShown(false),
    m_squelchOpen(false),
    m_audioSampleRate(-1),
    m_recentAudioFifoError(false)
{
	setAttribute(Qt::WA_DeleteOnClose, true);
    m_helpURL = "plugins/channelrx/demodrdf/readme.md";
    RollupContents *rollupContents = getRollupContents();
	ui->setupUi(rollupContents);
    setSizePolicy(rollupContents->sizePolicy());
    rollupContents->arrangeRollups();
	connect(rollupContents, SIGNAL(widgetRolled(QWidget*,bool)), this, SLOT(onWidgetRolled(QWidget*,bool)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onMenuDialogCalled(const QPoint &)));
    connect(getInputMessageQueue(), SIGNAL(messageEnqueued()), this, SLOT(handleInputMessages()));

	m_rdfDemod = (RDFDemod*) rxChannel;
	m_rdfDemod->setMessageQueueToGUI(getInputMessageQueue());

	connect(&MainCore::instance()->getMasterTimer(), SIGNAL(timeout()), this, SLOT(tick()));

    CRightClickEnabler *audioMuteRightClickEnabler = new CRightClickEnabler(ui->audioMute);
    connect(audioMuteRightClickEnabler, SIGNAL(rightClick(const QPoint &)), this, SLOT(audioSelect(const QPoint &)));

	ui->deltaFrequencyLabel->setText(QString("%1f").arg(QChar(0x94, 0x03)));
    ui->deltaFrequency->setColorMapper(ColorMapper(ColorMapper::GrayGold));
    //ui->deltaFrequency->setValueRange(false, 8, -99999999, 99999999);
    ui->deltaFrequency->setValueRange(false, 7, 0, 99999999);
    ui->deltaFrequencyLabel->setText("Delta Freq");

    ui->freqFromDF->setText("Frequency from DF : --");
    //// Setup Timer untuk API GET
    //// ==========================================
    //// RESET STATUS KRAKEN SYNC KE "0" SAAT STARTUP
    //// ==========================================
    //QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    //QDir().mkpath(path); // Pastikan folder ada
    //QFile syncFile(path + "/kraken_sync.txt");

    //if (syncFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    //    QTextStream out(&syncFile);
    //    out << "0"; // Paksa set ke 0 (Disconnected) di awal
    //    syncFile.close();
    //}
    //// ==========================================

    // Beri nilai awal untuk kedua label status
    ui->connStatusLabel->setText("Device Status : <span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Disconnected</span>");
    ui->daqStatusLabel->setText("DAQ Status : <span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Disconnected</span>");

    ui->channelPowerMeter->setColorTheme(LevelMeterSignalDB::ColorGreenAndBlue);

    ui->rfBW->setColorMapper(ColorMapper(ColorMapper::GrayYellow));
    ui->rfBW->setValueRange(RDFDemodSettings::m_rfBWDigits, RDFDemodSettings::m_rfBWMin, RDFDemodSettings::m_rfBWMax);

    m_channelMarker.blockSignals(true);
	m_channelMarker.setBandwidth(m_settings.m_rfBandwidth);
	m_channelMarker.setCenterFrequency(0);
    m_channelMarker.setTitle("RDF Demodulator");
    m_channelMarker.setColor(m_settings.m_rgbColor);
    m_channelMarker.blockSignals(false);
	m_channelMarker.setVisible(true); // activate signal on the last setting only

	setTitleColor(m_channelMarker.getColor());
    m_settings.setChannelMarker(&m_channelMarker);
    m_settings.setRollupState(&m_rollupState);

	m_deviceUISet->addChannelMarker(&m_channelMarker);

	connect(&m_channelMarker, SIGNAL(changedByCursor()), this, SLOT(channelMarkerChangedByCursor()));
    connect(&m_channelMarker, SIGNAL(highlightedByCursor()), this, SLOT(channelMarkerHighlightedByCursor()));

    displaySettings();
    makeUIConnections();
	applySettings(true);
    m_resizer.enableChildMouseTracking();

    // disable menu
    ui->channelPowerMeter_2->setVisible(false);
    ui->channelPowerMeter->setVisible(false);

    ui->rfBWLabel->setVisible(false);
    ui->rfBW->setVisible(false);
    ui->rfBWUnits->setVisible(false);

    ui->afBWLabel->setVisible(false);
    ui->afBW->setVisible(false);

    ui->afBWText->setVisible(false);
    ui->volumeLabel->setVisible(false);
    ui->volume->setVisible(false);
    ui->volumeText->setVisible(false);

    ui->squelchLabel->setVisible(false);
    ui->squelch->setVisible(false);
    ui->squelchText->setVisible(false);

    ui->channelPower->setVisible(false);
    ui->channelPowerUnits->setVisible(false);


    m_networkManager = new QNetworkAccessManager(this);
    m_networkManager->setProxy(QNetworkProxy::NoProxy);

    // Setup Timer untuk API GET
    m_apiTimer = new QTimer(this);
    connect(m_apiTimer, &QTimer::timeout, this, &RDFDemodGUI::fetchCenterFreq);
    connect(m_apiTimer, &QTimer::timeout, this, &RDFDemodGUI::fetchDaqStatus);
    m_apiTimer->start(2000);
}

RDFDemodGUI::~RDFDemodGUI()
{
    if (m_apiTimer) {
        m_apiTimer->stop();
        m_apiTimer->disconnect();
    }

    this->disconnect();

    if (m_rdfDemod) {
        m_rdfDemod->setMessageQueueToGUI(nullptr);
    }

    if (m_networkManager) {
        m_networkManager->disconnect();
        m_networkManager->deleteLater();
        m_networkManager = nullptr;
    }

    delete ui;
}

void RDFDemodGUI::blockApplySettings(bool block)
{
    m_doApplySettings = !block;
}

void RDFDemodGUI::applySettings(bool force)
{
	if (m_doApplySettings)
	{
        RDFDemod::MsgConfigureRDFDemod* msgConfig = RDFDemod::MsgConfigureRDFDemod::create( m_settings, force);
        m_rdfDemod->getInputMessageQueue()->push(msgConfig);
	}
}

void RDFDemodGUI::displaySettings()
{
    m_channelMarker.blockSignals(true);
    m_channelMarker.setCenterFrequency(m_settings.m_inputFrequencyOffset);
    m_channelMarker.setBandwidth(m_settings.m_rfBandwidth);
    m_channelMarker.setTitle(m_settings.m_title);
    m_channelMarker.blockSignals(false);
    m_channelMarker.setColor(m_settings.m_rgbColor); // activate signal on the last setting only

    setTitleColor(m_settings.m_rgbColor);
    setWindowTitle(m_channelMarker.getTitle());
    setTitle(m_channelMarker.getTitle());

    blockApplySettings(true);

    ui->deltaFrequency->setValue(m_channelMarker.getCenterFrequency());
    ui->rfBW->setValue(m_settings.m_rfBandwidth);
    ui->afBW->setValue(m_settings.m_afBandwidth/1000.0);
    ui->afBWText->setText(QString("%1 kHz").arg(m_settings.m_afBandwidth/1000.0));
    ui->volume->setValue(m_settings.m_volume * 10.0);
    ui->volumeText->setText(QString("%1").arg(m_settings.m_volume, 0, 'f', 1));
    ui->squelch->setValue(m_settings.m_squelch);
    ui->squelchText->setText(QString("%1 dB").arg(m_settings.m_squelch));
    ui->audioMute->setChecked(m_settings.m_audioMute);

    ui->btnMap->setVisible(false);

    updateIndexLabel();

    getRollupContents()->restoreState(m_rollupState);
    updateAbsoluteCenterFrequency();
    blockApplySettings(false);
}

void RDFDemodGUI::leaveEvent(QEvent* event)
{
	m_channelMarker.setHighlighted(false);
    ChannelGUI::leaveEvent(event);
}

void RDFDemodGUI::enterEvent(EnterEventType* event)
{
	m_channelMarker.setHighlighted(true);
    ChannelGUI::enterEvent(event);
}

void RDFDemodGUI::audioSelect(const QPoint& p)
{
    qDebug("RDFDemodGUI::audioSelect");
    AudioSelectDialog audioSelect(DSPEngine::instance()->getAudioDeviceManager(), m_settings.m_audioDeviceName);
    audioSelect.move(p);
    new DialogPositioner(&audioSelect, false);
    audioSelect.exec();

    if (audioSelect.m_selected)
    {
        m_settings.m_audioDeviceName = audioSelect.m_audioDeviceName;
        applySettings();
    }
}

void RDFDemodGUI::tick()
{
    double magsqAvg, magsqPeak;
    int nbMagsqSamples;
    m_rdfDemod->getMagSqLevels(magsqAvg, magsqPeak, nbMagsqSamples);
    double powDbAvg = CalcDb::dbPower(magsqAvg);
    double powDbPeak = CalcDb::dbPower(magsqPeak);

    ui->channelPower->setText(QString::number(powDbAvg, 'f', 1));
    ui->channelPowerMeter->levelChanged(
            (100.0f + powDbAvg) / 100.0f,
            (100.0f + powDbPeak) / 100.0f,
            nbMagsqSamples);

    int audioSampleRate = m_rdfDemod->getAudioSampleRate();
    bool squelchOpen = m_rdfDemod->getSquelchOpen();
    int secsSinceAudioFifoError = m_rdfDemod->getAudioFifoErrorDateTime().secsTo(QDateTime::currentDateTime());
    bool recentAudioFifoError = (secsSinceAudioFifoError < 1) && squelchOpen;

    if ((audioSampleRate != m_audioSampleRate) || (squelchOpen != m_squelchOpen) || (recentAudioFifoError != m_recentAudioFifoError))
    {
        if (audioSampleRate < 0) {
            ui->audioMute->setStyleSheet("QToolButton { background-color : red; }");
        } else if (recentAudioFifoError) {
            ui->audioMute->setStyleSheet("QToolButton { background-color : rgb(120,120,0); }");
        } else if (squelchOpen) {
            ui->audioMute->setStyleSheet("QToolButton { background-color : green; }");
        } else {
            ui->audioMute->setStyleSheet("QToolButton { background:rgb(79,79,79); }");
        }

        m_audioSampleRate = audioSampleRate;
        m_squelchOpen = squelchOpen;
        m_recentAudioFifoError = recentAudioFifoError;
    }
}

void RDFDemodGUI::makeUIConnections()
{
    QObject::connect(ui->deltaFrequency, &ValueDialZ::changed, this, &RDFDemodGUI::on_deltaFrequency_changed);
    QObject::connect(ui->rfBW, &ValueDial::changed, this, &RDFDemodGUI::on_rfBW_changed);
    QObject::connect(ui->afBW, &QSlider::valueChanged, this, &RDFDemodGUI::on_afBW_valueChanged);
    QObject::connect(ui->volume, &QSlider::valueChanged, this, &RDFDemodGUI::on_volume_valueChanged);
    QObject::connect(ui->squelch, &QSlider::valueChanged, this, &RDFDemodGUI::on_squelch_valueChanged);
    QObject::connect(ui->audioMute, &QToolButton::toggled, this, &RDFDemodGUI::on_audioMute_toggled);
    QObject::connect(ui->DFButton, &QToolButton::clicked, this, &RDFDemodGUI::on_DF);
    QObject::connect(ui->btnMap, &QToolButton::clicked, this, &RDFDemodGUI::onBtnMapClicked);

    
}

void RDFDemodGUI::updateAbsoluteCenterFrequency()
{
    setStatusFrequency(m_deviceCenterFrequency + m_settings.m_inputFrequencyOffset);
}

void RDFDemodGUI::onBtnMapClicked()
{
    qDebug() << "MAP button clicked";

    FeatureAddDialog dialog(this);

    QStringList featureNames;

    const auto* featureRegistrations =
        m_pluginAPI->getFeatureRegistrations();

    for (int i = 0; i < featureRegistrations->size(); i++)
    {
        const auto& reg = featureRegistrations->at(i);

        qDebug() << "Feature URI =" << reg.m_featureIdURI;

        featureNames.append(reg.m_featureId);
    }

    dialog.addFeatureNames(featureNames);

    dialog.exec();
}

void RDFDemodGUI::fetchCenterFreq()
{
    QPointer<RDFDemodGUI> guard(this);

    if (!guard || !m_networkManager) {
        return;
    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QFile syncFile(path + "/kraken_sync.txt");
    bool isConnected = false;

    if (syncFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (syncFile.readLine().trimmed() == "1") {
            isConnected = true;
        }

        syncFile.close();
    }

    if (!isConnected) {
        return;
    }

    QFile file(path + "/ip_device.txt");
    QString ip = "192.168.1.10";

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString savedIp = file.readLine().trimmed();

        if (!savedIp.isEmpty()) {
            ip = savedIp;
        }

        file.close();
    }

    QString urlString =
        QString("http://%1:8080/api/daq/center-freq").arg(ip);

    QNetworkRequest request{ QUrl(urlString) };
    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
    );

    QNetworkReply* reply = m_networkManager->get(request);

    if (!reply) {
        return;
    }

    connect(reply, &QNetworkReply::finished,
        this,
        [guard, reply]()
        {
            if (!guard) {
                if (reply) {
                    reply->deleteLater();
                }

                return;
            }

            if (reply->error() == QNetworkReply::NoError) {

                QByteArray response = reply->readAll();

                QJsonDocument doc =
                    QJsonDocument::fromJson(response);

                QJsonObject json = doc.object();

                if (json.contains("status") &&
                    json["status"].toString() == "ok")
                {
                    if (json.contains("vfo_0_freq")) {

                        double vfoFreqHz =
                            json["vfo_0_freq"].toDouble();

                        double vfoFreqKHz = vfoFreqHz / 1000.0;

                        if (guard->ui &&
                            guard->ui->freqFromDF)
                        {
                            qint64 freqKHz = qRound64(vfoFreqKHz);

                            QString formattedFreq = QString::number(freqKHz);

                            // Tambahkan pemisah ribuan koma
                            for (int i = formattedFreq.length() - 3; i > 0; i -= 3)
                            {
                                formattedFreq.insert(i, ',');
                            }

                            guard->ui->freqFromDF->setText(
                                QString("Frequency from DF : %1 kHz")
                                .arg(formattedFreq)
                            );
                        }
                    }
                }
            }
            else {

                if (guard->ui &&
                    guard->ui->freqFromDF)
                {
                    guard->ui->freqFromDF->setText("Frequency from DF : API Error");
                }
            }

            if (reply) {
                reply->deleteLater();
            }
        });
}

void RDFDemodGUI::fetchDaqStatus()
{
    QPointer<RDFDemodGUI> guard(this);

    if (!guard || !m_networkManager) {
        return;
    }

    QString path =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
        );

    QFile syncFile(path + "/kraken_sync.txt");
    bool isSyncEnabled = false;

    if (syncFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

        if (syncFile.readLine().trimmed() == "1") {
            isSyncEnabled = true;
        }

        syncFile.close();
    }

    if (!isSyncEnabled) {

        if (ui) {

            ui->connStatusLabel->setText(
                "Device Status : "
                "<span style='color: #cc0000; font-size: 18pt;'>"
                "&#9679;</span> "
                "Disconnected"
            );

            ui->daqStatusLabel->setText(
                "DAQ Status : "
                "<span style='color: #cc0000; font-size: 18pt;'>"
                "&#9679;</span> "
                "Disconnected"
            );
        }

        return;
    }

    QFile file(path + "/ip_device.txt");

    QString ip = "192.168.1.10";

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        QString savedIp =
            file.readLine().trimmed();

        if (!savedIp.isEmpty()) {
            ip = savedIp;
        }

        file.close();
    }

    QString urlString =
        QString("http://%1:9000/get_map_data").arg(ip);

    QNetworkRequest request{ QUrl(urlString) };

    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        "application/json"
    );

    QNetworkReply* reply =
        m_networkManager->get(request);

    if (!reply) {
        return;
    }

    connect(reply, &QNetworkReply::finished,
        this,
        [guard, reply]()
        {
            if (!guard) {

                if (reply) {
                    reply->deleteLater();
                }

                return;
            }

            if (reply->error() == QNetworkReply::NoError) {

                if (guard->ui) {

                    guard->ui->connStatusLabel->setText(
                        "Device Status : "
                        "<span style='color: #00cc00; font-size: 18pt;'>"
                        "&#9679;</span> "
                        "Connected"
                    );
                }

                QByteArray response =
                    reply->readAll();

                QJsonDocument doc =
                    QJsonDocument::fromJson(response);

                QJsonObject json =
                    doc.object();

                if (json.contains("body_daq_conn_status")) {

                    QString daqStatus =
                        json["body_daq_conn_status"].toString();

                    if (guard->ui) {

                        if (daqStatus.toLower() == "connected") {

                            guard->ui->daqStatusLabel->setText(
                                "DAQ Status : "
                                "<span style='color: #00cc00; font-size: 18pt;'>"
                                "&#9679;</span> "
                                "Connected"
                            );
                        }
                        else {

                            guard->ui->daqStatusLabel->setText(
                                QString(
                                    "DAQ Status : "
                                    "<span style='color: #ff9900; font-size: 18pt;'>"
                                    "&#9679;</span> %1"
                                ).arg(daqStatus)
                            );
                        }
                    }
                }
                else {

                    if (guard->ui) {

                        guard->ui->daqStatusLabel->setText(
                            "DAQ Status : "
                            "<span style='color: #cc0000; font-size: 18pt;'>"
                            "&#9679;</span> "
                            "Unknown"
                        );
                    }
                }
            }
            else {

                if (guard->ui) {

                    guard->ui->connStatusLabel->setText(
                        "Device Status : "
                        "<span style='color: #cc0000; font-size: 18pt;'>"
                        "&#9679;</span> "
                        "API Error"
                    );

                    guard->ui->daqStatusLabel->setText(
                        "DAQ Status : "
                        "<span style='color: #cc0000; font-size: 18pt;'>"
                        "&#9679;</span> "
                        "Unknown"
                    );
                }
            }

            if (reply) {
                reply->deleteLater();
            }
        });
}