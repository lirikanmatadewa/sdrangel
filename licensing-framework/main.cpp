#include <iostream>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLoggingCategory>

#include "license.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Qt CLI Licensing Tool");
    QCoreApplication::setApplicationVersion("1.0");
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false"));

    QCommandLineParser parser;
    parser.setApplicationDescription("CLI-only Qt licensing tool");
    parser.addHelpOption();
    parser.addVersionOption();
    // Add your CLI options here

    parser.process(app);

    QString cpuId = License::getCpuId();
    QString computerName = License::getComputerName();
    QStringList ntpServers = QStringList();
    ntpServers << "pool.ntp.org";
    ntpServers << "time.windows.com";
    ntpServers << "time.apple.com";
    QDateTime ntpUtcTime = License::getNtpUtcTime(ntpServers);

    std::cout << "CPU ID: " << cpuId.toStdString() << std::endl;
    std::cout << "Computer Name: " << computerName.toStdString() << std::endl;
    std::cout << "NTP UTC Time: " << ntpUtcTime.toString(Qt::ISODate).toStdString() << std::endl;

    // Test License()
#ifdef _WIN32
    License licenseDemo("Qt CLI Licensing Tool", ".\\lic\\id.lic", ".\\lic\\license-demo.lic");
    License licensePermanent("Qt CLI Licensing Tool", ".\\lic\\id.lic", ".\\lic\\license-permanent.lic");
    License licenseDemoOption("Qt CLI Licensing Tool", ".\\lic\\id.lic", ".\\lic\\k0-license-demo.lic", "K0");
    License licensePermanentOption("Qt CLI Licensing Tool", ".\\lic\\id.lic", ".\\lic\\k0-license-permanent.lic", "K0");
#else
    License licenseDemo("Qt CLI Licensing Tool", "lic/id2.lic", "lic/license-demo2.lic");
    License licensePermanent("Qt CLI Licensing Tool", "lic/id2.lic", "lic/license-permanent2.lic");
    License licenseDemoOption("Qt CLI Licensing Tool", "lic/id2.lic", "lic/license-demo2.lic", "K0");
    License licensePermanentOption("Qt CLI Licensing Tool", "lic/id2.lic", "lic/license-permanent2.lic", "K0");
#endif

    std::cout << "licenseDemo validity: " << (licenseDemo.checkValidity() ? "true" : "false") << std::endl;
    std::cout << "licenseDemo type: " << licenseDemo.getLicenseTypeString().toStdString() << std::endl;
    std::cout << "licensePermanent validity: " << (licensePermanent.checkValidity() ? "true" : "false") << std::endl;
    std::cout << "licensePermanent type: " << licensePermanent.getLicenseTypeString().toStdString() << std::endl;
    std::cout << "licenseDemoOption validity: " << (licenseDemoOption.checkValidity() ? "true" : "false") << std::endl;
    std::cout << "licenseDemoOption type: " << licenseDemoOption.getLicenseTypeString().toStdString() << std::endl;
    std::cout << "licensePermanentOption validity: " << (licensePermanentOption.checkValidity() ? "true" : "false") << std::endl;
    std::cout << "licensePermanentOption type: " << licensePermanentOption.getLicenseTypeString().toStdString() << std::endl;

    // Your CLI logic here
    std::cout
        << "Qt CLI tool running!" << std::endl;

    return 0;
}