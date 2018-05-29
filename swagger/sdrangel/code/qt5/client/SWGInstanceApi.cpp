/**
 * SDRangel
 * This is the web REST/JSON API of SDRangel SDR software. SDRangel is an Open Source Qt5/OpenGL 3.0+ (4.3+ in Windows) GUI and server Software Defined Radio and signal analyzer in software. It supports Airspy, BladeRF, HackRF, LimeSDR, PlutoSDR, RTL-SDR, SDRplay RSP1 and FunCube     ---   Limitations and specifcities:       * In SDRangel GUI the first Rx device set cannot be deleted. Conversely the server starts with no device sets and its number of device sets can be reduced to zero by as many calls as necessary to /sdrangel/deviceset with DELETE method.   * Preset import and export from/to file is a server only feature.   * Device set focus is a GUI only feature.   * The following channels are not implemented (status 501 is returned): ATV and DATV demodulators, Channel Analyzer NG, LoRa demodulator   * The device settings and report structures contains only the sub-structure corresponding to the device type. The DeviceSettings and DeviceReport structures documented here shows all of them but only one will be or should be present at a time   * The channel settings and report structures contains only the sub-structure corresponding to the channel type. The ChannelSettings and ChannelReport structures documented here shows all of them but only one will be or should be present at a time    --- 
 *
 * OpenAPI spec version: 4.0.0
 * Contact: f4exb06@gmail.com
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */

#include "SWGInstanceApi.h"
#include "SWGHelpers.h"
#include "SWGModelFactory.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace SWGSDRangel {

SWGInstanceApi::SWGInstanceApi() {}

SWGInstanceApi::~SWGInstanceApi() {}

SWGInstanceApi::SWGInstanceApi(QString host, QString basePath) {
    this->host = host;
    this->basePath = basePath;
}

