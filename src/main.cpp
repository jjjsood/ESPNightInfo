#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LedControl.h>
#include "StargazingInfo.h"
#include "Utils.h"
#include "secrets.h"

// Pin configuration for the D1 Mini and MAX7219
#define DIN_PIN D7
#define CLK_PIN D5
#define CS_PIN D6
#define NUM_DEVICES 4

// WiFi credentials (defined in include/secrets.h, which is gitignored)
const char *ssid = SECRET_SSID;
const char *password = SECRET_PASS;

// Geographic location for stargazing
GeoLocation location = {.latitude = 47.9827, .longitude = 7.713736};
FieldOfView fov = {.leftBound = 0, .rightBound = 360};

// Timekeeping and interval settings
unsigned long lastDisplaySwitchTime = 0;
const unsigned long displaySwitchInterval = 15000; // 15 seconds
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 3600000; // 1 hour in milliseconds
long berlinUtcOffset = 3600;

// Global instances
WiFiClient Wifi;
ESP8266WebServer server(80);
LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);
StargazingInfo stargazingInfo;
bool showsPlanets = false;

// Function prototypes
void fetchStargazingInfo();
void handleSubmit();
void handleRoot();
void setFullPanel(int row, int col);
void showSunandEarth();
void showMercury();
void showVenus();
void showMoon();
void showMars();
void showJupiter();
void showSaturn();
void showUranus();
void showNeptune();
void showPlanets(CelestialInfo celestialInfo);
void showStars();
void showClouds();
void showPanorama(StargazingInfo stargazingInfo);
void toggleDisplay();

void setup()
{
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize each device in the LED matrix
  for (int index = 0; index < NUM_DEVICES; index++)
  {
    lc.shutdown(index, false);
    lc.setIntensity(index, 8); // Brightness level: 0 (min) to 15 (max)
    lc.clearDisplay(index);
  }

  // Connect to the specified Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(5000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Configure web server routes
  server.on("/", handleRoot);
  server.on("/submit", handleSubmit);
  server.begin();
  fetchStargazingInfo();
}

void loop()
{
  // Handle incoming client requests
  server.handleClient();

  // Print location and field of view at regular intervals
  if (millis() - lastDisplaySwitchTime >= displaySwitchInterval)
  {
    lastDisplaySwitchTime = millis();
    String text = "Lat: " + String(location.latitude, 6) + " Lon: " + String(location.longitude, 6) + " | " + String(fov.leftBound) + " - " + String(fov.rightBound);
    Serial.println(text);
    toggleDisplay();
  }
  if (millis() - lastFetchTime >= fetchInterval)
  {
    // Call fetchStargazingInfo and update lastFetchTime
    lastFetchTime = millis();
    fetchStargazingInfo();
  }
}

// Fetch StargazingInfo
void fetchStargazingInfo()
{
  stargazingInfo = getStargazingInfo(location, fov);

  // Print weather info
  Serial.println("Weather Info:");
  Serial.print("Is Dew: ");
  Serial.println(stargazingInfo.weather.isDew ? "Yes" : "No");
  Serial.print("Rain Amount: ");
  Serial.println(stargazingInfo.weather.rainAmount);
  Serial.print("Cloud Cover: ");
  Serial.println(stargazingInfo.weather.cloudCover);
  Serial.println("Next Sunset Time:");
  printHumanReadableTime(stargazingInfo.weather.nextSunset, berlinUtcOffset);
  Serial.println("Next Sunrise Time:");
  printHumanReadableTime(stargazingInfo.weather.nextSunrise, berlinUtcOffset);

  // Print celestial info
  Serial.println("Celestial Info:");
  for (int i = 0; i < stargazingInfo.celestial.bodyCount; ++i)
  {
    Serial.print("Body Name: ");
    Serial.println(stargazingInfo.celestial.bodies[i].name);
    Serial.print("Rise Time: ");
    printHumanReadableTime(stargazingInfo.celestial.bodies[i].riseAndSet.riseTime, berlinUtcOffset);
    Serial.print("Set Time: ");
    printHumanReadableTime(stargazingInfo.celestial.bodies[i].riseAndSet.setTime, berlinUtcOffset);
    Serial.print("Altitude: ");
    Serial.println(stargazingInfo.celestial.bodies[i].positionCulmination.hc);
    Serial.print("Azimuth: ");
    Serial.println(stargazingInfo.celestial.bodies[i].positionCulmination.zn);
    Serial.print("Is Visible: ");
    Serial.println(stargazingInfo.celestial.bodies[i].isVisible ? "Yes" : "No");
  }
}

