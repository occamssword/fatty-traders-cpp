# FattyTraders C++ IB Connector

A high-performance C++ connector for Interactive Brokers, designed for reliable connections and fast execution.

## Features

- **Native C++ Performance**: Direct IB API integration without Python overhead
- **Reliable Connections**: Better connection stability compared to Python wrappers
- **Real-time Data**: Market data, account info, and order management
- **FA Account Support**: Full Financial Advisor account functionality
- **Cross-platform**: Works on macOS, Linux, and Windows

## Quick Start

### 1. Setup IB API

```bash
cd cpp_connector
./setup_ib_api.sh
```

This will download and build the Interactive Brokers C++ API automatically.

### 2. Build the Connector

```bash
mkdir build
cd build
cmake ..
make
```

### 3. Run the Application

```bash
./fatty_traders
```

## Prerequisites

- **macOS**: Xcode Command Line Tools (`xcode-select --install`)
- **CMake**: Install via Homebrew (`brew install cmake`)
- **TWS/Gateway**: Download from Interactive Brokers
- **API Access**: Enabled in TWS/Gateway settings

## Connection Ports

| Service | Port | Description |
|---------|------|-------------|
| TWS Paper | 7497 | Paper trading via TWS |
| TWS Live | 7496 | Live trading via TWS |
| Gateway Paper | 4002 | Paper trading via Gateway |
| Gateway Live | 4001 | Live trading via Gateway |

## Usage

1. **Start TWS/Gateway** and enable API access
2. **Run the connector**: `./fatty_traders`
3. **Connect** using option 1 in the menu
4. **Test functionality** using the menu options

### Menu Options

1. **Connect to IB** - Establish connection
2. **Get Account Summary** - View account metrics
3. **Get Positions** - Show current holdings
4. **Get Market Data** - Subscribe to price feeds
5. **Place Test Order** - Submit orders (paper trading recommended)
6. **Get Open Orders** - View pending orders
7. **Disconnect** - Close connection

## Configuration

### TWS/Gateway API Settings

1. **File → Global Configuration → API → Settings**
2. **Enable** "ActiveX and Socket Clients"
3. **Set Port** (see table above)
4. **Add** 127.0.0.1 to trusted IPs
5. **Restart** TWS/Gateway

### FA Account Setup

For Financial Advisor accounts:
- Ensure FA permissions are granted
- Use appropriate account codes
- Configure allocation methods in TWS

## Troubleshooting

### Connection Issues

**"Connection refused"**
- Check TWS/Gateway is running
- Verify API is enabled
- Confirm correct port number

**"Connection timeout"**
- TWS/Gateway may be running but API disabled
- Check firewall settings
- Verify IP address in trusted list

### Build Issues

**"make not found"**
```bash
xcode-select --install
```

**"cmake not found"**
```bash
brew install cmake
```

**Library build fails**
- Check Xcode Command Line Tools are installed
- Verify IB API downloaded correctly
- Try manual build in `IBApi/source/cppclient`

### Runtime Issues

**"No market data"**
- Market data subscriptions required for real-time data
- Paper trading accounts have limited data
- Check market hours and data permissions

**"Order rejected"**
- Verify account has trading permissions
- Check order parameters (price, quantity)
- Use paper trading for testing

## Development

### Project Structure

```
cpp_connector/
├── src/
│   ├── main.cpp           # Main application
│   ├── IBConnector.h/cpp  # IB API wrapper
│   └── ...
├── IBApi/                 # IB API (auto-downloaded)
├── build/                 # Build directory
└── CMakeLists.txt         # Build configuration
```

### Adding Features

1. **New Order Types**: Extend `IBConnector::placeOrder()`
2. **Market Data**: Add tick processing in `tickPrice()`
3. **Account Management**: Enhance FA account handling
4. **GUI Integration**: Connect to UI frameworks

### Thread Safety

The connector uses thread-safe patterns:
- Mutex protection for shared data
- Atomic variables for flags
- Separate message processing thread

## Performance Notes

- **Low Latency**: Direct C++ API calls
- **Memory Efficient**: Minimal overhead
- **Scalable**: Handle multiple connections
- **Reliable**: Better error handling than Python

## Security

- **Local Connections**: Only connects to localhost
- **API Keys**: Not required for IB API
- **Credentials**: Handled by TWS/Gateway
- **Permissions**: Configurable in TWS

## Support

For issues:
1. **Check logs** in console output
2. **Verify TWS/Gateway** settings
3. **Test connection** with menu option 1
4. **Review IB documentation** for API limits

## License

Educational use. Ensure compliance with Interactive Brokers API terms of service.