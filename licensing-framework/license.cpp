#ifdef _WIN32
#include <intrin.h>
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QTcpSocket>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include "license.hpp"
#include "public_key.h"
#include "aes_key.h"

/**
 * @brief Returns a string representation of the current license type.
 *
 * This method converts the internal license type enumeration to a human-readable string.
 * Possible return values include "Permanent", "Demo", "Unlicensed", "Expired", or "Unknown"
 * if the license type does not match any known value.
 *
 * @return QString Human-readable string describing the license type.
 */
QString License::getLicenseTypeString()
{
    if (this->licenseType == Type::Undefined)
    {
        this->checkValidity();
    }
    switch (this->licenseType)
    {
    case Type::Permanent:
        return this->option.isEmpty() ? "Permanent" : QString("Permanent (%1)").arg(this->option);
    case Type::Demo:
        return this->option.isEmpty() ? "Demo" : QString("Demo (%1): %2").arg(this->option).arg(this->expiryUtc.toString());
    case Type::Unlicensed:
        return "Unlicensed";
    case Type::Expired:
        return this->option.isEmpty() ? "Expired" : QString("Expired (%1): %2").arg(this->option).arg(this->expiryUtc.toString());
    default:
        return "Unknown";
    }
}

/**
 * @brief Retrieves a unique identifier for the CPU.
 * @return QString containing the CPU ID or serial number, or empty if unavailable.
 */
QString License::getCpuId()
{
#ifdef _WIN32
    int cpuInfo[4] = {0};
    __cpuid(cpuInfo, 0);
    char id[32];
    snprintf(id, sizeof(id), "%08X%08X%08X%08X", cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
    return QString::fromLatin1(id);
#else
    // Try lscpu or /proc/cpuinfo for "Serial"
    QProcess proc;
    proc.start("lscpu");
    proc.waitForFinished();
    QString output = proc.readAllStandardOutput();
    foreach (const QString &line, output.split('\n'))
    {
        if (line.startsWith("Serial"))
        {
            return line.section(':', 1).trimmed();
        }
    }
    QFile cpuinfo("/proc/cpuinfo");
    if (cpuinfo.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!cpuinfo.atEnd())
        {
            QString line = cpuinfo.readLine();
            if (line.startsWith("Serial"))
            {
                return line.section(':', 1).trimmed();
            }
        }
    }
    // Fallback: use /etc/machine-id if available
    QFile machineIdFile("/etc/machine-id");
    if (machineIdFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString machineId = QString::fromUtf8(machineIdFile.readAll()).trimmed();
        if (!machineId.isEmpty())
            return machineId;
    }
    return QString();
#endif
}

/**
 * @brief Gets the computer's host name.
 * @return QString containing the computer name, or empty if unavailable.
 */
QString License::getComputerName()
{
#ifdef _WIN32
    wchar_t name[256];
    DWORD size = 256;
    if (GetComputerNameW(name, &size))
    {
        return QString::fromWCharArray(name);
    }
    return QString();
#else
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        return QString::fromLocal8Bit(hostname);
    }
    return QString();
#endif
}

/**
 * @brief Retrieves the current UTC time from a public HTTP API (worldtimeapi.org).
 *        This is a fallback if NTP is blocked.
 * @return QDateTime containing the UTC time if successful, or invalid if failed.
 */
