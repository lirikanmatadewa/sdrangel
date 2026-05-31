#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QStringList>

class License
{
public:
    enum class Type
    {
        Undefined,
        Unlicensed,
        Permanent,
        Demo,
        Expired
    };

#ifdef _WIN32
    License(const QString &applicationName, const QString &idPath = ".\\lic\\id.lic", const QString &licensePath = ".\\lic\\license.lic", const QString &option = "");
#else
    License(const QString &applicationName, const QString &idPath = "./lic/id.lic", const QString &licensePath = "./lic/license.lic", const QString &option = "");
#endif

    void createFile(const QString &str);
    bool checkValidity();
    Type getLicenseType() const { return licenseType; };
    QString getLicenseTypeString();

    static QString getCpuId();
    static QString getMacAddress();
    static QString getComputerName();
    static QDateTime getUtcTimeViaHttp();
    static QDateTime getNtpUtcTime(const QStringList &ntpServers, int maxAttempt = 1);

private:
    QString applicationName;
    QString idPath;
    QString licensePath;
    QString option;
    Type licenseType;
    QDateTime expiryUtc;
    QString generateIdJSON();
    QString cpuId() const { return License::getCpuId(); };
    QString computerName() const { return License::getComputerName(); };
    QDateTime ntpUtcTime() const { return License::getUtcTimeViaHttp(); };
    QByteArray getDecryptedLicensePlain() const;
    QString encryptJsonHybrid(const QString &jsonString);
    static QByteArray base64Decode(const QString &b64);
    static QByteArray aesDecrypt(const QByteArray &enc, const unsigned char *key, const unsigned char *iv);
    static bool verifySignature(const QByteArray &data, const QByteArray &signature, const char *publicKeyPem);
};
