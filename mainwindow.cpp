#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ftpserver.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_server(new FtpServer(this))
{
    ui->setupUi(this);
    
    // Set up default root directory
    ui->rootDirEdit->setText(QDir::homePath() + "/ftp");
    
    // Connect signals and slots
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::onStopButtonClicked);
    connect(ui->browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseButtonClicked);
    connect(ui->clearLogButton, &QPushButton::clicked, this, &MainWindow::onClearLogButtonClicked);
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Qt FTP Server", 
                           "Qt FTP Server - A simple FTP server implementation using Qt\n\n"
                           "Copyright © 2025");
    });
    
    // Connect server signals
    connect(m_server, &FtpServer::newConnection, this, &MainWindow::onNewConnection);
    connect(m_server, &FtpServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(m_server, &FtpServer::logMessage, this, &MainWindow::onServerLogMessage);
    
    // Set initial UI state
    updateUiState(false);
    
    // Show startup message
    addLogMessage("Server ready. Click 'Start Server' to begin.");
}

MainWindow::~MainWindow()
{
    if (m_server->isRunning()) {
        m_server->stop();
    }
    delete ui;
}

void MainWindow::onStartButtonClicked()
{
    // Get server settings
    int port = ui->portSpinBox->value();
    QString rootPath = ui->rootDirEdit->text();
    
    // Validate root path
    QDir dir(rootPath);
    if (!dir.exists()) {
        if (QMessageBox::question(this, "Create Directory?", 
                                 "The specified directory does not exist. Create it?",
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            if (!dir.mkpath(".")) {
                QMessageBox::critical(this, "Error", "Failed to create directory");
                return;
            }
        } else {
            return;
        }
    }
    
    // Set root path and start server
    m_server->setRootPath(rootPath);
    if (!m_server->start(port)) {
        QMessageBox::critical(this, "Error", "Failed to start FTP server");
        return;
    }
    
    // Update UI state
    updateUiState(true);
}

void MainWindow::onStopButtonClicked()
{
    m_server->stop();
    updateUiState(false);
}

void MainWindow::onBrowseButtonClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Root Directory",
                                                    ui->rootDirEdit->text(),
                                                    QFileDialog::ShowDirsOnly | 
                                                    QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        ui->rootDirEdit->setText(dir);
    }
}

void MainWindow::onClearLogButtonClicked()
{
    ui->logTextEdit->clear();
}

void MainWindow::onNewConnection(const QString &clientAddress)
{
    addLogMessage("New connection from: " + clientAddress);
}

void MainWindow::onClientDisconnected(const QString &clientAddress)
{
    addLogMessage("Client disconnected: " + clientAddress);
}

void MainWindow::onServerLogMessage(const QString &message)
{
    addLogMessage(message);
}

void MainWindow::updateUiState(bool serverRunning)
{
    ui->startButton->setEnabled(!serverRunning);
    ui->stopButton->setEnabled(serverRunning);
    ui->portSpinBox->setEnabled(!serverRunning);
    ui->rootDirEdit->setEnabled(!serverRunning);
    ui->browseButton->setEnabled(!serverRunning);
    
    ui->statusLabel->setText(serverRunning ? "Running" : "Stopped");
    if (serverRunning) {
        ui->statusLabel->setStyleSheet("color: green;");
    } else {
        ui->statusLabel->setStyleSheet("color: red;");
    }
}

void MainWindow::addLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}
