#pragma once

#include <QLocalServer>
#include <QLocalSocket>
#include <QString>

class SingleInstanceGuard
{
public:
    explicit SingleInstanceGuard(const QString& key)
        : m_key(key)
    {
    }

    ~SingleInstanceGuard()
    {
        if (m_server)
        {
            m_server->close();
            delete m_server;
            m_server = nullptr;
        }
    }

    bool tryToRun()
    {
        QLocalSocket socket;
        socket.connectToServer(m_key);

        if (socket.waitForConnected(500))
        {
            // Another ES300 instance is already running.
            socket.disconnectFromServer();
            return false;
        }

        // Remove stale server left by a crashed instance.
        QLocalServer::removeServer(m_key);

        m_server = new QLocalServer();

        if (!m_server->listen(m_key))
        {
            delete m_server;
            m_server = nullptr;
            return false;
        }

        return true;
    }

private:
    QString m_key;
    QLocalServer* m_server = nullptr;

    Q_DISABLE_COPY(SingleInstanceGuard)
};
