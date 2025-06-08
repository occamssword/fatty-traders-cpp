#include "TradingApp.h"
#include <iostream>
#include <thread>
#include <chrono>

TradingApp::TradingApp() 
    : running(false)
    , initialized(false) {
    connector = std::make_unique<IBConnector>();
}

TradingApp::~TradingApp() {
    shutdown();
}

bool TradingApp::initialize(const std::string& host, int port, int clientId) {
    if (initialized) {
        return true;
    }
    
    std::cout << "Initializing FattyTraders Trading App..." << std::endl;
    
    if (connector->connect(host, port, clientId)) {
        initialized = true;
        running = true;
        std::cout << "Successfully initialized!" << std::endl;
        return true;
    }
    
    std::cout << "Failed to initialize - check connection settings" << std::endl;
    return false;
}

void TradingApp::run() {
    if (!initialized) {
        std::cout << "App not initialized. Call initialize() first." << std::endl;
        return;
    }
    
    while (running) {
        showMainMenu();
        handleUserInput();
    }
}

void TradingApp::shutdown() {
    if (connector && connector->isConnected()) {
        connector->disconnect();
    }
    running = false;
    initialized = false;
}

void TradingApp::showMainMenu() {
    std::cout << "\n=== FattyTraders Trading System ===" << std::endl;
    std::cout << "1. Account Summary" << std::endl;
    std::cout << "2. Positions" << std::endl;
    std::cout << "3. Market Data" << std::endl;
    std::cout << "4. Orders" << std::endl;
    std::cout << "5. Settings" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Choice: ";
}

void TradingApp::handleUserInput() {
    int choice;
    std::cin >> choice;
    
    switch (choice) {
    case 1:
        connector->requestAccountSummary();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            auto summary = connector->getAccountSummary();
            for (const auto& item : summary) {
                std::cout << item.tag << ": " << item.value << " " << item.currency << std::endl;
            }
        }
        break;
        
    case 2:
        connector->requestPositions();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            auto positions = connector->getPositions();
            for (const auto& pos : positions) {
                std::cout << pos.contract.symbol << ": " << pos.position << " @ " << pos.avgCost << std::endl;
            }
        }
        break;
        
    case 0:
        running = false;
        break;
        
    default:
        std::cout << "Invalid choice" << std::endl;
    }
}