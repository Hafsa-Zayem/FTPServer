#ifndef FTPCONNECTION_H
#define FTPCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QTcpServer>
#include <QHostAddress>

class FtpServer;

class FtpConnection : public QObject
{
    Q_OBJECT
public:
    explicit FtpConnection(QTcpSocket *socket, FtpServer *server);
    ~FtpConnection();

    void close();
    
    QHostAddress peerAddress() const;
    quint16 peerPort() const;
    
signals:
    void disconnected();
    void logMessage(const QString &message);

private slots:
    void processCommand();
    void onDataConnected();
    void onDataReadyRead();
    void onDataDisconnected();
    void onTimeout();
    void onBytesWritten(qint64 bytes);

private:
    // Command handlers
    void handleUSER(const QString &param);
    void handlePASS(const QString &param);
    void handleSYST(const QString &param);
    void handleQUIT(const QString &param);
    void handleTYPE(const QString &param);
    void handlePORT(const QString &param);
    void handlePASV(const QString &param);
    void handleLIST(const QString &param);
    void handleCWD(const QString &param);
    void handlePWD(const QString &param);
    void handleMKD(const QString &param);
    void handleRMD(const QString &param);
    void handleDELE(const QString &param);
    void handleRNFR(const QString &param);
    void handleRNTO(const QString &param);
    void handleSTOR(const QString &param);
    void handleRETR(const QString &param);
    void handleNOOP(const QString &param);
    
    // Helper methods
    void sendResponse(int code, const QString &message);
    void setupDataConnection();
    void closeDataConnection();
    bool checkLogin();
    QString resolvePath(const QString &path) const;
    
    // Member variables
    QTcpSocket *m_controlSocket;
    QTcpSocket *m_dataSocket;
    QTcpServer *m_passiveServer;
    FtpServer *m_server;
    QTimer *m_timer;
    
    // File transfer variables
    QFile *m_file;
    qint64 m_bytesTotal;
    qint64 m_bytesSent;
    
    // State variables
    enum TransferMode { Passive, Active };
    enum TransferType { ASCII, Binary };
    
    TransferMode m_transferMode;
    TransferType m_transferType;
    QString m_username;
    QString m_currentPath;
    QString m_renameFrom;
    bool m_isLoggedIn;
    bool m_waitingForPassword;
    
    // For active mode
    QHostAddress m_dataHostAddress;
    quint16 m_dataPort;
};

#endif // FTPCONNECTION_H
