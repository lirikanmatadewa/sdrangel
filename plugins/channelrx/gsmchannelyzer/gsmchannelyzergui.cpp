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

#include <QDebug>
#include <QAction>
#include <QClipboard>
#include <QMenu>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QComboBox>

#include "device/deviceset.h"
#include "device/deviceuiset.h"
#include "dsp/dspengine.h"
#include "dsp/dspcommands.h"
#include "ui_gsmchannelyzergui.h"
//#include "ui_gsmchannelyzergui.h"
#include "gui/basicchannelsettingsdialog.h"
#include "dsp/dspengine.h"
#include "gui/tabletapandhold.h"
#include "gui/dialogpositioner.h"
#include "gui/decimaldelegate.h"
#include "gui/frequencydelegate.h"
#include "gui/int64delegate.h"
#include "gui/glspectrum.h"
#include "channel/channelwebapiutils.h"
#include "maincore.h"

#include "gsmchannelyzergui.h"
#include "gsmchannelyzeraddrangedialog.h"
#include "gsmchannelyzer.h"

static const QList<qint64> hfFreqs = {
        951400000, 951600000, 951800000, 952000000, 952200000,
        952400000, 952600000, 952800000, 953000000, 953200000,
        953400000, 953600000, 953800000, 954000000, 954200000,
        954400000, 954600000, 954800000, 955000000, 1805200000,
        1805400000, 1805600000, 1805800000, 1806000000, 1825200000,
        1825400000, 1825600000, 1825800000, 1826000000, 1826200000, 1826400000,
        1826600000, 1826800000, 1827000000, 1827200000, 1827400000, 1827600000,
        1827800000, 1837000000, 1837200000, 1837400000, 1837600000, 1837800000,
        1838000000, 1838200000, 1838400000, 1856400000, 1856600000, 1856800000,
        1857000000, 1857200000, 1857400000, 1857600000, 1857800000, 1858000000,
        1858200000, 1858400000, 1858600000, 1858800000, 1859000000, 1859200000,
        1859400000, 1859600000, 1859800000, 1860000000, 1860200000, 1860400000,
        1860600000, 1860800000
};

