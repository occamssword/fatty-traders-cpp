#include "ConnectionStatusGUI.h"
#include "IBConnector.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDateTime>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QTableWidgetItem>
#include <iostream>

ConnectionStatusGUI::ConnectionStatusGUI(QWidget *parent)
    : QMainWindow(parent)
    , isConnected(false) {
    setupUI();
    
    // Setup timers for periodic updates
    statusUpdateTimer = new QTimer(this);
    connect(statusUpdateTimer, &QTimer::timeout, this, &ConnectionStatusGUI::updateStatus);
    statusUpdateTimer->start(1000); // Update every second
    
    logUpdateTimer = new QTimer(this);
    connect(logUpdateTimer, &QTimer::timeout, this, &ConnectionStatusGUI::processLogMessages);
    logUpdateTimer->start(100); // Process log messages every 100ms
    
    dataUpdateTimer = new QTimer(this);
    connect(dataUpdateTimer, &QTimer::timeout, this, &ConnectionStatusGUI::updateMarketData);
    connect(dataUpdateTimer, &QTimer::timeout, this, &ConnectionStatusGUI::updateAccountData);
    dataUpdateTimer->start(1000); // Update data every second
}

ConnectionStatusGUI::~ConnectionStatusGUI() {
    if (statusUpdateTimer) {
        statusUpdateTimer->stop();
        delete statusUpdateTimer;
    }
    if (logUpdateTimer) {
        logUpdateTimer->stop();
        delete logUpdateTimer;
    }
}

