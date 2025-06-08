#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include "IBConnector.h"
#include <memory>
#include <map>
#include <mutex>

class OrderManager {
public:
    OrderManager(std::shared_ptr<IBConnector> connector);
    ~OrderManager();
    
    // Order management functions
    int placeMarketOrder(const std::string& symbol, const std::string& action, double quantity);
    int placeLimitOrder(const std::string& symbol, const std::string& action, double quantity, double price);
    void cancelOrder(int orderId);
    void cancelAllOrders();
    
    // Order queries
    std::vector<IBConnector::OrderInfo> getOpenOrders();
    std::string getOrderStatus(int orderId);
    
private:
    std::shared_ptr<IBConnector> ibConnector;
    std::map<int, IBConnector::OrderInfo> orderCache;
    mutable std::mutex orderMutex;
    
    Contract createStockContract(const std::string& symbol);
    Order createMarketOrder(const std::string& action, double quantity);
    Order createLimitOrder(const std::string& action, double quantity, double price);
};

#endif // ORDERMANAGER_H