static const QList<qint64> ARFCN = {
    82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,
    100, 512, 513, 514, 515, 516, 612, 613, 614, 615, 616, 617, 618, 619, 620,
    621, 622, 623, 624, 625, 671, 672, 673, 674, 675, 676, 677, 678, 768, 769,
    770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784,
    785, 786, 787, 788, 789, 790
};
static const QList<qint64> MCC = { 21, 21, 21, 21, 21, 21, 21, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 21, 21, 21, 21, 21, 21, 21, 21, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
static const QList<qint64> MNC = { 510, 510, 510, 510, 510, 510, 510, 510, 510,
    510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510,
    510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510,
    510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510,
    510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510, 510
};

GsmChannelyzerGUI* GsmChannelyzerGUI::create(PluginAPI* pluginAPI, DeviceUISet *deviceUISet, BasebandSampleSink *rxChannel)
{
    GsmChannelyzerGUI* gui = new GsmChannelyzerGUI(pluginAPI, deviceUISet, rxChannel);
    return gui;
}

void GsmChannelyzerGUI::destroy()
{
    delete this;
}

void GsmChannelyzerGUI::resetToDefaults()
{
    m_settings.resetToDefaults();
    displaySettings();
    applyAllSettings();
}

QByteArray GsmChannelyzerGUI::serialize() const
{
    return m_settings.serialize();
}

bool GsmChannelyzerGUI::deserialize(const QByteArray& data)
{
    if(m_settings.deserialize(data))
    {
        displaySettings();
        applyAllSettings();
        return true;
    }
    else
    {
        resetToDefaults();
        return false;
    }
}

bool GsmChannelyzerGUI::handleMessage(const Message& message)
{
    
    ui->hiddenWidget->hide();

    //// hide menu
    //// row 1
    //ui->channelsLabel->hide();
    //ui->channels->hide();
    //ui->deltaFrequencyLabel->hide();
    //ui->deltaFrequency->hide();
    //ui->deltaUnits->hide();
    //ui->channelPower->hide();
    //ui->channelPowerUnits->hide();
    //
    //// row 2
    //ui->threshLabel->hide();
    //ui->thresh->hide();
    //ui->threshDec->hide();
    //ui->threshInc->hide();
    //ui->threshText->hide();
    //ui->threshLabel->hide();
    //ui->tuneTime->hide();
    //ui->tuneTimeDec->hide();
    //ui->tuneTimeInc->hide();
    //ui->tuneTimeLabel->hide();
    //ui->tuneTimeText->hide();
    //ui->retransmitTime->hide();
    //ui->retransmitTimeText->hide();
    //ui->retransmitTimeDec->hide();
    //ui->retransmitTimeInc->hide();
    //ui->retransmitTime->hide();
    //ui->retransmitTimeLabel->hide();
    //ui->scanTimeText->hide();
    //ui->scanTimeLabel->hide();
    //ui->scanTimeDec->hide();
    //ui->scanTimeInc->hide();
    //
    //// row 3
    //ui->rfBWLabel->hide();
    //ui->channelBandwidth->hide();
    //ui->rfBWUnits->hide();
    //ui->priorityLabel->hide();
    //ui->priority->hide();
    //ui->measurementLabel->hide();
    //ui->measurement->hide();

    //// row 4
    ui->mode->hide();

    // line
    ui->line->hide();
    ui->line_2->hide();
    ui->line_3->hide();
    ui->line_4->hide();
    ui->line_5->hide();
    ui->line_6->hide();
    ui->line_7->hide();
    ui->line_8->hide();
    ui->filterLine->hide();


    if (GsmChannelyzer::MsgConfigureGsmChannelyzer::match(message))
    {
        qDebug("GsmChannelyzerGUI::handleMessage: GsmChannelyzer::MsgConfigureGsmChannelyzer");
        const GsmChannelyzer::MsgConfigureGsmChannelyzer& cfg = (GsmChannelyzer::MsgConfigureGsmChannelyzer&) message;
        m_settings = cfg.getSettings();
        blockApplySettings(true);
        m_channelMarker.updateSettings(static_cast<const ChannelMarker*>(m_settings.m_channelMarker));
        displaySettings();
        blockApplySettings(false);
        return true;
    }
    else if (DSPSignalNotification::match(message))
    {
        DSPSignalNotification& notif = (DSPSignalNotification&) message;
        m_deviceCenterFrequency = notif.getCenterFrequency();
        m_basebandSampleRate = notif.getSampleRate();
        if (m_basebandSampleRate != 0)
        {
            ui->deltaFrequency->setValueRange(true, 8, 0, m_basebandSampleRate/2);
            ui->deltaFrequencyLabel->setToolTip(tr("Range %1 %L2 Hz").arg(QChar(0xB1)).arg(m_basebandSampleRate/2));
            ui->channelBandwidth->setValueRange(true, 8, 0, m_basebandSampleRate);
        }
        if (m_channelMarker.getBandwidth() == 0) {
            m_channelMarker.setBandwidth(m_basebandSampleRate);
        }
        updateAbsoluteCenterFrequency();
        return true;
    }
    else if (GsmChannelyzer::MsgReportChannels::match(message))
    {
        GsmChannelyzer::MsgReportChannels& report = (GsmChannelyzer::MsgReportChannels&)message;
        updateChannelsList(report.getChannels(), report.getRenameFrom(), report.getRenameTo());
        return true;
    }
    else if (GsmChannelyzer::MsgStatus::match(message))
    {
        GsmChannelyzer::MsgStatus& report = (GsmChannelyzer::MsgStatus&)message;
        ui->status->setText(report.getText());
        return true;
    }
    else if (GsmChannelyzer::MsgReportScanning::match(message))
    {
        ui->status->setText("Scanning");
        ui->table->clearSelection();
        ui->channelPower->setText("-");
        return true;
    }
    else if (GsmChannelyzer::MsgScanComplete::match(message))
    {
        ui->startStop->setChecked(false);
        return true;
    }
    else if (GsmChannelyzer::MsgReportActiveFrequency::match(message))
    {
        GsmChannelyzer::MsgReportActiveFrequency& report = (GsmChannelyzer::MsgReportActiveFrequency&)message;
        qint64 f = report.getCenterFrequency();
        QString frequency;
        QString annotation;
        QList<QTableWidgetItem*> items = ui->table->findItems(QString::number(f), Qt::MatchExactly);
        if (items.size() > 0)
        {
            ui->table->selectRow(items[0]->row());
            frequency = ui->table->item(items[0]->row(), COL_FREQUENCY)->text();
            annotation = ui->table->item(items[0]->row(), COL_ANNOTATION)->text();
        }
        FrequencyDelegate freqDelegate("Auto", 3);
        QString formattedFrequency = freqDelegate.displayText(frequency, QLocale::system());
        ui->status->setText(QString("Active: %1 %2").arg(formattedFrequency).arg(annotation));
        return true;
    }
    else if (GsmChannelyzer::MsgReportActivePower::match(message))
    {
        GsmChannelyzer::MsgReportActivePower& report = (GsmChannelyzer::MsgReportActivePower&)message;
        float power = report.getPower();
        ui->channelPower->setText(QString::number(power, 'f', 1));
        return true;
    }
    else if (GsmChannelyzer::MsgReportScanRange::match(message))
    {
        GsmChannelyzer::MsgReportScanRange& report = (GsmChannelyzer::MsgReportScanRange&)message;
        m_channelMarker.setCenterFrequency(report.getCenterFrequency());
        m_channelMarker.setBandwidth(report.getTotalBandwidth());
        m_channelMarker.setVisible(report.getTotalBandwidth() < m_basebandSampleRate); // Hide marker if full bandwidth
        return true;
    }
    else if (GsmChannelyzer::MsgScanResult::match(message))
    {
        GsmChannelyzer::MsgScanResult& report = (GsmChannelyzer::MsgScanResult&)message;
        QList<GsmChannelyzer::MsgScanResult::ScanResult> results = report.getScanResults();

        // Clear column
        for (int i = 0; i < ui->table->rowCount(); i++)
        {
            QTableWidgetItem* item = ui->table->item(i, COL_POWER);
            item->setText("");
            item->setBackground(QBrush());
        }
        // Add results
        for (int i = 0; i < results.size(); i++)
        {
            qint64 freq = results[i].m_frequency;
            QList<QTableWidgetItem *> items = ui->table->findItems(QString::number(freq), Qt::MatchExactly);
            for (auto item : items)
            {
                int row = item->row();
                QTableWidgetItem* powerItem = ui->table->item(row, COL_POWER);
                powerItem->setData(Qt::DisplayRole, results[i].m_power);
                GsmChannelyzerSettings::FrequencySettings *frequencySettings = m_settings.getFrequencySettings(freq);
                Real threshold = m_settings.getThreshold(frequencySettings);
                bool active = results[i].m_power >= threshold;
                if (active)
                {
                    powerItem->setBackground(Qt::darkGreen);
                    QTableWidgetItem* activeCountItem = ui->table->item(row, COL_ACTIVE_COUNT);
                    activeCountItem->setData(Qt::DisplayRole, activeCountItem->data(Qt::DisplayRole).toInt() + 1);
                }
            }
        }

        return true;
    }
    else if (GsmChannelyzer::MsgStartScan::match(message))
    {
        ui->startStop->doToggle(true);
        return true;
    }
    else if (GsmChannelyzer::MsgStopScan::match(message))
    {
        ui->startStop->doToggle(false);
        return true;
    }
    return false;
}

void GsmChannelyzerGUI::updateChannelsCombo(QComboBox *combo, const AvailableChannelOrFeatureList& channels, const QString& channel, bool empty)
{
    combo->blockSignals(true);
    combo->clear();
    if (empty) {
        combo->addItem("");
    }

    for (const auto& channel : channels)
    {
        // Add channels in this device set, other than ourself (Don't use ChannelGUI::getDeviceSetIndex()/getIndex() as not valid when this is first called)
        if ((channel.m_superIndex == m_freqScanner->getDeviceSetIndex()) && (channel.m_index != m_freqScanner->getIndexInDeviceSet())) {
            combo->addItem(channel.getId());
        }
    }

    // Channel can be created after this plugin, so select it
    // if the chosen channel appears
    int channelIndex = combo->findText(channel);

    if (channelIndex >= 0) {
        combo->setCurrentIndex(channelIndex);
    } else {
        combo->setCurrentIndex(-1); // return to nothing selected
    }

    combo->blockSignals(false);
}

void GsmChannelyzerGUI::updateChannelsList(const AvailableChannelOrFeatureList& channels, const QStringList& renameFrom, const QStringList& renameTo)
{
    m_availableChannels = channels;

    // Update channel setting if it has been renamed
    if (renameFrom.contains(m_settings.m_channel))
    {
        m_settings.m_channel = renameTo[renameFrom.indexOf(m_settings.m_channel)];
        applySetting("channel");
    }
    bool rename = false;
    for (auto& setting : m_settings.m_frequencySettings)
    {
         if (renameFrom.contains(setting.m_channel))
         {
             setting.m_channel = renameTo[renameFrom.indexOf(setting.m_channel)];
             rename = true;
         }
    }
    if (rename) {
         applySetting("frequencySettings");
    }

    updateChannelsCombo(ui->channels, channels, m_settings.m_channel, false);

    for (int row = 0; row < ui->table->rowCount(); row++)
    {
        QComboBox *combo = qobject_cast<QComboBox *>(ui->table->cellWidget(row, COL_CHANNEL));
        updateChannelsCombo(combo, channels, m_settings.m_frequencySettings[row].m_channel, true);
    }
}

void GsmChannelyzerGUI::on_channels_currentIndexChanged(int index)
{
    if (index >= 0)
    {
        m_settings.m_channel = ui->channels->currentText();
        applySetting("channel");
    }
}

void GsmChannelyzerGUI::handleInputMessages()
{
    Message* message;

    while ((message = getInputMessageQueue()->pop()) != 0)
    {
        if (handleMessage(*message)) {
            delete message;
        }
    }
}

void GsmChannelyzerGUI::channelMarkerChangedByCursor()
{
}

void GsmChannelyzerGUI::channelMarkerHighlightedByCursor()
{
    setHighlighted(m_channelMarker.getHighlighted());
}

void GsmChannelyzerGUI::on_deltaFrequency_changed(qint64 value)
{
    m_settings.m_channelFrequencyOffset = value;
    applySetting("channelFrequencyOffset");
}

void GsmChannelyzerGUI::on_channelBandwidth_changed(qint64 value)
{
    m_settings.m_channelBandwidth = value;
    applySetting("channelBandwidth");
}

void GsmChannelyzerGUI::on_scanTime_valueChanged(int value)
{
    ui->scanTimeText->setText(QString("%1 s").arg(value / 10.0, 0, 'f', 1));
    m_settings.m_scanTime = value / 10.0;
    applySetting("scanTime");
}

void GsmChannelyzerGUI::on_retransmitTime_valueChanged(int value)
{
    ui->retransmitTimeText->setText(QString("%1 s").arg(value / 10.0, 0, 'f', 1));
    m_settings.m_retransmitTime = value / 10.0;
    applySetting("retransmitTime");
}

void GsmChannelyzerGUI::on_tuneTime_valueChanged(int value)
{
    ui->tuneTimeText->setText(QString("%1 ms").arg(value));
    m_settings.m_tuneTime = value;
    applySetting("tuneTime");
}

void GsmChannelyzerGUI::on_thresh_valueChanged(int value)
{
    ui->threshText->setText(QString("%1 dB").arg(value / 10.0, 0, 'f', 1));
    m_settings.m_threshold = value / 10.0;
    applySetting("threshold");
}

void GsmChannelyzerGUI::scanTimeIncClick()
{
   ui->scanTime->setValue(ui->scanTime->value() + 1);
}

void GsmChannelyzerGUI::scanTimeDecClick()
{
   ui->scanTime->setValue(ui->scanTime->value() - 1);
}

void GsmChannelyzerGUI::retransmitTimeIncClick()
{
   ui->retransmitTime->setValue(ui->retransmitTime->value() + 1);
}

void GsmChannelyzerGUI::retransmitTimeDecClick()
{
   ui->retransmitTime->setValue(ui->retransmitTime->value() - 1);
}

void GsmChannelyzerGUI::tuneTimeIncClick()
{
   ui->tuneTime->setValue(ui->tuneTime->value() + 1);
}

void GsmChannelyzerGUI::tuneTimeDecClick()
{
   ui->tuneTime->setValue(ui->tuneTime->value() - 1);
}

void GsmChannelyzerGUI::threshIncClick()
{
   ui->thresh->setValue(ui->thresh->value() + 1);
}

void GsmChannelyzerGUI::threshDecClick()
{
   ui->thresh->setValue(ui->thresh->value() - 1);
}

void GsmChannelyzerGUI::on_priority_currentIndexChanged(int index)
{
    m_settings.m_priority = (GsmChannelyzerSettings::Priority)index;
    applySetting("priority");
}

void GsmChannelyzerGUI::on_measurement_currentIndexChanged(int index)
{
    m_settings.m_measurement = (GsmChannelyzerSettings::Measurement)index;
    applySetting("measurement");
}

void GsmChannelyzerGUI::on_mode_currentIndexChanged(int index)
{
    m_settings.m_mode = (GsmChannelyzerSettings::Mode)index;
    applySetting("mode");
}

void GsmChannelyzerGUI::onWidgetRolled(QWidget* widget, bool rollDown)
{
    (void) widget;
    (void) rollDown;

    getRollupContents()->saveState(m_rollupState);
    applySetting("rollupState");
}

void GsmChannelyzerGUI::onMenuDialogCalled(const QPoint &p)
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
            dialog.setNumberOfStreams(m_freqScanner->getNumberOfDeviceStreams());
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

        QList<QString> settingsKeys({
            "rgbColor",
            "title",
            "useReverseAPI",
            "reverseAPIAddress",
            "reverseAPIPort",
            "reverseAPIDeviceIndex",
            "reverseAPIChannelIndex"
        });

        if (m_deviceUISet->m_deviceMIMOEngine)
        {
            m_settings.m_streamIndex = dialog.getSelectedStreamIndex();
            m_channelMarker.clearStreamIndexes();
            m_channelMarker.addStreamIndex(m_settings.m_streamIndex);
            updateIndexLabel();
        }

        applySettings(settingsKeys);
    }

    resetContextMenuType();
}