void ConnectionStatusGUI::setupUI() {
    // Main widget and layout
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Connection Status Group
    QGroupBox* statusGroup = new QGroupBox("Connection Status", this);
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    // Status indicator (big colored circle/label)
    QLabel* statusIndicator = new QLabel("DISCONNECTED", this);
    statusIndicator->setObjectName("statusIndicator");
    statusIndicator->setAlignment(Qt::AlignCenter);
    statusIndicator->setMinimumHeight(60);
    statusIndicator->setStyleSheet(
        "QLabel#statusIndicator {"
        "  background-color: #ff4444;"
        "  color: white;"
        "  font-size: 24px;"
        "  font-weight: bold;"
        "  border-radius: 10px;"
        "  padding: 10px;"
        "}"
    );
    statusLayout->addWidget(statusIndicator);
    
    // Connection details
    QHBoxLayout* detailsLayout = new QHBoxLayout();
    
    QLabel* hostLabel = new QLabel("Host:", this);
    QLabel* hostValue = new QLabel("127.0.0.1", this);
    hostValue->setObjectName("hostValue");
    
    QLabel* portLabel = new QLabel("Port:", this);
    QLabel* portValue = new QLabel("4001", this);
    portValue->setObjectName("portValue");
    
    QLabel* clientIdLabel = new QLabel("Client ID:", this);
    QLabel* clientIdValue = new QLabel("1", this);
    clientIdValue->setObjectName("clientIdValue");
    
    detailsLayout->addWidget(hostLabel);
    detailsLayout->addWidget(hostValue);
    detailsLayout->addSpacing(20);
    detailsLayout->addWidget(portLabel);
    detailsLayout->addWidget(portValue);
    detailsLayout->addSpacing(20);
    detailsLayout->addWidget(clientIdLabel);
    detailsLayout->addWidget(clientIdValue);
    detailsLayout->addStretch();
    
    statusLayout->addLayout(detailsLayout);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* connectButton = new QPushButton("Connect", this);
    connectButton->setObjectName("connectButton");
    connectButton->setMinimumHeight(40);
    connectButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  font-weight: bold;"
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #3d8b40;"
        "}"
    );
    
    QPushButton* disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setObjectName("disconnectButton");
    disconnectButton->setMinimumHeight(40);
    disconnectButton->setEnabled(false);
    disconnectButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #f44336;"
        "  color: white;"
        "  font-weight: bold;"
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 10px;"
        "}"
        "QPushButton:hover:enabled {"
        "  background-color: #da190b;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #ba0000;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
    );
    
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);
    
    statusLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusGroup);
    
    // Activity Log Group
    QGroupBox* logGroup = new QGroupBox("Activity Log", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    
    QTextEdit* logTextEdit = new QTextEdit(this);
    logTextEdit->setObjectName("logTextEdit");
    logTextEdit->setReadOnly(true);
    // Note: setMaximumBlockCount is only available in QPlainTextEdit
    logTextEdit->setStyleSheet(
        "QTextEdit {"
        "  background-color: #f5f5f5;"
        "  color: black;"
        "  font-family: monospace;"
        "  font-size: 12px;"
        "}"
    );
    
    logLayout->addWidget(logTextEdit);
    
    // Create splitter for log and data panels
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(logGroup);
    
    // Data display panel
    QWidget* dataPanel = new QWidget(this);
    QVBoxLayout* dataPanelLayout = new QVBoxLayout(dataPanel);
    
    // Market Data Group
    QGroupBox* marketDataGroup = new QGroupBox("Market Data - AAPL", this);
    QVBoxLayout* marketDataLayout = new QVBoxLayout(marketDataGroup);
    
    marketDataLabel = new QLabel("Waiting for data...", this);
    marketDataLabel->setStyleSheet(
        "QLabel {"
        "  background-color: white;"
        "  color: black;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 10px;"
        "  border: 1px solid #ddd;"
        "  border-radius: 5px;"
        "}"
    );
    marketDataLayout->addWidget(marketDataLabel);
    
    // Account Summary Table
    QGroupBox* accountGroup = new QGroupBox("Account Summary", this);
    QVBoxLayout* accountLayout = new QVBoxLayout(accountGroup);
    
    accountTable = new QTableWidget(0, 4, this);
    accountTable->setHorizontalHeaderLabels(QStringList() << "Account" << "Item" << "Value" << "Currency");
    accountTable->horizontalHeader()->setStretchLastSection(true);
    accountTable->setAlternatingRowColors(true);
    accountTable->setStyleSheet(
        "QTableWidget { "
        "  background-color: white; "
        "  color: black; "
        "  gridline-color: #ccc; "
        "} "
        "QTableWidget::item { "
        "  color: black; "
        "  background-color: white; "
        "} "
        "QTableWidget::item:selected { "
        "  background-color: #3399ff; "
        "  color: white; "
        "} "
        "QHeaderView::section { "
        "  background-color: #f0f0f0; "
        "  color: black; "
        "  padding: 4px; "
        "  border: 1px solid #ddd; "
        "}"
    );
    accountLayout->addWidget(accountTable);
    
    // Positions Table
    QGroupBox* positionsGroup = new QGroupBox("Positions", this);
    QVBoxLayout* positionsLayout = new QVBoxLayout(positionsGroup);
    
    positionsTable = new QTableWidget(0, 5, this);
    positionsTable->setHorizontalHeaderLabels(QStringList() << "Account" << "Symbol" << "Position" << "Avg Cost" << "Value");
    positionsTable->horizontalHeader()->setStretchLastSection(true);
    positionsTable->setAlternatingRowColors(true);
    positionsTable->setStyleSheet(
        "QTableWidget { "
        "  background-color: white; "
        "  color: black; "
        "  gridline-color: #ccc; "
        "} "
        "QTableWidget::item { "
        "  color: black; "
        "  background-color: white; "
        "} "
        "QTableWidget::item:selected { "
        "  background-color: #3399ff; "
        "  color: white; "
        "} "
        "QHeaderView::section { "
        "  background-color: #f0f0f0; "
        "  color: black; "
        "  padding: 4px; "
        "  border: 1px solid #ddd; "
        "}"
    );
    positionsLayout->addWidget(positionsTable);
    
    dataPanelLayout->addWidget(marketDataGroup);
    dataPanelLayout->addWidget(accountGroup, 1);
    dataPanelLayout->addWidget(positionsGroup, 2);
    
    splitter->addWidget(dataPanel);
    splitter->setSizes(QList<int>() << 400 << 600);
    
    mainLayout->addWidget(splitter);
    
    // Connect buttons
    connect(connectButton, &QPushButton::clicked, this, &ConnectionStatusGUI::onConnectButtonClicked);
    connect(disconnectButton, &QPushButton::clicked, this, &ConnectionStatusGUI::onDisconnectButtonClicked);
    
    // Window properties
    setWindowTitle("IB Gateway Connection Status & Market Data");
    resize(1200, 700);
    setMinimumSize(1000, 600);
}

