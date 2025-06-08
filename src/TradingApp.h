#ifndef TRADINGAPP_H
#define TRADINGAPP_H

#include "IBConnector.h"
#include <memory>
#include <string>

class TradingApp {
public:
    TradingApp();
    ~TradingApp();
    
    bool initialize(const std::string& host = "127.0.0.1", int port = 7497, int clientId = 1);
    void run();
    void shutdown();
    
    bool isRunning() const { return running; }
    
private:
    std::unique_ptr<IBConnector> connector;
    bool running;
    bool initialized;
    
    void showMainMenu();
    void handleUserInput();
};

#endif // TRADINGAPP_H