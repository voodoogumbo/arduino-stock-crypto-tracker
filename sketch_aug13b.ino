#include <Arduino_GFX_Library.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "config.h"

// ===== CRYPTO =====
const char* CRYPTO_HOST = "api.coingecko.com";
const int   CRYPTO_PORT = 443; // HTTPS
const char* CRYPTO_PATH = "/api/v3/simple/price?ids=solana,shiba-inu&vs_currencies=usd&include_24hr_change=true";

// ===== STOCKS =====
const char* STOCK_HOST = "finnhub.io";
const int   STOCK_PORT = 443; // HTTPS

// ---------- display ----------
Arduino_DataBus *bus = new Arduino_UNOPAR8();
Arduino_GFX     *gfx = new Arduino_ILI9488(bus, -1, 0);

// Keep last good price so the screen isn't blank on failures
float g_lastSolPrice = NAN;
float g_lastSolChange = NAN;
float g_lastShibPrice = NAN;
float g_lastShibChange = NAN;
float g_lastMsftPrice = NAN;
float g_lastMsftChange = NAN;
float g_lastPltrPrice = NAN;
float g_lastPltrChange = NAN;
float g_lastSnowPrice = NAN;
float g_lastSnowChange = NAN;
uint8_t g_currentAsset = 0; // 0 = SOL, 1 = SHIB, 2 = MSFT, 3 = PLTR, 4 = SNOW

// ---------- LCD helpers ----------
void setMAD(uint8_t v) {
  bus->beginWrite(); bus->writeCommand(0x36); bus->write(v); bus->endWrite();
}


void colorBars() {
  gfx->fillScreen(BLACK);
  int w = gfx->width() / 3;
  gfx->fillRect(0,      0, w, gfx->height(), RGB565(255,0,0));   // should be vivid RED
  gfx->fillRect(w,      0, w, gfx->height(), RGB565(0,255,0));   // GREEN  
  gfx->fillRect(2*w,    0, w, gfx->height(), RGB565(0,0,255));   // BLUE
}

void printCentered(const String &txt, int y, uint8_t textSize) {
  int16_t x1, y1; uint16_t w, h;
  gfx->setTextSize(textSize);
  gfx->getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor((gfx->width() - (int)w) / 2, y);
  gfx->print(txt);
}

void drawMessage(const char* a, const char* b = nullptr) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE, BLACK);
  printCentered(a, (gfx->height()/2) - 12, 2);
  if (b) printCentered(b, (gfx->height()/2) + 12, 2);
}

void drawCrypto(const char* name, const char* symbol, float price, float change = NAN, const char* statusLine = nullptr) {
  // Clean white background with black text
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE, BLACK);

  // Title
  String title = String(name) + " (" + symbol + ")";
  printCentered(title, 12, 4);

  // Price big - different decimal places for different coins
  char buf[32];
  if (price < 0.01) {
    dtostrf(price, 0, 6, buf);  // 6 decimals for small coins like SHIB
  } else {
    dtostrf(price, 0, 2, buf);  // 2 decimals for larger coins like SOL
  }
  String line = String("$") + buf;
  int16_t bx, by; uint16_t bw, bh;
  gfx->setTextSize(6);
  gfx->getTextBounds(line, 0, 0, &bx, &by, &bw, &bh);
  printCentered(line, (gfx->height() - (int)bh) / 2 - 20, 6);

  // 24hr dollar change with colored text only
  if (!isnan(change)) {
    // Determine color for dollar change
    uint16_t changeColor = WHITE;
    if (change > 0) {
      changeColor = RGB565(0, 255, 0); // Bright green for gains
    } else if (change < 0) {
      changeColor = RGB565(220, 60, 40); // Toned down red for losses
    }
    
    gfx->setTextColor(changeColor, BLACK);
    char changeBuf[16];
    
    // Format dollar change based on magnitude
    if (abs(change) < 0.001) {
      dtostrf(change, 0, 6, changeBuf); // 6 decimals for very small changes (crypto)
    } else if (abs(change) < 1.0) {
      dtostrf(change, 0, 4, changeBuf); // 4 decimals for small changes
    } else {
      dtostrf(change, 0, 2, changeBuf); // 2 decimals for regular changes (stocks)
    }
    
    String changeLine = String((change >= 0) ? "+$" : "-$") + String(abs(atof(changeBuf)));
    printCentered(changeLine, (gfx->height() - (int)bh) / 2 + 40, 4);
    
    // Reset text color back to white
    gfx->setTextColor(WHITE, BLACK);
  }

  // Optional status / timestamp line
  if (statusLine) {
    printCentered(statusLine, gfx->height() - 28, 2);
  }

  // Dynamic frame color based on market performance
  uint16_t borderColor = WHITE; // Default for neutral/no data
  if (!isnan(change)) {
    if (change > 0) {
      borderColor = RGB565(0, 255, 0); // Green for gains
    } else if (change < 0) {
      borderColor = RGB565(220, 60, 40); // Red for losses
    }
  }
  gfx->drawRect(0, 0, gfx->width(), gfx->height(), borderColor);
}