void ConnectionStatusGUI::setConnector(std::shared_ptr<IBConnector> connector) {
    ibConnector = connector;
}

void ConnectionStatusGUI::updateConnectionStatus(bool connected) {
    isConnected = connected;
    updateUIState(connected);
}

void ConnectionStatusGUI::addLogMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    logMessages.push("[" + timestamp.toStdString() + "] " + message);
}

void ConnectionStatusGUI::onConnectButtonClicked() {
    if (!ibConnector) {
        QMessageBox::warning(this, "Error", "No IB connector instance available!");
        return;
    }
    
    addLogMessage("Attempting to connect to IB Gateway...");
    
    bool success = ibConnector->connect("127.0.0.1", 4001, 1);
    
    if (success) {
        addLogMessage("Connection successful!");
    } else {
        addLogMessage("Connection failed!");
        QMessageBox::critical(this, "Connection Failed", 
                            "Failed to connect to IB Gateway.\n"
                            "Please ensure IB Gateway is running and API is enabled.");
    }
}

void ConnectionStatusGUI::onDisconnectButtonClicked() {
    if (!ibConnector) {
        return;
    }
    
    addLogMessage("Disconnecting from IB Gateway...");
    ibConnector->disconnect();
    addLogMessage("Disconnected.");
}

void ConnectionStatusGUI::updateStatus() {
    if (!ibConnector) {
        return;
    }
    
    bool currentlyConnected = ibConnector->isConnected();
    if (currentlyConnected != isConnected) {
        updateConnectionStatus(currentlyConnected);
        
        if (currentlyConnected) {
            addLogMessage("Connection established with IB Gateway");
        } else {
            addLogMessage("Connection lost with IB Gateway");
        }
    }
}

void ConnectionStatusGUI::processLogMessages() {
    std::lock_guard<std::mutex> lock(logMutex);
    
    QTextEdit* logTextEdit = findChild<QTextEdit*>("logTextEdit");
    if (!logTextEdit) return;
    
    while (!logMessages.empty()) {
        QString message = QString::fromStdString(logMessages.front());
        logMessages.pop();
        
        logTextEdit->append(message);
    }
}

void ConnectionStatusGUI::updateUIState(bool connected) {
    // Update status indicator
    QLabel* statusIndicator = findChild<QLabel*>("statusIndicator");
    if (statusIndicator) {
        if (connected) {
            statusIndicator->setText("CONNECTED");
            statusIndicator->setStyleSheet(
                "QLabel#statusIndicator {"
                "  background-color: #4CAF50;"
                "  color: white;"
                "  font-size: 24px;"
                "  font-weight: bold;"
                "  border-radius: 10px;"
                "  padding: 10px;"
                "}"
            );
        } else {
            statusIndicator->setText("DISCONNECTED");
            statusIndicator->setStyleSheet(
                "QLabel#statusIndicator {"
                "  background-color: #ff4444;"
                "  color: white;"
                "  font-size: 24px;"
                "  font-weight: bold;"
                "  border-radius: 10px;"
                "  padding: 10px;"
                "}"
            );
        }
    }
    
    // Update button states
    QPushButton* connectButton = findChild<QPushButton*>("connectButton");
    QPushButton* disconnectButton = findChild<QPushButton*>("disconnectButton");
    
    if (connectButton) {
        connectButton->setEnabled(!connected);
    }
    
    if (disconnectButton) {
        disconnectButton->setEnabled(connected);
    }
}