GsmChannelyzerGUI::GsmChannelyzerGUI(PluginAPI* pluginAPI, DeviceUISet *deviceUISet, BasebandSampleSink *rxChannel, QWidget* parent) :
    ChannelGUI(parent),
    ui(new Ui::GsmChannelyzerGUI),
    m_pluginAPI(pluginAPI),
    m_deviceUISet(deviceUISet),
    m_channelMarker(this),
    m_deviceCenterFrequency(0),
    m_doApplySettings(true)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_helpURL = "plugins/channelrx/freqscanner/readme.md";
    RollupContents *rollupContents = getRollupContents();
    ui->setupUi(rollupContents);
    setSizePolicy(rollupContents->sizePolicy());
    rollupContents->arrangeRollups();
    connect(rollupContents, SIGNAL(widgetRolled(QWidget*,bool)), this, SLOT(onWidgetRolled(QWidget*,bool)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onMenuDialogCalled(const QPoint &)));

    m_freqScanner = reinterpret_cast<GsmChannelyzer*>(rxChannel);
    m_freqScanner->setMessageQueueToGUI(getInputMessageQueue());

    ui->deltaFrequencyLabel->setText(QString("%1f").arg(QChar(0x94, 0x03)));
    ui->deltaFrequency->setColorMapper(ColorMapper(ColorMapper::GrayGold));
    ui->deltaFrequency->setValueRange(true, 8, 0, 9999999);

    ui->channelBandwidth->setColorMapper(ColorMapper(ColorMapper::GrayGreenYellow));
    ui->channelBandwidth->setValueRange(true, 8, 0, 9999999);

    m_channelMarker.setColor(Qt::yellow);
    m_channelMarker.setCenterFrequency(m_settings.m_inputFrequencyOffset);
    m_channelMarker.setTitle("GSM Channelyzer");
    m_channelMarker.blockSignals(false);
    m_channelMarker.setVisible(true);

    setTitleColor(m_channelMarker.getColor());
    m_settings.setChannelMarker(&m_channelMarker);
    m_settings.setRollupState(&m_rollupState);

    m_deviceUISet->addChannelMarker(&m_channelMarker);

    connect(&m_channelMarker, SIGNAL(changedByCursor()), this, SLOT(channelMarkerChangedByCursor()));
    connect(&m_channelMarker, SIGNAL(highlightedByCursor()), this, SLOT(channelMarkerHighlightedByCursor()));
    connect(getInputMessageQueue(), SIGNAL(messageEnqueued()), this, SLOT(handleInputMessages()));

    // Resize the table using dummy data
    resizeTable();
    // Allow user to reorder columns
    ui->table->horizontalHeader()->setSectionsMovable(true);
    // Add context menu to allow hiding/showing of columns
    m_menu = new QMenu(ui->table);
    for (int i = 0; i < ui->table->horizontalHeader()->count(); i++)
    {
        QString text = ui->table->horizontalHeaderItem(i)->text();
        m_menu->addAction(createCheckableItem(text, i, true));
    }
    ui->table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(columnSelectMenu(QPoint)));
    // Get signals when columns change
    connect(ui->table->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)), SLOT(table_sectionMoved(int, int, int)));
    connect(ui->table->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), SLOT(table_sectionResized(int, int, int)));
    // Context menu
    ui->table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->table, &QTableWidget::customContextMenuRequested, this, &GsmChannelyzerGUI::table_customContextMenuRequested);
    TableTapAndHold* tableTapAndHold = new TableTapAndHold(ui->table);
    connect(tableTapAndHold, &TableTapAndHold::tapAndHold, this, &GsmChannelyzerGUI::table_customContextMenuRequested);

    ui->startStop->setStyleSheet(QString("QToolButton{ background-color: blue; } QToolButton:checked{ background-color: green; }"));

    displaySettings();
    makeUIConnections();
    ui->thresh->hide();
    ui->tuneTime->hide();
    ui->scanTime->hide();
    ui->retransmitTime->hide();
    applyAllSettings();
    m_resizer.enableChildMouseTracking();

    ui->table->setItemDelegateForColumn(COL_FREQUENCY, new FrequencyDelegate("Auto", 3, true, ui->table));
    ui->table->setItemDelegateForColumn(COL_POWER, new DecimalDelegate(1, ui->table));
    ui->table->setItemDelegateForColumn(COL_CHANNEL_BW, new Int64Delegate(0, 10000000, ui->table));
    ui->table->setItemDelegateForColumn(COL_TH, new DecimalDelegate(1, -120.0, 0.0, ui->table));
    ui->table->setItemDelegateForColumn(COL_SQ, new DecimalDelegate(1, -120.0, 0.0, ui->table));
    ui->table->setItemDelegateForColumn(COL_ARFCN, new DecimalDelegate(1, -120.0, 0.0, ui->table));
    ui->table->setItemDelegateForColumn(COL_MCC, new DecimalDelegate(1, -120.0, 0.0, ui->table));
    ui->table->setItemDelegateForColumn(COL_MNC, new DecimalDelegate(1, -120.0, 0.0, ui->table));

    //ui->table->setColumnHidden(COL_ANNOTATION, true);
    //ui->table->setColumnHidden(COL_ENABLE, true);
    //ui->table->setColumnHidden(COL_NOTES, true);
    //ui->table->setColumnHidden(COL_CHANNEL, true);
    /*ui->table->setColumnHidden(COL_CHANNEL_BW, true);*/
    //ui->table->setColumnHidden(COL_TH, true);
    //ui->table->setColumnHidden(COL_SQ, true);

    ui->table->setColumnHidden(4, true);
    ui->table->setColumnHidden(5, true);
    ui->table->setColumnHidden(8, true);
    ui->table->setColumnHidden(9, true);
    ui->table->setColumnHidden(10, true);
    ui->table->setColumnHidden(11, true);
    ui->table->setColumnHidden(12, true);

    connect(m_deviceUISet->m_spectrum->getSpectrumView(), &GLSpectrumView::updateAnnotations, this, &GsmChannelyzerGUI::updateAnnotations);
}

