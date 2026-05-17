#pragma once

// Example usage of the global licensing system

#include "licensing-framework/licensemanager.h"

/* 
 * USAGE EXAMPLES:
 * 
 * The licensing system is now globally accessible throughout your application.
 * After calling LicenseManager::getInstance().initialize() in main(), you can
 * access licensing functionality anywhere in your code.
 */

class ExampleUsage 
{
public:
    void checkLicenseStatus() 
    {
        // Method 1: Using the convenience macros (easiest)
        if (LICENSE_MANAGER.isLicenseValid()) {
            qDebug() << "License is valid";
        } else {
            qDebug() << "License is invalid";
        }
        
        // Get license type as string
        QString licenseType = LICENSE_MANAGER.getLicenseTypeString();
        qDebug() << "License type:" << licenseType;
        
        // Method 2: Direct access to License object
        License::Type type = LICENSE.getLicenseType();
        switch (type) {
            case License::Type::Permanent:
                qDebug() << "Permanent license - all features enabled";
                break;
            case License::Type::Demo:
                qDebug() << "Demo license - some features may be restricted";
                break;
            case License::Type::Unlicensed:
                qDebug() << "No license - running in trial mode";
                break;
            case License::Type::Expired:
                qDebug() << "License expired - features restricted";
                break;
            default:
                qDebug() << "Unknown license state";
        }
        
        // Method 3: Full control via LicenseManager instance
        LicenseManager& manager = LicenseManager::getInstance();
        bool valid = manager.isLicenseValid();
        qDebug() << "License validation result:" << valid;
    }
    
    void restrictFeatureBasedOnLicense() 
    {
        // Example: Enable/disable features based on license
        if (LICENSE_MANAGER.getLicenseType() == License::Type::Demo) {
            qDebug() << "Demo mode: Advanced features disabled";
            // Disable advanced features
        } else if (LICENSE_MANAGER.isLicenseValid()) {
            qDebug() << "Full license: All features enabled";
            // Enable all features
        } else {
            qDebug() << "Invalid license: Basic features only";
            // Restrict to basic features
        }
    }
    
    void periodicLicenseCheck() 
    {
        // You can periodically re-check the license validity
        // This is useful for time-limited licenses
        bool currentStatus = LICENSE.checkValidity();
        if (!currentStatus) {
            qWarning() << "License validation failed during runtime check!";
            // Take appropriate action (show dialog, restrict features, etc.)
        }
    }
};

/*
 * INTEGRATION TIPS:
 * 
 * 1. In any .cpp file where you want to check licensing:
 *    #include "licensing-framework/licensemanager.h"
 * 
 * 2. Use the convenience macros for quick access:
 *    LICENSE_MANAGER  - Access to LicenseManager instance
 *    LICENSE          - Direct access to License object
 * 
 * 3. Common patterns:
 *    - Check LICENSE_MANAGER.isLicenseValid() before enabling premium features
 *    - Use LICENSE_MANAGER.getLicenseType() to determine feature availability
 *    - Call LICENSE.checkValidity() periodically for time-based licenses
 * 
 * 4. The system automatically:
 *    - Creates license directories and ID files
 *    - Validates licenses at startup
 *    - Provides detailed logging of license status
 * 
 * 5. License files will be created in:
 *    - Windows: [Application Directory]/lic/
 *    - Contains: id.lic and license.lic files
 */