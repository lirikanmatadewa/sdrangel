///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2023 Jon Beniston, M7RCE <jon@beniston.com>                     //
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

#include "widebandscanner.h"

#include <QTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QThread>

#include <stdio.h>
#include <complex.h>

#include "SWGChannelSettings.h"
#include "SWGWorkspaceInfo.h"
#include "SWGFreqScannerSettings.h"
#include "SWGChannelReport.h"
#include "SWGChannelActions.h"

#include "device/deviceset.h"
#include "dsp/dspengine.h"
#include "dsp/dspcommands.h"
#include "dsp/devicesamplesource.h"
#include "dsp/devicesamplemimo.h"
#include "device/deviceapi.h"
#include "settings/serializable.h"
#include "channel/channelwebapiutils.h"
#include "maincore.h"
#include "dsp/spectrumvis.h"

MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgConfigureWidebandScanner, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgReportChannels, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgStartScan, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgStopScan, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgScanComplete, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgScanResult, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgStatus, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgReportActiveFrequency, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgReportActivePower, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgReportScanning, Message)
MESSAGE_CLASS_DEFINITION(WidebandScanner::MsgReportScanRange, Message)

const char * const WidebandScanner::m_channelIdURI = "sdrangel.channel.widebandscanner";
const char * const WidebandScanner::m_channelId = "WidebandScanner";

WidebandScanner::WidebandScanner(DeviceAPI *deviceAPI) :
        ChannelAPI(m_channelIdURI, ChannelAPI::StreamSingleSink),
        m_deviceAPI(deviceAPI),
        m_thread(nullptr),
        m_basebandSink(nullptr),
        m_running(false),
        m_basebandSampleRate(0),
        m_availableChannelHandler({}),
        m_scanDeviceSetIndex(0),
        m_scanChannelIndex(0),
        m_state(IDLE),
        m_timeoutTimer(this)
{
    setObjectName(m_channelId);

    applySettings(m_settings, QStringList(), true);

    m_deviceAPI->addChannelSink(this);
    m_deviceAPI->addChannelSinkAPI(this);

    m_networkManager = new QNetworkAccessManager();
    QObject::connect(
        m_networkManager,
        &QNetworkAccessManager::finished,
        this,
        &WidebandScanner::networkManagerFinished
    );
    QObject::connect(
        this,
        &ChannelAPI::indexInDeviceSetChanged,
        this,
        &WidebandScanner::handleIndexInDeviceSetChanged
    );

    start();

    QObject::connect(&m_availableChannelHandler, &AvailableChannelOrFeatureHandler::channelsOrFeaturesChanged, this, &WidebandScanner::channelsChanged);
    m_availableChannelHandler.scanAvailableChannelsAndFeatures();

    m_timeoutTimer.callOnTimeout(this, &WidebandScanner::timeout);
}

WidebandScanner::~WidebandScanner()
{
    qDebug("WidebandScanner::~WidebandScanner");
    QObject::disconnect(
        m_networkManager,
        &QNetworkAccessManager::finished,
        this,
        &WidebandScanner::networkManagerFinished
    );
    delete m_networkManager;
    m_deviceAPI->removeChannelSinkAPI(this);
    m_deviceAPI->removeChannelSink(this);

    stop();
}

void WidebandScanner::setDeviceAPI(DeviceAPI *deviceAPI)
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

uint32_t WidebandScanner::getNumberOfDeviceStreams() const
{
    return m_deviceAPI->getNbSourceStreams();
}

void WidebandScanner::feed(const SampleVector::const_iterator& begin, const SampleVector::const_iterator& end, bool firstOfBurst)
{
    (void) firstOfBurst;

    if (m_running) {
        m_basebandSink->feed(begin, end);
    }
}

void WidebandScanner::start()
{
    QMutexLocker m_lock(&m_mutex);

    if (m_running) {
        return;
    }

    qDebug("WidebandScanner::start");
    m_thread = new QThread();
    m_basebandSink = new WidebandScannerBaseband(this);
    m_basebandSink->setFifoLabel(QString("%1 [%2:%3]")
        .arg(m_channelId)
        .arg(m_deviceAPI->getDeviceSetIndex())
        .arg(getIndexInDeviceSet())
    );
    m_basebandSink->setMessageQueueToChannel(getInputMessageQueue());
    m_basebandSink->setChannel(this);
    m_basebandSink->moveToThread(m_thread);

    QObject::connect(
        m_thread,
        &QThread::finished,
        m_basebandSink,
        &QObject::deleteLater
    );
    QObject::connect(
        m_thread,
        &QThread::finished,
        m_thread,
        &QThread::deleteLater
    );

    m_thread->start();

    DSPSignalNotification *dspMsg = new DSPSignalNotification(m_basebandSampleRate, m_centerFrequency);
    m_basebandSink->getInputMessageQueue()->push(dspMsg);

    WidebandScannerBaseband::MsgConfigureWidebandScannerBaseband *msg = WidebandScannerBaseband::MsgConfigureWidebandScannerBaseband::create(m_settings, QStringList(), true);
    m_basebandSink->getInputMessageQueue()->push(msg);

    m_running = true;
}

