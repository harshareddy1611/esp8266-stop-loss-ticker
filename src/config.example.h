#pragma once

// Copy this file to config.h and fill in your own values.
// config.h is gitignored so your WiFi credentials and API key never get committed.

// ---- WiFi ----
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"   // leave as "" for an open network

// ---- Finnhub API ----
// Get a free key at https://finnhub.io/register (instant, no card needed)
#define FINNHUB_API_KEY "YOUR_FINNHUB_API_KEY"

// ---- Stock to track ----
#define STOCK_SYMBOL "NVDA"

// ---- Polling ----
#define POLL_INTERVAL_MS 30000UL   // how often to fetch a new quote (Finnhub free tier allows 60 calls/min)

// ---- Alert threshold ----
// Beep once when the stock moves this many percentage points (up or down)
// from the previous day's close.
#define BIG_MOVE_PERCENT 2.0f

// Also beep if price jumps/drops this many percent between two consecutive polls
// (catches fast moves without waiting for a big cumulative change)
#define STEP_MOVE_PERCENT 0.5f

// ---- Stop-loss alert ----
// When enabled, sounds a distinct triple-beep alarm (and shows a banner) the
// moment price drops to or below STOP_LOSS_PRICE. Set to the actual price
// level of your stop-loss order, not a percentage.
#define STOP_LOSS_ENABLED true
#define STOP_LOSS_PRICE 150.00f

// ---- Hardware pins (NodeMCU / ESP8266, SPI display) ----
// Clk/Data are the hardware SPI pins and are fixed: D5 (GPIO14) = SCK, D7 (GPIO13) = MOSI
#define OLED_DC   4    // D2
#define OLED_RST  16   // D0
#define OLED_CS   5    // D1
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// D6 = GPIO12 -- free now that D5 is used by SPI clock
#define BUZZER_PIN 12
#define BUZZER_IS_PASSIVE true   // true = tone() driven passive buzzer, false = simple on/off active buzzer
#define BUZZER_FREQ_HZ 2000
#define BEEP_DURATION_MS 150
