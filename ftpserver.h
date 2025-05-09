#ifndef FTPSERVER_H
#define FTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QList>
#include <QDir>

class FtpConnection;

class FtpServer : public QObject
{
    Q_OBJECT
public:
    explicit FtpServer(QObject *parent = nullptr);
    ~FtpServer();

    bool start(int port = 21);
    void stop();
    bool isRunning() const;
    
    void setRootPath(const QString &path);
    QString rootPath() const;
    
    // Authenticate user - very basic implementation for demo
    bool authenticateUser(const QString &username, const QString &password);

signals:
    void newConnection(const QString &clientAddress);
    void clientDisconnected(const QString &clientAddress);
    void logMessage(const QString &message);

private slots:
    void onNewConnection();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QList<FtpConnection*> m_connections;
    QString m_rootPath;
    int m_port;
    bool m_isRunning;
};

#endif // FTPSERVER_H