void WidebandScanner::stop()
{
    QMutexLocker m_lock(&m_mutex);

    if (!m_running) {
        return;
    }

    qDebug("WidebandScanner::stop");
    m_running = false;
    m_thread->exit();
#ifndef __EMSCRIPTEN__
    m_thread->wait();
#endif
}

bool WidebandScanner::handleMessage(const Message& cmd)
{
    if (MsgConfigureWidebandScanner::match(cmd))
    {
        MsgConfigureWidebandScanner& cfg = (MsgConfigureWidebandScanner&) cmd;
        qDebug() << "WidebandScanner::handleMessage: MsgConfigureWidebandScanner";
        applySettings(cfg.getSettings(), cfg.getSettingsKeys(), cfg.getForce());

        return true;
    }
    else if (DSPSignalNotification::match(cmd))
    {
        DSPSignalNotification& notif = (DSPSignalNotification&) cmd;
        int newSampleRate = notif.getSampleRate();
        if ((newSampleRate != m_basebandSampleRate) && (m_state != IDLE))
        {
            // Restart scan if sample rate changes
            startScan();
        }
        m_basebandSampleRate = newSampleRate;
        m_centerFrequency = notif.getCenterFrequency();
        qDebug() << "WidebandScanner::handleMessage: DSPSignalNotification";
        // Forward to the sink
        if (m_running)
        {
            DSPSignalNotification* rep = new DSPSignalNotification(notif); // make a copy
            m_basebandSink->getInputMessageQueue()->push(rep);
        }
        // Forward to GUI if any
        if (m_guiMessageQueue) {
            m_guiMessageQueue->push(new DSPSignalNotification(notif));
        }

        return true;
    }
    else if (MsgStartScan::match(cmd))
    {
        muteAll(m_settings);
        startScan();

        return true;
    }
    else if (MsgStopScan::match(cmd))
    {
        stopScan();

        return true;
    }
    else if (MsgScanResult::match(cmd))
    {
        MsgScanResult& result = (MsgScanResult&)cmd;
        const QList<MsgScanResult::ScanResult>& results = result.getScanResults();

        processScanResults(result.getFFTStartTime(), results);

        return true;
    }
    else
    {
        return false;
    }
}

void WidebandScanner::startScan()
{
    // Start scan
    m_state = START_SCAN;
}

void WidebandScanner::stopScan()
{
    // Stop scan
    m_state = IDLE;
    m_timeoutTimer.stop();

    if (m_guiMessageQueue) {
        m_guiMessageQueue->push(MsgStatus::create(""));
    }
}

void WidebandScanner::setDeviceCenterFrequency(qint64 frequency)
{
    DSPDeviceSourceEngine* deviceSourceEngine = getDeviceAPI()->getDeviceSourceEngine();
    DSPDeviceMIMOEngine *deviceMIMOEngine = getDeviceAPI()->getDeviceMIMOEngine();

    if (deviceSourceEngine) // Rx device
    {
        // For RTL SDR, setCenterFrequency takes ~50ms, which means tuneTime can be 0
        getDeviceAPI()->getSampleSource()->setCenterFrequency(frequency);
    } else if (deviceMIMOEngine) { // MIMO device - I/Q stream is the same as this channel
        getDeviceAPI()->getSampleMIMO()->setSourceCenterFrequency(frequency, m_settings.m_streamIndex);
    }

    m_minFFTStartTime = QDateTime::currentDateTime().addMSecs(m_settings.m_tuneTime);
}

void WidebandScanner::initScan()
{
    // if (m_scanChannelIndex < 0) { // Always false
    //     applyChannelSetting(m_settings.m_channel);
    // }
    ChannelWebAPIUtils::setAudioMute(m_scanDeviceSetIndex, m_scanChannelIndex, true);

    if (m_centerFrequency != m_stepStartFrequency) {
        setDeviceCenterFrequency(m_stepStartFrequency);
    }

    m_scanResults.clear();

    if (m_guiMessageQueue) {
        m_guiMessageQueue->push(WidebandScanner::MsgReportScanning::create());
    }

    m_state = SCAN_FOR_MAX_POWER;
}

