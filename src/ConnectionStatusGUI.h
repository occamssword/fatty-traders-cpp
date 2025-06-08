#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QTableWidget>
#include <memory>
#include <atomic>
#include <mutex>
#include <queue>
#include <string>

class IBConnector;

namespace Ui {
class ConnectionStatusGUI;
}

class ConnectionStatusGUI : public QMainWindow {
    Q_OBJECT

public:
    explicit ConnectionStatusGUI(QWidget *parent = nullptr);
    ~ConnectionStatusGUI();

    void setConnector(std::shared_ptr<IBConnector> connector);
    void updateConnectionStatus(bool connected);
    void addLogMessage(const std::string& message);

private slots:
    void onConnectButtonClicked();
    void onDisconnectButtonClicked();
    void updateStatus();
    void processLogMessages();
    void updateMarketData();
    void updateAccountData();

private:
    Ui::ConnectionStatusGUI *ui;
    std::shared_ptr<IBConnector> ibConnector;
    QTimer* statusUpdateTimer;
    QTimer* logUpdateTimer;
    QTimer* dataUpdateTimer;
    
    // UI elements for data display
    QLabel* marketDataLabel;
    QTableWidget* accountTable;
    QTableWidget* positionsTable;
    
    // Thread-safe log message queue
    std::mutex logMutex;
    std::queue<std::string> logMessages;
    
    // Connection state
    std::atomic<bool> isConnected;
    
    void setupUI();
    void updateUIState(bool connected);
};