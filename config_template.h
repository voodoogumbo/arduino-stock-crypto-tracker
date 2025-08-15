// Configuration Template
// Copy this file to 'config.h' and fill in your actual values
// DO NOT commit config.h to version control!

#ifndef CONFIG_H
#define CONFIG_H

// ===== WIFI CREDENTIALS =====
const char* WIFI_SSID = "YOUR_WIFI_NETWORK_NAME";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// ===== API KEYS =====
const char* FINNHUB_API_KEY = "YOUR_FINNHUB_API_KEY_HERE"; // Get free key from finnhub.io

// ===== DISPLAY SETTINGS =====
const uint8_t ROTATION = 1; // 0..3, adjust for your screen orientation

// ===== PORTFOLIO SETTINGS =====
const bool PORTFOLIO_MODE = false; // true = show portfolio total, false = cycle individual tickers
// Holdings (shares/coins owned) - adjust these to your actual holdings
const float SOL_HOLDINGS = 10.0;       // Number of SOL coins
const float SHIB_HOLDINGS = 1000000.0; // Number of SHIB coins  
const float MSFT_HOLDINGS = 5.0;       // Number of MSFT shares
const float PLTR_HOLDINGS = 50.0;      // Number of PLTR shares
const float SNOW_HOLDINGS = 3.0;       // Number of SNOW shares

#endif