void WidebandScanner::processScanResults(const QDateTime& fftStartTime, const QList<MsgScanResult::ScanResult>& results)
{
    switch (m_state)
    {
    case IDLE:
        break;

    case START_SCAN:
        {
            // Create ordered list of frequencies to scan
            QList<qint64> frequencies;
            for (int i = 0; i < m_settings.m_frequencySettings.size(); i++)
            {
                if (m_settings.m_frequencySettings[i].m_enabled) {
                    frequencies.append(m_settings.m_frequencySettings[i].m_frequency);
                }
            }
            std::sort(frequencies.begin(), frequencies.end());

            if ((frequencies.size() > 0) && (m_settings.m_channelBandwidth > 0) && (m_basebandSampleRate > 0))
            {
                // Calculate how many channels can be scanned in one go
                int fftSize;
                int binsPerChannel;
                calcScannerSampleRate(m_settings.m_channelBandwidth, m_basebandSampleRate, m_scannerSampleRate, fftSize, binsPerChannel);

                // Align first frequency so we cover as many channels as possible, while skipping channel guard band (12.5% either end)
                // Can we adjust this to avoid DC bin?
                m_stepStartFrequency = frequencies.front() + m_scannerSampleRate / 2 - (m_scannerSampleRate / 8);
                m_stepStopFrequency = frequencies.back();

                // If all frequencies fit within usable bandwidth, we can have the first frequency more central
                int totalBW = frequencies.back() - frequencies.front() + 2 * m_settings.m_channelBandwidth;
                if (totalBW < m_scannerSampleRate * 0.75f)
                {
                    int spareBWEachSide = (m_scannerSampleRate - totalBW) / 2;
                    int spareChannelsEachSide = spareBWEachSide / m_settings.m_channelBandwidth;
                    int offset = spareChannelsEachSide * m_settings.m_channelBandwidth;
                    m_stepStartFrequency -= offset;
                }

                initScan();
            }
        }
        break;

    case SCAN_FOR_MAX_POWER:
        if (fftStartTime >= m_minFFTStartTime)
        {
            if (results.size() > 0) {
                m_scanResults.append(results);
            }

            // Calculate next center frequency
            bool complete = false; // Have all frequencies been scanned?
            bool freqInRange = false;
            qint64 nextCenterFrequency = m_centerFrequency;
            int usableBW = (m_scannerSampleRate * 3 / 4) & ~1;
            do
            {
                if (nextCenterFrequency + usableBW / 2 > m_stepStopFrequency)
                {
                    nextCenterFrequency = m_stepStartFrequency;
                    complete = true;
                }
                else
                {
                    nextCenterFrequency += usableBW;
                    complete = false;
                }

                // Are any frequencies in this new range?
                for (int i = 0; i < m_settings.m_frequencySettings.size(); i++)
                {
                    if (m_settings.m_frequencySettings[i].m_enabled
                        && (m_settings.m_frequencySettings[i].m_frequency >= nextCenterFrequency - usableBW / 2)
                        && (m_settings.m_frequencySettings[i].m_frequency < nextCenterFrequency + usableBW / 2))
                    {
                        freqInRange = true;
                        break;
                    }
                }
            }
            while (!complete && !freqInRange);

            if (complete)
            {
                if (m_scanResults.size() > 0)
                {
                    // Send scan results to GUI for display in table
                    if (m_guiMessageQueue)
                    {
                        WidebandScanner::MsgScanResult* msg = WidebandScanner::MsgScanResult::create(QDateTime());
                        QList<WidebandScanner::MsgScanResult::ScanResult>& guiResults = msg->getScanResults();
                        guiResults.append(m_scanResults);
                        m_guiMessageQueue->push(msg);
                    }

                    int frequency = -1;
                    WidebandScannerSettings::FrequencySettings *frequencySettings = nullptr;
                    WidebandScannerSettings::FrequencySettings *activeFrequencySettings = nullptr;

                    if (m_settings.m_priority == WidebandScannerSettings::MAX_POWER)
                    {
                        Real maxPower = -200.0f;

                        // Find frequency with max power that exceeds thresholds
                        for (int i = 0; i < m_scanResults.size(); i++)
                        {
                            frequencySettings = m_settings.getFrequencySettings(m_scanResults[i].m_frequency);
                            Real threshold = m_settings.getThreshold(frequencySettings);
                            if (m_scanResults[i].m_power >= threshold)
                            {
                                if (!activeFrequencySettings || (m_scanResults[i].m_power > maxPower))
                                {
                                    frequency = m_scanResults[i].m_frequency;
                                    maxPower = m_scanResults[i].m_power;
                                    activeFrequencySettings = frequencySettings;
                                }
                            }
                        }
                    }
                    else
                    {
                        // Find first frequency in list above threshold
                        for (int i = 0; i < m_scanResults.size(); i++)
                        {
                            frequencySettings = m_settings.getFrequencySettings(m_scanResults[i].m_frequency);
                            Real threshold = m_settings.getThreshold(frequencySettings);
                            if (m_scanResults[i].m_power >= threshold)
                            {
                                frequency = m_scanResults[i].m_frequency;
                                activeFrequencySettings = frequencySettings;
                                break;
                            }
                        }
                    }

                    if (m_settings.m_mode != WidebandScannerSettings::SCAN_ONLY)
                    {
                        // Were any frequencies found to be active?
                        //if (maxPower >= m_settings.m_threshold)
                        if (activeFrequencySettings)
                        {
                            // Tune device/channel to frequency
                            int offset;
                            if ((frequency < m_centerFrequency - usableBW / 2) || (frequency >= m_centerFrequency + usableBW / 2))
                            {
                                nextCenterFrequency = frequency;
                                offset = 0;
                            }
                            else
                            {
                                nextCenterFrequency = m_centerFrequency;
                                offset = frequency - m_centerFrequency;
                            }

                            // Ensure we have minimum offset from DC
                            if (offset >= 0)
                            {
                                while (offset < m_settings.m_channelFrequencyOffset)
                                {
                                    nextCenterFrequency -= m_settings.m_channelBandwidth;
                                    offset += m_settings.m_channelBandwidth;
                                }
                            }
                            else
                            {
                                while (abs(offset) < m_settings.m_channelFrequencyOffset)
                                {
                                    nextCenterFrequency += m_settings.m_channelBandwidth;
                                    offset -= m_settings.m_channelBandwidth;
                                }
                            }

                            //qDebug() << "Tuning to active freq:" << frequency << "m_centerFrequency" << m_centerFrequency << "nextCenterFrequency" << nextCenterFrequency << "offset: " << offset << "deviceset: R" << m_scanDeviceSetIndex << ":" << m_scanChannelIndex;

                            QString channel = m_settings.m_channel;
                            if (!activeFrequencySettings->m_channel.isEmpty()) {
                                channel = activeFrequencySettings->m_channel;
                            }
                            applyChannelSetting(channel);

                            // Tune the channel
                            ChannelWebAPIUtils::setFrequencyOffset(m_scanDeviceSetIndex, m_scanChannelIndex, offset);

                            // Unmute the channel
                            ChannelWebAPIUtils::setAudioMute(m_scanDeviceSetIndex, m_scanChannelIndex, false);

                            // Apply squelch
                            if (!activeFrequencySettings->m_squelch.isEmpty())
                            {
                                bool ok;
                                Real squelch = activeFrequencySettings->m_squelch.toFloat(&ok);
                                if (ok) {
                                    ChannelWebAPIUtils::patchChannelSetting(m_scanDeviceSetIndex, m_scanChannelIndex, "squelch", squelch);
                                }
                            }

                            m_activeFrequency = frequency;

                            if (m_settings.m_mode == WidebandScannerSettings::SINGLE)
                            {
                                // Scan complete
                                if (m_guiMessageQueue) {
                                    m_guiMessageQueue->push(MsgScanComplete::create());
                                }
                                m_state = IDLE;
                            }
                            else
                            {
                                // Wait for transmission to finish
                                m_state = WAIT_FOR_END_TX;
                            }

                            // Becareful to only do this at the end here, as it can recursively call handleMessage with new settings
                            if (m_guiMessageQueue) {
                                m_guiMessageQueue->push(MsgReportActiveFrequency::create(m_activeFrequency));
                            }
                        }
                        else
                        {
                            if (m_guiMessageQueue) {
                                m_guiMessageQueue->push(MsgStatus::create("Scanning..."));
                            }
                        }
                    }
                }
            }

            if (nextCenterFrequency != m_centerFrequency) {
                setDeviceCenterFrequency(nextCenterFrequency);
            }

            if (complete)
            {
                m_scanResultsForReport = m_scanResults;
                m_scanResults.clear();
            }
        }
        break;

    case WAIT_FOR_END_TX:
        for (int i = 0; i < results.size(); i++)
        {
            if (results[i].m_frequency == m_activeFrequency)
            {
                if (m_guiMessageQueue) {
                    m_guiMessageQueue->push(MsgReportActivePower::create(results[i].m_power));
                }

                // Wait until power drops below threshold
                WidebandScannerSettings::FrequencySettings *frequencySettings = m_settings.getFrequencySettings(m_activeFrequency);
                Real threshold = m_settings.getThreshold(frequencySettings);
                if (results[i].m_power < threshold)
                {
                    m_timeoutTimer.setSingleShot(true);
                    m_timeoutTimer.start((int)(m_settings.m_retransmitTime * 1000.0));
                    m_state = WAIT_FOR_RETRANSMISSION;
                    break;
                }
            }
        }
        break;

    case WAIT_FOR_RETRANSMISSION:
        for (int i = 0; i < results.size(); i++)
        {
            if (results[i].m_frequency == m_activeFrequency)
            {
                if (m_guiMessageQueue) {
                    m_guiMessageQueue->push(MsgReportActivePower::create(results[i].m_power));
                }

                // Check if power has returned to being above threshold
                WidebandScannerSettings::FrequencySettings *frequencySettings = m_settings.getFrequencySettings(m_activeFrequency);
                Real threshold = m_settings.getThreshold(frequencySettings);
                if (results[i].m_power >= threshold)
                {
                    m_timeoutTimer.stop();
                    m_state = WAIT_FOR_END_TX;
                }
            }
        }
        break;

    }
}

