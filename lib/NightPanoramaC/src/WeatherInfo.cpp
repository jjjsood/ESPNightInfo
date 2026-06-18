#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "WeatherInfo.h"
#include "Utils.h"

// Constants
const float DEW_POINT_DIFF_THRESHOLD = 2.0; // Threshold for dew point difference

/**
 * Fetches weather information for a given geographic location.
 *
 * @param location The geographical location (latitude and longitude).
 * @return A WeatherInfo struct filled with weather data for the night period.
 */
WeatherInfo getWeatherInfo(const GeoLocation &location) {
    WeatherInfo info = {};
    WiFiClient client;
    HTTPClient http;

    // Compose API URL with the user's latitude and longitude
    String url = "http://api.open-meteo.com/v1/forecast?latitude=" +
                 String(location.latitude, 6) +
                 "&longitude=" +
                 String(location.longitude, 6) +
                 "&current=is_day&hourly=temperature_2m,dew_point_2m,rain,cloud_cover&daily=sunrise,sunset&timezone=auto&forecast_days=3";

    http.begin(client, url);
    int httpCode = http.GET();
    Serial.print("HTTP Response Code: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {
        const size_t capacity = 12288;
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, http.getString());

        if (error) {
            // Handle JSON parsing error
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            http.end();
            return info; // Return empty info struct
        }

        // Get the current time and UTC offset from the JSON
        const char *currentTimeISO8601 = doc["current"]["time"];
        long utcOffsetSeconds = doc["utc_offset_seconds"];

        // Convert current time to time_t
        time_t currentTime = iso8601ToTime(currentTimeISO8601, utcOffsetSeconds);

        JsonObject hourly = doc["hourly"];
        JsonArray timeArray = hourly["time"].as<JsonArray>();
        JsonArray temperatureArray = hourly["temperature_2m"].as<JsonArray>();
        JsonArray dewPointArray = hourly["dew_point_2m"].as<JsonArray>();
        JsonArray rainArray = hourly["rain"].as<JsonArray>();
        JsonArray cloudCoverArray = hourly["cloud_cover"].as<JsonArray>();

        JsonObject daily = doc["daily"];
        JsonArray sunriseArray = daily["sunrise"].as<JsonArray>();
        JsonArray sunsetArray = daily["sunset"].as<JsonArray>();

        // Convert the first sunset and sunrise times to time_t
        time_t sunsetTime0 = iso8601ToTime(sunsetArray[0], utcOffsetSeconds);

        // Check if the current time is past the time of sunset 0
        if (currentTime > sunsetTime0) {
            // If the current time is later than the time of sunset 0,
            // then pick sunset 1 and sunrise 2
            time_t sunsetTime1 = iso8601ToTime(sunsetArray[1], utcOffsetSeconds);
            time_t sunriseTime2 = iso8601ToTime(sunriseArray[2], utcOffsetSeconds);

            info.nextSunset = sunsetTime1;
            info.nextSunrise = sunriseTime2;
        } else {
            // If the current time is earlier than the time of sunset 0,
            // then pick sunset 0 and sunrise 1
            time_t sunriseTime1 = iso8601ToTime(sunriseArray[1], utcOffsetSeconds);

            info.nextSunset = sunsetTime0;
            info.nextSunrise = sunriseTime1;
        }

        // Calculate the total rain amount and average cloud cover
        uint16_t rainSum = 0;
        uint16_t cloudCoverSum = 0;
        uint16_t dataPoints = 0;

        for (size_t i = 0; i < timeArray.size(); i++) {
            time_t time = iso8601ToTime(timeArray[i], utcOffsetSeconds);

            // If the time has surpassed the sunrise, exit the loop
            if (time >= info.nextSunrise) {
                break;
            }

            // Accumulate data only for the time window between sunset and sunrise
            if (time >= info.nextSunset) {
                rainSum += static_cast<uint16_t>(rainArray[i].as<float>());
                cloudCoverSum += static_cast<uint16_t>(cloudCoverArray[i].as<float>());
                dataPoints++;
            }
        }

        if (dataPoints > 0) {
            info.rainAmount = static_cast<uint8_t>(rainSum);
            info.cloudCover = static_cast<uint8_t>(cloudCoverSum / dataPoints);
        }

        // Determine if dew is likely
        for (size_t i = 0; i < temperatureArray.size(); i++) {
            float temperature = temperatureArray[i];
            float dewPoint = dewPointArray[i];
            if (temperature - dewPoint < DEW_POINT_DIFF_THRESHOLD) {
                info.isDew = true;
                break;
            }
        }
    } else {
        // Handle HTTP error by logging to Serial for debugging
        Serial.print("Weather API query URL: ");
        Serial.println(url);

        // Print the response to help with debugging
        String payload = http.getString();
        Serial.println(payload);

        // Print a more descriptive error message
        Serial.print("HTTP GET failed, error: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end(); // Close the connection
    return info;
}
