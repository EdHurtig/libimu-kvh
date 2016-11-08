/**
 * This material is proprietary of The OpenLoop Alliance and its members.
 *
 * All rights reserved.
 *
 * The methods and techniques described herein are considered proprietary
 * information. Reproduction or distribution, in whole or in part, is forbidden
 * except by express written permission of OpenLoop.
 *
 * Source that is published publicly is for demonstration purposes only and shall
 * not be utilized to any extent without express written permission of OpenLoop.
 */

#ifndef __OPENLOOPALLIANCE_IMU_H__
#define __OPENLOOPALLIANCE_IMU_H__

#include <stdint.h>
#include <unistd.h>

/**
 * Represents an IMU datagram packet
 */
typedef struct imu_datagram {
  // header
  uint32_t hd;
  // linear acceleration X
  float x;
  // linear acceleration Y
  float y;
  // linear acceleration Z
  float z;
  // angular acceleration X
  float wx;
  // angular acceleration Y
  float wy;
  // angular acceleration Z
  float wz;
  // sequence
  uint8_t sequence;
  // status
  uint8_t status;
  // temperature
  uint16_t temperature;
  // CRC check (actual)
  uint32_t actual_crc;
  // CRC check (computed)
  uint32_t computed_crc;
} imu_datagram_t;

/**
 * Connect to the IMU on the given device path
 *
 * @param path The path to the IMU device, ex. "/dev/ttyUSB0"
 *
 * @return file descriptor on success, -1 on failure
 */
int imu_connect(const char * device);

/**
 * Disconnect the IMU
 *
 * @param fd The fd of the connected IMU
 *
 * @return the result of close(2)
 */
int imu_disconnect(const int fd);


/**
 * Connect to the IMU on the given device path
 *
 * @param fd The file descriptor for the IMU
 * @param path The path to the IMU device, ex. "/dev/ttyUSB0"
 *
 * @return 0 on success, -1 on failure
 */
int imu_read(int fd, imu_datagram_t * data);

#endif
