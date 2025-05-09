#include "ftpconnection.h"
#include "ftpserver.h"
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QRandomGenerator>
#include <QHostAddress>
#include <QDebug>

FtpConnection::FtpConnection(QTcpSocket *socket, FtpServer *server) : QObject(server),
    m_controlSocket(socket),
    m_server(server)
{
    // Verify socket
    if (!m_controlSocket || !m_controlSocket->isOpen()) {
        qDebug() << "Invalid socket in FtpConnection constructor";
        deleteLater();
        return;
    }

    // Basic setup
    m_currentPath = "/";
    m_controlSocket->setParent(this);

    // Connect signals
    connect(m_controlSocket, &QTcpSocket::readyRead, this, &FtpConnection::processCommand);
    connect(m_controlSocket, &QTcpSocket::disconnected, this, &FtpConnection::disconnected);

    // Send welcome message after a short delay
    QTimer::singleShot(100, this, [this]() {
        if (m_controlSocket && m_controlSocket->state() == QTcpSocket::ConnectedState) {
            sendResponse(220, "FTP Server Ready");
            qDebug() << "Welcome message sent";
        }
    });

    qDebug() << "FtpConnection created";
}
FtpConnection::~FtpConnection()
{
    qDebug() << "FtpConnection destroyed";
    if (m_controlSocket) {
        qDebug() << "Socket state before destruction:" << m_controlSocket->state();
    }
}
void FtpConnection::close()
{
    if (m_controlSocket && m_controlSocket->isOpen()) {
        m_controlSocket->close();
    }
    
    closeDataConnection();
}

QHostAddress FtpConnection::peerAddress() const
{
    if (m_controlSocket) {
        return m_controlSocket->peerAddress();
    }
    return QHostAddress();
}

quint16 FtpConnection::peerPort() const
{
    if (m_controlSocket) {
        return m_controlSocket->peerPort();
    }
    return 0;
}

void FtpConnection::processCommand()
{
    // Reset timer on each command
    m_timer->start();
    
    while (m_controlSocket->canReadLine()) {
        QString line = QString::fromUtf8(m_controlSocket->readLine()).trimmed();
        
        if (line.isEmpty()) {
            continue;
        }
        
        // Log received command
        emit logMessage("Received: " + line);
        
        // Parse command and parameters
        QString command, parameter;
        int spaceIndex = line.indexOf(' ');
        
        if (spaceIndex == -1) {
            command = line.toUpper();
            parameter = "";
        } else {
            command = line.left(spaceIndex).toUpper();
            parameter = line.mid(spaceIndex + 1);
        }
        
        // Handle different commands
        if (command == "USER") {
            handleUSER(parameter);
        } else if (command == "PASS") {
            handlePASS(parameter);
        } else if (command == "SYST") {
            handleSYST(parameter);
        } else if (command == "QUIT") {
            handleQUIT(parameter);
        } else if (command == "TYPE") {
            handleTYPE(parameter);
        } else if (command == "PORT") {
            handlePORT(parameter);
        } else if (command == "PASV") {
            handlePASV(parameter);
        } else if (command == "LIST") {
            handleLIST(parameter);
        } else if (command == "CWD") {
            handleCWD(parameter);
        } else if (command == "PWD") {
            handlePWD(parameter);
        } else if (command == "MKD") {
            handleMKD(parameter);
        } else if (command == "RMD") {
            handleRMD(parameter);
        } else if (command == "DELE") {
            handleDELE(parameter);
        } else if (command == "RNFR") {
            handleRNFR(parameter);
        } else if (command == "RNTO") {
            handleRNTO(parameter);
        } else if (command == "STOR") {
            handleSTOR(parameter);
        } else if (command == "RETR") {
            handleRETR(parameter);
        } else if (command == "NOOP") {
            handleNOOP(parameter);
        } else {
            // Unrecognized command
            sendResponse(502, "Command not implemented");
        }
    }
}