GsmChannelyzerGUI::~GsmChannelyzerGUI()
{
    delete ui;
}

void GsmChannelyzerGUI::blockApplySettings(bool block)
{
    m_doApplySettings = !block;
}

void GsmChannelyzerGUI::applySetting(const QString& settingsKey)
{
    applySettings({settingsKey});
}

void GsmChannelyzerGUI::applySettings(const QStringList& settingsKeys, bool force)
{
    m_settingsKeys.append(settingsKeys);
    if (m_doApplySettings)
    {
        GsmChannelyzer::MsgConfigureGsmChannelyzer* message = GsmChannelyzer::MsgConfigureGsmChannelyzer::create(m_settings, m_settingsKeys, force);
        m_freqScanner->getInputMessageQueue()->push(message);
        m_settingsKeys.clear();
    }
}

void GsmChannelyzerGUI::applyAllSettings()
{
    applySettings(QStringList(), true);
}

void GsmChannelyzerGUI::displaySettings()
{
    m_channelMarker.blockSignals(true);
    m_channelMarker.setBandwidth(m_basebandSampleRate);
    m_channelMarker.setCenterFrequency(m_settings.m_inputFrequencyOffset);
    m_channelMarker.setTitle(m_settings.m_title);
    m_channelMarker.blockSignals(false);
    m_channelMarker.setColor(m_settings.m_rgbColor); // activate signal on the last setting only

    setTitleColor(m_settings.m_rgbColor);
    setWindowTitle(m_channelMarker.getTitle());
    setTitle(m_channelMarker.getTitle());

    blockApplySettings(true);
    int channelIndex = ui->channels->findText(m_settings.m_channel);
    if (channelIndex >= 0) {
        ui->channels->setCurrentIndex(channelIndex);
    }
    ui->deltaFrequency->setValue(m_settings.m_channelFrequencyOffset);
    
    // mode default
    m_settings.m_channelBandwidth = 50000;
    ui->channelBandwidth->setValue(m_settings.m_channelBandwidth);

    ui->scanTime->setValue(m_settings.m_scanTime * 10.0);
    ui->scanTimeText->setText(QString("%1 s").arg(m_settings.m_scanTime, 0, 'f', 1));
    ui->retransmitTime->setValue(m_settings.m_retransmitTime * 10.0);
    ui->retransmitTimeText->setText(QString("%1 s").arg(m_settings.m_retransmitTime, 0, 'f', 1));
    ui->tuneTime->setValue(m_settings.m_tuneTime);
    ui->tuneTimeText->setText(QString("%1 ms").arg(m_settings.m_tuneTime));
    
    // mode default
    ui->thresh->setValue(m_settings.m_threshold * 10.0);
    ui->threshText->setText(QString("%1 dB").arg(m_settings.m_threshold, 0, 'f', 1));
    
    // mode default
    m_settings.m_priority = static_cast<GsmChannelyzerSettings::Priority>(1);
    ui->priority->setCurrentIndex((int)m_settings.m_priority);
    ui->measurement->setCurrentIndex((int)m_settings.m_measurement);

    // mode by user
    // ui->mode->setCurrentIndex((int)m_settings.m_mode);
    // mode default
    m_settings.m_mode = static_cast<GsmChannelyzerSettings::Mode>(2);
    ui->mode->setCurrentIndex((int)m_settings.m_mode);
    
    ui->table->blockSignals(true);
    ui->table->setRowCount(0);
    for (int i = 0; i < m_settings.m_frequencySettings.size(); i++)
    {
        m_settings.m_frequencySettings[i].m_arfcn = ARFCN[i];
        m_settings.m_frequencySettings[i].m_mcc = MCC[i];
        m_settings.m_frequencySettings[i].m_mnc = MNC[i];

        addRow(m_settings.m_frequencySettings[i]);
        updateAnnotation(i);
    }
    ui->table->blockSignals(false);

    // Order and size columns
    QHeaderView* header = ui->table->horizontalHeader();
    for (int i = 0; i < m_settings.m_columnSizes.size(); i++)
    {
        bool hidden = m_settings.m_columnSizes[i] == 0;
        header->setSectionHidden(i, hidden);
        m_menu->actions().at(i)->setChecked(!hidden);
        if (m_settings.m_columnSizes[i] > 0) {
            ui->table->setColumnWidth(i, m_settings.m_columnSizes[i]);
        }
        header->moveSection(header->visualIndex(i), m_settings.m_columnIndexes[i]);
    }

    updateIndexLabel();

    getRollupContents()->restoreState(m_rollupState);
    updateAbsoluteCenterFrequency();
    blockApplySettings(false);
}