void WidebandScanner::timeout()
{
    // Power hasn't returned above threshold - Restart scan
    initScan();
}

void WidebandScanner::calcScannerSampleRate(int channelBW, int basebandSampleRate, int& scannerSampleRate, int& fftSize, int& binsPerChannel)
{
    const int maxFFTSize = 16384;
    const int minBinsPerChannel = 8;

    // Base FFT size on that used for main spectrum
    std::vector<DeviceSet*>& deviceSets = MainCore::instance()->getDeviceSets();
    DeviceSet* deviceSet = deviceSets[m_deviceAPI->getDeviceSetIndex()];
    const SpectrumSettings& spectrumSettings = deviceSet->m_spectrumVis->getSettings();
    fftSize = spectrumSettings.m_fftSize;

    // But ensure we have several bins per channel
    // Adjust sample rate, to ensure we don't get massive FFT size
    scannerSampleRate = basebandSampleRate;
    if (scannerSampleRate < channelBW) {
        channelBW = scannerSampleRate; // Prevent divide by 0
    }
    while (fftSize / (scannerSampleRate / channelBW) < minBinsPerChannel)
    {
        if (fftSize == maxFFTSize) {
            scannerSampleRate /= 2;
        } else {
            fftSize *= 2;
        }
    }

    binsPerChannel = fftSize / (scannerSampleRate / (float)channelBW);
}

