#pragma once

#include <ch.h>
#include <hal.h>

class Lidar {
public:
    Lidar(I2CDriver* driver);

    // Returns range in centimeters or -1 in case of error
    int getRange();

private:
    I2CDriver* driver_;
};
