#include "IBConnector.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include "EReaderOSSignal.h"
#include "EReader.h"

IBConnector::IBConnector() 
    : connected(false)
    , nextOrderId(1)
    , shouldProcessMessages(false)
    , connectionEstablished(false) {
    
    signal = std::make_unique<EReaderOSSignal>(2000);
    client = std::make_unique<EClientSocket>(this, signal.get());
    log("IBConnector initialized");
}

IBConnector::~IBConnector() {
    disconnect();
}

bool IBConnector::connect(const std::string& host, int port, int clientId) {
    log("Attempting to connect to " + host + ":" + std::to_string(port) + " with client ID " + std::to_string(clientId));
    
    if (connected) {
        log("Already connected");
        return true;
    }
    
    // Attempt connection
    bool success = client->eConnect(host.c_str(), port, clientId, false);
    
    if (!success) {
        log("Failed to establish socket connection");
        return false;
    }
    
    // Create reader after successful connection
    reader = std::unique_ptr<EReader>(new EReader(client.get(), signal.get()));
    reader->start();
    
    // Start message processing thread
    shouldProcessMessages = true;
    messageProcessingThread = std::thread(&IBConnector::processMessages, this);
    
    // Wait for connection acknowledgment
    std::unique_lock<std::mutex> lock(connectionMutex);
    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    
    if (connectionCV.wait_until(lock, timeout, [this] { return connectionEstablished.load(); })) {
        connected = true;
        log("Successfully connected to IB");
        
        // Request initial data
        client->reqManagedAccts();
        
        // Request account summary
        requestAccountSummary();
        
        // Request positions
        requestPositions();
        
        // Request delayed market data for Apple stock (free)
        client->reqMarketDataType(3); // 3 = Delayed data
        
        Contract appleContract;
        appleContract.symbol = "AAPL";
        appleContract.secType = "STK";
        appleContract.currency = "USD";
        appleContract.exchange = "SMART";
        requestMarketData(1, appleContract);
        
        return true;
    } else {
        log("Connection timeout");
        disconnect();
        return false;
    }
}

void IBConnector::disconnect() {
    if (!connected) {
        return;
    }
    
    log("Disconnecting from IB");
    
    connected = false;
    connectionEstablished = false;
    shouldProcessMessages = false;
    
    if (client->isConnected()) {
        client->eDisconnect();
    }
    
    if (messageProcessingThread.joinable()) {
        messageProcessingThread.join();
    }
    
    clearData();
    log("Disconnected from IB");
}

bool IBConnector::isConnected() const {
    return connected && client->isConnected();
}

void IBConnector::processMessages() {
    while (shouldProcessMessages) {
        if (client->isConnected() && reader) {
            signal->waitForSignal();
            errno = 0;
            reader->processMsgs();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void IBConnector::nextValidId(OrderId orderId) {
    log("Next valid order ID: " + std::to_string(orderId));
    nextOrderId = orderId;
    
    // Signal that connection is established
    {
        std::lock_guard<std::mutex> lock(connectionMutex);
        connectionEstablished = true;
    }
    connectionCV.notify_one();
}

void IBConnector::connectAck() {
    log("Connection acknowledged by TWS/Gateway");
}

void IBConnector::connectionClosed() {
    log("Connection closed by TWS/Gateway");
    connected = false;
    connectionEstablished = false;
}

void IBConnector::error(int id, int errorCode, const std::string& errorString) {
    std::string logMsg = "Error " + std::to_string(errorCode) + ": " + errorString;
    if (id != -1) {
        logMsg += " (ID: " + std::to_string(id) + ")";
    }
    log(logMsg);
    
    // Handle connection errors
    if (errorCode == 502 || errorCode == 503 || errorCode == 504) {
        log("Connection error detected");
        connected = false;
        connectionEstablished = false;
    }
}

void IBConnector::managedAccounts(const std::string& accountsList) {
    std::lock_guard<std::mutex> lock(dataMutex);
    managedAccountsList.clear();
    
    std::istringstream ss(accountsList);
    std::string account;
    
    while (std::getline(ss, account, ',')) {
        if (!account.empty()) {
            managedAccountsList.push_back(account);
        }
    }
    
    log("Managed accounts: " + accountsList);
}

void IBConnector::requestAccountSummary() {
    if (!isConnected()) {
        log("Not connected - cannot request account summary");
        return;
    }
    
    std::lock_guard<std::mutex> lock(dataMutex);
    accountSummaryData.clear();
    
    // Request account summary for all accounts
    client->reqAccountSummary(1, "All", "NetLiquidation,TotalCashValue,SettledCash,AccruedCash,BuyingPower,EquityWithLoanValue,PreviousEquityWithLoanValue,GrossPositionValue");
    
    log("Requested account summary");
}

void IBConnector::accountSummary(int reqId, const std::string& account, const std::string& tag,
                                const std::string& value, const std::string& currency) {
    std::lock_guard<std::mutex> lock(dataMutex);
    accountSummaryData.push_back({account, tag, value, currency});
    
    // Only log important account info
    if (tag == "NetLiquidation" || tag == "TotalCashValue" || tag == "BuyingPower" || 
        tag == "AvailableFunds" || tag == "GrossPositionValue") {
        log("Account " + account + " - " + tag + ": $" + value);
    }
}

void IBConnector::accountSummaryEnd(int reqId) {
    log("Account summary complete");
}

void IBConnector::requestPositions() {
    if (!isConnected()) {
        log("Not connected - cannot request positions");
        return;
    }
    
    std::lock_guard<std::mutex> lock(dataMutex);
    positionsData.clear();
    
    client->reqPositions();
    log("Requested positions");
}

void IBConnector::position(const std::string& account, const Contract& contract,
                          double position, double avgCost) {
    std::lock_guard<std::mutex> lock(dataMutex);
    positionsData.push_back({account, contract, position, avgCost});
    
    log("Position: " + account + " " + contract.symbol + " " + std::to_string(position) + " @ " + std::to_string(avgCost));
}

void IBConnector::positionEnd() {
    log("Positions complete");
}

void IBConnector::requestMarketData(int tickerId, const Contract& contract) {
    if (!isConnected()) {
        log("Not connected - cannot request market data");
        return;
    }
    
    client->reqMktData(tickerId, contract, "", false, false, TagValueListSPtr());
    log("Requested market data for " + contract.symbol + " (ID: " + std::to_string(tickerId) + ")");
}

void IBConnector::cancelMarketData(int tickerId) {
    if (!isConnected()) {
        return;
    }
    
    client->cancelMktData(tickerId);
    log("Cancelled market data for ID: " + std::to_string(tickerId));
}

void IBConnector::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        tickPrices[tickerId * 100 + field] = price;
    }
    
    std::string fieldName;
    switch (field) {
        case 1: fieldName = "BID"; break;
        case 2: fieldName = "ASK"; break;
        case 4: fieldName = "LAST"; break;
        default: fieldName = "FIELD_" + std::to_string(field); break;
    }
    
    // Only log significant price updates to avoid spam
    if (field == 1 || field == 2 || field == 4) {
        log("AAPL " + fieldName + ": $" + std::to_string(price));
    }
}

void IBConnector::tickSize(TickerId tickerId, TickType field, int size) {
    tickSizes[tickerId * 100 + field] = size;
}

void IBConnector::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
    // Handle string-based tick data
}

