# Arduino Financial Dashboard 📈

A real-time financial asset tracker that cycles through cryptocurrencies and stocks on a 3.5" TFT display. Shows current prices, daily dollar changes, and motivational messaging.

## ✨ Features

- **Real-time price tracking** for 5 assets (expandable)
- **Portfolio mode** - Toggle between individual assets or combined portfolio view
- **Dual API integration**: CoinGecko for crypto, Finnhub for stocks  
- **Dynamic visual feedback** - Border and text colors reflect market performance
- **Enhanced typography** - Larger, more readable text for names and changes
- **Dollar change display** with vivid color coding (green gains, red losses)
- **Auto-cycling display** every 15 seconds with "Win Each Day" messaging
- **Color-corrected display** with proper RGB channel ordering
- **Robust error handling** with automatic retry logic
- **WiFi connectivity** with automatic reconnection
- **Clean, readable interface** optimized for trading desk viewing

## 🎯 Default Assets Tracked

- **SOL** (Solana) - Cryptocurrency
- **SHIB** (Shiba Inu) - Cryptocurrency  
- **MSFT** (Microsoft) - Stock
- **PLTR** (Palantir) - Stock
- **SNOW** (Snowflake) - Stock

## 💼 Portfolio Mode

**NEW!** Switch between individual asset tracking and portfolio overview:

### Individual Mode (Default)
- Cycles through each asset every 15 seconds
- Shows individual price and daily change
- Perfect for monitoring specific assets

### Portfolio Mode  
- Shows total portfolio value and daily P&L
- Aggregates all your holdings into one view
- Dynamic colors reflect overall portfolio performance
- Green border/text for profitable days, red for losses

### Setup Portfolio Mode
1. **Configure holdings** in `config.h`:
   ```cpp
   const bool PORTFOLIO_MODE = true;  // Enable portfolio view
   const float SOL_HOLDINGS = 25.0;   // Your actual SOL holdings
   const float MSFT_HOLDINGS = 10.0;  // Your actual MSFT shares
   // ... set all your holdings
   ```

2. **Example calculation**:
   - 25 SOL @ $150 = $3,750
   - 10 MSFT @ $420 = $4,200  
   - **Total Portfolio: $7,950**
   - **Daily Change: +$125** (green border/text)

## 🛠️ Hardware Requirements

### Essential Components
- **Arduino WiFi R4** (or compatible board with WiFi)
- **3.5" ILI9488 TFT Display** (480x320 resolution)
- **Breadboard jumper wires**
- **5V power supply** (USB or external)

### Display Connection (UNOPAR8 interface)
```
ILI9488 → Arduino WiFi R4
VCC     → 5V
GND     → GND
CS      → Pin 10
RESET   → Pin 9  
DC      → Pin 8
SDI     → Pin 11 (MOSI)
SCK     → Pin 13 (SCK)
LED     → 3.3V
```

## 📦 Software Setup

### 1. Arduino IDE Setup
1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. Install Arduino WiFi R4 board package:
   - Go to Tools → Board → Boards Manager
   - Search "Arduino WiFi R4" and install

### 2. Required Libraries
Install these libraries via Library Manager (Tools → Manage Libraries):
```
- Arduino_GFX_Library (by Moon On Our Nation)
- WiFiS3 (usually pre-installed)
- ArduinoHttpClient
- ArduinoJson
```

### 3. Project Setup
1. **Clone this repository**
   ```bash
   git clone https://github.com/yourusername/arduino-financial-dashboard.git
   ```

2. **Create your config file**
   ```bash
   cd arduino-financial-dashboard/sketch_aug13b
   cp config_template.h config.h
   ```

3. **Edit config.h with your details**
   ```cpp
   const char* WIFI_SSID = "YourWiFiNetwork";
   const char* WIFI_PASS = "YourWiFiPassword"; 
   const char* FINNHUB_API_KEY = "your_api_key_here";
   ```

## 🔑 API Setup

### CoinGecko (Cryptocurrency Data)
- **Cost**: FREE ✅
- **Setup**: No account required
- **Rate Limit**: 30 calls/minute
- **Usage**: Automatic for SOL and SHIB prices