// Helper function to display current asset
void drawCurrentAsset(const char* statusLine = nullptr) {
  switch (g_currentAsset) {
    case 0: // Solana
      if (!isnan(g_lastSolPrice)) {
        drawCrypto("Solana", "SOL", g_lastSolPrice, g_lastSolChange, statusLine);
      } else {
        drawMessage("Solana (SOL)", "No data available");
        Serial.println("DEBUG: SOL data missing");
      }
      break;
    case 1: // Shiba Inu
      if (!isnan(g_lastShibPrice)) {
        drawCrypto("Shiba Inu", "SHIB", g_lastShibPrice, g_lastShibChange, statusLine);
      } else {
        drawMessage("Shiba Inu (SHIB)", "No data available");
        Serial.println("DEBUG: SHIB data missing");
      }
      break;
    case 2: // Microsoft
      if (!isnan(g_lastMsftPrice)) {
        drawCrypto("Microsoft", "MSFT", g_lastMsftPrice, g_lastMsftChange, statusLine);
      } else {
        drawMessage("Microsoft (MSFT)", "No data available");
        Serial.println("DEBUG: MSFT data missing");
      }
      break;
    case 3: // Palantir
      if (!isnan(g_lastPltrPrice)) {
        drawCrypto("Palantir", "PLTR", g_lastPltrPrice, g_lastPltrChange, statusLine);
      } else {
        drawMessage("Palantir (PLTR)", "No data available");
        Serial.println("DEBUG: PLTR data missing");
      }
      break;
    case 4: // Snowflake
      if (!isnan(g_lastSnowPrice)) {
        drawCrypto("Snowflake", "SNOW", g_lastSnowPrice, g_lastSnowChange, statusLine);
      } else {
        drawMessage("Snowflake (SNOW)", "No data available");
        Serial.println("DEBUG: SNOW data missing");
      }
      break;
  }
}

// ---------- Portfolio helpers ----------
float calculatePortfolioValue() {
  float totalValue = 0.0;
  if (!isnan(g_lastSolPrice)) totalValue += g_lastSolPrice * SOL_HOLDINGS;
  if (!isnan(g_lastShibPrice)) totalValue += g_lastShibPrice * SHIB_HOLDINGS;
  if (!isnan(g_lastMsftPrice)) totalValue += g_lastMsftPrice * MSFT_HOLDINGS;
  if (!isnan(g_lastPltrPrice)) totalValue += g_lastPltrPrice * PLTR_HOLDINGS;
  if (!isnan(g_lastSnowPrice)) totalValue += g_lastSnowPrice * SNOW_HOLDINGS;
  return totalValue;
}

