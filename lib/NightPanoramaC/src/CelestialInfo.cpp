#include <SiderealPlanets.h>
#include <random>
#include "CelestialInfo.h"
#include "Utils.h"

// Instance to perform astronomical calculations.
SiderealPlanets astro;

// Random number generator for simulation purposes.
std::random_device rd;
std::mt19937 gen(rd());

// Array of celestial body names.
const char *names[MAX_CELESTIAL_BODIES] = {"Moon", "Mercury", "Venus", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};

// Initialize astronomical calculations with a given geographic location.
void initSiderealPlanets(const GeoLocation &location)
{
    astro.setLatLong(location.latitude, location.longitude);
}

// Compute rise and set times for a celestial object.
RiseAndSet getRiseAndSetTimes(const CelestialObject &object, time_t sunset)
{
    RiseAndSet times = {0, 0};
    tmElements_t timeElements;
    breakTime(sunset, timeElements);
    astro.setGMTdate(timeElements.Year + 1970, timeElements.Month, timeElements.Day);
    bool hasRisen = false;

    switch (object)
    {
    case Moon:
        astro.doMoon();
        astro.doRAdec2AltAz();
        astro.doMoonRiseSetTimes();
        break;
    case Mercury:
        astro.doMercury();
        astro.doRAdec2AltAz();
        astro.doRiseSetTimes(0.0);
        break;
    case Venus:
        astro.doVenus();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    case Mars:
        astro.doMars();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    case Jupiter:
        astro.doJupiter();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    case Saturn:
        astro.doSaturn();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    case Uranus:
        astro.doUranus();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    case Neptune:
        astro.doNeptune();
        astro.doRAdec2AltAz();
        hasRisen = astro.doRiseSetTimes(0.0);
        break;
    default:
        break;
    }

    if (hasRisen)
    {
        times.riseTime = convertDecimalHoursToTimeT(astro.getRiseTime(), timeElements);
        times.setTime = convertDecimalHoursToTimeT(astro.getSetTime(), timeElements);
    }

    return times;
}

// Determine the altitude and azimuth for a celestial object at culmination.
AlmanacData calculateAlmanacData(const CelestialObject &object, time_t riseTime, time_t setTime)
{
    AlmanacData result;
    time_t culminationTime = riseTime + (setTime - riseTime) / 2;
    tmElements_t timeElements;
    breakTime(culminationTime, timeElements);
    astro.setGMTdate(timeElements.Year + 1970, timeElements.Month, timeElements.Day);
    astro.setGMTtime(timeElements.Hour, timeElements.Minute, timeElements.Second);

    switch (object)
    {
    case Moon:
        astro.doMoon();
        break;
    case Mercury:
        astro.doMercury();
        break;
    case Venus:
        astro.doVenus();
        break;
    case Mars:
        astro.doMars();
        break;
    case Jupiter:
        astro.doJupiter();
        break;
    case Saturn:
        astro.doSaturn();
        break;
    case Uranus:
        astro.doUranus();
        break;
    case Neptune:
        astro.doNeptune();
        break;
    default:
        break;
    }

    astro.doRAdec2AltAz();
    result.hc = astro.getAltitude();
    result.zn = astro.getAzimuth();

    return result;
}

// Generate random AlmanacData for testing or simulation.
AlmanacData calculateAlmanacData()
{
    AlmanacData result;
    std::uniform_real_distribution<float> altDist(-90.0f, 90.0f);
    result.hc = altDist(gen);
    std::uniform_real_distribution<float> aziDist(0.0f, 359.0f);
    result.zn = aziDist(gen);

    return result;
}

// Check if a celestial body is visible within the observer's field of view.
bool isVisible(const FieldOfView &fov, const AlmanacData &data)
{
    if (data.hc <= 0)
        return false;

    if (fov.leftBound == 0 && fov.rightBound == 360)
        return true;

    uint16_t normalizedZN = static_cast<uint16_t>(round(fmod(data.zn, 360.0)));
    uint16_t normalizedLeftBound = fov.leftBound % 360;
    uint16_t normalizedRightBound = fov.rightBound % 360;

    return (normalizedZN >= normalizedLeftBound && normalizedZN <= normalizedRightBound) ||
           (normalizedLeftBound > normalizedRightBound &&
            (normalizedZN >= normalizedLeftBound || normalizedZN <= normalizedRightBound));
}

/**
 * Retrieves information about celestial bodies based on a given geographical location, times of sunset and sunrise, and the observer's field of view.
 * The function initializes the SiderealPlanets object with the provided location to perform astronomical calculations.
 * It calculates the rise and set times for the celestial bodies using `getRiseAndSetTimes` and determines their culminating position using `calculateAlmanacData`.
 * Visibility of each celestial body within the specified field of view is assessed using the `isVisible` function.
 *
 * @param location The geographical coordinates where observations are made.
 * @param sunset The expected time of the next sunset at the given location.
 * @param sunrise The expected time of the next sunrise at the given location.
 * @param fov The field of view from the observer's location.
 * @return CelestialInfo A struct containing an array of CelestialBodyInfo for each body, and the count of bodies.
 */
CelestialInfo getCelestialInfo(const GeoLocation &location, time_t sunset, time_t sunrise, const FieldOfView &fov)
{
    initSiderealPlanets(location);

    CelestialInfo info;
    info.bodyCount = MAX_CELESTIAL_BODIES;

    for (int i = 0; i < MAX_CELESTIAL_BODIES; ++i)
    {
        strncpy(info.bodies[i].name, names[i], sizeof(info.bodies[i].name) - 1);
        info.bodies[i].name[sizeof(info.bodies[i].name) - 1] = '\0';
        RiseAndSet riseAndSet = getRiseAndSetTimes(static_cast<CelestialObject>(i), sunset);
        info.bodies[i].riseAndSet = riseAndSet;
        info.bodies[i].positionCulmination = calculateAlmanacData(static_cast<CelestialObject>(i), info.bodies[i].riseAndSet.riseTime, info.bodies[i].riseAndSet.setTime);
        if ((sunrise > riseAndSet.setTime && sunset < riseAndSet.riseTime) ||
            (sunrise < riseAndSet.riseTime && sunset > riseAndSet.setTime))
        {
            info.bodies[i].isVisible = isVisible(fov, info.bodies[i].positionCulmination);
        }
        else
        {
            info.bodies[i].isVisible = false;
        }
    }
    return info;
}