QDateTime License::getUtcTimeViaHttp()
{
    qDebug() << "[HTTP UTC] Requesting UTC time from worldtimeapi.org...";
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl("http://worldtimeapi.org/api/timezone/Etc/UTC")));
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray response = reply->readAll();
        qDebug() << "[HTTP UTC] Response received:" << response;
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject())
        {
            QJsonObject obj = doc.object();
            QString utcStr = obj.value("utc_datetime").toString();
            qDebug() << "[HTTP UTC] utc_datetime field:" << utcStr;
            QDateTime dt = QDateTime::fromString(utcStr, Qt::ISODate);
            dt.setTimeSpec(Qt::UTC);
            qDebug() << "[HTTP UTC] Parsed QDateTime:" << dt;
            if (dt.isValid())
                return dt;
        }
        else
        {
            qDebug() << "[HTTP UTC] JSON parse error.";
        }
    }
    else
    {
        qDebug() << "[HTTP UTC] Network error:" << reply->errorString();
    }
    reply->deleteLater();

    // Fallback: Try to get UTC from HTTP Date header of a common website
    qDebug() << "[HTTP UTC] Trying fallback: HTTP Date header from google.com...";
    QNetworkRequest req(QUrl("http://www.google.com"));
    QNetworkReply *dateReply = manager.head(req);
    QObject::connect(dateReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (dateReply->error() == QNetworkReply::NoError)
    {
        QByteArray dateRaw = dateReply->rawHeader("Date");
        qDebug() << "[HTTP UTC] Date header (raw):" << dateRaw;
        if (!dateRaw.isEmpty())
        {
            // Example: "Sat, 16 Aug 2025 12:34:56 GMT"
            QString dateStr = QString::fromLatin1(dateRaw);
            QDateTime dt = QDateTime::fromString(dateStr, "ddd, dd MMM yyyy HH:mm:ss 'GMT'");
            dt.setTimeSpec(Qt::UTC);
            qDebug() << "[HTTP UTC] Parsed QDateTime from Date header:" << dt;
            if (dt.isValid())
            {
                dateReply->deleteLater();
                return dt;
            }
        }
    }
    else
    {
        qDebug() << "[HTTP UTC] Fallback network error:" << dateReply->errorString();
    }
    dateReply->deleteLater();
    qDebug() << "[HTTP UTC] All HTTP methods failed.";
    return QDateTime();
}

/**
 * @brief Retrieves the current UTC time from an NTP server.
 *
 * This method attempts to connect to one or more NTP servers and fetch the current UTC time.
 * It sends an NTP request to each server and parses the response to extract the timestamp.
 * If all attempts fail, it retries with "pool.ntp.org" as a fallback server.
 *
 * @param ntpServers A list of NTP server hostnames to query. If empty, "pool.ntp.org" is used by default.
 * @param maxAttempt The maximum number of connection attempts per server (default is 3).
 * @return QDateTime The UTC time received from the NTP server, or an invalid QDateTime if all attempts fail.
 */
QDateTime License::getNtpUtcTime(const QStringList &ntpServers, int maxAttempt)
{
    QStringList servers = ntpServers;
    if (servers.isEmpty())
        servers << "pool.ntp.org";

    for (const QString &ntpServer : servers)
    {
        for (int attempt = 0; attempt < maxAttempt; ++attempt)
        {
            QTcpSocket socket;
            socket.connectToHost(ntpServer, 123);
            if (!socket.waitForConnected(3000))
                continue;

            QByteArray request(48, 0);
            request[0] = 0x1B;
            socket.write(request);
            if (!socket.waitForBytesWritten(1000))
            {
                socket.disconnectFromHost();
                continue;
            }
            if (!socket.waitForReadyRead(3000))
            {
                socket.disconnectFromHost();
                continue;
            }
            QByteArray response = socket.read(48);
            socket.disconnectFromHost();
            if (response.size() < 48)
                continue;

            quint32 secondsSince1900 = 0;
            QDataStream ds(response.mid(40, 4));
            ds.setByteOrder(QDataStream::BigEndian);
            ds >> secondsSince1900;
            quint32 secondsSince1970 = secondsSince1900 - 2208988800U;
            return QDateTime::fromSecsSinceEpoch(secondsSince1970, Qt::UTC);
        }
    }
    // All attempts failed, try HTTP fallback
    QDateTime httpTime = getUtcTimeViaHttp();
    return httpTime;
}

/**
 * @brief Generates a JSON representation of the license identification data.
 *
 * This function creates a JSON object containing the CPU ID, computer name,
 * and the first run UTC time of the license. The resulting JSON is serialized
 * and returned as a QString.
 *
 * @return QString A JSON string containing the license identification information.
 */