float calculatePortfolioChange() {
  float totalChange = 0.0;
  if (!isnan(g_lastSolChange)) totalChange += g_lastSolChange * SOL_HOLDINGS;
  if (!isnan(g_lastShibChange)) totalChange += g_lastShibChange * SHIB_HOLDINGS;
  if (!isnan(g_lastMsftChange)) totalChange += g_lastMsftChange * MSFT_HOLDINGS;
  if (!isnan(g_lastPltrChange)) totalChange += g_lastPltrChange * PLTR_HOLDINGS;
  if (!isnan(g_lastSnowChange)) totalChange += g_lastSnowChange * SNOW_HOLDINGS;
  return totalChange;
}

void drawPortfolio(const char* statusLine = nullptr) {
  float totalValue = calculatePortfolioValue();
  float totalChange = calculatePortfolioChange();
  
  if (totalValue > 0) {
    drawCrypto("Portfolio", "TOTAL", totalValue, totalChange, statusLine);
  } else {
    drawMessage("Portfolio", "No data available");
  }
}

// ---------- Wi-Fi + IP helpers ----------
String ipToString(IPAddress ip){ return String(ip[0])+"."+ip[1]+"."+ip[2]+"."+ip[3]; }
bool haveRealIP(){ IPAddress ip = WiFi.localIP(); return (ip[0]|ip[1]|ip[2]|ip[3]) != 0; }

bool connectAttempt(uint8_t attempt) {
  WiFi.disconnect(); WiFi.end(); delay(200);
  drawMessage("Connecting Wi-Fi...", (String("Attempt ") + attempt).c_str());
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) delay(200);
  if (WiFi.status() != WL_CONNECTED) return false;
  unsigned long t1 = millis();
  while (!haveRealIP() && millis() - t1 < 8000) delay(200);
  return haveRealIP();
}
bool ensureWifiRealIP(uint8_t tries=3){
  if (haveRealIP()) return true;
  for(uint8_t i=1;i<=tries;i++) if (connectAttempt(i)) return true;
  return false;
}
void drawIP(){ drawMessage("Wi-Fi connected", ("IP: " + ipToString(WiFi.localIP())).c_str()); }

bool verifyInternet(uint32_t timeoutMs=7000){
  WiFiClient wifi; HttpClient client(wifi, "example.com", 80);
  client.get("/");
  unsigned long t0 = millis();
  while(!client.connected() && millis()-t0<timeoutMs) delay(50);
  return client.responseStatusCode() > 0;
}

// ---------- CRYPTO fetch (HTTPS) ----------
WiFiSSLClient ssl;  // WiFiS3 SSL client

bool fetchCrypto() {
  HttpClient client(ssl, CRYPTO_HOST, CRYPTO_PORT);

  Serial.print("GET https://"); Serial.print(CRYPTO_HOST); Serial.println(CRYPTO_PATH);
  int rc = client.get(CRYPTO_PATH);
  if (rc != 0) {
    Serial.print("HTTP rc="); Serial.println(rc);
    client.stop();
    return false;
  }

  int status = client.responseStatusCode();
  Serial.print("HTTP status="); Serial.println(status);
  if (status != 200) {
    client.stop();
    return false;
  }

  String body = client.responseBody();
  Serial.print("Crypto Body: "); Serial.println(body);
  client.stop();

  StaticJsonDocument<256> doc;  // Increased size for multiple coins
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    Serial.print("JSON err: "); Serial.println(err.c_str());
    return false;
  }

  // Parse Solana data
  if (doc.containsKey("solana")) {
    g_lastSolPrice = doc["solana"]["usd"].as<float>();
    float percentChange = doc["solana"]["usd_24h_change"].as<float>();
    // Convert percentage to dollar change
    g_lastSolChange = g_lastSolPrice * (percentChange / 100.0);
  }
  
  // Parse Shiba Inu data
  if (doc.containsKey("shiba-inu")) {
    g_lastShibPrice = doc["shiba-inu"]["usd"].as<float>();
    float percentChange = doc["shiba-inu"]["usd_24h_change"].as<float>();
    // Convert percentage to dollar change
    g_lastShibChange = g_lastShibPrice * (percentChange / 100.0);
  }
  
  return doc.containsKey("solana") || doc.containsKey("shiba-inu");
}