void FtpConnection::sendResponse(int code, const QString &message)
{
    QString response = QString("%1 %2\r\n").arg(code).arg(message);
    m_controlSocket->write(response.toUtf8());
    m_controlSocket->flush();
    
    // Log sent response
    emit logMessage("Sent: " + response.trimmed());
}

void FtpConnection::setupDataConnection()
{
    closeDataConnection();
    
    if (m_transferMode == Passive) {
        // In passive mode, we need to create a server and wait for client to connect
        m_passiveServer = new QTcpServer(this);
        
        // Connect to server signals
        connect(m_passiveServer, &QTcpServer::newConnection, this, &FtpConnection::onDataConnected);
        
        // Start listening on a random port
        if (!m_passiveServer->listen(QHostAddress::Any, 0)) {
            sendResponse(425, "Cannot open data connection");
            delete m_passiveServer;
            m_passiveServer = nullptr;
            return;
        }
    } else {
        // In active mode, we connect to the client
        m_dataSocket = new QTcpSocket(this);
        
        // Connect to socket signals
        connect(m_dataSocket, &QTcpSocket::connected, this, &FtpConnection::onDataConnected);
        connect(m_dataSocket, &QTcpSocket::readyRead, this, &FtpConnection::onDataReadyRead);
        connect(m_dataSocket, &QTcpSocket::disconnected, this, &FtpConnection::onDataDisconnected);
        connect(m_dataSocket, &QTcpSocket::bytesWritten, this, &FtpConnection::onBytesWritten);
        
        // Connect to the specified address and port
        m_dataSocket->connectToHost(m_dataHostAddress, m_dataPort);
    }
}

void FtpConnection::closeDataConnection()
{
    // Clean up data socket
    if (m_dataSocket) {
        m_dataSocket->close();
        m_dataSocket->deleteLater();
        m_dataSocket = nullptr;
    }
    
    // Clean up passive server
    if (m_passiveServer) {
        m_passiveServer->close();
        m_passiveServer->deleteLater();
        m_passiveServer = nullptr;
    }
    
    // Clean up file
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
    }
}

bool FtpConnection::checkLogin()
{
    if (!m_isLoggedIn) {
        sendResponse(530, "Not logged in");
        return false;
    }
    return true;
}

QString FtpConnection::resolvePath(const QString &path) const
{
    QString resolvedPath;
    
    if (path.startsWith('/')) {
        // Absolute path
        resolvedPath = path;
    } else if (path.isEmpty()) {
        // Current directory
        resolvedPath = m_currentPath;
    } else {
        // Relative path
        if (m_currentPath.endsWith('/')) {
            resolvedPath = m_currentPath + path;
        } else {
            resolvedPath = m_currentPath + '/' + path;
        }
    }
    
    // Resolve ".." and "."
    QStringList segments = resolvedPath.split('/', Qt::SkipEmptyParts);
    QStringList result;
    
    for (const QString &segment : segments) {
        if (segment == ".") {
            continue;
        } else if (segment == "..") {
            if (!result.isEmpty()) {
                result.removeLast();
            }
        } else {
            result.append(segment);
        }
    }
    
    resolvedPath = '/' + result.join('/');
    if (resolvedPath.isEmpty()) {
        resolvedPath = "/";
    }
    
    return resolvedPath;
}

void FtpConnection::onDataConnected()
{
    if (m_transferMode == Passive && m_passiveServer) {
        // In passive mode, get the socket from the server
        m_dataSocket = m_passiveServer->nextPendingConnection();
        
        if (m_dataSocket) {
            // Connect to socket signals
            connect(m_dataSocket, &QTcpSocket::readyRead, this, &FtpConnection::onDataReadyRead);
            connect(m_dataSocket, &QTcpSocket::disconnected, this, &FtpConnection::onDataDisconnected);
            connect(m_dataSocket, &QTcpSocket::bytesWritten, this, &FtpConnection::onBytesWritten);
            
            // Clean up the server as it's no longer needed
            m_passiveServer->close();
            m_passiveServer->deleteLater();
            m_passiveServer = nullptr;
        }
    }
    
    // Signal successful connection
    sendResponse(150, "Data connection established");
}