void GsmChannelyzerGUI::leaveEvent(QEvent* event)
{
    m_channelMarker.setHighlighted(false);
    ChannelGUI::leaveEvent(event);
}

void GsmChannelyzerGUI::enterEvent(EnterEventType* event)
{
    m_channelMarker.setHighlighted(true);
    ChannelGUI::enterEvent(event);
}

void GsmChannelyzerGUI::on_startStop_toggled(bool checked)
{
    if (checked)
    {
        GsmChannelyzer::MsgStartScan* message = GsmChannelyzer::MsgStartScan::create();
        m_freqScanner->getInputMessageQueue()->push(message);
    }
    else
    {
        GsmChannelyzer::MsgStopScan* message = GsmChannelyzer::MsgStopScan::create();
        m_freqScanner->getInputMessageQueue()->push(message);
    }
}

void GsmChannelyzerGUI::addRow(const GsmChannelyzerSettings::FrequencySettings& frequencySettings)
{
    int row = ui->table->rowCount();
    ui->table->setRowCount(row + 1);

    // Must create before frequency so updateAnnotation can work
    QTableWidgetItem* annotationItem = new QTableWidgetItem();
    annotationItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->table->setItem(row, COL_ANNOTATION, annotationItem);

    ui->table->setItem(row, COL_FREQUENCY, new QTableWidgetItem(QString("%1").arg(frequencySettings.m_frequency)));

    QTableWidgetItem *enableItem = new QTableWidgetItem();
    enableItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    enableItem->setCheckState(frequencySettings.m_enabled ? Qt::Checked : Qt::Unchecked);
    ui->table->setItem(row, COL_ENABLE, enableItem);

    QTableWidgetItem* powerItem = new QTableWidgetItem();
    powerItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->table->setItem(row, COL_POWER, powerItem);

    QTableWidgetItem *activeCountItem = new QTableWidgetItem();
    activeCountItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->table->setItem(row, COL_ACTIVE_COUNT, activeCountItem);
    activeCountItem->setData(Qt::DisplayRole, 0);

    QTableWidgetItem* notesItem = new QTableWidgetItem(frequencySettings.m_notes);
    ui->table->setItem(row, COL_NOTES, notesItem);

    QComboBox *channelComboBox = new QComboBox();
    updateChannelsCombo(channelComboBox, m_availableChannels, frequencySettings.m_channel, true);
    ui->table->setCellWidget(row, COL_CHANNEL, channelComboBox);
    connect(channelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GsmChannelyzerGUI::on_table_channel_currentIndexChanged);

    QTableWidgetItem* channelBandwidthItem = new QTableWidgetItem(frequencySettings.m_channelBandwidth);
    ui->table->setItem(row, COL_CHANNEL_BW, channelBandwidthItem);

    QTableWidgetItem* thresholdItem = new QTableWidgetItem(frequencySettings.m_threshold);
    ui->table->setItem(row, COL_TH, thresholdItem);

    QTableWidgetItem* squelchItem = new QTableWidgetItem(frequencySettings.m_squelch);
    ui->table->setItem(row, COL_SQ, squelchItem);

    ui->table->setItem(row, COL_ARFCN, new QTableWidgetItem(QString("%1").arg(frequencySettings.m_arfcn)));

    ui->table->setItem(row, COL_MCC, new QTableWidgetItem(QString("%1").arg(frequencySettings.m_mcc)));

    ui->table->setItem(row, COL_MNC, new QTableWidgetItem(QString("%1").arg(frequencySettings.m_mnc)));

}

