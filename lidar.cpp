#include "lidar.h"

/* Default I2C Address of LIDAR-Lite. */
#define LIDARLite_Adr   0x62

/* Commands */
#define SET_CommandReg       0x00   /// Register to write to initiate ranging
#define AcqMode              0x04   /// Value to set in control register to initiate ranging
 
/* Read Registers */
#define GET_DistanceHBReg    0x0f   /// High byte of distance reading data
#define GET_DistanceLBReg    0x10   /// Low byte of distance reading data
#define GET_Distance2BReg    0x8f   /// Register to get both High and Low bytes of distance reading data in 1 call
#define GET_VelocityReg      0x09   /// Velocity measutement data
#define GET_Status           0x01


Lidar::Lidar(I2CDriver* driver)
    : driver_(driver)
{
}

int Lidar::getRange()
{   
    uint8_t write[2] = {SET_CommandReg, AcqMode};
    uint8_t read_dist[1] = {GET_Distance2BReg};
    uint8_t is_ready[1] = {GET_Status};
 
    if (i2cMasterTransmitTimeout(driver_, LIDARLite_Adr, write, 2, 0, 0, TIME_INFINITE) != MSG_OK)
        return -1;

    uint8_t lidarStatus = 1;
    do {
        // Here is a catch. i2cMasterTransmitTimeout can also receive response in one call.
        // But it uses repeated start technique, which makes that transaction atomic.
        // LidarLite does not support repeated start so separate call must be used.
        if (i2cMasterTransmitTimeout(driver_, LIDARLite_Adr, is_ready, 1, 0, 0, TIME_INFINITE) != MSG_OK)
            return -1;
        if (i2cMasterReceiveTimeout(driver_, LIDARLite_Adr, &lidarStatus, 1, TIME_INFINITE) != MSG_OK)
            return -1;
    } while (lidarStatus & 1);

    // Read out two bytes with the distance
    uint8_t dstBytes[2];
    if (i2cMasterTransmitTimeout(driver_, LIDARLite_Adr, read_dist, 1, 0, 0, TIME_INFINITE) != MSG_OK)
        return -1;
    if (i2cMasterReceiveTimeout(driver_, LIDARLite_Adr, dstBytes, 2, TIME_INFINITE) != MSG_OK)
        return -1;
    return (dstBytes[0] << 8) | dstBytes[1];
}