QString License::generateIdJSON()
{
    QJsonObject json;
    json["applicationName"] = this->applicationName;
    json["cpuId"] = this->cpuId();
    json["computerName"] = this->computerName();
    json["firstRun"] = this->ntpUtcTime().toString(Qt::ISODate);

    QJsonDocument doc(json);
    return doc.toJson();
}

/**
 * @brief Encrypts a JSON string using hybrid AES+RSA and outputs as base64 JSON.
 * @param jsonString The JSON string to encrypt.
 * @return QString containing a JSON object with base64-encoded encrypted key and data.
 */
QString License::encryptJsonHybrid(const QString &jsonString)
{
    // 1. Generate random AES key and IV
    QByteArray aesKey(32, 0); // AES-256
    QByteArray aesIv(16, 0);  // AES block size
    RAND_bytes(reinterpret_cast<unsigned char *>(aesKey.data()), aesKey.size());
    RAND_bytes(reinterpret_cast<unsigned char *>(aesIv.data()), aesIv.size());

    // 2. Encrypt the JSON string with AES-256-CBC
    QByteArray plainData = jsonString.toUtf8();
    QByteArray cipherData;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int outlen1 = plainData.size() + 16;
    cipherData.resize(outlen1);
    int len = 0;
    if (!EVP_EncryptInit_ex(
            ctx, EVP_aes_256_cbc(),
            nullptr,
            reinterpret_cast<const unsigned char *>(aesKey.constData()),
            reinterpret_cast<const unsigned char *>(aesIv.constData())))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }
    if (!EVP_EncryptUpdate(
            ctx,
            reinterpret_cast<unsigned char *>(cipherData.data()),
            &len,
            reinterpret_cast<const unsigned char *>(plainData.constData()),
            plainData.size()))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }
    int totalLen = len;
    if (!EVP_EncryptFinal_ex(
            ctx,
            reinterpret_cast<unsigned char *>(cipherData.data()) + len,
            &len))
    {
        EVP_CIPHER_CTX_free(ctx);
        return QString();
    }
    totalLen += len;
    cipherData.resize(totalLen);
    EVP_CIPHER_CTX_free(ctx);

    // 3. Encrypt the AES key+IV with RSA public key
    QByteArray keyIv = aesKey + aesIv;
    BIO *bio = BIO_new_mem_buf(PUBLIC_KEY_PEM, -1);
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa)
        return QString();
    QByteArray encryptedKeyIv(RSA_size(rsa), 0);
    int encLen = RSA_public_encrypt(
        keyIv.size(),
        reinterpret_cast<const unsigned char *>(keyIv.constData()),
        reinterpret_cast<unsigned char *>(encryptedKeyIv.data()),
        rsa,
        RSA_PKCS1_OAEP_PADDING);

    RSA_free(rsa);
    if (encLen <= 0)
        return QString();
    encryptedKeyIv.resize(encLen);

    // 4. Output as JSON with base64 fields
    QJsonObject outObj;
    outObj["key"] = QString::fromLatin1(encryptedKeyIv.toBase64());
    outObj["data"] = QString::fromLatin1(cipherData.toBase64());
    outObj["iv"] = QString::fromLatin1(aesIv.toBase64()); // Optionally include IV separately
    return QString::fromUtf8(QJsonDocument(outObj).toJson(QJsonDocument::Compact));
}

QByteArray License::base64Decode(const QString &b64)
{
    return QByteArray::fromBase64(b64.toUtf8());
}

QByteArray License::aesDecrypt(const QByteArray &enc, const unsigned char *key, const unsigned char *iv)
{
    QByteArray out(enc.size(), 0);
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, outlen = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(out.data()), &len,
                      reinterpret_cast<const unsigned char *>(enc.constData()), enc.size());
    outlen = len;
    EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(out.data()) + len, &len);
    outlen += len;
    EVP_CIPHER_CTX_free(ctx);
    out.resize(outlen);
    return out;
}

