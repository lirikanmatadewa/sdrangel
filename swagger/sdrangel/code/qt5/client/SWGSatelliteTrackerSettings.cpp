/**
 * SDRangel
 * This is the web REST/JSON API of SDRangel SDR software. SDRangel is an Open Source Qt5/OpenGL 3.0+ (4.3+ in Windows) GUI and server Software Defined Radio and signal analyzer in software. It supports Airspy, BladeRF, HackRF, LimeSDR, PlutoSDR, RTL-SDR, SDRplay RSP1 and FunCube    ---   Limitations and specifcities:    * In SDRangel GUI the first Rx device set cannot be deleted. Conversely the server starts with no device sets and its number of device sets can be reduced to zero by as many calls as necessary to /sdrangel/deviceset with DELETE method.   * Preset import and export from/to file is a server only feature.   * Device set focus is a GUI only feature.   * The following channels are not implemented (status 501 is returned): ATV and DATV demodulators, Channel Analyzer NG, LoRa demodulator   * The device settings and report structures contains only the sub-structure corresponding to the device type. The DeviceSettings and DeviceReport structures documented here shows all of them but only one will be or should be present at a time   * The channel settings and report structures contains only the sub-structure corresponding to the channel type. The ChannelSettings and ChannelReport structures documented here shows all of them but only one will be or should be present at a time    --- 
 *
 * OpenAPI spec version: 7.0.0
 * Contact: f4exb06@gmail.com
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */


#include "SWGSatelliteTrackerSettings.h"

#include "SWGHelpers.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QObject>
#include <QDebug>