void WidebandScanner::setCenterFrequency(qint64 frequency)
{
    WidebandScannerSettings settings = m_settings;
    settings.m_inputFrequencyOffset = frequency;
    applySettings(settings, {"inputFrequencyOffset"}, false);

    if (m_guiMessageQueue) // forward to GUI if any
    {
        MsgConfigureWidebandScanner *msgToGUI = MsgConfigureWidebandScanner::create(settings, {"inputFrequencyOffset"}, false);
        m_guiMessageQueue->push(msgToGUI);
    }
}

// Mute all channels
void WidebandScanner::muteAll(const WidebandScannerSettings& settings)
{
    QStringList channels;

    channels.append(settings.m_channel);
    for (int i = 0; i < settings.m_frequencySettings.size(); i++)
    {
        QString channel = settings.m_frequencySettings[i].m_channel;
        if (!channel.isEmpty() && !channels.contains(channel)) {
            channels.append(channel);
        }
    }

    for (const auto& channel : channels)
    {
        unsigned int deviceSetIndex, channelIndex;

        if (MainCore::getDeviceAndChannelIndexFromId(channel, deviceSetIndex, channelIndex)) {
            ChannelWebAPIUtils::setAudioMute(deviceSetIndex, channelIndex, true);
        }
    }
}

void WidebandScanner::applyChannelSetting(const QString& channel)
{
    if (!MainCore::getDeviceAndChannelIndexFromId(channel, m_scanDeviceSetIndex, m_scanChannelIndex)) {
        qDebug() << "WidebandScanner::applySettings: Failed to parse channel" << channel;
    }
}

void WidebandScanner::applySettings(const WidebandScannerSettings& settings, const QStringList& settingsKeys, bool force)
{
    qDebug() << "WidebandScanner::applySettings:"
            << settings.getDebugString(settingsKeys, force)
            << " force: " << force;

    if (settingsKeys.contains("streamIndex"))
    {
        if (m_deviceAPI->getSampleMIMO()) // change of stream is possible for MIMO devices only
        {
            m_deviceAPI->removeChannelSinkAPI(this);
            m_deviceAPI->removeChannelSink(this, m_settings.m_streamIndex);
            m_deviceAPI->addChannelSink(this, settings.m_streamIndex);
            m_deviceAPI->addChannelSinkAPI(this);
            //FIXME:scanAvailableChannels(); // re-scan
            emit streamIndexChanged(settings.m_streamIndex);
        }
    }

    if (m_running)
    {
        WidebandScannerBaseband::MsgConfigureWidebandScannerBaseband *msg = WidebandScannerBaseband::MsgConfigureWidebandScannerBaseband::create(settings, settingsKeys, force);
        m_basebandSink->getInputMessageQueue()->push(msg);
    }

    if (settings.m_useReverseAPI)
    {
        bool fullUpdate = (settingsKeys.contains("useReverseAPI") && settings.m_useReverseAPI) ||
            settingsKeys.contains("reverseAPIAddress") ||
            settingsKeys.contains("reverseAPIPort") ||
            settingsKeys.contains("reverseAPIDeviceIndex") ||
            settingsKeys.contains("reverseAPIChannelIndex");
        webapiReverseSendSettings(settingsKeys, settings, fullUpdate || force);
    }

    if (settingsKeys.contains("frequencySettings")
        || settingsKeys.contains("priority")
        || settingsKeys.contains("measurement")
        || settingsKeys.contains("mode")
        || settingsKeys.contains("channelBandwidth")
        || force)
    {
        // Restart scan if any settings change
        if (m_state != IDLE) {
            startScan();
        }
    }

    if (force) {
        m_settings = settings;
    } else {
        m_settings.applySettings(settingsKeys, settings);
    }
}

QByteArray WidebandScanner::serialize() const
{
    return m_settings.serialize();
}

bool WidebandScanner::deserialize(const QByteArray& data)
{
    if (m_settings.deserialize(data))
    {
        MsgConfigureWidebandScanner *msg = MsgConfigureWidebandScanner::create(m_settings, QStringList(), true);
        m_inputMessageQueue.push(msg);
        return true;
    }
    else
    {
        m_settings.resetToDefaults();
        MsgConfigureWidebandScanner *msg = MsgConfigureWidebandScanner::create(m_settings, QStringList(), true);
        m_inputMessageQueue.push(msg);
        return false;
    }
}

int WidebandScanner::webapiSettingsGet(
        SWGSDRangel::SWGChannelSettings& response,
        QString& errorMessage)
{
    (void) errorMessage;
    response.setFreqScannerSettings(new SWGSDRangel::SWGFreqScannerSettings());
    response.getFreqScannerSettings()->init();
    webapiFormatChannelSettings(response, m_settings);
    return 200;
}

int WidebandScanner::webapiWorkspaceGet(
        SWGSDRangel::SWGWorkspaceInfo& response,
        QString& errorMessage)
{
    (void) errorMessage;
    response.setIndex(m_settings.m_workspaceIndex);
    return 200;
}

