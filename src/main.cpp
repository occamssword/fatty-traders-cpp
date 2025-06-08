#include "IBConnector.h"
#include "Contract.h"
#include "Order.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

void printMenu() {
    std::cout << "\n=== FattyTraders IB Connector ===" << std::endl;
    std::cout << "1. Connect to IB" << std::endl;
    std::cout << "2. Get Account Summary" << std::endl;
    std::cout << "3. Get Positions" << std::endl;
    std::cout << "4. Get Market Data (AAPL)" << std::endl;
    std::cout << "5. Place Test Order (Paper trading only!)" << std::endl;
    std::cout << "6. Get Open Orders" << std::endl;
    std::cout << "7. Disconnect" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

Contract createStockContract(const std::string& symbol, const std::string& exchange = "SMART") {
    Contract contract;
    contract.symbol = symbol;
    contract.secType = "STK";
    contract.exchange = exchange;
    contract.currency = "USD";
    return contract;
}

Order createMarketOrder(const std::string& action, double quantity) {
    Order order;
    order.action = action;
    order.orderType = "MKT";
    order.totalQuantity = quantity;
    return order;
}

Order createLimitOrder(const std::string& action, double quantity, double price) {
    Order order;
    order.action = action;
    order.orderType = "LMT";
    order.totalQuantity = quantity;
    order.lmtPrice = price;
    return order;
}

int main() {
    std::cout << "FattyTraders - Interactive Brokers C++ Connector" << std::endl;
    std::cout << "=================================================" << std::endl;
    
    IBConnector connector;
    bool connected = false;
    int marketDataId = 1001;
    
    while (true) {
        printMenu();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
        case 1: {
            if (connected) {
                std::cout << "Already connected!" << std::endl;
                break;
            }
            
            std::string host;
            int port, clientId;
            
            std::cout << "Enter host (127.0.0.1): ";
            std::cin >> host;
            if (host.empty()) host = "127.0.0.1";
            
            std::cout << "Enter port (4001 for Gateway Live, 4002 for Gateway Paper, 7497 for TWS Paper): ";
            std::cin >> port;
            
            std::cout << "Enter client ID (1): ";
            std::cin >> clientId;
            
            std::cout << "Connecting to " << host << ":" << port << " with client ID " << clientId << "..." << std::endl;
            
            connected = connector.connect(host, port, clientId);
            
            if (connected) {
                std::cout << "✅ Connected successfully!" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2)); // Allow time for account data
                
                auto accounts = connector.getManagedAccounts();
                std::cout << "Managed accounts: ";
                for (const auto& account : accounts) {
                    std::cout << account << " ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "❌ Connection failed!" << std::endl;
            }
            break;
        }
        
        case 2: {
            if (!connected) {
                std::cout << "Not connected! Connect first." << std::endl;
                break;
            }
            
            std::cout << "Requesting account summary..." << std::endl;
            connector.requestAccountSummary();
            
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for data
            
            auto summary = connector.getAccountSummary();
            std::cout << "\nAccount Summary:" << std::endl;
            std::cout << "=================" << std::endl;
            
            for (const auto& item : summary) {
                std::cout << item.account << " | " << item.tag << ": " 
                         << item.value << " " << item.currency << std::endl;
            }
            
            if (summary.empty()) {
                std::cout << "No account summary data received." << std::endl;
            }
            break;
        }
        
        case 3: {
            if (!connected) {
                std::cout << "Not connected! Connect first." << std::endl;
                break;
            }
            
            std::cout << "Requesting positions..." << std::endl;
            connector.requestPositions();
            
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for data
            
            auto positions = connector.getPositions();
            std::cout << "\nPositions:" << std::endl;
            std::cout << "==========" << std::endl;
            
            for (const auto& pos : positions) {
                std::cout << pos.account << " | " << pos.contract.symbol 
                         << " (" << pos.contract.secType << "): " 
                         << pos.position << " @ $" << pos.avgCost << std::endl;
            }
            
            if (positions.empty()) {
                std::cout << "No positions found." << std::endl;
            }
            break;
        }
        
        case 4: {
            if (!connected) {
                std::cout << "Not connected! Connect first." << std::endl;
                break;
            }
            
            Contract aaplContract = createStockContract("AAPL");
            
            std::cout << "Requesting market data for AAPL..." << std::endl;
            connector.requestMarketData(marketDataId, aaplContract);
            
            std::cout << "Market data requested. Check console for price updates." << std::endl;
            std::cout << "Note: You need market data subscription for real-time data." << std::endl;
            break;
        }
        
        case 5: {
            if (!connected) {
                std::cout << "Not connected! Connect first." << std::endl;
                break;
            }
            
            std::cout << "⚠️  WARNING: This will place a real order!" << std::endl;
            std::cout << "Only use this with paper trading accounts!" << std::endl;
            std::cout << "Continue? (y/N): ";
            
            char confirm;
            std::cin >> confirm;
            
            if (confirm == 'y' || confirm == 'Y') {
                Contract contract = createStockContract("AAPL");
                Order order = createLimitOrder("BUY", 1, 100.0); // Buy 1 share at $100
                
                int orderId = connector.getNextValidOrderId();
                
                std::cout << "Placing test order: BUY 1 AAPL @ $100.00 (Order ID: " << orderId << ")" << std::endl;
                connector.placeOrder(orderId, contract, order);
                
                std::cout << "Order placed! Check TWS/Gateway for confirmation." << std::endl;
            } else {
                std::cout << "Order cancelled." << std::endl;
            }
            break;
        }
        
        case 6: {
            if (!connected) {
                std::cout << "Not connected! Connect first." << std::endl;
                break;
            }
            
            std::cout << "Requesting open orders..." << std::endl;
            connector.requestAllOpenOrders();
            
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for data
            
            auto orders = connector.getOpenOrders();
            std::cout << "\nOpen Orders:" << std::endl;
            std::cout << "============" << std::endl;
            
            for (const auto& orderInfo : orders) {
                std::cout << "Order " << orderInfo.orderId << ": " 
                         << orderInfo.order.action << " " << orderInfo.order.totalQuantity 
                         << " " << orderInfo.contract.symbol 
                         << " @ " << orderInfo.order.lmtPrice 
                         << " (" << orderInfo.status << ")" << std::endl;
            }
            
            if (orders.empty()) {
                std::cout << "No open orders found." << std::endl;
            }
            break;
        }
        
        case 7: {
            if (connected) {
                std::cout << "Disconnecting..." << std::endl;
                connector.disconnect();
                connected = false;
                std::cout << "Disconnected." << std::endl;
            } else {
                std::cout << "Not connected." << std::endl;
            }
            break;
        }
        
        case 0: {
            if (connected) {
                std::cout << "Disconnecting..." << std::endl;
                connector.disconnect();
            }
            std::cout << "Goodbye!" << std::endl;
            return 0;
        }
        
        default: {
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
    
    return 0;
}