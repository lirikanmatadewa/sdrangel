///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2015-2017, 2019 Edouard Griffiths, F4EXB <f4exb06@gmail.com>    //
// Copyright (C) 2015 John Greb <hexameron@spam.no>                              //
// Copyright (C) 2020, 2022 Jon Beniston, M7RCE <jon@beniston.com>               //
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

#include "gui/mypositiondialog.h"
#include "ui_myposdialog.h"
#include "maincore.h"

#include <QGeoCoordinate>

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QTimer>

MyPositionDialog::MyPositionDialog(MainSettings& mainSettings, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::MyPositionDialog),
	m_mainSettings(mainSettings)
{
	ui->setupUi(this);
    ui->name->setText(m_mainSettings.getStationName());
    ui->latitudeSpinBox->setValue(m_mainSettings.getLatitude());
    ui->longitudeSpinBox->setValue(m_mainSettings.getLongitude());
    ui->altitudeSpinBox->setValue(m_mainSettings.getAltitude());
    ui->autoUpdatePosition->setChecked(m_mainSettings.getAutoUpdatePosition());

    ui->ipDevice->setText(loadIPAddress());

    m_syncManager = new QNetworkAccessManager(this);

    // 1. BACA STATUS TERAKHIR DARI FILE
    m_syncConnected = loadSyncState();

    // 2. KONDISIKAN UI BERDASARKAN STATUS TERAKHIR
    if (m_syncConnected)
    {
        // Jika sebelumnya sudah connect, kembalikan tampilan ke hijau
        ui->statusKraken->setText("<span style='color: #00cc00; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Connected</span>");
        ui->btnSyncKraken->setText("DF Disconnect");
        updateSyncUI(true);

        // Lanjutkan timer polling
        m_syncTimer.start(1000);
    }
    else
    {
        // Jika belum connect, tampilkan merah
        ui->statusKraken->setText("<span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Disconnected</span>");
        ui->btnSyncKraken->setText("DF Connect");
        updateSyncUI(false);
    }

    connect(
        m_syncManager,
        &QNetworkAccessManager::finished,
        this,
        &MyPositionDialog::syncKrakenReply
    );

    connect(
        &m_syncTimer,
        &QTimer::timeout,
        this,
        &MyPositionDialog::syncKrakenRequest
    );

    /*ui->statusKraken->setText("Disconnected");

    ui->statusKraken->setFixedSize(16, 16);
    ui->statusKraken->setStyleSheet(
        "background:red;"
        "border-radius:8px;"
    );*/

    ui->statusKraken->setText("<span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Disconnected</span>");
}

MyPositionDialog::~MyPositionDialog()
{
	delete ui;
}

void MyPositionDialog::accept()
{
    m_mainSettings.setStationName(ui->name->text());
    m_mainSettings.setLatitude(ui->latitudeSpinBox->value());
    m_mainSettings.setLongitude(ui->longitudeSpinBox->value());
    m_mainSettings.setAltitude(ui->altitudeSpinBox->value());
    m_mainSettings.setAutoUpdatePosition(ui->autoUpdatePosition->isChecked());

    saveIPAddress(ui->ipDevice->text());

	QDialog::accept();
}

void MyPositionDialog::on_gps_clicked()
{
    const QGeoPositionInfo& position = MainCore::instance()->getPosition();
    if (position.isValid())
    {
        QGeoCoordinate coord = position.coordinate();
        ui->latitudeSpinBox->setValue(coord.latitude());
        ui->longitudeSpinBox->setValue(coord.longitude());
        ui->altitudeSpinBox->setValue(coord.altitude());
    }
    else
    {
        qDebug() << "MyPositionDialog::on_gps_clicked: Position is not valid.";
    }
}

QString MyPositionDialog::loadIPAddress()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir().mkpath(path);

    QFile file(path + "/ip_device.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return "192.168.1.10";
    }

    QTextStream in(&file);
    QString ip = in.readLine().trimmed();

    file.close();

    if (ip.isEmpty())
    {
        return "192.168.1.10";
    }

    return ip;
}

void MyPositionDialog::saveIPAddress(const QString& ip)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir().mkpath(path);

    QFile file(path + "/ip_device.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed save IP file";
        return;
    }

    QTextStream out(&file);
    out << ip;

    file.close();
}