int WidebandScanner::webapiSettingsPutPatch(
        bool force,
        const QStringList& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings& response,
        QString& errorMessage)
{
    (void) errorMessage;
    WidebandScannerSettings settings = m_settings;
    webapiUpdateChannelSettings(settings, channelSettingsKeys, response);

    MsgConfigureWidebandScanner *msg = MsgConfigureWidebandScanner::create(settings, channelSettingsKeys, force);
    m_inputMessageQueue.push(msg);

    qDebug("WidebandScanner::webapiSettingsPutPatch: forward to GUI: %p", m_guiMessageQueue);
    if (m_guiMessageQueue) // forward to GUI if any
    {
        MsgConfigureWidebandScanner *msgToGUI = MsgConfigureWidebandScanner::create(settings, channelSettingsKeys, force);
        m_guiMessageQueue->push(msgToGUI);
    }

    webapiFormatChannelSettings(response, settings);

    return 200;
}

int WidebandScanner::webapiReportGet(
            SWGSDRangel::SWGChannelReport& response,
            QString& errorMessage)
{
    (void) errorMessage;
    response.setFreqScannerReport(new SWGSDRangel::SWGFreqScannerReport());
    response.getFreqScannerReport()->init();
    webapiFormatChannelReport(response);
    return 200;
}

int WidebandScanner::webapiActionsPost(
        const QStringList& channelActionsKeys,
        SWGSDRangel::SWGChannelActions& query,
        QString& errorMessage)
{
    SWGSDRangel::SWGFreqScannerActions *SWGFreqScannerActions = query.getFreqScannerActions();

    if (SWGFreqScannerActions)
    {
        if (channelActionsKeys.contains("run"))
        {
            bool run = SWGFreqScannerActions->getRun() != 0;
            if (run)
            {
                MsgStartScan *start = MsgStartScan::create();
                if (getMessageQueueToGUI()) {
                    getMessageQueueToGUI()->push(start);
                } else {
                    getInputMessageQueue()->push(start);
                }
            }
            else
            {
                MsgStopScan *stop = MsgStopScan::create();
                if (getMessageQueueToGUI()) {
                    getMessageQueueToGUI()->push(stop);
                } else {
                    getInputMessageQueue()->push(stop);
                }
            }
        }

        return 202;
    }
    else
    {
        errorMessage = "Missing WidebandScannerActions in query";
        return 400;
    }
}

void WidebandScanner::webapiUpdateChannelSettings(
        WidebandScannerSettings& settings,
        const QStringList& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings& response)
{
    if (channelSettingsKeys.contains("channelFrequencyOffset")) {
        settings.m_channelFrequencyOffset = response.getFreqScannerSettings()->getChannelFrequencyOffset();
    }
    if (channelSettingsKeys.contains("channelBandwidth")) {
        settings.m_channelBandwidth = response.getFreqScannerSettings()->getChannelBandwidth();
    }
    if (channelSettingsKeys.contains("threshold")) {
        settings.m_threshold = response.getFreqScannerSettings()->getThreshold();
    }
    if (channelSettingsKeys.contains("frequencies"))
    {
        settings.m_frequencySettings.clear();
        QList<SWGSDRangel::SWGFreqScannerFrequency *> *frequencies = response.getFreqScannerSettings()->getFrequencies();
        if (frequencies)
        {
            for (const auto frequency : *frequencies)
            {
                WidebandScannerSettings::FrequencySettings freqSetting;
                freqSetting.m_frequency = frequency->getFrequency();
                if (frequency->getNotes()) {
                    freqSetting.m_notes = *frequency->getNotes();
                }
                if (frequency->getChannel()) {
                    freqSetting.m_channel = *frequency->getChannel();
                }
                if (frequency->getChannelBandwidth()) {
                    freqSetting.m_channelBandwidth = *frequency->getChannelBandwidth();
                }
                if (frequency->getThreshold()) {
                    freqSetting.m_threshold = *frequency->getThreshold();
                }
                if (frequency->getSquelch()) {
                    freqSetting.m_squelch = *frequency->getSquelch();
                }
                settings.m_frequencySettings.append(freqSetting);
            }
        }
    }
    if (channelSettingsKeys.contains("rgbColor")) {
        settings.m_rgbColor = response.getFreqScannerSettings()->getRgbColor();
    }
    if (channelSettingsKeys.contains("title")) {
        settings.m_title = *response.getFreqScannerSettings()->getTitle();
    }
    if (channelSettingsKeys.contains("streamIndex")) {
        settings.m_streamIndex = response.getFreqScannerSettings()->getStreamIndex();
    }
    if (channelSettingsKeys.contains("useReverseAPI")) {
        settings.m_useReverseAPI = response.getFreqScannerSettings()->getUseReverseApi() != 0;
    }
    if (channelSettingsKeys.contains("reverseAPIAddress")) {
        settings.m_reverseAPIAddress = *response.getFreqScannerSettings()->getReverseApiAddress();
    }
    if (channelSettingsKeys.contains("reverseAPIPort")) {
        settings.m_reverseAPIPort = response.getFreqScannerSettings()->getReverseApiPort();
    }
    if (channelSettingsKeys.contains("reverseAPIDeviceIndex")) {
        settings.m_reverseAPIDeviceIndex = response.getFreqScannerSettings()->getReverseApiDeviceIndex();
    }
    if (channelSettingsKeys.contains("reverseAPIChannelIndex")) {
        settings.m_reverseAPIChannelIndex = response.getFreqScannerSettings()->getReverseApiChannelIndex();
    }
    if (settings.m_channelMarker && channelSettingsKeys.contains("channelMarker")) {
        settings.m_channelMarker->updateFrom(channelSettingsKeys, response.getFreqScannerSettings()->getChannelMarker());
    }
    if (settings.m_rollupState && channelSettingsKeys.contains("rollupState")) {
        settings.m_rollupState->updateFrom(channelSettingsKeys, response.getFreqScannerSettings()->getRollupState());
    }
}