void FtpConnection::onDataReadyRead()
{
    if (m_dataSocket && m_file) {
        // Read data from socket and write to file
        QByteArray data = m_dataSocket->readAll();
        m_file->write(data);
    }
}

void FtpConnection::onDataDisconnected()
{
    if (m_file) {
        m_file->close();
        delete m_file;
        m_file = nullptr;
        
        sendResponse(226, "Transfer complete");
    }
    
    if (m_dataSocket) {
        m_dataSocket->deleteLater();
        m_dataSocket = nullptr;
    }
}

void FtpConnection::onTimeout()
{
    sendResponse(421, "Timeout: closing control connection");
    emit disconnected();
}

void FtpConnection::onBytesWritten(qint64 bytes)
{
    m_bytesSent += bytes;
    
    if (m_file && m_dataSocket) {
        // If we're sending a file, continue sending data
        if (m_bytesSent < m_bytesTotal) {
            QByteArray data = m_file->read(qMin(m_bytesTotal - m_bytesSent, qint64(4096)));
            m_dataSocket->write(data);
        } else {
            // File transfer complete
            m_dataSocket->disconnectFromHost();
        }
    }
}

// Command handlers
void FtpConnection::handleUSER(const QString &param)
{
    m_username = param;
    m_waitingForPassword = true;
    sendResponse(331, "User name okay, need password");
}

void FtpConnection::handlePASS(const QString &param)
{
    if (!m_waitingForPassword) {
        sendResponse(503, "Bad sequence of commands");
        return;
    }
    
    if (m_server->authenticateUser(m_username, param)) {
        m_isLoggedIn = true;
        sendResponse(230, "User logged in, proceed");
    } else {
        m_isLoggedIn = false;
        sendResponse(530, "Login incorrect");
    }
    
    m_waitingForPassword = false;
}

void FtpConnection::handleSYST(const QString &param)
{
    Q_UNUSED(param);
    sendResponse(215, "UNIX Type: L8");
}

void FtpConnection::handleQUIT(const QString &param)
{
    Q_UNUSED(param);
    sendResponse(221, "Goodbye");
    emit disconnected();
}

void FtpConnection::handleTYPE(const QString &param)
{
    if (param == "A" || param == "A N") {
        m_transferType = ASCII;
        sendResponse(200, "Type set to ASCII");
    } else if (param == "I" || param == "L 8") {
        m_transferType = Binary;
        sendResponse(200, "Type set to Binary");
    } else {
        sendResponse(504, "Type not implemented");
    }
}

void FtpConnection::handlePORT(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    // Parse PORT command
    QStringList parts = param.split(',');
    if (parts.size() != 6) {
        sendResponse(501, "Invalid PORT command");
        return;
    }
    
    // Extract IP address and port
    QStringList ipParts = parts.mid(0, 4);
    QString ipAddress = ipParts.join('.');
    int portHi = parts[4].toInt();
    int portLo = parts[5].toInt();
    quint16 port = (portHi << 8) + portLo;
    
    // Set up data connection details
    m_dataHostAddress = QHostAddress(ipAddress);
    m_dataPort = port;
    m_transferMode = Active;
    
    sendResponse(200, "PORT command successful");
}

void FtpConnection::handlePASV(const QString &param)
{
    Q_UNUSED(param);
    
    if (!checkLogin()) {
        return;
    }
    
    m_transferMode = Passive;
    
    // Set up the passive server
    setupDataConnection();
    
    if (!m_passiveServer) {
        return;
    }
    
    // Get the server port
    quint16 port = m_passiveServer->serverPort();
    
    // Get the server IP address
    QHostAddress address = m_controlSocket->localAddress();
    QString ipAddress = address.toString();
    
    // Replace ":" with "." for IPv4 format
    ipAddress.replace(':', '.');
    
    // If we have IPv6, convert to IPv4
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        if (address.isEqual(QHostAddress::LocalHostIPv6)) {
            ipAddress = "127.0.0.1";
        } else {
            // Just use localhost if we can't determine the address
            ipAddress = "127.0.0.1";
        }
    }
    
    // Format the response
    QString response = QString("Entering Passive Mode (%1,%2,%3)").arg(
        ipAddress.replace('.', ','),
        QString::number(port >> 8),
        QString::number(port & 0xFF)
    );
    
    sendResponse(227, response);
}

