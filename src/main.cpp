#include <Arduino.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include "config.h"

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &SPI, OLED_DC, OLED_RST, OLED_CS);

static float previousClose = 0;
static float lastPolledPrice = 0;
static bool haveLastPolledPrice = false;
static bool bigMoveAlerted = false; // latches so we beep once per threshold crossing, not every poll
static bool stopLossHit = false;    // latches once price drops to/below STOP_LOSS_PRICE

static unsigned long lastPollTime = 0;

void beep() {
#if BUZZER_IS_PASSIVE
  tone(BUZZER_PIN, BUZZER_FREQ_HZ, BEEP_DURATION_MS);
#else
  digitalWrite(BUZZER_PIN, HIGH);
  delay(BEEP_DURATION_MS);
  digitalWrite(BUZZER_PIN, LOW);
#endif
}

void alarmStopLoss() {
  // distinct pattern from the single big-move beep: three short beeps
  for (int i = 0; i < 3; i++) {
    beep();
    delay(100);
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected: ");
  Serial.println(WiFi.localIP());
}

bool fetchQuote(float &price, float &prevClose) {
  WiFiClientSecure client;
  client.setInsecure(); // no cert pinning -- fine for a hobby project hitting Finnhub's public API

  HTTPClient http;
  String url = String("https://finnhub.io/api/v1/quote?symbol=") + STOCK_SYMBOL + "&token=" + FINNHUB_API_KEY;
  if (!http.begin(client, url)) return false;

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed, code=%d\n", code);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.print("JSON parse failed: ");
    Serial.println(err.c_str());
    return false;
  }

  price = doc["c"] | 0.0f;
  prevClose = doc["pc"] | 0.0f;
  return price > 0.0f && prevClose > 0.0f;
}

void drawScreen(float price, float pctFromClose, bool stopLossActive) {
  display.clearDisplay();

  display.setFont(NULL);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(STOCK_SYMBOL);
  if (WiFi.status() != WL_CONNECTED) display.print(" (offline)");

  display.setFont(&FreeSansBold12pt7b);
  display.setCursor(0, 32);
  display.print("$");
  display.print(price, 2);
  display.setFont(NULL);

  display.setFont(&FreeSansBold9pt7b);
  display.setCursor(0, 50);
  if (pctFromClose >= 0) display.print("+");
  display.print(pctFromClose, 2);
  display.print("%");
  display.setFont(NULL);

  if (stopLossActive) {
    display.fillRect(0, OLED_HEIGHT - 8, OLED_WIDTH, 8, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, OLED_HEIGHT - 8);
    display.print("STOP-LOSS HIT!");
    display.setTextColor(SSD1306_WHITE);
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("SSD1306 init failed");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();

  connectWiFi();

  beep(); // startup beep so you know it's alive
}

void loop() {
  unsigned long now = millis();
  if (lastPollTime == 0 || now - lastPollTime >= POLL_INTERVAL_MS) {
    lastPollTime = now;

    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }

    float price, prevClose;
    if (fetchQuote(price, prevClose)) {
      previousClose = prevClose;

      float pctFromClose = (price - previousClose) / previousClose * 100.0f;

#if STOP_LOSS_ENABLED
      // Highest priority alert: stop-loss level breached
      if (price <= STOP_LOSS_PRICE) {
        if (!stopLossHit) {
          alarmStopLoss();
          stopLossHit = true;
        }
      } else {
        stopLossHit = false;
      }
#endif

      // Beep once when the day's move crosses the threshold; re-arm once it falls back under
      if (fabs(pctFromClose) >= BIG_MOVE_PERCENT) {
        if (!bigMoveAlerted) {
          beep();
          bigMoveAlerted = true;
        }
      } else {
        bigMoveAlerted = false;
      }

      // Also beep on a fast move between two consecutive polls
      if (haveLastPolledPrice) {
        float stepPct = (price - lastPolledPrice) / lastPolledPrice * 100.0f;
        if (fabs(stepPct) >= STEP_MOVE_PERCENT) {
          beep();
        }
      }
      lastPolledPrice = price;
      haveLastPolledPrice = true;

      drawScreen(price, pctFromClose, STOP_LOSS_ENABLED && stopLossHit);
    } else {
      Serial.println("Quote fetch failed, will retry next poll");
    }
  }
}