void ConnectionStatusGUI::updateMarketData() {
    if (!ibConnector || !ibConnector->isConnected()) {
        return;
    }
    
    auto prices = ibConnector->getTickPrices();
    
    if (marketDataLabel) {
        QString marketText = "AAPL Market Data (Delayed)\n";
        
        // Field IDs: 1=BID, 2=ASK, 4=LAST
        double bid = prices.count(101) ? prices[101] : 0.0;  // tickerId(1) * 100 + field(1)
        double ask = prices.count(102) ? prices[102] : 0.0;  // tickerId(1) * 100 + field(2)
        double last = prices.count(104) ? prices[104] : 0.0; // tickerId(1) * 100 + field(4)
        
        if (bid > 0) marketText += QString("Bid: $%1  ").arg(bid, 0, 'f', 2);
        else marketText += "Bid: --  ";
        
        if (ask > 0) marketText += QString("Ask: $%1\n").arg(ask, 0, 'f', 2);
        else marketText += "Ask: --\n";
        
        if (last > 0) marketText += QString("Last: $%1").arg(last, 0, 'f', 2);
        else marketText += "Last: --";
        
        marketDataLabel->setText(marketText);
    }
}

void ConnectionStatusGUI::updateAccountData() {
    if (!ibConnector || !ibConnector->isConnected()) {
        return;
    }
    
    // Update account summary table
    auto accountSummary = ibConnector->getAccountSummary();
    accountTable->setRowCount(0);
    
    for (const auto& item : accountSummary) {
        int row = accountTable->rowCount();
        accountTable->insertRow(row);
        
        auto* accountItem = new QTableWidgetItem(QString::fromStdString(item.account));
        accountItem->setForeground(Qt::black);
        accountTable->setItem(row, 0, accountItem);
        
        auto* tagItem = new QTableWidgetItem(QString::fromStdString(item.tag));
        tagItem->setForeground(Qt::black);
        accountTable->setItem(row, 1, tagItem);
        
        auto* valueItem = new QTableWidgetItem(QString::fromStdString(item.value));
        valueItem->setForeground(Qt::black);
        accountTable->setItem(row, 2, valueItem);
        
        auto* currencyItem = new QTableWidgetItem(QString::fromStdString(item.currency));
        currencyItem->setForeground(Qt::black);
        accountTable->setItem(row, 3, currencyItem);
    }
    
    // Update positions table
    auto positions = ibConnector->getPositions();
    positionsTable->setRowCount(0);
    
    for (const auto& pos : positions) {
        int row = positionsTable->rowCount();
        positionsTable->insertRow(row);
        
        auto* accountItem = new QTableWidgetItem(QString::fromStdString(pos.account));
        accountItem->setForeground(Qt::black);
        positionsTable->setItem(row, 0, accountItem);
        
        auto* symbolItem = new QTableWidgetItem(QString::fromStdString(pos.contract.symbol));
        symbolItem->setForeground(Qt::black);
        positionsTable->setItem(row, 1, symbolItem);
        
        auto* positionItem = new QTableWidgetItem(QString::number(pos.position));
        positionItem->setForeground(Qt::black);
        positionsTable->setItem(row, 2, positionItem);
        
        auto* avgCostItem = new QTableWidgetItem(QString::number(pos.avgCost, 'f', 2));
        avgCostItem->setForeground(Qt::black);
        positionsTable->setItem(row, 3, avgCostItem);
        
        double value = pos.position * pos.avgCost;
        auto* valueItem = new QTableWidgetItem(QString::number(value, 'f', 2));
        valueItem->setForeground(Qt::black);
        positionsTable->setItem(row, 4, valueItem);
    }
}