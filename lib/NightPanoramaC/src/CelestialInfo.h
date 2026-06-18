#ifndef CELESTIALINFO_H
#define CELESTIALINFO_H

#include "SharedStructs.h"

struct AlmanacData
{
    float hc;
    float zn;
};

struct RiseAndSet
{
    time_t riseTime;
    time_t setTime;
};

struct CelestialBodyInfo
{
    char name[10];
    RiseAndSet riseAndSet;
    AlmanacData positionCulmination;
    bool isVisible;
};

const int MAX_CELESTIAL_BODIES = 8;


struct CelestialInfo
{
    CelestialBodyInfo bodies[MAX_CELESTIAL_BODIES];
    uint8_t bodyCount;
};

CelestialInfo getCelestialInfo(const GeoLocation &location, time_t sunset, time_t sunrise, const FieldOfView &fov);

#endif // CELESTIALINFO_H