// Verify signature with public key (PEM string)
bool License::verifySignature(const QByteArray &data, const QByteArray &signature, const char *publicKeyPem)
{
    BIO *bio = BIO_new_mem_buf(publicKeyPem, -1);
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey)
        return false;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    bool ok = false;
    EVP_PKEY_CTX *pkey_ctx = nullptr;
    if (EVP_DigestVerifyInit(ctx, &pkey_ctx, EVP_sha256(), nullptr, pkey) == 1)
    {
        // Set PSS padding and explicit salt length to 32 bytes (SHA-256 hash size)
        if (EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING) == 1 &&
            EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, 32) == 1 &&
            EVP_DigestVerifyUpdate(ctx, data.constData(), data.size()) == 1 &&
            EVP_DigestVerifyFinal(ctx, reinterpret_cast<const unsigned char *>(signature.constData()), signature.size()) == 1)
        {
            ok = true;
        }
    }
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return ok;
}

/**
 * @brief Retrieves and decrypts the license file contents.
 *
 * This method attempts to open the license file specified by `licensePath`.
 * If the file exists and can be opened, it reads the file contents, parses the JSON,
 * extracts the initialization vector ("iv") and encrypted data ("data"), decodes them from Base64,
 * and then decrypts the data using AES decryption with a predefined key.
 *
 * @return QByteArray The decrypted license data as a QByteArray.
 *         Returns an empty QByteArray if the file does not exist, cannot be opened,
 *         or if decryption fails.
 */
QByteArray License::getDecryptedLicensePlain() const
{
    QFile licenseFile(this->licensePath);
    if (!licenseFile.exists())
        return QByteArray();
    if (licenseFile.open(QIODevice::ReadOnly))
    {
        QByteArray jsonData = licenseFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        QJsonObject obj = doc.object();
        QByteArray iv = base64Decode(obj.value("iv").toString());
        QByteArray encData = base64Decode(obj.value("data").toString());
        return aesDecrypt(encData, AES_KEY, reinterpret_cast<const unsigned char *>(iv.constData()));
    }
    return QByteArray();
}

/**
 * @brief Checks the validity of the current license.
 *
 * This function performs several steps to verify the authenticity and validity of the license:
 * - Decrypts and parses the license file.
 * - Verifies the digital signature using the provided public key.
 * - Checks license data fields (such as CPU ID, computer name, and optional fields) against the current system.
 * - For demo licenses, validates the expiry date against the current NTP time.
 * - For permanent licenses, validates the MD5 hash of system information.
 * - Sets the license type accordingly (Demo, Permanent, Expired, or Unlicensed).
 *
 * Debug information is logged at various steps for troubleshooting.
 *
 * @return true if the license is valid and passes all checks; false otherwise.
 */