void MyPositionDialog::saveSyncState(bool enabled)
{
    QString path =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);

    QDir().mkpath(path);

    QFile file(path + "/kraken_sync.txt");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed save sync state";
        return;
    }

    QTextStream out(&file);

    out << (enabled ? "1" : "0");

    file.close();
}

bool MyPositionDialog::loadSyncState()
{
    QString path =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation);

    QFile file(path + "/kraken_sync.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream in(&file);

    QString value =
        in.readLine().trimmed();

    file.close();

    return value == "1";
}

void MyPositionDialog::on_btnSyncKraken_clicked()
{
    if (!m_syncConnected)
    {
        // --- GANTI PENGATURAN TEKS & STYLE ---
        ui->statusKraken->setText("<span style='color: #ff9900; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Connecting...</span>");

        ui->btnSyncKraken->setText("Connecting...");

        ui->ipDevice->setEnabled(false);
        ui->name->setEnabled(false);
        ui->latitudeSpinBox->setEnabled(false);
        ui->longitudeSpinBox->setEnabled(false);

        syncKrakenRequest();
    }
    else
    {
        m_syncConnected = false;
        m_syncTimer.stop();

        saveSyncState(false);
        updateSyncUI(false);

        ui->btnSyncKraken->setText("DF Connect");

        // --- GANTI PENGATURAN TEKS DISCONNECTED ---
        ui->statusKraken->setText("<span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Disconnected</span>");
    }
}

void MyPositionDialog::syncKrakenReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Koneksi Error:" << reply->errorString();
        m_syncConnected = false;
        m_syncTimer.stop();
        saveSyncState(false);
        updateSyncUI(false);
        ui->btnSyncKraken->setText("DF Connect");

        // ERROR / TIMEOUT (MERAH)
        ui->statusKraken->setText("<span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Error / Timeout</span>");

        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << "Parsing JSON Error";
        m_syncConnected = false;
        m_syncTimer.stop();
        saveSyncState(false);
        updateSyncUI(false);
        ui->btnSyncKraken->setText("DF Connect");

        // PARSE ERROR (MERAH)
        ui->statusKraken->setText("<span style='color: #cc0000; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Parse Error</span>");

        reply->deleteLater();
        return;
    }

    if (!m_syncConnected)
    {
        m_syncConnected = true;
        saveSyncState(true);
        updateSyncUI(true);
        ui->btnSyncKraken->setText("DF Disconnect");
        m_syncTimer.start(1000);
    }

    QJsonObject obj = doc.object();
    double latitude = obj["start_lat"].toDouble();
    double longitude = obj["start_lon"].toDouble();
    QString stationId = obj["station_id"].toString();

    m_mainSettings.setStationName(stationId);
    m_mainSettings.setLatitude(latitude);
    m_mainSettings.setLongitude(longitude);

    ui->latitudeSpinBox->setValue(latitude);
    ui->longitudeSpinBox->setValue(longitude);

    if (!stationId.isEmpty())
    {
        ui->name->setText(stationId);
    }

    // CONNECTED (HIJAU)
    ui->statusKraken->setText("<span style='color: #00cc00; font-size: 18pt; vertical-align: middle;'>&#9679;</span> <span style='vertical-align: middle;'>Connected</span>");

    reply->deleteLater();
}

void MyPositionDialog::updateSyncUI(bool connected)
{
    if (connected)
    {
        // Hapus: ui->statusKraken->setStyleSheet(...);
        ui->name->setEnabled(false);
        ui->latitudeSpinBox->setEnabled(false);
        ui->longitudeSpinBox->setEnabled(false);
        ui->ipDevice->setEnabled(false);
    }
    else
    {
        // Hapus: ui->statusKraken->setStyleSheet(...);
        ui->name->setEnabled(true);
        ui->latitudeSpinBox->setEnabled(true);
        ui->longitudeSpinBox->setEnabled(true);
        ui->ipDevice->setEnabled(true);
    }
}

void MyPositionDialog::syncKrakenRequest()
{
    QString ip = ui->ipDevice->text().trimmed();

    QString url =
        QString("http://%1:9000/get_map_data")
        .arg(ip);

    QNetworkRequest request{ QUrl(url) };

    m_syncManager->get(request);
}