void GsmChannelyzerGUI::on_table_channel_currentIndexChanged(int index)
{
    if (index >= 0)
    {
        QComboBox *combo = qobject_cast<QComboBox *>(sender());
        QModelIndex tableIndex = ui->table->indexAt(combo->pos());
        on_table_cellChanged(tableIndex.row(), tableIndex.column());
    }
}

void GsmChannelyzerGUI::on_addSingle_clicked()
{
    GsmChannelyzerSettings::FrequencySettings frequencySettings;
    frequencySettings.m_frequency = 0;
    frequencySettings.m_enabled = true;
    addRow(frequencySettings);
}

void GsmChannelyzerGUI::on_addRange_clicked()
{
    /*GsmChannelyzerAddRangeDialog dialog(m_settings.m_channelBandwidth, this);
    new DialogPositioner(&dialog, false);
    if (dialog.exec())
    {
        blockApplySettings(true);
        for (const auto f : dialog.m_frequencies)
        {
            GsmChannelyzerSettings::FrequencySettings frequencySettings;
            frequencySettings.m_frequency = f;
            frequencySettings.m_enabled = true;
            addRow(frequencySettings);
        }
        blockApplySettings(false);
        applySetting("frequencySettings");
    }*/

    

    blockApplySettings(true);
    //for (const auto f : hfFreqs)
    for (int i = 0; i < hfFreqs.size(); ++i)
    {
        GsmChannelyzerSettings::FrequencySettings frequencySettings;
        frequencySettings.m_frequency = hfFreqs[i];
        frequencySettings.m_enabled = true;
        frequencySettings.m_arfcn = ARFCN[i];
        frequencySettings.m_mcc = MCC[i];
        frequencySettings.m_mnc = MNC[i];
        addRow(frequencySettings);
    }
    blockApplySettings(false);
    applySetting("frequencySettings");
}

void GsmChannelyzerGUI::on_remove_clicked()
{
    QList<QTableWidgetItem*> items = ui->table->selectedItems();

    for (auto item : items)
    {
        int row = ui->table->row(item);
        ui->table->removeRow(row);
        m_settings.m_frequencySettings.removeAt(row);
    }
    applySetting("frequencySettings");
}

void GsmChannelyzerGUI::on_removeInactive_clicked()
{
    for (int i = ui->table->rowCount() - 1; i >= 0; i--)
    {
        QTableWidgetItem* activeCountItem = ui->table->item(i, COL_ACTIVE_COUNT);
        activeCountItem->setData(Qt::DisplayRole, 0);
    }
    applySetting("frequencySettings");
}

static QList<QTableWidgetItem*> takeRow(QTableWidget* table, int row)
{
    QList<QTableWidgetItem*> rowItems;

    for (int col = 0; col < table->columnCount(); col++) {
        rowItems.append(table->takeItem(row, col));
    }
    return rowItems;
}

static void setRow(QTableWidget* table, int row, const QList<QTableWidgetItem*>& rowItems)
{
    for (int col = 0; col < rowItems.size(); col++) {
        table->setItem(row, col, rowItems.at(col));
    }
}

void GsmChannelyzerGUI::on_up_clicked()
{
    QList<QTableWidgetItem*> items = ui->table->selectedItems();
    for (auto item : items)
    {
        int row = ui->table->row(item);
        if (row > 0)
        {
            QList<QTableWidgetItem*> sourceItems = takeRow(ui->table, row);
            QList<QTableWidgetItem*> destItems = takeRow(ui->table, row - 1);
            setRow(ui->table, row - 1, sourceItems);
            setRow(ui->table, row, destItems);
            ui->table->setCurrentCell(row - 1, 0);
        }
    }
}

void GsmChannelyzerGUI::on_down_clicked()
{
    QList<QTableWidgetItem*> items = ui->table->selectedItems();
    for (auto item : items)
    {
        int row = ui->table->row(item);
        if (row < ui->table->rowCount() - 1)
        {
            QList<QTableWidgetItem*> sourceItems = takeRow(ui->table, row);
            QList<QTableWidgetItem*> destItems = takeRow(ui->table, row + 1);
            setRow(ui->table, row + 1, sourceItems);
            setRow(ui->table, row, destItems);
            ui->table->setCurrentCell(row + 1, 0);
        }
    }
}

void GsmChannelyzerGUI::on_clearActiveCount_clicked()
{
    for (int i = 0; i < ui->table->rowCount(); i++) {
        ui->table->item(i, COL_ACTIVE_COUNT)->setData(Qt::DisplayRole, 0);
    }
}

void GsmChannelyzerGUI::on_table_cellChanged(int row, int column)
{
    QTableWidgetItem* item = ui->table->item(row, column);
    if (item)
    {
        if (column == COL_FREQUENCY)
        {
            qint64 value = item->text().toLongLong();
            while (m_settings.m_frequencySettings.size() <= row)
            {
                GsmChannelyzerSettings::FrequencySettings frequencySettings;
                frequencySettings.m_frequency = 0;
                frequencySettings.m_enabled = true;
                frequencySettings.m_arfcn = 0;
                frequencySettings.m_mcc = 0;
                frequencySettings.m_mnc = 0;
                m_settings.m_frequencySettings.append(frequencySettings);
            }
            m_settings.m_frequencySettings[row].m_frequency = value;
            updateAnnotation(row);
            applySetting("frequencySettings");
        }
        else if (column == COL_ENABLE)
        {
            m_settings.m_frequencySettings[row].m_enabled = item->checkState() == Qt::Checked;
            applySetting("frequencySettings");
        }
        else if (column == COL_NOTES)
        {
            m_settings.m_frequencySettings[row].m_notes = item->text();
            applySetting("frequencySettings");
        }
        else if (column == COL_CHANNEL_BW)
        {
            m_settings.m_frequencySettings[row].m_channelBandwidth = item->text();
            applySetting("frequencySettings");
        }
        else if (column == COL_TH)
        {
            m_settings.m_frequencySettings[row].m_threshold = item->text();
            applySetting("frequencySettings");
        }
        else if (column == COL_SQ)
        {
            m_settings.m_frequencySettings[row].m_squelch = item->text();
            applySetting("frequencySettings");
        }
        else if (column == COL_ARFCN)
        {
            qint64 value = item->text().toLongLong();
            m_settings.m_frequencySettings[row].m_arfcn = value;
            applySetting("frequencySettings");
        }
        else if (column == COL_MCC)
        {
            qint64 value = item->text().toLongLong();
            m_settings.m_frequencySettings[row].m_mcc = value;
            applySetting("frequencySettings");
        }
        else if (column == COL_MNC)
        {
            qint64 value = item->text().toLongLong();
            m_settings.m_frequencySettings[row].m_mnc = value;
            applySetting("frequencySettings");
        }
    }
    else if (column == COL_CHANNEL)
    {
        QComboBox *combo = qobject_cast<QComboBox *>(ui->table->cellWidget(row, COL_CHANNEL));
        m_settings.m_frequencySettings[row].m_channel = combo->currentText();
        qDebug() << "Setting row" << row << "to" << combo->currentText();
        applySetting("frequencySettings");
    }
}