// ---------- STOCKS fetch (HTTPS) ----------
bool fetchStock(const char* symbol, float &outPrice, float &outChange, int maxRetries = 3) {
  for (int attempt = 1; attempt <= maxRetries; attempt++) {
    WiFiSSLClient stockSsl;
    HttpClient client(stockSsl, STOCK_HOST, STOCK_PORT);
    
    String path = String("/api/v1/quote?symbol=") + symbol + "&token=" + FINNHUB_API_KEY;
    
    if (attempt == 1) {
      Serial.print("GET https://"); Serial.print(STOCK_HOST); Serial.println(path);
    } else {
      Serial.print("RETRY "); Serial.print(attempt); Serial.print(" for "); Serial.println(symbol);
    }
    
    int rc = client.get(path);
    if (rc != 0) {
      Serial.print("Stock HTTP rc="); Serial.println(rc);
      client.stop();
      
      if (attempt < maxRetries) {
        int delayMs = 1000 * (1 << (attempt - 1)); // 1s, 2s, 4s
        Serial.print("Retrying in "); Serial.print(delayMs); Serial.println("ms");
        delay(delayMs);
        continue;
      }
      return false;
    }

    int status = client.responseStatusCode();
    Serial.print("Stock HTTP status="); Serial.println(status);
    
    // Handle different error statuses
    if (status == 403 || status == 429 || status >= 500) {
      String body = client.responseBody(); // Read error response
      Serial.print("Error response: "); Serial.println(body);
      client.stop();
      
      if (attempt < maxRetries) {
        int delayMs = 1000 * (1 << (attempt - 1)); // 1s, 2s, 4s
        Serial.print("HTTP "); Serial.print(status); Serial.print(" - retrying in "); 
        Serial.print(delayMs); Serial.println("ms");
        delay(delayMs);
        continue;
      }
      return false;
    }
    
    if (status != 200) {
      client.stop();
      return false;
    }

    String body = client.responseBody();
    Serial.print("Stock Body: "); Serial.println(body);
    client.stop();

    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
      Serial.print("Stock JSON err: "); Serial.println(err.c_str());
      
      if (attempt < maxRetries) {
        int delayMs = 1000 * (1 << (attempt - 1));
        Serial.print("JSON error - retrying in "); Serial.print(delayMs); Serial.println("ms");
        delay(delayMs);
        continue;
      }
      return false;
    }

    // Finnhub format: {"c":current,"d":dollar_change,"dp":percent_change,...}
    if (doc.containsKey("c") && doc.containsKey("d")) {
      outPrice = doc["c"].as<float>();
      outChange = doc["d"].as<float>(); // dollar change (not percentage!)
      Serial.print("SUCCESS "); Serial.print(symbol); Serial.print(" after "); 
      Serial.print(attempt); Serial.println(" attempts");
      return true;
    }
    
    if (attempt < maxRetries) {
      Serial.println("Invalid response format - retrying");
      int delayMs = 1000 * (1 << (attempt - 1));
      delay(delayMs);
      continue;
    }
  }
  
  Serial.print("FAILED "); Serial.print(symbol); Serial.print(" after "); 
  Serial.print(maxRetries); Serial.println(" attempts");
  return false;
}

bool fetchStocks() {
  bool msftSuccess = fetchStock("MSFT", g_lastMsftPrice, g_lastMsftChange);
  delay(100); // Small delay between API calls
  bool pltrSuccess = fetchStock("PLTR", g_lastPltrPrice, g_lastPltrChange);
  delay(100); // Small delay between API calls
  bool snowSuccess = fetchStock("SNOW", g_lastSnowPrice, g_lastSnowChange);
  
  return msftSuccess || pltrSuccess || snowSuccess;
}

