#ifndef INCLUDE_INSTRUMENTCONFIGMANAGER_H
#define INCLUDE_INSTRUMENTCONFIGMANAGER_H

#include <QList>
#include <QMap>
#include <QString>

#include "export.h"

struct SDRBASE_API InstrumentConfigEntry
{
    int position;
    QString deviceName;
    QString id;
    QString rxTx;
    int port;
};

class SDRBASE_API InstrumentConfigManager
{
public:
    static InstrumentConfigManager& getInstance();

    bool initialize();
    bool reload();
    bool hasConfig() const { return m_hasConfig; }
    QString getConfigPath() const { return m_configPath; }
    QString getLastError() const { return m_lastError; }

    QMap<int, QList<int>> resolveOrderedDeviceKeys(
        const QMap<int, QString>& detectedDeviceMap,
        const QString& rxTx) const;

    InstrumentConfigManager(const InstrumentConfigManager&) = delete;
    InstrumentConfigManager& operator=(const InstrumentConfigManager&) = delete;

private:
    InstrumentConfigManager() = default;
    ~InstrumentConfigManager() = default;

    bool loadFromFile(const QString& filePath);
    static bool parseDetectedDevice(
        const QString& text,
        QString& deviceName,
        int& position,
        int& port,
        QString& id);

    bool m_initialized = false;
    bool m_hasConfig = false;
    QString m_configPath;
    QString m_lastError;
    QList<InstrumentConfigEntry> m_entries;
};

#define INSTRUMENT_CONFIG_MANAGER InstrumentConfigManager::getInstance()

#endif // INCLUDE_INSTRUMENTCONFIGMANAGER_H