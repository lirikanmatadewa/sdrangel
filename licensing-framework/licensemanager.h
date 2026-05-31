#pragma once

#include "license.hpp"
#include <map>
#include <memory>
#include <QString>

struct LicenseEntry
{
    QString path;
    std::unique_ptr<License> license;
};

class LicenseManager
{
public:
    static constexpr const char *DefaultLicense = "Default";

    // Get the global instance (singleton pattern)
    static LicenseManager &getInstance();

    // Initialize the licensing system (call this early in main())
    void initialize();

    // Add a named license entry
    void addLicense(const QString &name, const QString &licensePath);

    // Access license objects by name (defaults to "Default")
    License &getLicense(const QString &name = DefaultLicense);

    // Convenience methods (operate on the named license, default = "Default")
    bool isLicenseValid(const QString &name = DefaultLicense);
    License::Type getLicenseType(const QString &name = DefaultLicense);
    QString getLicenseTypeString(const QString &name = DefaultLicense);

    // Get all registered license names
    QStringList licenseNames() const;

    // Prevent copying
    LicenseManager(const LicenseManager &) = delete;
    LicenseManager &operator=(const LicenseManager &) = delete;

private:
    LicenseManager() = default;
    ~LicenseManager() = default;

    std::map<QString, LicenseEntry> m_licenses;
    bool m_initialized = false;
};

// Convenience macros for easy access anywhere in the code
#define LICENSE_MANAGER LicenseManager::getInstance()
#define LICENSE LICENSE_MANAGER.getLicense()