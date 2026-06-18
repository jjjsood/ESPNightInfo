# 🌌 ESPNightInfo

> A WiFi-connected stargazing companion for the ESP8266 that renders tonight's sky onto a 32×8 LED matrix.

ESPNightInfo turns a Wemos D1 Mini and a chain of MAX7219 LED matrices into a tiny "night panorama" display. It fetches live weather for your location, computes which planets and the Moon are above the horizon between sunset and sunrise, and animates the result on the matrix — clouds when the sky is poor, stars and planets when it's clear.

<p align="left">
  <img alt="Platform" src="https://img.shields.io/badge/platform-ESP8266-blue">
  <img alt="Framework" src="https://img.shields.io/badge/framework-Arduino-00979D">
  <img alt="Build" src="https://img.shields.io/badge/build-PlatformIO-orange">
  <img alt="License" src="https://img.shields.io/badge/license-MIT-green">
</p>

---

## ✨ Features

- **Live weather** — pulls cloud cover, rain and dew-point data from the free [Open-Meteo](https://open-meteo.com/) API (no API key required).
- **Real astronomy** — computes rise/set times and culmination altitude/azimuth for the Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus and Neptune using [SiderealPlanets](https://github.com/DavidArmstrong/SiderealPlanets).
- **Night-window awareness** — only considers conditions and bodies visible between the *next* sunset and sunrise.
- **Field-of-view filtering** — restrict visibility to a slice of the horizon (e.g. only what you can see out a window).
- **Web configuration** — a built-in web server lets you set latitude, longitude and FOV bounds from any browser, no reflash needed.
- **Two display modes** — alternates between a planet panorama and a sky panorama (stars vs. clouds) every 15 seconds.
- **Reusable library** — the astronomy/weather logic ships as a standalone Arduino library, `NightPanoramaC`.

---

## 🛠 Hardware

| Part | Notes |
|------|-------|
| **Wemos/LOLIN D1 Mini Lite** | ESP8266 board (`d1_mini_lite`) |
| **4× MAX7219 8×8 LED matrix** | Daisy-chained into a single 32×8 panel |
| Jumper wires / USB cable | Power + flashing |

### Wiring (D1 Mini → MAX7219)

| MAX7219 | D1 Mini pin | GPIO |
|---------|-------------|------|
| DIN     | `D7`        | GPIO13 |
| CLK     | `D5`        | GPIO14 |
| CS / LOAD | `D6`      | GPIO12 |
| VCC     | `5V`        | — |
| GND     | `G`         | — |

Pin assignments live at the top of [`src/main.cpp`](src/main.cpp):

```cpp
#define DIN_PIN D7
#define CLK_PIN D5
#define CS_PIN  D6
#define NUM_DEVICES 4
```

---

## 🖥 Display layout

The four matrices form one **32 columns × 8 rows** canvas. The firmware draws two scenes:

- **Planet panorama** — the Sun and Earth on the left, then each *visible* planet placed across the row in solar-system order.
- **Sky panorama** — a starfield when the night is clear, or a cloud bank when `rainAmount > 0` or `cloudCover > 20%`.

Pixels are addressed with a single helper that maps a global `(row, col)` to the right matrix:

```cpp
void setFullPanel(int row, int col); // row 0-7, col 0-31
```

---

## 📦 Software & dependencies

Built with [PlatformIO](https://platformio.org/) on the Arduino framework. Dependencies are declared in [`platformio.ini`](platformio.ini) and fetched automatically:

| Library | Purpose |
|---------|---------|
| `paulstoffregen/Time` | `time_t` / calendar handling |
| `bblanchon/ArduinoJson` | Parsing the Open-Meteo response |
| `davidarmstrong/SiderealPlanets` | Planetary & lunar positions, rise/set times |
| `wayoda/LedControl` | MAX7219 driver |

---

## 🚀 Build & flash

```bash
# 1. Install PlatformIO Core (or use the VS Code extension)
pip install platformio

# 2. Clone
git clone https://github.com/jjjsood/ESPNightInfo.git
cd ESPNightInfo

# 3. Build
pio run

# 4. Flash + open serial monitor (115200 baud)
pio run --target upload && pio device monitor
```

Adjust `upload_port` in `platformio.ini` if your board is not on `/dev/ttyUSB0`.

---

## ⚙️ Configuration

### 1. WiFi credentials

Credentials live in `include/secrets.h`, which is **gitignored** so they never get committed. Create it from the template:

```bash
cp include/secrets.h.example include/secrets.h
# then edit include/secrets.h with your SSID and password
```

```cpp
#define SECRET_SSID "YOUR_WIFI_SSID"
#define SECRET_PASS "YOUR_WIFI_PASSWORD"
```

> ⚠️ **Security note:** never hardcode credentials in `main.cpp` or any tracked file. Anything pushed to a public repo must be treated as leaked, even after a history rewrite — rotate the secret, don't just delete it.

### 2. Location & field of view

Defaults live in `main.cpp`:

```cpp
GeoLocation location = {.latitude = 47.9827, .longitude = 7.713736};
FieldOfView fov      = {.leftBound = 0, .rightBound = 360};
```

At runtime, the device hosts a small web UI. After it connects to WiFi, the serial monitor prints its IP address:

```
Connected, IP address: 192.168.x.x
```

Open that IP in a browser to set **latitude**, **longitude**, and the **left/right horizon bounds** live — changes immediately trigger a refetch.

---

## 📚 Library: `NightPanoramaC`

The data layer is packaged as a reusable Arduino library under [`lib/NightPanoramaC`](lib/NightPanoramaC). One call gives you everything for the night:

```cpp
#include <NightPanoramaC.h>

GeoLocation location = {47.9827, 7.713736};
FieldOfView fov      = {0, 360};

StargazingInfo info = getStargazingInfo(location, fov);

// Weather for the night window
info.weather.cloudCover;   // average %, sunset → sunrise
info.weather.rainAmount;   // accumulated mm
info.weather.isDew;        // dew likely?
info.weather.nextSunset;   // time_t
info.weather.nextSunrise;  // time_t

// Celestial bodies
for (int i = 0; i < info.celestial.bodyCount; ++i) {
    auto &b = info.celestial.bodies[i];
    b.name;                          // "Jupiter"
    b.isVisible;                     // in FOV & above horizon during night?
    b.positionCulmination.hc;        // altitude at culmination
    b.positionCulmination.zn;        // azimuth at culmination
    b.riseAndSet.riseTime;           // time_t
    b.riseAndSet.setTime;            // time_t
}
```

### Public API

| Function | Returns | Description |
|----------|---------|-------------|
| `getStargazingInfo(location, fov)` | `StargazingInfo` | Weather + celestial bodies in one shot |
| `getWeatherInfo(location)` | `WeatherInfo` | Night-window weather from Open-Meteo |
| `getCelestialInfo(location, sunset, sunrise, fov)` | `CelestialInfo` | Visible bodies for the given night window |

Utility helpers (time conversion, ISO-8601 parsing, enum mapping) live in [`Utils.h`](lib/NightPanoramaC/src/Utils.h).

---

## 🗂 Project structure

```
ESPNightInfo/
├── platformio.ini              # Board, deps, upload config
├── src/
│   └── main.cpp                # Firmware: WiFi, web server, LED rendering
└── lib/
    └── NightPanoramaC/         # Reusable astronomy + weather library
        └── src/
            ├── NightPanoramaC.h    # Umbrella header
            ├── StargazingInfo.*    # getStargazingInfo()
            ├── WeatherInfo.*       # Open-Meteo client
            ├── CelestialInfo.*     # SiderealPlanets wrapper
            ├── SharedStructs.h     # GeoLocation, FieldOfView, enums
            └── Utils.*             # Time / string helpers
```

---

## 🧭 How it works

```
        ┌──────────────┐   HTTP    ┌───────────────┐
        │   D1 Mini    │──────────▶│  Open-Meteo   │  weather, sunrise/sunset
        │  (ESP8266)   │◀──────────│     API       │
        └──────┬───────┘           └───────────────┘
               │  getStargazingInfo(location, fov)
               ▼
   ┌───────────────────────────┐
   │      NightPanoramaC        │  SiderealPlanets → rise/set, alt/az
   │  weather + celestial info  │
   └─────────────┬─────────────┘
                 │  toggleDisplay() every 15 s
                 ▼
         ┌───────────────┐
         │ 32×8 MAX7219  │  planets │ stars │ clouds
         │  LED matrix   │
         └───────────────┘
```

Weather and positions refresh once per hour; the display alternates scenes every 15 seconds.

---

## 🧩 Roadmap / known limitations

- The bundled `library.propetries` / `keywords.txt` / example sketches are placeholders.
- No persistent storage — location/FOV reset to defaults on reboot.

---

## 📄 License

MIT — see `LICENSE`.