QList<SWGSDRangel::SWGFreqScannerFrequency *> *WidebandScanner::createFrequencyList(const WidebandScannerSettings& settings)
{
    QList<SWGSDRangel::SWGFreqScannerFrequency *> *frequencies = new QList<SWGSDRangel::SWGFreqScannerFrequency *>();
    for (int i = 0; i < settings.m_frequencySettings.size(); i++)
    {
        SWGSDRangel::SWGFreqScannerFrequency *frequency = new SWGSDRangel::SWGFreqScannerFrequency();
        frequency->init();
        frequency->setFrequency(settings.m_frequencySettings[i].m_frequency);
        frequency->setEnabled(settings.m_frequencySettings[i].m_enabled);
        if (!settings.m_frequencySettings[i].m_notes.isEmpty()) {
            frequency->setNotes(new QString(settings.m_frequencySettings[i].m_notes));
        }
        if (!settings.m_frequencySettings[i].m_channel.isEmpty()) {
            frequency->setChannel(new QString(settings.m_frequencySettings[i].m_channel));
        }
        if (!settings.m_frequencySettings[i].m_channelBandwidth.isEmpty()) {
            frequency->setChannelBandwidth(new QString(settings.m_frequencySettings[i].m_channelBandwidth));
        }
        if (!settings.m_frequencySettings[i].m_threshold.isEmpty()) {
            frequency->setThreshold(new QString(settings.m_frequencySettings[i].m_threshold));
        }
        if (!settings.m_frequencySettings[i].m_squelch.isEmpty()) {
            frequency->setSquelch(new QString(settings.m_frequencySettings[i].m_squelch));
        }
        frequencies->append(frequency);
    }
    return frequencies;
}

void WidebandScanner::webapiFormatChannelSettings(SWGSDRangel::SWGChannelSettings& response, const WidebandScannerSettings& settings)
{
    response.getFreqScannerSettings()->setChannelFrequencyOffset(settings.m_channelFrequencyOffset);
    response.getFreqScannerSettings()->setChannelBandwidth(settings.m_channelBandwidth);
    response.getFreqScannerSettings()->setThreshold(settings.m_threshold);

    QList<SWGSDRangel::SWGFreqScannerFrequency *> *frequencies = createFrequencyList(settings);
    if (response.getFreqScannerSettings()->getFrequencies()) {
        *response.getFreqScannerSettings()->getFrequencies() = *frequencies;
    } else {
        response.getFreqScannerSettings()->setFrequencies(frequencies);
    }

    response.getFreqScannerSettings()->setRgbColor(settings.m_rgbColor);
    if (response.getFreqScannerSettings()->getTitle()) {
        *response.getFreqScannerSettings()->getTitle() = settings.m_title;
    } else {
        response.getFreqScannerSettings()->setTitle(new QString(settings.m_title));
    }

    response.getFreqScannerSettings()->setStreamIndex(settings.m_streamIndex);
    response.getFreqScannerSettings()->setUseReverseApi(settings.m_useReverseAPI ? 1 : 0);

    if (response.getFreqScannerSettings()->getReverseApiAddress()) {
        *response.getFreqScannerSettings()->getReverseApiAddress() = settings.m_reverseAPIAddress;
    } else {
        response.getFreqScannerSettings()->setReverseApiAddress(new QString(settings.m_reverseAPIAddress));
    }

    response.getFreqScannerSettings()->setReverseApiPort(settings.m_reverseAPIPort);
    response.getFreqScannerSettings()->setReverseApiDeviceIndex(settings.m_reverseAPIDeviceIndex);
    response.getFreqScannerSettings()->setReverseApiChannelIndex(settings.m_reverseAPIChannelIndex);

    if (settings.m_channelMarker)
    {
        if (response.getFreqScannerSettings()->getChannelMarker())
        {
            settings.m_channelMarker->formatTo(response.getFreqScannerSettings()->getChannelMarker());
        }
        else
        {
            SWGSDRangel::SWGChannelMarker *swgChannelMarker = new SWGSDRangel::SWGChannelMarker();
            settings.m_channelMarker->formatTo(swgChannelMarker);
            response.getFreqScannerSettings()->setChannelMarker(swgChannelMarker);
        }
    }

    if (settings.m_rollupState)
    {
        if (response.getFreqScannerSettings()->getRollupState())
        {
            settings.m_rollupState->formatTo(response.getFreqScannerSettings()->getRollupState());
        }
        else
        {
            SWGSDRangel::SWGRollupState *swgRollupState = new SWGSDRangel::SWGRollupState();
            settings.m_rollupState->formatTo(swgRollupState);
            response.getFreqScannerSettings()->setRollupState(swgRollupState);
        }
    }
}