void showWeather()
{
}

void showPlanets()
{
}

void handleRoot()
{
  String page = "<html><head><style>";
  page += "body { font-family: Arial, sans-serif; background-color: #f0f0f0; text-align: center; padding: 50px; }";
  page += "h2 { color: #333; margin-bottom: 20px; }";
  page += "form { background: #fff; padding: 20px; border-radius: 8px; display: inline-block; text-align: left; width: 350px; }";
  page += ".form-row { display: flex; margin-bottom: 10px; }";
  page += ".form-row label { flex: 1; }";                                                                         // 1/3 of the space
  page += ".form-row input[type='text'] { flex: 2; padding: 10px; border: 1px solid #ddd; border-radius: 4px; }"; // 2/3 of the space
  page += "input[type='submit'] { width: 100%; background-color: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }";
  page += "input[type='submit']:hover { background-color: #0056b3; }";
  page += "</style>";
  page += "<script type='text/javascript'>";
  page += "function validateInput(event) {";
  page += "  var lat = document.getElementById('lat').value;";
  page += "  var lon = document.getElementById('lon').value;";
  page += "  var left = document.getElementById('left').value;";
  page += "  var right = document.getElementById('right').value;";
  page += "  if ((lat && isNaN(parseFloat(lat))) || (lon && isNaN(parseFloat(lon))) ||";
  page += "      (left && isNaN(parseFloat(left))) || (right && isNaN(parseFloat(right)))) {";
  page += "    alert('Please enter valid float numbers');";
  page += "    event.preventDefault();";
  page += "  }";
  page += "}";
  page += "</script>";
  page += "</head><body>";
  page += "</head><body>";
  page += "<h2>Night Panorama</h2><form action='/submit'>";
  page += "<div class='form-row'><label for='lat'>Latitude:</label><input type='text' id='lat' name='lat' value='" + String(location.latitude, 6) + "'></div>";
  page += "<div class='form-row'><label for='lon'>Longitude:</label><input type='text' id='lon' name='lon' value='" + String(location.longitude, 6) + "'></div>";
  page += "<div class='form-row'><label for='left'>Left Border:</label><input type='text' id='left' name='left' value='" + String(fov.leftBound) + "'></div>";
  page += "<div class='form-row'><label for='right'>Right Border:</label><input type='text' id='right' name='right' value='" + String(fov.rightBound) + "'></div>";
  page += "<input type='submit'>";
  page += "<form action='/submit' method='post' onsubmit='validateInput(event)'>";
  page += "</form></body></html>";
  server.send(200, "text/html", page);
}

void handleSubmit()
{
  if (server.hasArg("lat") && server.arg("lat") != "")
  {
    location.latitude = server.arg("lat").toFloat();
  }
  if (server.hasArg("lon") && server.arg("lon") != "")
  {
    location.longitude = server.arg("lon").toFloat();
  }
  if (server.hasArg("left") && server.arg("left") != "")
  {
    fov.leftBound = server.arg("left").toInt();
  }
  if (server.hasArg("right") && server.arg("right") != "")
  {
    fov.rightBound = server.arg("right").toInt();
  }

  server.sendHeader("Location", "/");
  server.send(303);
  fetchStargazingInfo();
}

void setFullPanel(int row, int col)
{
  // Validate row and column values
  if (row < 0 || row > 7 || col < 0 || col > 31)
  {
    // Handle invalid inputs here (e.g., log an error or silently return)
    return;
  }

  // Calculate the address based on the column
  int addr = col / 8;

  // Calculate the column within the specific 8x8 matrix
  int matrixCol = col % 8;

  // Set the LED state using the setLed function
  lc.setLed(addr, row, matrixCol, true);
}

