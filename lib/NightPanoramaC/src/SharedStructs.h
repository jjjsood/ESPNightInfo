#ifndef SHAREDSTRUCTS_H
#define SHAREDSTRUCTS_H

#include <stdint.h> // For datatypes
#include <TimeLib.h> // Include Time library


// Celestrial Bodies
enum CelestialObject
{
    Moon,
    Mercury,
    Venus,
    Mars,
    Jupiter,
    Saturn,
    Uranus,
    Neptune,
    Undefined
};

// Define a struct for the observer's location
struct GeoLocation {
    float latitude;
    float longitude;
};

// Define a struct for field of view
struct FieldOfView {
    uint16_t leftBound;
    uint16_t rightBound;
};

#endif // SHAREDSTRUCTS_H