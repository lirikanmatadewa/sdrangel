/**
  @file
  @author Stefan Frings
*/

#include "httplistener.h"
#include "httpconnectionhandler.h"
#include "httpconnectionhandlerpool.h"

#include <QCoreApplication>

using namespace qtwebapp;

HttpListener::HttpListener(QSettings* settings, HttpRequestHandler* requestHandler, QObject *parent)
    : QTcpServer(parent), useQtSettings(true)
{
    Q_ASSERT(settings != 0);
    Q_ASSERT(requestHandler != 0);
    pool = 0;
    this->settings = settings;
    this->requestHandler = requestHandler;
    // Reqister type of socketDescriptor for signal/slot handling
    qRegisterMetaType<tSocketDescriptor>("tSocketDescriptor");
    // Start listening
    listen();
}

HttpListener::HttpListener(const HttpListenerSettings& settings, HttpRequestHandler* requestHandler, QObject *parent)
    : QTcpServer(parent), useQtSettings(false)
{
    Q_ASSERT(requestHandler != 0);
    pool = 0;
    this->settings = 0;
    listenerSettings = settings;
    this->requestHandler = requestHandler;
    // Reqister type of socketDescriptor for signal/slot handling
    qRegisterMetaType<tSocketDescriptor>("tSocketDescriptor");
    // Start listening
    listen();
}


HttpListener::~HttpListener()
{
    close();
}


void HttpListener::listen()
{
    if (!pool)
    {
        if (useQtSettings) {
            pool = new HttpConnectionHandlerPool(settings, requestHandler);
        } else {
            pool = new HttpConnectionHandlerPool(&listenerSettings, requestHandler);
        }
    }
    QString host = useQtSettings ? settings->value("host").toString() : listenerSettings.host;
    int port = useQtSettings ? settings->value("port").toInt() : listenerSettings.port;
    QTcpServer::listen(host.isEmpty() ? QHostAddress::Any : QHostAddress(host), port);
    if (!isListening())
    {
        ;
    }
    else {
        ;
    }
}


void HttpListener::close() {
    QTcpServer::close();
    if (pool) {
        delete pool;
        pool=NULL;
    }
}

void HttpListener::incomingConnection(tSocketDescriptor socketDescriptor) {
#ifdef SUPERVERBOSE

#endif

    HttpConnectionHandler* freeHandler=NULL;
    if (pool)
    {
        freeHandler=pool->getConnectionHandler();
    }

    // Let the handler process the new connection.
    if (freeHandler)
    {
        // The descriptor is passed via event queue because the handler lives in another thread
        QMetaObject::invokeMethod(freeHandler, "handleConnection", Qt::QueuedConnection, Q_ARG(tSocketDescriptor, socketDescriptor));
    }
    else
    {
        // Reject the connection
        QTcpSocket* socket=new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->write("HTTP/1.1 503 too many connections\r\nConnection: close\r\n\r\nToo many connections\r\n");
        socket->disconnectFromHost();
    }
}