### Finnhub (Stock Data)  
- **Cost**: FREE tier available ✅
- **Setup Required**:
  1. Go to [finnhub.io](https://finnhub.io)
  2. Create free account
  3. Get your API key from dashboard
  4. Add to `config.h`
- **Rate Limit**: 60 calls/minute (free tier)
- **Usage**: Stock prices for MSFT, PLTR, SNOW

## 🎨 Customization Guide

### Adding New Stocks

1. **Add global variables** (after line 35):
   ```cpp
   float g_lastYourStockPrice = NAN;
   float g_lastYourStockChange = NAN;
   ```

2. **Update asset count** (line 36):
   ```cpp
   uint8_t g_currentAsset = 0; // 0 = SOL, 1 = SHIB, 2 = MSFT, 3 = PLTR, 4 = SNOW, 5 = YOURSTOCKHERE
   ```

3. **Add to fetchStocks()** function:
   ```cpp
   bool yourStockSuccess = fetchStock("YOUR_SYMBOL", g_lastYourStockPrice, g_lastYourStockChange);
   ```

4. **Add display case** in `drawCurrentAsset()`:
   ```cpp
   case 5: // Your Stock
     if (!isnan(g_lastYourStockPrice)) {
       drawCrypto("Company Name", "SYMBOL", g_lastYourStockPrice, g_lastYourStockChange, statusLine);
     } else {
       drawMessage("Company Name (SYMBOL)", "No data available");
     }
     break;
   ```

5. **Update cycling logic** (line 383):
   ```cpp
   g_currentAsset = (g_currentAsset + 1) % 6; // Update to new total count
   ```

### Adding New Cryptocurrencies

1. **Update CoinGecko API path** (line 10):
   ```cpp
   const char* CRYPTO_PATH = "/api/v3/simple/price?ids=solana,shiba-inu,your-crypto-id&vs_currencies=usd&include_24hr_change=true";
   ```

2. **Add parsing** in `fetchCrypto()` function:
   ```cpp
   if (doc.containsKey("your-crypto-id")) {
     g_lastYourCryptoPrice = doc["your-crypto-id"]["usd"].as<float>();
     float percentChange = doc["your-crypto-id"]["usd_24h_change"].as<float>();
     g_lastYourCryptoChange = g_lastYourCryptoPrice * (percentChange / 100.0);
   }
   ```

> **Finding Crypto IDs**: Search [CoinGecko](https://coingecko.com) for your token, the ID is in the URL (e.g., `ethereum`, `bitcoin`, `cardano`)

### Memory & Performance Limits

- **Theoretical Max**: ~50-100 assets (memory permitting)
- **Practical Limit**: 10-15 assets for good UX
- **API Limits**: Well within free tier limits even with 20+ assets
- **Cycle Time**: 15 seconds × number of assets = full rotation time

## ⚙️ Configuration Options

### Refresh Intervals
```cpp
// Data refresh (line ~383)
if (millis() - lastFetch > 1800000UL) // 30 minutes default

// Display cycling (line ~381)  
if (millis() - lastCycle > 15000UL) // 15 seconds default
```

### Display Settings
```cpp
// In config.h
const uint8_t ROTATION = 1; // 0=0°, 1=90°, 2=180°, 3=270°
```

### Portfolio Settings
```cpp
// Portfolio mode toggle
const bool PORTFOLIO_MODE = false; // true = portfolio view, false = individual assets

// Your actual holdings (adjust to match your investments)
const float SOL_HOLDINGS = 10.0;       // SOL coins owned
const float SHIB_HOLDINGS = 1000000.0; // SHIB coins owned
const float MSFT_HOLDINGS = 5.0;       // MSFT shares owned
const float PLTR_HOLDINGS = 50.0;      // PLTR shares owned  
const float SNOW_HOLDINGS = 3.0;       // SNOW shares owned
```

## 🔧 Troubleshooting

### Common Issues

**WiFi Connection Problems**
- Check SSID and password in `config.h`
- Ensure 2.4GHz network (Arduino doesn't support 5GHz)
- Move closer to router during setup

**API Errors**
- **Error 403**: Invalid API key or rate limit exceeded
- **Error 429**: Too many requests, built-in retry logic handles this
- **Error 500**: API server issues, retry logic will attempt again

**Display Issues**  
- Check wiring connections
- Verify display orientation with ROTATION setting
- Ensure 5V power supply can handle display current

**Color Issues (Purple/Pink Reds)**
If red colors appear purple or pink on your ILI9488 display:
1. **Built-in color correction** automatically handles most issues
2. **Test with color bars** - On boot, you'll see red/green/blue bars for 3 seconds
3. **If left bar is blue instead of red**:
   - Change line ~419 in setup(): `setMAD(0xA0);` → `setMAD(0xA8);`
   - This flips BGR bit to correct color channel ordering
4. **Color correction features**:
   - Forces 16-bit RGB565 pixel format
   - Disables display inversion
   - Corrects color channel ordering

**Compilation Errors**
- Ensure all libraries are installed
- Check that `config.h` exists (copy from template)
- Verify Arduino WiFi R4 board is selected

### Debug Mode
Enable detailed logging by opening Serial Monitor (Tools → Serial Monitor) at 115200 baud to see:
- API request/response details
- WiFi connection status  
- Retry attempt logging
- Asset data parsing

## 🚀 Performance Stats

- **Memory Usage**: ~40 bytes for price data (5 assets)
- **API Calls**: 5 calls every 30 minutes = 240 calls/day
- **Rate Limits**: Well under free tier limits
- **Boot Time**: ~10-15 seconds including WiFi connection
- **Display Update**: Instant when cycling assets

## 🤝 Contributing

### Adding New Features
1. Fork the repository
2. Create feature branch: `git checkout -b feature-name`
3. Follow existing code style (no comments unless necessary)
4. Test thoroughly with multiple assets
5. Submit pull request with clear description

### Code Style
- Use existing variable naming conventions
- Maintain consistent spacing and indentation  
- Add retry logic for any new API calls
- Update README if adding new functionality

## 📄 License

MIT License - feel free to modify and distribute!

## 💡 Inspiration

Perfect for:
- **Trading desks** - Quick price monitoring
- **Crypto enthusiasts** - Portfolio tracking  
- **Developers** - Learning Arduino + API integration
- **Students** - Financial data visualization projects

---

**"Win Each Day"** 💪 - Built with Arduino and determination!