void WidebandScanner::webapiFormatChannelReport(SWGSDRangel::SWGChannelReport& response)
{
    response.getFreqScannerReport()->setChannelSampleRate(m_basebandSink->getChannelSampleRate());
    response.getFreqScannerReport()->setScanState((int) m_state);

    QList<SWGSDRangel::SWGFreqScannerChannelState *> *list = response.getFreqScannerReport()->getChannelState();

    for (int i = 0; i < m_scanResultsForReport.size(); i++)
    {
        SWGSDRangel::SWGFreqScannerChannelState *channelState = new SWGSDRangel::SWGFreqScannerChannelState();
        channelState->setFrequency(m_scanResultsForReport[i].m_frequency);
        channelState->setPower(m_scanResultsForReport[i].m_power);
        list->append(channelState);
    }
}

void WidebandScanner::webapiReverseSendSettings(const QStringList& channelSettingsKeys, const WidebandScannerSettings& settings, bool force)
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

void WidebandScanner::webapiFormatChannelSettings(
        const QStringList& channelSettingsKeys,
        SWGSDRangel::SWGChannelSettings *swgChannelSettings,
        const WidebandScannerSettings& settings,
        bool force
)
{
    swgChannelSettings->setDirection(0); // Single sink (Rx)
    swgChannelSettings->setOriginatorChannelIndex(getIndexInDeviceSet());
    swgChannelSettings->setOriginatorDeviceSetIndex(getDeviceSetIndex());
    swgChannelSettings->setChannelType(new QString("WidebandScanner"));
    swgChannelSettings->setFreqScannerSettings(new SWGSDRangel::SWGFreqScannerSettings());
    SWGSDRangel::SWGFreqScannerSettings *SWGFreqScannerSettings = swgChannelSettings->getFreqScannerSettings();

    // transfer data that has been modified. When force is on transfer all data except reverse API data

    if (channelSettingsKeys.contains("channelFrequencyOffset") || force) {
        SWGFreqScannerSettings->setChannelFrequencyOffset(settings.m_channelFrequencyOffset);
    }
    if (channelSettingsKeys.contains("channelBandwidth") || force) {
        SWGFreqScannerSettings->setChannelBandwidth(settings.m_channelBandwidth);
    }
    if (channelSettingsKeys.contains("threshold") || force) {
        SWGFreqScannerSettings->setThreshold(settings.m_threshold);
    }
    if (channelSettingsKeys.contains("frequencies") || force) {
        QList<SWGSDRangel::SWGFreqScannerFrequency *> *frequencies = createFrequencyList(settings);
        if (SWGFreqScannerSettings->getFrequencies()) {
            *SWGFreqScannerSettings->getFrequencies() = *frequencies;
        } else {
            SWGFreqScannerSettings->setFrequencies(frequencies);
        }
    }

    if (channelSettingsKeys.contains("rgbColor") || force) {
        SWGFreqScannerSettings->setRgbColor(settings.m_rgbColor);
    }
    if (channelSettingsKeys.contains("title") || force) {
        SWGFreqScannerSettings->setTitle(new QString(settings.m_title));
    }
    if (channelSettingsKeys.contains("streamIndex") || force) {
        SWGFreqScannerSettings->setStreamIndex(settings.m_streamIndex);
    }

    if (settings.m_channelMarker && (channelSettingsKeys.contains("channelMarker") || force))
    {
        SWGSDRangel::SWGChannelMarker *swgChannelMarker = new SWGSDRangel::SWGChannelMarker();
        settings.m_channelMarker->formatTo(swgChannelMarker);
        SWGFreqScannerSettings->setChannelMarker(swgChannelMarker);
    }

    if (settings.m_rollupState && (channelSettingsKeys.contains("rollupState") || force))
    {
        SWGSDRangel::SWGRollupState *swgRollupState = new SWGSDRangel::SWGRollupState();
        settings.m_rollupState->formatTo(swgRollupState);
        SWGFreqScannerSettings->setRollupState(swgRollupState);
    }
}

void WidebandScanner::networkManagerFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError replyError = reply->error();

    if (replyError)
    {
        qWarning() << "WidebandScanner::networkManagerFinished:"
                << " error(" << (int) replyError
                << "): " << replyError
                << ": " << reply->errorString();
    }
    else
    {
        QString answer = reply->readAll();
        answer.chop(1); // remove last \n
        qDebug("WidebandScanner::networkManagerFinished: reply:\n%s", answer.toStdString().c_str());
    }

    reply->deleteLater();
}

void WidebandScanner::handleIndexInDeviceSetChanged(int index)
{
    if (!m_running || (index < 0)) {
        return;
    }

    QString fifoLabel = QString("%1 [%2:%3]")
        .arg(m_channelId)
        .arg(m_deviceAPI->getDeviceSetIndex())
        .arg(index);
    m_basebandSink->setFifoLabel(fifoLabel);
}

void WidebandScanner::channelsChanged(const QStringList& renameFrom, const QStringList& renameTo)
{
    m_availableChannels = m_availableChannelHandler.getAvailableChannelOrFeatureList();
    notifyUpdateChannels(renameFrom, renameTo);
}

void WidebandScanner::notifyUpdateChannels(const QStringList& renameFrom, const QStringList& renameTo)
{
    if (getMessageQueueToGUI())
    {
        MsgReportChannels* msgToGUI = MsgReportChannels::create(renameFrom, renameTo);
        msgToGUI->getChannels() = m_availableChannels;
        getMessageQueueToGUI()->push(msgToGUI);
    }
}
