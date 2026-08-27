#include "licensemanager.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

LicenseManager &LicenseManager::getInstance()
{
    static LicenseManager instance;
    return instance;
}

void LicenseManager::initialize()
{
    if (m_initialized)
    {
        qDebug() << "[LicenseManager] Already initialized";
        return;
    }

    qDebug() << "[LicenseManager] Initializing licensing system";

    // Register the default license
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultPath = appDir + "/lic/license.lic";
    addLicense(DefaultLicense, defaultPath);

    m_initialized = true;
}

void LicenseManager::addLicense(const QString &name, const QString &licensePath)
{
    QString appDir = QCoreApplication::applicationDirPath();
    //QString appName = QCoreApplication::applicationName();
    //QString appName = "Navix PD100";
    QString appName = "ES300";
    QString idPath = appDir + "/lic/id.lic";

    qDebug() << "[LicenseManager] Adding license" << name;
    qDebug() << "  ID Path:" << idPath;
    qDebug() << "  License Path:" << licensePath;

    LicenseEntry entry;
    entry.path = licensePath;
    entry.license = std::make_unique<License>(appName, idPath, licensePath, "");

    bool isValid = entry.license->checkValidity();
    License::Type type = entry.license->getLicenseType();
    QString typeStr = entry.license->getLicenseTypeString();

    qDebug() << "[LicenseManager] License" << name << "Status:";
    qDebug() << "  Valid:" << isValid;
    qDebug() << "  Type:" << typeStr;

    if (type == License::Type::Unlicensed || type == License::Type::Expired)
    {
        qWarning() << "[LicenseManager]" << name << "is not licensed or has expired!";
    }

    m_licenses.emplace(name, std::move(entry));
}

License &LicenseManager::getLicense(const QString &name)
{
    if (!m_initialized)
    {
        qWarning() << "[LicenseManager] Not initialized! Call initialize() first.";
        initialize();
    }

    auto it = m_licenses.find(name);
    if (it == m_licenses.end())
    {
        qWarning() << "[LicenseManager] License not found:" << name << "- falling back to Default";
        it = m_licenses.find(DefaultLicense);
    }

    return *it->second.license;
}

bool LicenseManager::isLicenseValid(const QString &name)
{
    return getLicense(name).checkValidity();
}

License::Type LicenseManager::getLicenseType(const QString &name)
{
    return getLicense(name).getLicenseType();
}

QString LicenseManager::getLicenseTypeString(const QString &name)
{
    return getLicense(name).getLicenseTypeString();
}

QStringList LicenseManager::licenseNames() const
{
    QStringList names;
    for (const auto &pair : m_licenses)
    {
        names.append(pair.first);
    }
    return names;
}