#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include "SharedStructs.h"

struct WeatherInfo {
    float isDew;
    uint8_t rainAmount;
    uint8_t cloudCover;
    time_t nextSunset;
    time_t nextSunrise;

};

WeatherInfo getWeatherInfo(const GeoLocation& location);

#endif // WEATHERINFO_H
