#include "instrumentconfigmanager.h"

#include <algorithm>

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QDebug>

InstrumentConfigManager& InstrumentConfigManager::getInstance()
{
    static InstrumentConfigManager instance;
    return instance;
}

bool InstrumentConfigManager::initialize()
{
    if (m_initialized) {
        return m_hasConfig;
    }

    m_configPath = QCoreApplication::applicationDirPath() + "/instrumentConfig/instruments.json";
    m_initialized = true;
    return loadFromFile(m_configPath);
}

bool InstrumentConfigManager::reload()
{
    if (m_configPath.isEmpty()) {
        m_configPath = QCoreApplication::applicationDirPath() + "/instrumentConfig/instruments.json";
    }

    m_initialized = true;
    return loadFromFile(m_configPath);
}

bool InstrumentConfigManager::loadFromFile(const QString& filePath)
{
    m_entries.clear();
    m_lastError.clear();
    m_hasConfig = false;

    QFile file(filePath);

    if (!file.exists()) {
        m_lastError = QString("Instrument config not found: %1").arg(filePath);
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open instrument config: %1").arg(filePath);
        qWarning() << "[InstrumentConfigManager]" << m_lastError;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("Invalid JSON in %1: %2").arg(filePath).arg(parseError.errorString());
        qWarning() << "[InstrumentConfigManager]" << m_lastError;
        return false;
    }

    if (!doc.isObject()) {
        m_lastError = QString("Expected JSON object root in %1").arg(filePath);
        qWarning() << "[InstrumentConfigManager]" << m_lastError;
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonValue cfgVal = root.value("workspaceConfiguration");

    if (!cfgVal.isObject()) {
        m_lastError = QString("Missing or invalid 'workspaceConfiguration' object in %1").arg(filePath);
        qWarning() << "[InstrumentConfigManager]" << m_lastError;
        return false;
    }

    const QJsonObject cfgObj = cfgVal.toObject();

    // Iterate over "rx", "tx", etc. keys -- each value is an array of device entries
    for (QJsonObject::const_iterator itGroup = cfgObj.constBegin(); itGroup != cfgObj.constEnd(); ++itGroup)
    {
        const QString rxTx = itGroup.key().toLower().trimmed();

        if (!itGroup.value().isArray()) {
            qWarning() << "[InstrumentConfigManager] Skipping non-array group key:" << rxTx;
            continue;
        }

        const QJsonArray array = itGroup.value().toArray();

        for (int i = 0; i < array.size(); ++i)
        {
            if (!array.at(i).isObject()) {
                continue;
            }

            const QJsonObject obj = array.at(i).toObject();

            InstrumentConfigEntry entry;
            entry.position   = obj.value("position").toInt(-1);
            entry.deviceName = obj.value("deviceName").toString().trimmed();
            entry.id         = obj.value("id").toString().trimmed();
            entry.rxTx       = rxTx;
            entry.port       = obj.value("port").toInt(-1);

            if ((entry.position < 0) || entry.deviceName.isEmpty() || entry.rxTx.isEmpty() || (entry.port < 0)) {
                qWarning() << "[InstrumentConfigManager] Skipping incomplete entry at index" << i << "in group" << rxTx;
                continue;
            }

            m_entries.append(entry);
        }
    }

    std::sort(m_entries.begin(), m_entries.end(), [](const InstrumentConfigEntry& a, const InstrumentConfigEntry& b) {
        return a.position < b.position;
    });

    m_hasConfig = !m_entries.isEmpty();

    if (!m_hasConfig) {
        m_lastError = QString("No valid entries found in %1").arg(filePath);
    }

    return m_hasConfig;
}

bool InstrumentConfigManager::parseDetectedDevice(
    const QString& text,
    QString& deviceName,
    int& position,
    int& port,
    QString& id)
{
    static const QRegularExpression re("^\\s*([^\\[]+)\\[(\\d+):(\\d+)\\]\\s*(.*)$");
    const QRegularExpressionMatch match = re.match(text);

    if (!match.hasMatch()) {
        return false;
    }

    deviceName = match.captured(1).trimmed();
    position = match.captured(2).toInt();
    port = match.captured(3).toInt();
    id = match.captured(4).trimmed();
    return true;
}

QMap<int, int> InstrumentConfigManager::resolveOrderedDeviceKeys(
    const QMap<int, QString>& detectedDeviceMap,
    const QString& rxTx) const
{
    QMap<int, int> positionToDeviceKey;

    if (!m_hasConfig || rxTx.isEmpty()) {
        return positionToDeviceKey;
    }

    QSet<int> usedKeys;

    for (QList<InstrumentConfigEntry>::const_iterator itCfg = m_entries.cbegin(); itCfg != m_entries.cend(); ++itCfg)
    {
        const InstrumentConfigEntry& entry = *itCfg;  // Dereference the iterator
        
        if (entry.rxTx.compare(rxTx, Qt::CaseInsensitive) != 0) {
            continue;
        }

        bool matched = false;

        for (QMap<int, QString>::const_iterator itDev = detectedDeviceMap.cbegin(); itDev != detectedDeviceMap.cend(); ++itDev)
        {
            if (usedKeys.contains(itDev.key())) {
                continue;
            }

            QString detectedName;
            QString detectedId;
            int detectedPosition = -1;  // This is the OS-detected sequence, we ignore it for matching
            int detectedPort = -1;

            if (!parseDetectedDevice(itDev.value(), detectedName, detectedPosition, detectedPort, detectedId)) {
                continue;
            }

            // Debug output
            qDebug() << "[DEBUG] Checking device - Key:" << itDev.key();
            qDebug() << "  detectedPort:" << detectedPort << "vs entry.port:" << entry.port;
            qDebug() << "  detectedName:" << detectedName << "vs entry.deviceName:" << entry.deviceName;
            qDebug() << "  detectedId:" << detectedId;
            qDebug() << "  entry.id:" << entry.id;
            
            bool portMatch = (detectedPort == entry.port);
            bool nameMatch = (detectedName.compare(entry.deviceName, Qt::CaseInsensitive) == 0);
            bool idMatch = (entry.id.isEmpty() || (detectedId.compare(entry.id, Qt::CaseInsensitive) == 0));
            
            qDebug() << "  portMatch:" << portMatch << "nameMatch:" << nameMatch << "idMatch:" << idMatch;
            
            // Match ONLY on: deviceName, port, and ID
            if (portMatch && nameMatch && idMatch)
            {
                qDebug() << "[DEBUG] MATCH FOUND! Inserting key:" << itDev.key() << "at position:" << entry.position;
                positionToDeviceKey.insert(entry.position, itDev.key());
                usedKeys.insert(itDev.key());
                matched = true;
                break;
            }
        }

        if (!matched) {
            qWarning() << "[InstrumentConfigManager] No device found for config entry:"
                       << entry.deviceName << "port:" << entry.port << "id:" << entry.id;
            return QMap<int, int>(); // Return empty -- all-or-nothing
        }
    }

    return positionToDeviceKey;
}