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

QList<int> InstrumentConfigManager::resolveOrderedDeviceKeys(
    const QMap<int, QString>& detectedDeviceMap,
    const QString& rxTx) const
{
    QList<int> orderedKeys;

    if (!m_hasConfig || rxTx.isEmpty()) {
        return orderedKeys;
    }

    int expectedCount = 0;
    QSet<int> usedKeys;

    for (QList<InstrumentConfigEntry>::const_iterator itCfg = m_entries.cbegin(); itCfg != m_entries.cend(); ++itCfg)
    {
        if (itCfg->rxTx.compare(rxTx, Qt::CaseInsensitive) != 0) {
            continue;
        }

        expectedCount++;
        bool matched = false;

        for (QMap<int, QString>::const_iterator itDev = detectedDeviceMap.cbegin(); itDev != detectedDeviceMap.cend(); ++itDev)
        {
            if (usedKeys.contains(itDev.key())) {
                continue;
            }

            QString detectedName;
            QString detectedId;
            int detectedPosition = -1;
            int detectedPort = -1;

            if (!parseDetectedDevice(itDev.value(), detectedName, detectedPosition, detectedPort, detectedId)) {
                continue;
            }

            if ((detectedPosition == itCfg->position)
                && (detectedPort == itCfg->port)
                && (detectedName.compare(itCfg->deviceName, Qt::CaseInsensitive) == 0)
                && (itCfg->id.isEmpty() || (detectedId.compare(itCfg->id, Qt::CaseInsensitive) == 0)))
            {
                orderedKeys.append(itDev.key());
                usedKeys.insert(itDev.key());
                matched = true;
                break;
            }
        }

        if (!matched) {
            qWarning() << "[InstrumentConfigManager] No device found for config entry:"
                       << itCfg->deviceName << "pos:" << itCfg->position << "port:" << itCfg->port;
            return QList<int>(); // Return empty -- all-or-nothing
        }
    }

    // Sanity check: should always be true if the loop above ran correctly
    if (orderedKeys.size() != expectedCount) {
        return QList<int>();
    }

    return orderedKeys;
}