void showSunandEarth()
{
  // Sun
  setFullPanel(0, 0);
  setFullPanel(0, 1);

  setFullPanel(1, 0);
  setFullPanel(1, 1);
  setFullPanel(1, 2);

  setFullPanel(2, 0);
  setFullPanel(2, 1);
  setFullPanel(2, 2);

  setFullPanel(3, 0);
  setFullPanel(3, 1);
  setFullPanel(3, 2);
  setFullPanel(3, 3);

  setFullPanel(4, 0);
  setFullPanel(4, 1);
  setFullPanel(4, 2);
  setFullPanel(4, 3);

  setFullPanel(5, 0);
  setFullPanel(5, 1);
  setFullPanel(5, 2);

  setFullPanel(6, 0);
  setFullPanel(6, 1);
  setFullPanel(6, 2);

  setFullPanel(7, 0);
  setFullPanel(7, 1);

  // Earth
  setFullPanel(4, 11);
  setFullPanel(5, 11);
  setFullPanel(4, 12);
  setFullPanel(5, 12);
}
void showMercury()
{
  setFullPanel(4, 5);
  setFullPanel(5, 5);
  setFullPanel(4, 6);
  setFullPanel(5, 6);
}
void showVenus()
{
  setFullPanel(4, 8);
  setFullPanel(5, 8);
  setFullPanel(4, 9);
  setFullPanel(5, 9);
}
void showMoon()
{
  setFullPanel(2, 12);
}
void showMars()
{
  setFullPanel(4, 14);
  setFullPanel(5, 14);
  setFullPanel(4, 15);
  setFullPanel(5, 15);
}
void showJupiter()
{
  setFullPanel(4, 18);
  setFullPanel(5, 18);
  setFullPanel(6, 18);
  setFullPanel(4, 19);
  setFullPanel(5, 19);
  setFullPanel(6, 19);
  setFullPanel(4, 20);
  setFullPanel(5, 20);
  setFullPanel(6, 20);
}
void showSaturn()
{
  setFullPanel(4, 23);
  setFullPanel(5, 23);
  setFullPanel(4, 24);
  setFullPanel(5, 24);
}
void showUranus()
{
  setFullPanel(4, 26);
  setFullPanel(5, 26);
  setFullPanel(4, 27);
  setFullPanel(5, 27);
}
void showNeptune()
{
  setFullPanel(4, 29);
  setFullPanel(5, 29);
  setFullPanel(4, 30);
  setFullPanel(5, 30);
}

// Function to show planets based on their visibility
void showPlanets(CelestialInfo celestialInfo)
{
  for (int index = 0; index < NUM_DEVICES; index++)
  {
    lc.clearDisplay(index);
  }
  showSunandEarth();
  for (int i = 0; i < celestialInfo.bodyCount; ++i)
  {
    if (celestialInfo.bodies[i].isVisible)
    {
      CelestialObject it = stringToEnum(celestialInfo.bodies[i].name);
      switch (it)
      {
      case Moon:
        showMoon();
        break;
      case Mercury:
        showMercury();
        break;
      case Venus:
        showVenus();
        break;
      case Mars:
        showMars();
        break;
      case Jupiter:
        showJupiter();
        break;
      case Saturn:
        showSaturn();
        break;
      case Uranus:
        showUranus();
        break;
      case Neptune:
        showNeptune();
        break;
      default:
        break;
      }
    }
  }
}

void showStars()
{
  for (int index = 0; index < NUM_DEVICES; index++)
  {
    lc.clearDisplay(index);
  }

  setFullPanel(0, 2);
  setFullPanel(0, 6);
  setFullPanel(0, 9);
  setFullPanel(0, 12);
  setFullPanel(0, 17);
  setFullPanel(0, 21);
  setFullPanel(0, 23);
  setFullPanel(0, 27);
  setFullPanel(0, 30);

  setFullPanel(1, 0);
  setFullPanel(1, 5);
  setFullPanel(1, 11);
  setFullPanel(1, 14);
  setFullPanel(1, 18);
  setFullPanel(1, 22);
  setFullPanel(1, 25);
  setFullPanel(1, 31);

  setFullPanel(2, 3);
  setFullPanel(2, 7);
  setFullPanel(2, 10);
  setFullPanel(2, 13);
  setFullPanel(2, 20);
  setFullPanel(2, 28);

  setFullPanel(4, 3);
  setFullPanel(4, 4);
  setFullPanel(4, 5);
  setFullPanel(4, 9);
  setFullPanel(4, 10);
  setFullPanel(4, 11);

  setFullPanel(4, 18);
  setFullPanel(4, 19);
  setFullPanel(4, 20);
  setFullPanel(4, 28);
  setFullPanel(4, 29);
  setFullPanel(4, 30);

  setFullPanel(5, 3);
  setFullPanel(5, 4);
  setFullPanel(5, 5);
  setFullPanel(5, 9);
  setFullPanel(5, 10);
  setFullPanel(5, 11);

  setFullPanel(5, 18);
  setFullPanel(5, 19);
  setFullPanel(5, 20);
  setFullPanel(5, 28);
  setFullPanel(5, 29);
  setFullPanel(5, 30);

  setFullPanel(6, 4);
  setFullPanel(6, 10);
  setFullPanel(6, 19);
  setFullPanel(6, 29);

  for (int index = 0; index < 32; index++)
  {
    setFullPanel(7, index);
  }
}

