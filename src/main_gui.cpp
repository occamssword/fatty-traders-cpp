#include <QApplication>
#include <memory>
#include <iostream>
#include "ConnectionStatusGUI.h"
#include "IBConnector.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application style
    app.setStyle("Fusion");
    
    // Create IB connector instance
    auto ibConnector = std::make_shared<IBConnector>();
    
    // Create and show GUI
    ConnectionStatusGUI window;
    window.setConnector(ibConnector);
    window.show();
    
    // Log startup
    window.addLogMessage("IB Gateway Connection Monitor started");
    window.addLogMessage("Ready to connect to IB Gateway on port 4001");
    
    return app.exec();
}