void FtpConnection::handleLIST(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    // Set up data connection
    setupDataConnection();
    
    if (!m_dataSocket && !m_passiveServer) {
        sendResponse(425, "Can't open data connection");
        return;
    }
    
    if (m_transferMode == Active && m_dataSocket) {
        // Active mode, wait for connection
        if (!m_dataSocket->waitForConnected(5000)) {
            sendResponse(425, "Can't open data connection");
            closeDataConnection();
            return;
        }
        
        onDataConnected();
    } else {
        // Passive mode, notify client that we're ready
        sendResponse(150, "Opening data connection for directory listing");
    }
    
    // Resolve the path
    QString path = resolvePath(param);
    QString fullPath = m_server->rootPath() + path;
    
    QDir dir(fullPath);
    if (!dir.exists()) {
        sendResponse(550, "Directory not found");
        closeDataConnection();
        return;
    }
    
    // Get directory entries
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    
    // Wait for data connection if in passive mode
    if (m_transferMode == Passive && !m_dataSocket) {
        if (!m_passiveServer->waitForNewConnection(5000)) {
            sendResponse(425, "Can't open data connection");
            closeDataConnection();
            return;
        }
    }
    
    // Send directory listing
    QString listing;
    for (const QFileInfo &info : entries) {
        QString permissions = "-rw-r--r--";
        if (info.isDir()) {
            permissions[0] = 'd';
        }
        
        QString ownerName = "owner";
        QString groupName = "group";
        qint64 size = info.size();
        
        // Format date (as per Unix 'ls' command)
        QDateTime modTime = info.lastModified();
        QString dateStr = modTime.toString("MMM dd  yyyy");
        if (modTime.date().year() == QDate::currentDate().year()) {
            dateStr = modTime.toString("MMM dd hh:mm");
        }
        
        QString filename = info.fileName();
        
        // Format the line similar to Unix 'ls -l' output
        listing += QString("%1 %2 %3 %4 %5 %6 %7\r\n")
            .arg(permissions)
            .arg(1, 3)  // Link count
            .arg(ownerName, 8)
            .arg(groupName, 8)
            .arg(size, 8)
            .arg(dateStr)
            .arg(filename);
    }
    
    if (m_dataSocket) {
        m_dataSocket->write(listing.toUtf8());
        m_dataSocket->disconnectFromHost();
    }
}

void FtpConnection::handleCWD(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    QString newPath = resolvePath(param);
    QString fullPath = m_server->rootPath() + newPath;
    
    QDir dir(fullPath);
    if (!dir.exists()) {
        sendResponse(550, "Directory not found");
        return;
    }
    
    m_currentPath = newPath;
    sendResponse(250, "Directory changed to " + newPath);
}

void FtpConnection::handlePWD(const QString &param)
{
    Q_UNUSED(param);
    
    if (!checkLogin()) {
        return;
    }
    
    sendResponse(257, "\"" + m_currentPath + "\" is current directory");
}

void FtpConnection::handleMKD(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing directory name");
        return;
    }
    
    QString newPath = resolvePath(param);
    QString fullPath = m_server->rootPath() + newPath;
    
    QDir dir;
    if (dir.mkdir(fullPath)) {
        sendResponse(257, "\"" + newPath + "\" created");
    } else {
        sendResponse(550, "Failed to create directory");
    }
}

void FtpConnection::handleRMD(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing directory name");
        return;
    }
    
    QString path = resolvePath(param);
    QString fullPath = m_server->rootPath() + path;
    
    QDir dir;
    if (dir.rmdir(fullPath)) {
        sendResponse(250, "Directory removed");
    } else {
        sendResponse(550, "Failed to remove directory");
    }
}