void IBConnector::placeOrder(int orderId, const Contract& contract, const Order& order) {
    if (!isConnected()) {
        log("Not connected - cannot place order");
        return;
    }
    
    client->placeOrder(orderId, contract, order);
    log("Placed order " + std::to_string(orderId) + " for " + contract.symbol);
}

void IBConnector::cancelOrder(int orderId) {
    if (!isConnected()) {
        log("Not connected - cannot cancel order");
        return;
    }
    
    client->cancelOrder(orderId);
    log("Cancelled order " + std::to_string(orderId));
}

void IBConnector::requestAllOpenOrders() {
    if (!isConnected()) {
        log("Not connected - cannot request open orders");
        return;
    }
    
    std::lock_guard<std::mutex> lock(dataMutex);
    openOrdersData.clear();
    
    client->reqAllOpenOrders();
    log("Requested all open orders");
}

void IBConnector::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Find existing order or create new entry
    auto it = std::find_if(openOrdersData.begin(), openOrdersData.end(),
                          [orderId](const OrderInfo& info) { return info.orderId == orderId; });
    
    if (it != openOrdersData.end()) {
        it->contract = contract;
        it->order = order;
        it->orderState = orderState;
    } else {
        OrderInfo info;
        info.orderId = orderId;
        info.contract = contract;
        info.order = order;
        info.orderState = orderState;
        openOrdersData.push_back(info);
    }
    
    log("Open order: " + std::to_string(orderId) + " " + contract.symbol + " " + order.action + " " + std::to_string(order.totalQuantity));
}

void IBConnector::openOrderEnd() {
    log("Open orders complete");
}

void IBConnector::orderStatus(OrderId orderId, const std::string& status, double filled,
                             double remaining, double avgFillPrice, int permId, int parentId,
                             double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Update order status
    auto it = std::find_if(openOrdersData.begin(), openOrdersData.end(),
                          [orderId](const OrderInfo& info) { return info.orderId == orderId; });
    
    if (it != openOrdersData.end()) {
        it->status = status;
        it->filled = filled;
        it->remaining = remaining;
        it->avgFillPrice = avgFillPrice;
    }
    
    log("Order status: " + std::to_string(orderId) + " " + status + " filled: " + std::to_string(filled) + 
        " remaining: " + std::to_string(remaining) + " avg price: " + std::to_string(avgFillPrice));
}

std::vector<std::string> IBConnector::getManagedAccounts() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return managedAccountsList;
}

std::vector<IBConnector::AccountSummaryItem> IBConnector::getAccountSummary() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return accountSummaryData;
}

std::vector<IBConnector::PositionItem> IBConnector::getPositions() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return positionsData;
}

std::vector<IBConnector::OrderInfo> IBConnector::getOpenOrders() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return openOrdersData;
}

std::map<int, double> IBConnector::getTickPrices() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return tickPrices;
}

void IBConnector::clearData() {
    std::lock_guard<std::mutex> lock(dataMutex);
    managedAccountsList.clear();
    accountSummaryData.clear();
    positionsData.clear();
    openOrdersData.clear();
    tickPrices.clear();
    tickSizes.clear();
}

void IBConnector::log(const std::string& message) const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] " << message << std::endl;
}