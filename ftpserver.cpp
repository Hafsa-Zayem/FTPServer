#include "ftpserver.h"
#include "ftpconnection.h"
#include <QDir>
#include <QDebug>

FtpServer::FtpServer(QObject *parent) : QObject(parent),
    m_server(new QTcpServer(this)),
    m_port(21),
    m_isRunning(false)
{
    // Initialize with default root path
    m_rootPath = QDir::homePath() + "/ftp";
    
    // Create directory if it doesn't exist
    QDir dir(m_rootPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Connect signal for incoming connections
    connect(m_server, &QTcpServer::newConnection, this, &FtpServer::onNewConnection);
}

FtpServer::~FtpServer()
{
    stop();
}

bool FtpServer::start(int port)
{
    if (m_isRunning) {
        stop();
    }
    
    m_port = port;
    
    // Start listening on the specified port
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        emit logMessage("Server failed to start: " + m_server->errorString());
        return false;
    }
    
    m_isRunning = true;
    emit logMessage(QString("FTP Server started on port %1").arg(m_port));
    
    return true;
}

void FtpServer::stop()
{
    if (m_isRunning) {
        m_server->close();
        
        // Clean up all active connections
        for (FtpConnection *connection : m_connections) {
            connection->close();
            connection->deleteLater();
        }
        
        m_connections.clear();
        m_isRunning = false;
        
        emit logMessage("FTP Server stopped");
    }
}

bool FtpServer::isRunning() const
{
    return m_isRunning;
}

void FtpServer::setRootPath(const QString &path)
{
    m_rootPath = path;
    
    // Create directory if it doesn't exist
    QDir dir(m_rootPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    emit logMessage("Root path set to: " + m_rootPath);
}

QString FtpServer::rootPath() const
{
    return m_rootPath;
}

bool FtpServer::authenticateUser(const QString &username, const QString &password)
{
    // This is a very basic implementation for demo purposes
    // In a real application, you would implement proper authentication
    
    // For this demo, accept a predefined username/password
    return (username == "admin" && password == "password");
}

void FtpServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();

        if (socket) {
            qDebug() << "New connection from:" << socket->peerAddress().toString();

            // Verify socket state
            if (!socket->isOpen()) {
                qDebug() << "Socket not open, closing";
                socket->deleteLater();
                continue;
            }

            // Create and store connection
            FtpConnection *connection = new FtpConnection(socket, this);
            m_connections.append(connection);

            // Connect signals with lambda to ensure proper cleanup
            connect(connection, &FtpConnection::disconnected, this, [this, connection]() {
                QString clientAddress = connection->peerAddress().toString() + ":" +
                                        QString::number(connection->peerPort());

                qDebug() << "Client disconnected:" << clientAddress;
                m_connections.removeOne(connection);
                connection->deleteLater();

                emit clientDisconnected(clientAddress);
                emit logMessage("Client disconnected: " + clientAddress);
            });

            emit logMessage("New connection from: " + socket->peerAddress().toString());
        }
    }
}
void FtpServer::onClientDisconnected()
{
    FtpConnection *connection = qobject_cast<FtpConnection*>(sender());
    
    if (connection) {
        QString clientAddress = connection->peerAddress().toString() + ":" + 
                               QString::number(connection->peerPort());
        
        m_connections.removeOne(connection);
        connection->deleteLater();
        
        emit clientDisconnected(clientAddress);
        emit logMessage("Client disconnected: " + clientAddress);
    }
}