bool License::checkValidity()
{
    QByteArray plain = this->getDecryptedLicensePlain();
    if (plain.isEmpty())
    {
        qDebug() << "[License] License file does not exist or decryption failed:" << this->licensePath;
        return false;
    }
    QJsonDocument licenseDoc = QJsonDocument::fromJson(plain);
    QJsonObject licenseObj = licenseDoc.object();
    QString data = licenseObj["data"].toString();
    QStringList listData = data.split(" | ");

    qDebug() << "licenseObj: " << licenseObj << Qt::endl;
    qDebug() << "plain: " << plain << Qt::endl;
    qDebug() << "data: " << data << Qt::endl;

    // Debug: print base64 signature string
    QByteArray signB64 = licenseObj["sign"].toString().toUtf8();
    qDebug() << "signB64: " << signB64 << Qt::endl;

    // Decode base64 signature
    QByteArray sign = QByteArray::fromBase64(signB64);

    // Debug: print public key fingerprint (SHA-256 hash of DER)
    BIO *bio = BIO_new_mem_buf(PUBLIC_KEY_PEM, -1);
    EVP_PKEY *pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (pkey)
    {
        unsigned char *der = nullptr;
        int derlen = i2d_PUBKEY(pkey, &der);
        if (derlen > 0 && der)
        {
            unsigned char hash[32];
            EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
            EVP_DigestUpdate(mdctx, der, derlen);
            EVP_DigestFinal_ex(mdctx, hash, nullptr);
            EVP_MD_CTX_free(mdctx);
            QByteArray fingerprint = QByteArray::fromRawData(reinterpret_cast<const char *>(hash), 32).toHex();
            OPENSSL_free(der);
        }
        EVP_PKEY_free(pkey);
    }

    // Set 'sign' to empty string before serialization
    licenseObj["sign"] = "";
    QByteArray licenseObjBytes = QJsonDocument(licenseObj).toJson(QJsonDocument::Compact);

    bool result = License::verifySignature(licenseObjBytes, sign, PUBLIC_KEY_PEM);
    if (1 < listData.size())
    {
        // Demo license
        result &= listData[0] == this->cpuId();
        result &= listData[1] == this->computerName();
        result &= listData[2] == this->applicationName;
        if (!this->option.isEmpty())
        {
            result &= listData.size() == 5 ? true : false;
            result &= listData[3] == this->option;
            qDebug() << "Option check:" << this->option << " = " << result << Qt::endl;
        }
        qDebug() << "result before Time comparison:" << result << Qt::endl;
        this->expiryUtc = QDateTime::fromString(listData[listData.size() - 1], Qt::ISODate);
        qDebug() << "[License] Expiry date from license:" << this->expiryUtc.toString(Qt::ISODate) << Qt::endl;
        QDateTime ntpUtcTime = this->ntpUtcTime();
        if (!ntpUtcTime.isValid())
        {
            // Handle invalid time, e.g., set result to false or log an error
            result = false;
            this->licenseType = Type::Unlicensed;
        }
        else
        {
            this->licenseType = result ? Type::Demo : Type::Unlicensed;
            result &= ntpUtcTime > this->expiryUtc;
            this->licenseType = this->licenseType == Type::Demo ? result ? Type::Demo : Type::Expired : Type::Unlicensed;
        }
    }
    else
    {
        // Permanent license
        // Calculate MD5
        QString hash = QString("%1 | %2 | %3").arg(this->cpuId()).arg(this->computerName()).arg(this->applicationName);
        hash += this->option.isEmpty() ? QString() : QString(" | %1").arg(this->option);
        qDebug() << "[License] Hash for MD5:" << hash << Qt::endl;
        QByteArray md5 = QCryptographicHash::hash(hash.toUtf8(), QCryptographicHash::Md5).toHex();
        qDebug() << "[License] MD5:" << md5 << Qt::endl;
        result &= md5 == data;
        this->licenseType = result ? Type::Permanent : Type::Unlicensed;
    }
    return result;
}

/**
 * @brief Constructs a License object.
 */
License::License(const QString &applicationName, const QString &idPath, const QString &licensePath, const QString &option)
{
    this->applicationName = applicationName;
    this->idPath = idPath;
    this->licensePath = licensePath;
    this->licenseType = Type::Undefined;
    this->option = option;

    QFile idFile(this->idPath);
    if (idFile.exists())
    {
        qDebug() << "[License] idPath exists:" << this->idPath;
    }
    else
    {
        qDebug() << "[License] idPath does not exist:" << this->idPath;
        // Ensure the directory for idPath exists
        QFileInfo fileInfo(this->idPath);
        QDir dir = fileInfo.absoluteDir();
        if (!dir.exists())
        {
            if (dir.mkpath("."))
            {
                qDebug() << "[License] Created directory:" << dir.absolutePath();
            }
            else
            {
                qDebug() << "[License] Failed to create directory:" << dir.absolutePath();
            }
        }
        QString jsonId = this->generateIdJSON();
        qDebug() << "[License] Generated ID JSON:" << jsonId;
        QString encryptedId = this->encryptJsonHybrid(jsonId);
        qDebug() << "[License] Encrypted ID JSON:" << encryptedId;

        // Save the encrypted ID to the license file
        QFile licenseFile(this->idPath);
        if (licenseFile.open(QIODevice::WriteOnly))
        {
            licenseFile.write(encryptedId.toUtf8());
            licenseFile.close();
            qDebug() << "[License] Saved encrypted ID to:" << this->licensePath;
        }
        else
        {
            qDebug() << "[License] Failed to open license file for writing:" << this->licensePath;
        }
    }
}