void GsmChannelyzerGUI::updateAnnotation(int row)
{
    QTableWidgetItem* item = ui->table->item(row, COL_FREQUENCY);
    QTableWidgetItem* annotationItem = ui->table->item(row, COL_ANNOTATION);
    if (item && annotationItem)
    {
        qint64 frequency = item->text().toLongLong();
        const QList<SpectrumAnnotationMarker>& markers = m_deviceUISet->m_spectrum->getAnnotationMarkers();
        const SpectrumAnnotationMarker* closest = nullptr;
        for (const auto& marker : markers)
        {
            qint64 start1 = marker.m_startFrequency;
            qint64 stop1 = marker.m_startFrequency + marker.m_bandwidth;
            qint64 start2 = frequency - m_settings.m_channelBandwidth / 2;
            qint64 stop2 = frequency + m_settings.m_channelBandwidth / 2;
            if (   ((start2 >= start1) && (start2 <= stop1))
                || ((stop2 >= start1) && (stop2 <= stop1))
               )
            {
                if (marker.m_bandwidth == (unsigned)m_settings.m_channelBandwidth) {
                    // Exact match
                    annotationItem->setText(marker.m_text);
                    return;
                }
                else if (!closest)
                {
                    closest = &marker;
                }
                else
                {
                    if (marker.m_bandwidth < closest->m_bandwidth) {
                        closest = &marker;
                    }
                }
            }
        }
        if (closest) {
            annotationItem->setText(closest->m_text);
        }
    }
}

void GsmChannelyzerGUI::updateAnnotations()
{
    for (int i = 0; i < ui->table->rowCount(); i++) {
        updateAnnotation(i);
    }
}

void GsmChannelyzerGUI::setAllEnabled(bool enable)
{
    for (int i = 0; i < ui->table->rowCount(); i++) {
        ui->table->item(i, COL_ENABLE)->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
    }
}

void GsmChannelyzerGUI::table_customContextMenuRequested(QPoint pos)
{
    QTableWidgetItem* item = ui->table->itemAt(pos);
    if (item)
    {
        int row = item->row();

        QMenu* tableContextMenu = new QMenu(ui->table);
        connect(tableContextMenu, &QMenu::aboutToHide, tableContextMenu, &QMenu::deleteLater);

        // Copy current cell

        QAction* copyAction = new QAction("Copy", tableContextMenu);
        const QString text = item->text();
        connect(copyAction, &QAction::triggered, this, [text]()->void {
            QClipboard* clipboard = QGuiApplication::clipboard();
            clipboard->setText(text);
            });
        tableContextMenu->addAction(copyAction);

        tableContextMenu->addSeparator();

        // Enable all

        QAction* enableAllAction = new QAction("Enable all", tableContextMenu);
        connect(enableAllAction, &QAction::triggered, this, [this]()->void {
            setAllEnabled(true);
            });
        tableContextMenu->addAction(enableAllAction);

        // Disable all

        QAction* disableAllAction = new QAction("Disable all", tableContextMenu);
        connect(disableAllAction, &QAction::triggered, this, [this]()->void {
            setAllEnabled(false);
            });
        tableContextMenu->addAction(disableAllAction);

        // Remove selected rows

        QAction* removeAction = new QAction("Remove", tableContextMenu);
        connect(removeAction, &QAction::triggered, this, [this]()->void {
            on_remove_clicked();
            });
        tableContextMenu->addAction(removeAction);

        tableContextMenu->addSeparator();

        // Tune to frequency

        qint64 frequency = ui->table->item(row, COL_FREQUENCY)->text().toLongLong();
        GsmChannelyzerSettings::FrequencySettings *frequencySettings = m_settings.getFrequencySettings(frequency);
        QString channel = m_settings.getChannel(frequencySettings);
        unsigned int scanDeviceSetIndex, scanChannelIndex;

        if (MainCore::getDeviceAndChannelIndexFromId(channel, scanDeviceSetIndex, scanChannelIndex))
        {
            ButtonSwitch *startStop = ui->startStop;

            QAction* findChannelMapAction = new QAction(QString("Tune %1 to %2").arg(channel).arg(frequency), tableContextMenu);
            connect(findChannelMapAction, &QAction::triggered, this, [this, scanDeviceSetIndex, scanChannelIndex, frequency, startStop]()->void {

                // Stop scanning
                if (startStop->isChecked()) {
                    startStop->click();
                }

                // Mute all channels
                m_freqScanner->muteAll(m_settings);

                // Tune to frequency
                if ((frequency - m_settings.m_channelBandwidth / 2 < m_deviceCenterFrequency - m_basebandSampleRate / 2)
                    || (frequency + m_settings.m_channelBandwidth / 2 >= m_deviceCenterFrequency + m_basebandSampleRate / 2))
                {
                    qint64 centerFrequency = frequency;
                    int offset = 0;
                    while (frequency - centerFrequency < m_settings.m_channelFrequencyOffset)
                    {
                        centerFrequency -= m_settings.m_channelBandwidth;
                        offset += m_settings.m_channelBandwidth;
                    }

                    if (!ChannelWebAPIUtils::setCenterFrequency(getDeviceSetIndex(), centerFrequency)) {
                        qWarning() << "Scanner failed to set frequency" << centerFrequency;
                    }
                    ChannelWebAPIUtils::setFrequencyOffset(scanDeviceSetIndex, scanChannelIndex, offset);
                }
                else
                {
                    int offset = frequency - m_deviceCenterFrequency;
                    ChannelWebAPIUtils::setFrequencyOffset(scanDeviceSetIndex, scanChannelIndex, offset);
                }

                // Unmute channel
                ChannelWebAPIUtils::setAudioMute(scanDeviceSetIndex, scanChannelIndex, false);

                });
            tableContextMenu->addAction(findChannelMapAction);
        }
        else
        {
            qDebug() << "Failed to parse channel" << m_settings.m_channel;
        }

        tableContextMenu->popup(ui->table->viewport()->mapToGlobal(pos));
    }
}

