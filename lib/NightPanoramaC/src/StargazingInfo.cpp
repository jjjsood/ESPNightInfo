#include "StargazingInfo.h"

/**
 * Retrieves stargazing information based on a given geographical location and field of view.
 * This function first calls `getWeatherInfo` to obtain the current weather conditions,
 * which include data about dew, rain, cloud cover, and the times for the next sunset and sunrise.
 * It then calls `getCelestialInfo` to get information about celestial bodies that are
 * visible between the times of the next sunset and sunrise, within the provided field of view.
 *
 * @param location The geographical location for which stargazing info is requested.
 * @param fov The field of view from the observer's location.
 * @return StargazingInfo A struct containing weather conditions and visible celestial bodies.
 */
StargazingInfo getStargazingInfo(const GeoLocation &location, const FieldOfView &fov)
{
    StargazingInfo stargazingInfo;

    // Get weather information
    WeatherInfo weather = getWeatherInfo(location);
    stargazingInfo.weather = weather;

    // Use sunset and sunrise times from the weather info to get celestial information
    CelestialInfo celestial = getCelestialInfo(location, weather.nextSunset, weather.nextSunrise, fov);
    stargazingInfo.celestial = celestial;
    return stargazingInfo;
}
