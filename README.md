# NVDA Stock Ticker — ESP8266 + SSD1306

Polls NVIDIA's stock price from Finnhub every 30s, shows price + % change
on a 128x64 SSD1306 OLED, and beeps a passive buzzer on:
- a big cumulative move from the previous close (`BIG_MOVE_PERCENT`)
- a fast move between two polls (`STEP_MOVE_PERCENT`)
- price dropping to/below your stop-loss level (`STOP_LOSS_PRICE`) — a distinct triple-beep + on-screen banner

## Hardware

NodeMCU (ESP8266) + SSD1306 (SPI) + passive buzzer (2-pin, +/-).

| Component pin | NodeMCU pin |
|---|---|
| OLED GND | GND |
| OLED Vin | 3V3 |
| OLED Clk | D5 (hardware SPI clock, fixed) |
| OLED Data | D7 (hardware SPI MOSI, fixed) |
| OLED DC | D2 |
| OLED Rst | D0 |
| OLED CS | D1 |
| Buzzer + | D6 |
| Buzzer − | GND |

This display module supports both I2C and SPI (selected by a solder jumper
on the back — SJ1/SJ2 closed = I2C, open = SPI/default). We're using SPI.

## Setup

1. Install [PlatformIO](https://platformio.org/install) — easiest via the
   [VS Code extension](https://platformio.org/install/ide?install=vscode),
   or `pip install platformio` for CLI-only.
2. Get a free Finnhub API key at https://finnhub.io/register (instant, no card).
3. Copy `src/config.example.h` to `src/config.h` (gitignored, so your secrets never get committed) and edit `src/config.h`:
   - `WIFI_SSID` / `WIFI_PASSWORD`
   - `FINNHUB_API_KEY`
   - `STOP_LOSS_PRICE` to your actual stop-loss level
   - Adjust `BIG_MOVE_PERCENT` / `STEP_MOVE_PERCENT` if you want more/less sensitive alerts
4. Plug in the NodeMCU via USB, then from this folder:
   ```
   pio run -t upload
   pio device monitor
   ```
   (Or use the PlatformIO sidebar in VS Code: Build → Upload → Monitor.)

## Notes

- Finnhub's free tier covers this easily (60 calls/min limit; we poll every 30s).
- Quotes only update during US market hours — outside those hours the price
  will just hold steady (no false "big move" beeps).
- `WiFiClientSecure::setInsecure()` is used for HTTPS (no cert pinning) — fine
  for a hobby project, not something to reuse for anything sensitive.