// ---------- app ----------
void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  gfx->begin();
  gfx->setRotation(ROTATION);
  
  // Force 16-bit RGB565 pixel format (ILI9488 default is often 18-bit)
  bus->beginWrite();
  bus->writeCommand(0x3A);    // COLMOD
  bus->write(0x55);           // 16-bit/pixel, RGB565
  bus->endWrite();
  
  // Set MADCTL: try RGB order first (0xA0 = MY | MV, without BGR bit)
  setMAD(0xA0);               // If red looks blue, change this to 0xA8
  
  // Ensure display inversion OFF
  bus->beginWrite();
  bus->writeCommand(0x20);    // Display Inversion OFF
  bus->endWrite();
  
  
  // Test color channels - left bar should be RED
  colorBars();
  delay(3000);  // Show color test for 3 seconds

  if (!ensureWifiRealIP(3)) {
    drawMessage("Wi-Fi failed", "Showing last price");
    delay(800);
    if (!isnan(g_lastSolPrice) || !isnan(g_lastShibPrice) || !isnan(g_lastMsftPrice) || !isnan(g_lastPltrPrice) || !isnan(g_lastSnowPrice)) {
      if (PORTFOLIO_MODE) {
        drawPortfolio("Offline");
      } else {
        drawCurrentAsset("Offline");
      }
    }
    else drawMessage("No price yet", "Offline");
    return;
  }
  drawIP();
  delay(1500);

  drawMessage("Checking internet...");
  if (!verifyInternet()) {
    drawMessage("No internet", "Will retry");
    delay(1200);
  }

  // First fetch
  bool cryptoSuccess = fetchCrypto();
  bool stockSuccess = fetchStocks();
  
  if (cryptoSuccess || stockSuccess) {
    if (PORTFOLIO_MODE) {
      drawPortfolio("Updated just now");
    } else {
      drawCurrentAsset("Updated just now");
    }
  } else {
    drawMessage("Price fetch failed", "Will retry");
  }
}

void loop() {
  static uint32_t lastFetch = 0;
  static uint32_t lastCycle = 0;

  // Display logic based on mode
  if (PORTFOLIO_MODE) {
    // Portfolio mode: refresh display every 15 seconds
    if (millis() - lastCycle > 15000UL) {
      drawPortfolio("Win Each Day");
      lastCycle = millis();
    }
  } else {
    // Individual ticker mode: cycle between assets every 15 seconds
    if (millis() - lastCycle > 15000UL) {
      g_currentAsset = (g_currentAsset + 1) % 5; // Cycle through 0, 1, 2, 3, 4
      drawCurrentAsset("Win Each Day");
      lastCycle = millis();
    }
  }

  // Refresh every 30 min
  if (millis() - lastFetch > 1800000UL) {
    if (WiFi.status() != WL_CONNECTED || !haveRealIP()) {
      drawMessage("Wi-Fi dropped", "Reconnecting...");
      if (ensureWifiRealIP(3)) {
        drawIP();
        delay(1000);
      }
    }

    bool cryptoSuccess = fetchCrypto();
    bool stockSuccess = fetchStocks();
    
    if (cryptoSuccess || stockSuccess) {
      if (PORTFOLIO_MODE) {
        drawPortfolio("Updated");
      } else {
        drawCurrentAsset("Updated");
      }
    } else {
      // keep last good price on screen; flash a quick note
      if (!isnan(g_lastSolPrice) || !isnan(g_lastShibPrice) || !isnan(g_lastMsftPrice) || !isnan(g_lastPltrPrice) || !isnan(g_lastSnowPrice)) {
        if (PORTFOLIO_MODE) {
          drawPortfolio("Fetch failed");
        } else {
          drawCurrentAsset("Fetch failed");
        }
      } else {
        drawMessage("Price fetch failed", "No prior data");
      }
    }
    lastFetch = millis();
  }

  // Tiny heartbeat dot (optional)
  delay(50);
}
