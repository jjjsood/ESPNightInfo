#ifndef STARGAZINGINFO_H
#define STARGAZINGINFO_H

#include "WeatherInfo.h"
#include "CelestialInfo.h"

struct StargazingInfo {
    WeatherInfo weather;
    CelestialInfo celestial;
};

StargazingInfo getStargazingInfo(const GeoLocation& location, const FieldOfView& fov);

#endif // STARGAZINGINFO_H