// Columns in table reordered
void GsmChannelyzerGUI::table_sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    (void)oldVisualIndex;
    m_settings.m_columnIndexes[logicalIndex] = newVisualIndex;
}

// Column in table resized (when hidden size is 0)
void GsmChannelyzerGUI::table_sectionResized(int logicalIndex, int oldSize, int newSize)
{
    (void)oldSize;
    m_settings.m_columnSizes[logicalIndex] = newSize;
}

// Right click in ADSB table header - show column select menu
void GsmChannelyzerGUI::columnSelectMenu(QPoint pos)
{
    m_menu->popup(ui->table->horizontalHeader()->viewport()->mapToGlobal(pos));
}

// Hide/show column when menu selected
void GsmChannelyzerGUI::columnSelectMenuChecked(bool checked)
{
    (void)checked;
    QAction* action = qobject_cast<QAction*>(sender());
    if (action != nullptr)
    {
        int idx = action->data().toInt(nullptr);
        ui->table->setColumnHidden(idx, !action->isChecked());
    }
}

// Create column select menu item
QAction* GsmChannelyzerGUI::createCheckableItem(QString& text, int idx, bool checked)
{
    QAction* action = new QAction(text, this);
    action->setCheckable(true);
    action->setChecked(checked);
    action->setData(QVariant(idx));
    connect(action, SIGNAL(triggered()), this, SLOT(columnSelectMenuChecked()));
    return action;
}

void GsmChannelyzerGUI::resizeTable()
{
    // Fill table with a row of dummy data that will size the columns nicely
    int row = ui->table->rowCount();
    ui->table->setRowCount(row + 1);
    ui->table->setItem(row, COL_FREQUENCY, new QTableWidgetItem("800,000.5 MHz"));
    ui->table->setItem(row, COL_ANNOTATION, new QTableWidgetItem("London VOLMET"));
    ui->table->setItem(row, COL_ENABLE, new QTableWidgetItem("Enable"));
    ui->table->setItem(row, COL_POWER, new QTableWidgetItem("-100.0"));
    ui->table->setItem(row, COL_ACTIVE_COUNT, new QTableWidgetItem("10000"));
    ui->table->setItem(row, COL_NOTES, new QTableWidgetItem("A channel name"));
    ui->table->setItem(row, COL_CHANNEL, new QTableWidgetItem("Enter some notes"));
    ui->table->setItem(row, COL_CHANNEL_BW, new QTableWidgetItem("100000000"));
    ui->table->setItem(row, COL_TH, new QTableWidgetItem("-100.0"));
    ui->table->setItem(row, COL_SQ, new QTableWidgetItem("-100.0"));
    ui->table->setItem(row, COL_ARFCN, new QTableWidgetItem("100000000"));
    ui->table->setItem(row, COL_MCC, new QTableWidgetItem("100000000"));
    ui->table->setItem(row, COL_MNC, new QTableWidgetItem("100000000"));
    ui->table->resizeColumnsToContents();
    ui->table->setRowCount(row);
}

void GsmChannelyzerGUI::makeUIConnections()
{
    QObject::connect(ui->channels, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GsmChannelyzerGUI::on_channels_currentIndexChanged);
    QObject::connect(ui->deltaFrequency, &ValueDialZ::changed, this, &GsmChannelyzerGUI::on_deltaFrequency_changed);
    QObject::connect(ui->channelBandwidth, &ValueDialZ::changed, this, &GsmChannelyzerGUI::on_channelBandwidth_changed);
    QObject::connect(ui->scanTime, &QDial::valueChanged, this, &GsmChannelyzerGUI::on_scanTime_valueChanged);
    QObject::connect(ui->retransmitTime, &QDial::valueChanged, this, &GsmChannelyzerGUI::on_retransmitTime_valueChanged);
    QObject::connect(ui->tuneTime, &QDial::valueChanged, this, &GsmChannelyzerGUI::on_tuneTime_valueChanged);
    QObject::connect(ui->thresh, &QDial::valueChanged, this, &GsmChannelyzerGUI::on_thresh_valueChanged);
    QObject::connect(ui->priority, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GsmChannelyzerGUI::on_priority_currentIndexChanged);
    QObject::connect(ui->measurement, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GsmChannelyzerGUI::on_measurement_currentIndexChanged);
    QObject::connect(ui->mode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GsmChannelyzerGUI::on_mode_currentIndexChanged);
    QObject::connect(ui->startStop, &ButtonSwitch::toggled, this, &GsmChannelyzerGUI::on_startStop_toggled);
    QObject::connect(ui->table, &QTableWidget::cellChanged, this, &GsmChannelyzerGUI::on_table_cellChanged);
    QObject::connect(ui->addSingle, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_addSingle_clicked);
    QObject::connect(ui->addRange, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_addRange_clicked);
    QObject::connect(ui->remove, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_remove_clicked);
    QObject::connect(ui->removeInactive, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_removeInactive_clicked);
    QObject::connect(ui->up, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_up_clicked);
    QObject::connect(ui->down, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_down_clicked);
    QObject::connect(ui->clearActiveCount, &QToolButton::clicked, this, &GsmChannelyzerGUI::on_clearActiveCount_clicked);
    QObject::connect(ui->threshInc, &QToolButton::clicked, this, &GsmChannelyzerGUI::threshIncClick);
    QObject::connect(ui->threshDec, &QToolButton::clicked, this, &GsmChannelyzerGUI::threshDecClick);
    QObject::connect(ui->tuneTimeInc, &QToolButton::clicked, this, &GsmChannelyzerGUI::tuneTimeIncClick);
    QObject::connect(ui->tuneTimeDec, &QToolButton::clicked, this, &GsmChannelyzerGUI::tuneTimeDecClick);
    QObject::connect(ui->scanTimeInc, &QToolButton::clicked, this, &GsmChannelyzerGUI::scanTimeIncClick);
    QObject::connect(ui->scanTimeDec, &QToolButton::clicked, this, &GsmChannelyzerGUI::scanTimeDecClick);
    QObject::connect(ui->retransmitTimeInc, &QToolButton::clicked, this, &GsmChannelyzerGUI::retransmitTimeIncClick);
    QObject::connect(ui->retransmitTimeDec, &QToolButton::clicked, this, &GsmChannelyzerGUI::retransmitTimeDecClick);
}

void GsmChannelyzerGUI::updateAbsoluteCenterFrequency()
{
    setStatusFrequency(m_deviceCenterFrequency + m_settings.m_inputFrequencyOffset);
}