void FtpConnection::handleDELE(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing file name");
        return;
    }
    
    QString path = resolvePath(param);
    QString fullPath = m_server->rootPath() + path;
    
    QFile file(fullPath);
    if (file.remove()) {
        sendResponse(250, "File deleted");
    } else {
        sendResponse(550, "Failed to delete file");
    }
}

void FtpConnection::handleRNFR(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing file name");
        return;
    }
    
    m_renameFrom = resolvePath(param);
    QString fullPath = m_server->rootPath() + m_renameFrom;
    
    if (QFile::exists(fullPath)) {
        sendResponse(350, "Ready for RNTO");
    } else {
        sendResponse(550, "File not found");
        m_renameFrom.clear();
    }
}

void FtpConnection::handleRNTO(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (m_renameFrom.isEmpty()) {
        sendResponse(503, "RNFR required first");
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing file name");
        m_renameFrom.clear();
        return;
    }
    
    QString newPath = resolvePath(param);
    QString oldFullPath = m_server->rootPath() + m_renameFrom;
    QString newFullPath = m_server->rootPath() + newPath;
    
    if (QFile::rename(oldFullPath, newFullPath)) {
        sendResponse(250, "File renamed");
    } else {
        sendResponse(550, "Failed to rename file");
    }
    
    m_renameFrom.clear();
}

void FtpConnection::handleSTOR(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing file name");
        return;
    }
    
    // Set up data connection
    setupDataConnection();
    
    if (!m_dataSocket && !m_passiveServer) {
        sendResponse(425, "Can't open data connection");
        return;
    }
    
    // Resolve path
    QString path = resolvePath(param);
    QString fullPath = m_server->rootPath() + path;
    
    // Create file
    m_file = new QFile(fullPath);
    if (!m_file->open(QIODevice::WriteOnly)) {
        sendResponse(550, "Failed to open file");
        closeDataConnection();
        return;
    }
    
    if (m_transferMode == Active && m_dataSocket) {
        // Active mode, wait for connection
        if (!m_dataSocket->waitForConnected(5000)) {
            sendResponse(425, "Can't open data connection");
            closeDataConnection();
            return;
        }
        
        onDataConnected();
    } else {
        // Passive mode, notify client that we're ready
        sendResponse(150, "Opening data connection for file upload");
    }
}

void FtpConnection::handleRETR(const QString &param)
{
    if (!checkLogin()) {
        return;
    }
    
    if (param.isEmpty()) {
        sendResponse(501, "Missing file name");
        return;
    }
    
    // Resolve path
    QString path = resolvePath(param);
    QString fullPath = m_server->rootPath() + path;
    
    // Open file
    m_file = new QFile(fullPath);
    if (!m_file->open(QIODevice::ReadOnly)) {
        sendResponse(550, "Failed to open file");
        delete m_file;
        m_file = nullptr;
        return;
    }
    
    // Set up data connection
    setupDataConnection();
    
    if (!m_dataSocket && !m_passiveServer) {
        sendResponse(425, "Can't open data connection");
        closeDataConnection();
        return;
    }
    
    m_bytesTotal = m_file->size();
    m_bytesSent = 0;
    
    if (m_transferMode == Active && m_dataSocket) {
        // Active mode, wait for connection
        if (!m_dataSocket->waitForConnected(5000)) {
            sendResponse(425, "Can't open data connection");
            closeDataConnection();
            return;
        }
        
        onDataConnected();
        
        // Start sending data
        QByteArray data = m_file->read(qMin(m_bytesTotal, qint64(4096)));
        if (!data.isEmpty()) {
            m_dataSocket->write(data);
        } else {
            m_dataSocket->disconnectFromHost();
        }
    } else {
        // Passive mode, notify client that we're ready
        sendResponse(150, "Opening data connection for file download");
    }
}

void FtpConnection::handleNOOP(const QString &param)
{
    Q_UNUSED(param);
    sendResponse(200, "NOOP command successful");
}