namespace SWGSDRangel {

SWGSatelliteTrackerSettings::SWGSatelliteTrackerSettings(QString* json) {
    init();
    this->fromJson(*json);
}

SWGSatelliteTrackerSettings::SWGSatelliteTrackerSettings() {
    latitude = 0.0f;
    m_latitude_isSet = false;
    longitude = 0.0f;
    m_longitude_isSet = false;
    height_above_sea_level = 0.0f;
    m_height_above_sea_level_isSet = false;
    target = nullptr;
    m_target_isSet = false;
    satellites = nullptr;
    m_satellites_isSet = false;
    tles = nullptr;
    m_tles_isSet = false;
    date_time = nullptr;
    m_date_time_isSet = false;
    min_aos_elevation = 0;
    m_min_aos_elevation_isSet = false;
    min_pass_elevation = 0;
    m_min_pass_elevation_isSet = false;
    rotator_max_azimuth = 0;
    m_rotator_max_azimuth_isSet = false;
    rotator_max_elevation = 0;
    m_rotator_max_elevation_isSet = false;
    az_el_units = 0;
    m_az_el_units_isSet = false;
    ground_track_points = 0;
    m_ground_track_points_isSet = false;
    date_format = nullptr;
    m_date_format_isSet = false;
    utc = 0;
    m_utc_isSet = false;
    update_period = 0.0f;
    m_update_period_isSet = false;
    doppler_period = 0.0f;
    m_doppler_period_isSet = false;
    prediction_period = 0;
    m_prediction_period_isSet = false;
    pass_start_time = nullptr;
    m_pass_start_time_isSet = false;
    pass_finish_time = nullptr;
    m_pass_finish_time_isSet = false;
    default_frequency = 0.0f;
    m_default_frequency_isSet = false;
    draw_on_map = 0;
    m_draw_on_map_isSet = false;
    auto_target = 0;
    m_auto_target_isSet = false;
    aos_speech = nullptr;
    m_aos_speech_isSet = false;
    los_speech = nullptr;
    m_los_speech_isSet = false;
    aos_command = nullptr;
    m_aos_command_isSet = false;
    los_command = nullptr;
    m_los_command_isSet = false;
    device_settings = nullptr;
    m_device_settings_isSet = false;
    azimuth_offset = 0.0f;
    m_azimuth_offset_isSet = false;
    elevation_offset = 0.0f;
    m_elevation_offset_isSet = false;
    title = nullptr;
    m_title_isSet = false;
    rgb_color = 0;
    m_rgb_color_isSet = false;
    use_reverse_api = 0;
    m_use_reverse_api_isSet = false;
    reverse_api_address = nullptr;
    m_reverse_api_address_isSet = false;
    reverse_api_port = 0;
    m_reverse_api_port_isSet = false;
    reverse_api_feature_set_index = 0;
    m_reverse_api_feature_set_index_isSet = false;
    reverse_api_feature_index = 0;
    m_reverse_api_feature_index_isSet = false;
    rollup_state = nullptr;
    m_rollup_state_isSet = false;
}

SWGSatelliteTrackerSettings::~SWGSatelliteTrackerSettings() {
    this->cleanup();
}

void
SWGSatelliteTrackerSettings::init() {
    latitude = 0.0f;
    m_latitude_isSet = false;
    longitude = 0.0f;
    m_longitude_isSet = false;
    height_above_sea_level = 0.0f;
    m_height_above_sea_level_isSet = false;
    target = new QString("");
    m_target_isSet = false;
    satellites = new QList<QString*>();
    m_satellites_isSet = false;
    tles = new QList<QString*>();
    m_tles_isSet = false;
    date_time = new QString("");
    m_date_time_isSet = false;
    min_aos_elevation = 0;
    m_min_aos_elevation_isSet = false;
    min_pass_elevation = 0;
    m_min_pass_elevation_isSet = false;
    rotator_max_azimuth = 0;
    m_rotator_max_azimuth_isSet = false;
    rotator_max_elevation = 0;
    m_rotator_max_elevation_isSet = false;
    az_el_units = 0;
    m_az_el_units_isSet = false;
    ground_track_points = 0;
    m_ground_track_points_isSet = false;
    date_format = new QString("");
    m_date_format_isSet = false;
    utc = 0;
    m_utc_isSet = false;
    update_period = 0.0f;
    m_update_period_isSet = false;
    doppler_period = 0.0f;
    m_doppler_period_isSet = false;
    prediction_period = 0;
    m_prediction_period_isSet = false;
    pass_start_time = new QString("");
    m_pass_start_time_isSet = false;
    pass_finish_time = new QString("");
    m_pass_finish_time_isSet = false;
    default_frequency = 0.0f;
    m_default_frequency_isSet = false;
    draw_on_map = 0;
    m_draw_on_map_isSet = false;
    auto_target = 0;
    m_auto_target_isSet = false;
    aos_speech = new QString("");
    m_aos_speech_isSet = false;
    los_speech = new QString("");
    m_los_speech_isSet = false;
    aos_command = new QString("");
    m_aos_command_isSet = false;
    los_command = new QString("");
    m_los_command_isSet = false;
    device_settings = new QList<SWGSatelliteDeviceSettingsList*>();
    m_device_settings_isSet = false;
    azimuth_offset = 0.0f;
    m_azimuth_offset_isSet = false;
    elevation_offset = 0.0f;
    m_elevation_offset_isSet = false;
    title = new QString("");
    m_title_isSet = false;
    rgb_color = 0;
    m_rgb_color_isSet = false;
    use_reverse_api = 0;
    m_use_reverse_api_isSet = false;
    reverse_api_address = new QString("");
    m_reverse_api_address_isSet = false;
    reverse_api_port = 0;
    m_reverse_api_port_isSet = false;
    reverse_api_feature_set_index = 0;
    m_reverse_api_feature_set_index_isSet = false;
    reverse_api_feature_index = 0;
    m_reverse_api_feature_index_isSet = false;
    rollup_state = new SWGRollupState();
    m_rollup_state_isSet = false;
}

void
SWGSatelliteTrackerSettings::cleanup() {



    if(target != nullptr) { 
        delete target;
    }
    if(satellites != nullptr) { 
        auto arr = satellites;
        for(auto o: *arr) { 
            delete o;
        }
        delete satellites;
    }
    if(tles != nullptr) { 
        auto arr = tles;
        for(auto o: *arr) { 
            delete o;
        }
        delete tles;
    }
    if(date_time != nullptr) { 
        delete date_time;
    }






    if(date_format != nullptr) { 
        delete date_format;
    }




    if(pass_start_time != nullptr) { 
        delete pass_start_time;
    }
    if(pass_finish_time != nullptr) { 
        delete pass_finish_time;
    }



    if(aos_speech != nullptr) { 
        delete aos_speech;
    }
    if(los_speech != nullptr) { 
        delete los_speech;
    }
    if(aos_command != nullptr) { 
        delete aos_command;
    }
    if(los_command != nullptr) { 
        delete los_command;
    }
    if(device_settings != nullptr) { 
        auto arr = device_settings;
        for(auto o: *arr) { 
            delete o;
        }
        delete device_settings;
    }


    if(title != nullptr) { 
        delete title;
    }


    if(reverse_api_address != nullptr) { 
        delete reverse_api_address;
    }



    if(rollup_state != nullptr) { 
        delete rollup_state;
    }
}

SWGSatelliteTrackerSettings*
SWGSatelliteTrackerSettings::fromJson(QString &json) {
    QByteArray array (json.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
    return this;
}

void
SWGSatelliteTrackerSettings::fromJsonObject(QJsonObject &pJson) {
    ::SWGSDRangel::setValue(&latitude, pJson["latitude"], "float", "");
    
    ::SWGSDRangel::setValue(&longitude, pJson["longitude"], "float", "");
    
    ::SWGSDRangel::setValue(&height_above_sea_level, pJson["heightAboveSeaLevel"], "float", "");
    
    ::SWGSDRangel::setValue(&target, pJson["target"], "QString", "QString");
    
    
    ::SWGSDRangel::setValue(&satellites, pJson["satellites"], "QList", "QString");
    
    ::SWGSDRangel::setValue(&tles, pJson["tles"], "QList", "QString");
    ::SWGSDRangel::setValue(&date_time, pJson["dateTime"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&min_aos_elevation, pJson["minAOSElevation"], "qint32", "");
    
    ::SWGSDRangel::setValue(&min_pass_elevation, pJson["minPassElevation"], "qint32", "");
    
    ::SWGSDRangel::setValue(&rotator_max_azimuth, pJson["rotatorMaxAzimuth"], "qint32", "");
    
    ::SWGSDRangel::setValue(&rotator_max_elevation, pJson["rotatorMaxElevation"], "qint32", "");
    
    ::SWGSDRangel::setValue(&az_el_units, pJson["azElUnits"], "qint32", "");
    
    ::SWGSDRangel::setValue(&ground_track_points, pJson["groundTrackPoints"], "qint32", "");
    
    ::SWGSDRangel::setValue(&date_format, pJson["dateFormat"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&utc, pJson["utc"], "qint32", "");
    
    ::SWGSDRangel::setValue(&update_period, pJson["updatePeriod"], "float", "");
    
    ::SWGSDRangel::setValue(&doppler_period, pJson["dopplerPeriod"], "float", "");
    
    ::SWGSDRangel::setValue(&prediction_period, pJson["predictionPeriod"], "qint32", "");
    
    ::SWGSDRangel::setValue(&pass_start_time, pJson["passStartTime"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&pass_finish_time, pJson["passFinishTime"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&default_frequency, pJson["defaultFrequency"], "float", "");
    
    ::SWGSDRangel::setValue(&draw_on_map, pJson["drawOnMap"], "qint32", "");
    
    ::SWGSDRangel::setValue(&auto_target, pJson["autoTarget"], "qint32", "");
    
    ::SWGSDRangel::setValue(&aos_speech, pJson["aosSpeech"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&los_speech, pJson["losSpeech"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&aos_command, pJson["aosCommand"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&los_command, pJson["losCommand"], "QString", "QString");
    
    
    ::SWGSDRangel::setValue(&device_settings, pJson["deviceSettings"], "QList", "SWGSatelliteDeviceSettingsList");
    ::SWGSDRangel::setValue(&azimuth_offset, pJson["azimuthOffset"], "float", "");
    
    ::SWGSDRangel::setValue(&elevation_offset, pJson["elevationOffset"], "float", "");
    
    ::SWGSDRangel::setValue(&title, pJson["title"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&rgb_color, pJson["rgbColor"], "qint32", "");
    
    ::SWGSDRangel::setValue(&use_reverse_api, pJson["useReverseAPI"], "qint32", "");
    
    ::SWGSDRangel::setValue(&reverse_api_address, pJson["reverseAPIAddress"], "QString", "QString");
    
    ::SWGSDRangel::setValue(&reverse_api_port, pJson["reverseAPIPort"], "qint32", "");
    
    ::SWGSDRangel::setValue(&reverse_api_feature_set_index, pJson["reverseAPIFeatureSetIndex"], "qint32", "");
    
    ::SWGSDRangel::setValue(&reverse_api_feature_index, pJson["reverseAPIFeatureIndex"], "qint32", "");
    
    ::SWGSDRangel::setValue(&rollup_state, pJson["rollupState"], "SWGRollupState", "SWGRollupState");
    
}

QString
SWGSatelliteTrackerSettings::asJson ()
{
    QJsonObject* obj = this->asJsonObject();

    QJsonDocument doc(*obj);
    QByteArray bytes = doc.toJson();
    delete obj;
    return QString(bytes);
}

QJsonObject*
SWGSatelliteTrackerSettings::asJsonObject() {
    QJsonObject* obj = new QJsonObject();
    if(m_latitude_isSet){
        obj->insert("latitude", QJsonValue(latitude));
    }
    if(m_longitude_isSet){
        obj->insert("longitude", QJsonValue(longitude));
    }
    if(m_height_above_sea_level_isSet){
        obj->insert("heightAboveSeaLevel", QJsonValue(height_above_sea_level));
    }
    if(target != nullptr && *target != QString("")){
        toJsonValue(QString("target"), target, obj, QString("QString"));
    }
    if(satellites && satellites->size() > 0){
        toJsonArray((QList<void*>*)satellites, obj, "satellites", "QString");
    }
    if(tles && tles->size() > 0){
        toJsonArray((QList<void*>*)tles, obj, "tles", "QString");
    }
    if(date_time != nullptr && *date_time != QString("")){
        toJsonValue(QString("dateTime"), date_time, obj, QString("QString"));
    }
    if(m_min_aos_elevation_isSet){
        obj->insert("minAOSElevation", QJsonValue(min_aos_elevation));
    }
    if(m_min_pass_elevation_isSet){
        obj->insert("minPassElevation", QJsonValue(min_pass_elevation));
    }
    if(m_rotator_max_azimuth_isSet){
        obj->insert("rotatorMaxAzimuth", QJsonValue(rotator_max_azimuth));
    }
    if(m_rotator_max_elevation_isSet){
        obj->insert("rotatorMaxElevation", QJsonValue(rotator_max_elevation));
    }
    if(m_az_el_units_isSet){
        obj->insert("azElUnits", QJsonValue(az_el_units));
    }
    if(m_ground_track_points_isSet){
        obj->insert("groundTrackPoints", QJsonValue(ground_track_points));
    }
    if(date_format != nullptr && *date_format != QString("")){
        toJsonValue(QString("dateFormat"), date_format, obj, QString("QString"));
    }
    if(m_utc_isSet){
        obj->insert("utc", QJsonValue(utc));
    }
    if(m_update_period_isSet){
        obj->insert("updatePeriod", QJsonValue(update_period));
    }
    if(m_doppler_period_isSet){
        obj->insert("dopplerPeriod", QJsonValue(doppler_period));
    }
    if(m_prediction_period_isSet){
        obj->insert("predictionPeriod", QJsonValue(prediction_period));
    }
    if(pass_start_time != nullptr && *pass_start_time != QString("")){
        toJsonValue(QString("passStartTime"), pass_start_time, obj, QString("QString"));
    }
    if(pass_finish_time != nullptr && *pass_finish_time != QString("")){
        toJsonValue(QString("passFinishTime"), pass_finish_time, obj, QString("QString"));
    }
    if(m_default_frequency_isSet){
        obj->insert("defaultFrequency", QJsonValue(default_frequency));
    }
    if(m_draw_on_map_isSet){
        obj->insert("drawOnMap", QJsonValue(draw_on_map));
    }
    if(m_auto_target_isSet){
        obj->insert("autoTarget", QJsonValue(auto_target));
    }
    if(aos_speech != nullptr && *aos_speech != QString("")){
        toJsonValue(QString("aosSpeech"), aos_speech, obj, QString("QString"));
    }
    if(los_speech != nullptr && *los_speech != QString("")){
        toJsonValue(QString("losSpeech"), los_speech, obj, QString("QString"));
    }
    if(aos_command != nullptr && *aos_command != QString("")){
        toJsonValue(QString("aosCommand"), aos_command, obj, QString("QString"));
    }
    if(los_command != nullptr && *los_command != QString("")){
        toJsonValue(QString("losCommand"), los_command, obj, QString("QString"));
    }
    if(device_settings && device_settings->size() > 0){
        toJsonArray((QList<void*>*)device_settings, obj, "deviceSettings", "SWGSatelliteDeviceSettingsList");
    }
    if(m_azimuth_offset_isSet){
        obj->insert("azimuthOffset", QJsonValue(azimuth_offset));
    }
    if(m_elevation_offset_isSet){
        obj->insert("elevationOffset", QJsonValue(elevation_offset));
    }
    if(title != nullptr && *title != QString("")){
        toJsonValue(QString("title"), title, obj, QString("QString"));
    }
    if(m_rgb_color_isSet){
        obj->insert("rgbColor", QJsonValue(rgb_color));
    }
    if(m_use_reverse_api_isSet){
        obj->insert("useReverseAPI", QJsonValue(use_reverse_api));
    }
    if(reverse_api_address != nullptr && *reverse_api_address != QString("")){
        toJsonValue(QString("reverseAPIAddress"), reverse_api_address, obj, QString("QString"));
    }
    if(m_reverse_api_port_isSet){
        obj->insert("reverseAPIPort", QJsonValue(reverse_api_port));
    }
    if(m_reverse_api_feature_set_index_isSet){
        obj->insert("reverseAPIFeatureSetIndex", QJsonValue(reverse_api_feature_set_index));
    }
    if(m_reverse_api_feature_index_isSet){
        obj->insert("reverseAPIFeatureIndex", QJsonValue(reverse_api_feature_index));
    }
    if((rollup_state != nullptr) && (rollup_state->isSet())){
        toJsonValue(QString("rollupState"), rollup_state, obj, QString("SWGRollupState"));
    }

    return obj;
}

float
SWGSatelliteTrackerSettings::getLatitude() {
    return latitude;
}
void
SWGSatelliteTrackerSettings::setLatitude(float latitude) {
    this->latitude = latitude;
    this->m_latitude_isSet = true;
}

float
SWGSatelliteTrackerSettings::getLongitude() {
    return longitude;
}
void
SWGSatelliteTrackerSettings::setLongitude(float longitude) {
    this->longitude = longitude;
    this->m_longitude_isSet = true;
}

float
SWGSatelliteTrackerSettings::getHeightAboveSeaLevel() {
    return height_above_sea_level;
}
void
SWGSatelliteTrackerSettings::setHeightAboveSeaLevel(float height_above_sea_level) {
    this->height_above_sea_level = height_above_sea_level;
    this->m_height_above_sea_level_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getTarget() {
    return target;
}
void
SWGSatelliteTrackerSettings::setTarget(QString* target) {
    this->target = target;
    this->m_target_isSet = true;
}

QList<QString*>*
SWGSatelliteTrackerSettings::getSatellites() {
    return satellites;
}
void
SWGSatelliteTrackerSettings::setSatellites(QList<QString*>* satellites) {
    this->satellites = satellites;
    this->m_satellites_isSet = true;
}

QList<QString*>*
SWGSatelliteTrackerSettings::getTles() {
    return tles;
}
void
SWGSatelliteTrackerSettings::setTles(QList<QString*>* tles) {
    this->tles = tles;
    this->m_tles_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getDateTime() {
    return date_time;
}
void
SWGSatelliteTrackerSettings::setDateTime(QString* date_time) {
    this->date_time = date_time;
    this->m_date_time_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getMinAosElevation() {
    return min_aos_elevation;
}
void
SWGSatelliteTrackerSettings::setMinAosElevation(qint32 min_aos_elevation) {
    this->min_aos_elevation = min_aos_elevation;
    this->m_min_aos_elevation_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getMinPassElevation() {
    return min_pass_elevation;
}
void
SWGSatelliteTrackerSettings::setMinPassElevation(qint32 min_pass_elevation) {
    this->min_pass_elevation = min_pass_elevation;
    this->m_min_pass_elevation_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getRotatorMaxAzimuth() {
    return rotator_max_azimuth;
}
void
SWGSatelliteTrackerSettings::setRotatorMaxAzimuth(qint32 rotator_max_azimuth) {
    this->rotator_max_azimuth = rotator_max_azimuth;
    this->m_rotator_max_azimuth_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getRotatorMaxElevation() {
    return rotator_max_elevation;
}
void
SWGSatelliteTrackerSettings::setRotatorMaxElevation(qint32 rotator_max_elevation) {
    this->rotator_max_elevation = rotator_max_elevation;
    this->m_rotator_max_elevation_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getAzElUnits() {
    return az_el_units;
}
void
SWGSatelliteTrackerSettings::setAzElUnits(qint32 az_el_units) {
    this->az_el_units = az_el_units;
    this->m_az_el_units_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getGroundTrackPoints() {
    return ground_track_points;
}
void
SWGSatelliteTrackerSettings::setGroundTrackPoints(qint32 ground_track_points) {
    this->ground_track_points = ground_track_points;
    this->m_ground_track_points_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getDateFormat() {
    return date_format;
}
void
SWGSatelliteTrackerSettings::setDateFormat(QString* date_format) {
    this->date_format = date_format;
    this->m_date_format_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getUtc() {
    return utc;
}
void
SWGSatelliteTrackerSettings::setUtc(qint32 utc) {
    this->utc = utc;
    this->m_utc_isSet = true;
}

float
SWGSatelliteTrackerSettings::getUpdatePeriod() {
    return update_period;
}
void
SWGSatelliteTrackerSettings::setUpdatePeriod(float update_period) {
    this->update_period = update_period;
    this->m_update_period_isSet = true;
}

float
SWGSatelliteTrackerSettings::getDopplerPeriod() {
    return doppler_period;
}
void
SWGSatelliteTrackerSettings::setDopplerPeriod(float doppler_period) {
    this->doppler_period = doppler_period;
    this->m_doppler_period_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getPredictionPeriod() {
    return prediction_period;
}
void
SWGSatelliteTrackerSettings::setPredictionPeriod(qint32 prediction_period) {
    this->prediction_period = prediction_period;
    this->m_prediction_period_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getPassStartTime() {
    return pass_start_time;
}
void
SWGSatelliteTrackerSettings::setPassStartTime(QString* pass_start_time) {
    this->pass_start_time = pass_start_time;
    this->m_pass_start_time_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getPassFinishTime() {
    return pass_finish_time;
}
void
SWGSatelliteTrackerSettings::setPassFinishTime(QString* pass_finish_time) {
    this->pass_finish_time = pass_finish_time;
    this->m_pass_finish_time_isSet = true;
}

float
SWGSatelliteTrackerSettings::getDefaultFrequency() {
    return default_frequency;
}
void
SWGSatelliteTrackerSettings::setDefaultFrequency(float default_frequency) {
    this->default_frequency = default_frequency;
    this->m_default_frequency_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getDrawOnMap() {
    return draw_on_map;
}
void
SWGSatelliteTrackerSettings::setDrawOnMap(qint32 draw_on_map) {
    this->draw_on_map = draw_on_map;
    this->m_draw_on_map_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getAutoTarget() {
    return auto_target;
}
void
SWGSatelliteTrackerSettings::setAutoTarget(qint32 auto_target) {
    this->auto_target = auto_target;
    this->m_auto_target_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getAosSpeech() {
    return aos_speech;
}
void
SWGSatelliteTrackerSettings::setAosSpeech(QString* aos_speech) {
    this->aos_speech = aos_speech;
    this->m_aos_speech_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getLosSpeech() {
    return los_speech;
}
void
SWGSatelliteTrackerSettings::setLosSpeech(QString* los_speech) {
    this->los_speech = los_speech;
    this->m_los_speech_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getAosCommand() {
    return aos_command;
}
void
SWGSatelliteTrackerSettings::setAosCommand(QString* aos_command) {
    this->aos_command = aos_command;
    this->m_aos_command_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getLosCommand() {
    return los_command;
}
void
SWGSatelliteTrackerSettings::setLosCommand(QString* los_command) {
    this->los_command = los_command;
    this->m_los_command_isSet = true;
}

QList<SWGSatelliteDeviceSettingsList*>*
SWGSatelliteTrackerSettings::getDeviceSettings() {
    return device_settings;
}
void
SWGSatelliteTrackerSettings::setDeviceSettings(QList<SWGSatelliteDeviceSettingsList*>* device_settings) {
    this->device_settings = device_settings;
    this->m_device_settings_isSet = true;
}

float
SWGSatelliteTrackerSettings::getAzimuthOffset() {
    return azimuth_offset;
}
void
SWGSatelliteTrackerSettings::setAzimuthOffset(float azimuth_offset) {
    this->azimuth_offset = azimuth_offset;
    this->m_azimuth_offset_isSet = true;
}

float
SWGSatelliteTrackerSettings::getElevationOffset() {
    return elevation_offset;
}
void
SWGSatelliteTrackerSettings::setElevationOffset(float elevation_offset) {
    this->elevation_offset = elevation_offset;
    this->m_elevation_offset_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getTitle() {
    return title;
}
void
SWGSatelliteTrackerSettings::setTitle(QString* title) {
    this->title = title;
    this->m_title_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getRgbColor() {
    return rgb_color;
}
void
SWGSatelliteTrackerSettings::setRgbColor(qint32 rgb_color) {
    this->rgb_color = rgb_color;
    this->m_rgb_color_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getUseReverseApi() {
    return use_reverse_api;
}
void
SWGSatelliteTrackerSettings::setUseReverseApi(qint32 use_reverse_api) {
    this->use_reverse_api = use_reverse_api;
    this->m_use_reverse_api_isSet = true;
}

QString*
SWGSatelliteTrackerSettings::getReverseApiAddress() {
    return reverse_api_address;
}
void
SWGSatelliteTrackerSettings::setReverseApiAddress(QString* reverse_api_address) {
    this->reverse_api_address = reverse_api_address;
    this->m_reverse_api_address_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getReverseApiPort() {
    return reverse_api_port;
}
void
SWGSatelliteTrackerSettings::setReverseApiPort(qint32 reverse_api_port) {
    this->reverse_api_port = reverse_api_port;
    this->m_reverse_api_port_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getReverseApiFeatureSetIndex() {
    return reverse_api_feature_set_index;
}
void
SWGSatelliteTrackerSettings::setReverseApiFeatureSetIndex(qint32 reverse_api_feature_set_index) {
    this->reverse_api_feature_set_index = reverse_api_feature_set_index;
    this->m_reverse_api_feature_set_index_isSet = true;
}

qint32
SWGSatelliteTrackerSettings::getReverseApiFeatureIndex() {
    return reverse_api_feature_index;
}
void
SWGSatelliteTrackerSettings::setReverseApiFeatureIndex(qint32 reverse_api_feature_index) {
    this->reverse_api_feature_index = reverse_api_feature_index;
    this->m_reverse_api_feature_index_isSet = true;
}

SWGRollupState*
SWGSatelliteTrackerSettings::getRollupState() {
    return rollup_state;
}
void
SWGSatelliteTrackerSettings::setRollupState(SWGRollupState* rollup_state) {
    this->rollup_state = rollup_state;
    this->m_rollup_state_isSet = true;
}


bool
SWGSatelliteTrackerSettings::isSet(){
    bool isObjectUpdated = false;
    do{
        if(m_latitude_isSet){
            isObjectUpdated = true; break;
        }
        if(m_longitude_isSet){
            isObjectUpdated = true; break;
        }
        if(m_height_above_sea_level_isSet){
            isObjectUpdated = true; break;
        }
        if(target && *target != QString("")){
            isObjectUpdated = true; break;
        }
        if(satellites && (satellites->size() > 0)){
            isObjectUpdated = true; break;
        }
        if(tles && (tles->size() > 0)){
            isObjectUpdated = true; break;
        }
        if(date_time && *date_time != QString("")){
            isObjectUpdated = true; break;
        }
        if(m_min_aos_elevation_isSet){
            isObjectUpdated = true; break;
        }
        if(m_min_pass_elevation_isSet){
            isObjectUpdated = true; break;
        }
        if(m_rotator_max_azimuth_isSet){
            isObjectUpdated = true; break;
        }
        if(m_rotator_max_elevation_isSet){
            isObjectUpdated = true; break;
        }
        if(m_az_el_units_isSet){
            isObjectUpdated = true; break;
        }
        if(m_ground_track_points_isSet){
            isObjectUpdated = true; break;
        }
        if(date_format && *date_format != QString("")){
            isObjectUpdated = true; break;
        }
        if(m_utc_isSet){
            isObjectUpdated = true; break;
        }
        if(m_update_period_isSet){
            isObjectUpdated = true; break;
        }
        if(m_doppler_period_isSet){
            isObjectUpdated = true; break;
        }
        if(m_prediction_period_isSet){
            isObjectUpdated = true; break;
        }
        if(pass_start_time && *pass_start_time != QString("")){
            isObjectUpdated = true; break;
        }
        if(pass_finish_time && *pass_finish_time != QString("")){
            isObjectUpdated = true; break;
        }
        if(m_default_frequency_isSet){
            isObjectUpdated = true; break;
        }
        if(m_draw_on_map_isSet){
            isObjectUpdated = true; break;
        }
        if(m_auto_target_isSet){
            isObjectUpdated = true; break;
        }
        if(aos_speech && *aos_speech != QString("")){
            isObjectUpdated = true; break;
        }
        if(los_speech && *los_speech != QString("")){
            isObjectUpdated = true; break;
        }
        if(aos_command && *aos_command != QString("")){
            isObjectUpdated = true; break;
        }
        if(los_command && *los_command != QString("")){
            isObjectUpdated = true; break;
        }
        if(device_settings && (device_settings->size() > 0)){
            isObjectUpdated = true; break;
        }
        if(m_azimuth_offset_isSet){
            isObjectUpdated = true; break;
        }
        if(m_elevation_offset_isSet){
            isObjectUpdated = true; break;
        }
        if(title && *title != QString("")){
            isObjectUpdated = true; break;
        }
        if(m_rgb_color_isSet){
            isObjectUpdated = true; break;
        }
        if(m_use_reverse_api_isSet){
            isObjectUpdated = true; break;
        }
        if(reverse_api_address && *reverse_api_address != QString("")){
            isObjectUpdated = true; break;
        }
        if(m_reverse_api_port_isSet){
            isObjectUpdated = true; break;
        }
        if(m_reverse_api_feature_set_index_isSet){
            isObjectUpdated = true; break;
        }
        if(m_reverse_api_feature_index_isSet){
            isObjectUpdated = true; break;
        }
        if(rollup_state && rollup_state->isSet()){
            isObjectUpdated = true; break;
        }
    }while(false);
    return isObjectUpdated;
}
}

