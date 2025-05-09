#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include "ftpserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onBrowseButtonClicked();
    void onClearLogButtonClicked();
    void onNewConnection(const QString &clientAddress);
    void onClientDisconnected(const QString &clientAddress);
    void onServerLogMessage(const QString &message);

private:
    Ui::MainWindow *ui;
    FtpServer *m_server;
    
    void updateUiState(bool serverRunning);
    void addLogMessage(const QString &message);
};
#endif // MAINWINDOW_H