void showClouds()
{
  for (int index = 0; index < NUM_DEVICES; index++)
  {
    lc.clearDisplay(index);
  }

  setFullPanel(0, 2);
  setFullPanel(0, 3);
  setFullPanel(0, 4);
  setFullPanel(0, 9);
  setFullPanel(0, 10);
  setFullPanel(0, 11);
  setFullPanel(0, 16);
  setFullPanel(0, 17);
  setFullPanel(0, 18);
  setFullPanel(0, 23);
  setFullPanel(0, 24);
  setFullPanel(0, 25);

  setFullPanel(1, 0);
  setFullPanel(1, 1);
  setFullPanel(1, 2);
  setFullPanel(1, 3);
  setFullPanel(1, 4);
  setFullPanel(1, 8);
  setFullPanel(1, 9);
  setFullPanel(1, 10);
  setFullPanel(1, 11);
  setFullPanel(1, 12);
  setFullPanel(1, 16);
  setFullPanel(1, 17);
  setFullPanel(1, 18);
  setFullPanel(1, 19);
  setFullPanel(1, 20);
  setFullPanel(1, 24);
  setFullPanel(1, 25);
  setFullPanel(1, 26);
  setFullPanel(1, 27);
  setFullPanel(1, 28);

  setFullPanel(2, 2);
  setFullPanel(2, 3);
  setFullPanel(2, 4);
  setFullPanel(2, 9);
  setFullPanel(2, 10);
  setFullPanel(2, 11);
  setFullPanel(2, 15);
  setFullPanel(2, 16);
  setFullPanel(2, 17);
  setFullPanel(2, 22);
  setFullPanel(2, 23);
  setFullPanel(2, 24);

  setFullPanel(4, 3);
  setFullPanel(4, 4);
  setFullPanel(4, 5);
  setFullPanel(4, 9);
  setFullPanel(4, 10);
  setFullPanel(4, 11);

  setFullPanel(4, 18);
  setFullPanel(4, 19);
  setFullPanel(4, 20);
  setFullPanel(4, 28);
  setFullPanel(4, 29);
  setFullPanel(4, 30);

  setFullPanel(5, 3);
  setFullPanel(5, 4);
  setFullPanel(5, 5);
  setFullPanel(5, 9);
  setFullPanel(5, 10);
  setFullPanel(5, 11);

  setFullPanel(5, 18);
  setFullPanel(5, 19);
  setFullPanel(5, 20);
  setFullPanel(5, 28);
  setFullPanel(5, 29);
  setFullPanel(5, 30);

  setFullPanel(6, 4);
  setFullPanel(6, 10);
  setFullPanel(6, 19);
  setFullPanel(6, 29);

  for (int index = 0; index < 32; index++)
  {
    setFullPanel(7, index);
  }
}

void showPanorama(WeatherInfo WeatherInfo)
{
  if (WeatherInfo.rainAmount > 0 || WeatherInfo.cloudCover > 20)
  {
    showClouds();
  }
  else
  {
    showStars();
  }
}

void toggleDisplay()
{
  if (showsPlanets)
  {
    showPlanets(stargazingInfo.celestial);
  }
  else
  {
    showPanorama(stargazingInfo.weather);
  }
  showsPlanets = !showsPlanets;
}
