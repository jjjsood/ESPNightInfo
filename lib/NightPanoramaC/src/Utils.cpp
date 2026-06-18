#include "Utils.h"
#include "SharedStructs.h"
#include <map>

/**
 * Converts an ISO8601 formatted time string to a time_t value, considering UTC offset.
 * It handles the year as an offset from 1970 to stay compatible with time_t definition.
 *
 * @param iso8601 The ISO8601 formatted string.
 * @param utcOffsetSeconds The number of seconds to offset from UTC.
 * @return The time_t representation of the given time string adjusted for UTC offset, or -1 if conversion fails.
 */
time_t iso8601ToTime(const char *iso8601, long utcOffsetSeconds)
{
    tmElements_t tm;
    int year, month, day, hour, minute;
    if (sscanf(iso8601, "%4d-%2d-%2dT%2d:%2d", &year, &month, &day, &hour, &minute) == 5)
    {
        tm.Year = year - 1970;
        tm.Month = month;
        tm.Day = day;
        tm.Hour = hour;
        tm.Minute = minute;
        tm.Second = 0;
        time_t time = makeTime(tm);
        return time != 0 ? time - utcOffsetSeconds : -1;
    }
    else
    {
        Serial.println("Error: Invalid ISO8601 format.");
        return -1;
    }
}

/**
 * Formats a time_t value to an ISO 8601 date-time string.
 * Assumes the input time is in UTC.
 *
 * @param time The time value to format.
 * @return A String representing the formatted date and time.
 */
String formatTimeISO8601(time_t time)
{
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", gmtime(&time));
    return String(buf);
}

/**
 * Converts decimal hours to a time_t value based on the provided date elements.
 * Handles conversion of decimal hours to a time structure.
 *
 * @param decimalHours The decimal representation of the hours.
 * @param dateElements The date elements to which the time will be added.
 * @return A time_t representation of the combined date and decimal hours.
 */
time_t convertDecimalHoursToTimeT(double decimalHours, tmElements_t &dateElements) {
    int hours = static_cast<int>(decimalHours);
    int minutes = static_cast<int>((decimalHours - hours) * 60);
    int seconds = static_cast<int>(((decimalHours - hours) * 60 - minutes) * 60);
    dateElements.Hour = hours;
    dateElements.Minute = minutes;
    dateElements.Second = seconds;
    return makeTime(dateElements);
}

/**
 * Prints a human-readable representation of a time_t value, adjusted for the UTC offset.
 * Outputs the time in DD/MM/YYYY HH:MM:SS format.
 *
 * @param rawTime The raw time_t value.
 * @param utcOffsetSeconds The number of seconds to offset from UTC.
 */
void printHumanReadableTime(time_t rawTime, long utcOffsetSeconds)
{
    tmElements_t tm;
    breakTime(rawTime, tm);
    time_t localTime = rawTime + utcOffsetSeconds;
    breakTime(localTime, tm);
    Serial.print(tm.Day);
    Serial.print("/");
    Serial.print(tm.Month);
    Serial.print("/");
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.print(" ");
    Serial.print(tm.Hour);
    Serial.print(":");
    Serial.print(tm.Minute < 10 ? "0" : "");
    Serial.print(tm.Minute);
    Serial.print(":");
    Serial.print(tm.Second < 10 ? "0" : "");
    Serial.println(tm.Second);
}

// Map to associate strings with enum values for celestial objects.
static const std::map<std::string, CelestialObject> nameToEnumMap = {
     {"Moon", Moon},
    {"Mercury", Mercury},
    {"Venus", Venus},
    {"Mars", Mars},
    {"Jupiter", Jupiter},
    {"Saturn", Saturn},
    {"Uranus", Uranus},
    {"Neptune", Neptune}
};

/**
 * Converts a string to the corresponding CelestialObject enum value.
 * If the string does not match any celestial object, it returns Undefined.
 *
 * @param name The string representing the celestial object's name.
 * @return The corresponding CelestialObject enum value or Undefined if not found.
 */
CelestialObject stringToEnum(const std::string& name) {
    auto it = nameToEnumMap.find(name);
    if (it != nameToEnumMap.end()) {
        return it->second;
    }
    Serial.print("Error: '");
    Serial.print(name.c_str());
    Serial.println("' is not a valid celestial object name.");
    return CelestialObject::Undefined;
}
