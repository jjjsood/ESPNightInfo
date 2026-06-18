#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <TimeLib.h>
#include "SharedStructs.h"
#include <string>

time_t iso8601ToTime(const char *iso8601, long utcOffsetSeconds);
String formatTimeISO8601(time_t time);
time_t convertDecimalHoursToTimeT(double decimalHours, tmElements_t &dateElements);
void printHumanReadableTime(time_t rawTime, long utcOffsetSeconds);
CelestialObject stringToEnum(const std::string& name);

#endif // UTILS_H
