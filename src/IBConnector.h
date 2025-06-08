#pragma once

#include "DefaultEWrapper.h"
#include "EClientSocket.h"
#include "Contract.h"
#include "Order.h"
#include "OrderState.h"
#include "EReaderOSSignal.h"
#include "EReader.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

class IBConnector : public DefaultEWrapper {
public:
    IBConnector();
    ~IBConnector();

    // Connection management
    bool connect(const std::string& host = "127.0.0.1", int port = 4001, int clientId = 1);
    void disconnect();
    bool isConnected() const;
    
    // Account management
    std::vector<std::string> getManagedAccounts() const;
    void requestAccountSummary();
    void requestPositions();
    
    // Market data
    void requestMarketData(int tickerId, const Contract& contract);
    void cancelMarketData(int tickerId);
    
    // Orders
    void placeOrder(int orderId, const Contract& contract, const Order& order);
    void cancelOrder(int orderId);
    void requestAllOpenOrders();
    
    // EWrapper interface implementation
    void nextValidId(OrderId orderId) override;
    void connectAck() override;
    void connectionClosed() override;
    void error(int id, int errorCode, const std::string& errorString) override;
    
    // Account callbacks
    void managedAccounts(const std::string& accountsList) override;
    void accountSummary(int reqId, const std::string& account, const std::string& tag,
                       const std::string& value, const std::string& currency) override;
    void accountSummaryEnd(int reqId) override;
    void position(const std::string& account, const Contract& contract,
                 double position, double avgCost) override;
    void positionEnd() override;
    
    // Market data callbacks
    void tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib& attribs) override;
    void tickSize(TickerId tickerId, TickType field, int size) override;
    void tickString(TickerId tickerId, TickType tickType, const std::string& value) override;
    
    // Order callbacks
    void openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) override;
    void openOrderEnd() override;
    void orderStatus(OrderId orderId, const std::string& status, double filled,
                    double remaining, double avgFillPrice, int permId, int parentId,
                    double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) override;
    
    // Getters for data
    OrderId getNextValidOrderId() const { return nextOrderId; }
    
    struct AccountSummaryItem {
        std::string account;
        std::string tag;
        std::string value;
        std::string currency;
    };
    
    struct PositionItem {
        std::string account;
        Contract contract;
        double position;
        double avgCost;
    };
    
    struct OrderInfo {
        OrderId orderId;
        Contract contract;
        Order order;
        OrderState orderState;
        std::string status;
        double filled;
        double remaining;
        double avgFillPrice;
    };
    
    std::vector<AccountSummaryItem> getAccountSummary() const;
    std::vector<PositionItem> getPositions() const;
    std::vector<OrderInfo> getOpenOrders() const;
    std::map<int, double> getTickPrices() const;

private:
    std::unique_ptr<EClientSocket> client;
    std::unique_ptr<EReaderOSSignal> signal;
    std::unique_ptr<EReader> reader;
    std::atomic<bool> connected;
    std::atomic<OrderId> nextOrderId;
    
    // Data storage
    mutable std::mutex dataMutex;
    std::vector<std::string> managedAccountsList;
    std::vector<AccountSummaryItem> accountSummaryData;
    std::vector<PositionItem> positionsData;
    std::vector<OrderInfo> openOrdersData;
    
    // Market data
    std::map<int, double> tickPrices;
    std::map<int, int> tickSizes;
    
    // Threading
    std::thread messageProcessingThread;
    std::atomic<bool> shouldProcessMessages;
    void processMessages();
    
    // Synchronization
    std::mutex connectionMutex;
    std::condition_variable connectionCV;
    std::atomic<bool> connectionEstablished;
    
    // Helper methods
    void clearData();
    void log(const std::string& message) const;
};