void
SWGInstanceApi::instanceAudioGet() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioGetCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioGetCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGAudioDevices* output = static_cast<SWGAudioDevices*>(create(json, QString("SWGAudioDevices")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioGetSignal(output);
    } else {
        emit instanceAudioGetSignalE(output, error_type, error_str);
        emit instanceAudioGetSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioInputCleanupPatch() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/input/cleanup");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioInputCleanupPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioInputCleanupPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGSuccessResponse* output = static_cast<SWGSuccessResponse*>(create(json, QString("SWGSuccessResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioInputCleanupPatchSignal(output);
    } else {
        emit instanceAudioInputCleanupPatchSignalE(output, error_type, error_str);
        emit instanceAudioInputCleanupPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioInputDelete(SWGAudioInputDevice& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/input/parameters");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "DELETE");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioInputDeleteCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioInputDeleteCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGAudioInputDevice* output = static_cast<SWGAudioInputDevice*>(create(json, QString("SWGAudioInputDevice")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioInputDeleteSignal(output);
    } else {
        emit instanceAudioInputDeleteSignalE(output, error_type, error_str);
        emit instanceAudioInputDeleteSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioInputPatch(SWGAudioInputDevice& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/input/parameters");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioInputPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioInputPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGAudioInputDevice* output = static_cast<SWGAudioInputDevice*>(create(json, QString("SWGAudioInputDevice")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioInputPatchSignal(output);
    } else {
        emit instanceAudioInputPatchSignalE(output, error_type, error_str);
        emit instanceAudioInputPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioOutputCleanupPatch() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/output/cleanup");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioOutputCleanupPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioOutputCleanupPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGSuccessResponse* output = static_cast<SWGSuccessResponse*>(create(json, QString("SWGSuccessResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioOutputCleanupPatchSignal(output);
    } else {
        emit instanceAudioOutputCleanupPatchSignalE(output, error_type, error_str);
        emit instanceAudioOutputCleanupPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioOutputDelete(SWGAudioOutputDevice& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/output/parameters");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "DELETE");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioOutputDeleteCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioOutputDeleteCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGAudioOutputDevice* output = static_cast<SWGAudioOutputDevice*>(create(json, QString("SWGAudioOutputDevice")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioOutputDeleteSignal(output);
    } else {
        emit instanceAudioOutputDeleteSignalE(output, error_type, error_str);
        emit instanceAudioOutputDeleteSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceAudioOutputPatch(SWGAudioOutputDevice& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/audio/output/parameters");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceAudioOutputPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceAudioOutputPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGAudioOutputDevice* output = static_cast<SWGAudioOutputDevice*>(create(json, QString("SWGAudioOutputDevice")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceAudioOutputPatchSignal(output);
    } else {
        emit instanceAudioOutputPatchSignalE(output, error_type, error_str);
        emit instanceAudioOutputPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceChannels(qint32 tx) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/channels");


    if (fullPath.indexOf("?") > 0)
      fullPath.append("&");
    else
      fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("tx"))
        .append("=")
        .append(QUrl::toPercentEncoding(stringValue(tx)));


    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceChannelsCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceChannelsCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGInstanceChannelsResponse* output = static_cast<SWGInstanceChannelsResponse*>(create(json, QString("SWGInstanceChannelsResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceChannelsSignal(output);
    } else {
        emit instanceChannelsSignalE(output, error_type, error_str);
        emit instanceChannelsSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceDVSerialPatch(qint32 dvserial) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/dvserial");


    if (fullPath.indexOf("?") > 0)
      fullPath.append("&");
    else
      fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("dvserial"))
        .append("=")
        .append(QUrl::toPercentEncoding(stringValue(dvserial)));


    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceDVSerialPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceDVSerialPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGDVSeralDevices* output = static_cast<SWGDVSeralDevices*>(create(json, QString("SWGDVSeralDevices")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceDVSerialPatchSignal(output);
    } else {
        emit instanceDVSerialPatchSignalE(output, error_type, error_str);
        emit instanceDVSerialPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceDelete() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "DELETE");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceDeleteCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceDeleteCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGInstanceSummaryResponse* output = static_cast<SWGInstanceSummaryResponse*>(create(json, QString("SWGInstanceSummaryResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceDeleteSignal(output);
    } else {
        emit instanceDeleteSignalE(output, error_type, error_str);
        emit instanceDeleteSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceDeviceSetsGet() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/devicesets");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceDeviceSetsGetCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceDeviceSetsGetCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGDeviceSetList* output = static_cast<SWGDeviceSetList*>(create(json, QString("SWGDeviceSetList")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceDeviceSetsGetSignal(output);
    } else {
        emit instanceDeviceSetsGetSignalE(output, error_type, error_str);
        emit instanceDeviceSetsGetSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceDevices(qint32 tx) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/devices");


    if (fullPath.indexOf("?") > 0)
      fullPath.append("&");
    else
      fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("tx"))
        .append("=")
        .append(QUrl::toPercentEncoding(stringValue(tx)));


    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceDevicesCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceDevicesCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGInstanceDevicesResponse* output = static_cast<SWGInstanceDevicesResponse*>(create(json, QString("SWGInstanceDevicesResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceDevicesSignal(output);
    } else {
        emit instanceDevicesSignalE(output, error_type, error_str);
        emit instanceDevicesSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceLocationGet() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/location");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceLocationGetCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceLocationGetCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGLocationInformation* output = static_cast<SWGLocationInformation*>(create(json, QString("SWGLocationInformation")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceLocationGetSignal(output);
    } else {
        emit instanceLocationGetSignalE(output, error_type, error_str);
        emit instanceLocationGetSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceLocationPut(SWGLocationInformation& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/location");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PUT");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceLocationPutCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceLocationPutCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGLocationInformation* output = static_cast<SWGLocationInformation*>(create(json, QString("SWGLocationInformation")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceLocationPutSignal(output);
    } else {
        emit instanceLocationPutSignalE(output, error_type, error_str);
        emit instanceLocationPutSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceLoggingGet() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/logging");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceLoggingGetCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceLoggingGetCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGLoggingInfo* output = static_cast<SWGLoggingInfo*>(create(json, QString("SWGLoggingInfo")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceLoggingGetSignal(output);
    } else {
        emit instanceLoggingGetSignalE(output, error_type, error_str);
        emit instanceLoggingGetSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceLoggingPut(SWGLoggingInfo& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/logging");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PUT");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceLoggingPutCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceLoggingPutCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGLoggingInfo* output = static_cast<SWGLoggingInfo*>(create(json, QString("SWGLoggingInfo")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceLoggingPutSignal(output);
    } else {
        emit instanceLoggingPutSignalE(output, error_type, error_str);
        emit instanceLoggingPutSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetDelete(SWGPresetIdentifier& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "DELETE");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetDeleteCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetDeleteCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetDeleteSignal(output);
    } else {
        emit instancePresetDeleteSignalE(output, error_type, error_str);
        emit instancePresetDeleteSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetFilePost(SWGPresetExport& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset/file");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "POST");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetFilePostCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetFilePostCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetFilePostSignal(output);
    } else {
        emit instancePresetFilePostSignalE(output, error_type, error_str);
        emit instancePresetFilePostSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetFilePut(SWGPresetImport& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset/file");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PUT");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetFilePutCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetFilePutCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetFilePutSignal(output);
    } else {
        emit instancePresetFilePutSignalE(output, error_type, error_str);
        emit instancePresetFilePutSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetGet() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/presets");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetGetCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetGetCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresets* output = static_cast<SWGPresets*>(create(json, QString("SWGPresets")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetGetSignal(output);
    } else {
        emit instancePresetGetSignalE(output, error_type, error_str);
        emit instancePresetGetSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetPatch(SWGPresetTransfer& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PATCH");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetPatchCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetPatchCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetPatchSignal(output);
    } else {
        emit instancePresetPatchSignalE(output, error_type, error_str);
        emit instancePresetPatchSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetPost(SWGPresetTransfer& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "POST");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetPostCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetPostCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetPostSignal(output);
    } else {
        emit instancePresetPostSignalE(output, error_type, error_str);
        emit instancePresetPostSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instancePresetPut(SWGPresetTransfer& body) {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel/preset");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "PUT");


    
    QString output = body.asJson();
    input.request_body.append(output);
    


    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instancePresetPutCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instancePresetPutCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGPresetIdentifier* output = static_cast<SWGPresetIdentifier*>(create(json, QString("SWGPresetIdentifier")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instancePresetPutSignal(output);
    } else {
        emit instancePresetPutSignalE(output, error_type, error_str);
        emit instancePresetPutSignalEFull(worker, error_type, error_str);
    }
}

void
SWGInstanceApi::instanceSummary() {
    QString fullPath;
    fullPath.append(this->host).append(this->basePath).append("/sdrangel");



    SWGHttpRequestWorker *worker = new SWGHttpRequestWorker();
    SWGHttpRequestInput input(fullPath, "GET");





    foreach(QString key, this->defaultHeaders.keys()) {
        input.headers.insert(key, this->defaultHeaders.value(key));
    }

    connect(worker,
            &SWGHttpRequestWorker::on_execution_finished,
            this,
            &SWGInstanceApi::instanceSummaryCallback);

    worker->execute(&input);
}

void
SWGInstanceApi::instanceSummaryCallback(SWGHttpRequestWorker * worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    }
    else {
        msg = "Error: " + worker->error_str;
    }


    QString json(worker->response);
    SWGInstanceSummaryResponse* output = static_cast<SWGInstanceSummaryResponse*>(create(json, QString("SWGInstanceSummaryResponse")));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit instanceSummarySignal(output);
    } else {
        emit instanceSummarySignalE(output, error_type, error_str);
        emit instanceSummarySignalEFull(worker, error_type, error_str);
    }
}


}
