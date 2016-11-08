/*****************************************************************************
 * Copyright (c) OpenLoop, 2016
 *
 * This material is proprietary of The OpenLoop Alliance and its members.
 * All rights reserved.
 * The methods and techniques described herein are considered proprietary
 * information. Reproduction or distribution, in whole or in part, is
 * forbidden except by express written permission of OpenLoop.
 *
 * Source that is published publicly is for demonstration purposes only and
 * shall not be utilized to any extent without express written permission of
 * OpenLoop.
 *
 * Please see http://www.opnlp.co for contact information
 ****************************************************************************/

#include "imu.h"
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <zlib.h>
#include <errno.h>
#include "crc.h"

#define IMU_MESSAGE_SIZE 36

#ifndef B921600
#define B921600 921600
#endif

union bytes_to_float
{
   unsigned char b[4];
   float f;
} b2f;


// example datagram, MSB is always printed first
// uint8_t example[32] = {
//   0xFE, 0x81, 0xFF, 0x55, // Message Header, exactly these bytes
//   0x01, 0x23, 0x45, 0x65, // X rotational (4 Bytes to form a float)
//   0x01, 0x23, 0x45, 0x65, // Y rotational (4 Bytes to form a float)
//   0x01, 0x23, 0x45, 0x65, // Z rotational (4 Bytes to form a float)
//   0x01, 0x23, 0x45, 0x65, // Z linear (4 Bytes to form a float)
//   0x01, 0x23, 0x45, 0x65, // Y linear  (4 Bytes to form a float)
//   0x01, 0x23, 0x45, 0x65, // Z linear  (4 Bytes to form a float)
//   0xEE, // Status (each bit wx, wy, wz, reserved, ax, ay, az, reserved)
//   0xFF, // sequence number (0-127 wraps)
//   0x00, 0x01 // temperature bits (UInt16)
// };

unsigned char imubuf[IMU_MESSAGE_SIZE] = {0};

/**
 * fills the buffer pointed to (should be imubuf) until it contains n elements
 */
int serial_read(int fd, unsigned char *buf, int n) {

  int r = 0;
  while (r < n) {
    int _r = read(fd, &buf[r], n - r);

    if (_r < 0) {
      if (errno != EAGAIN) {
        perror("===Read Failed===");
        return -1;
      }
    }
    r += _r;
  }
  return r;
}

int imu_read(int fd, imu_datagram_t * gram) {

  while (1) {
    if (serial_read(fd, &imubuf[0], 1) < 0) {
      return -1;
    }


    unsigned char ideal[4] = {0xFE, 0x81, 0xFF,0x55};

    if (imubuf[0] == ideal[0]) {
      if (serial_read(fd, &imubuf[1], 3) < 0) {
        return -1;
      }

      int i;
      int success = 1;
      for (i = 1; i < 4; i++) {
        if (imubuf[i] != ideal[i]) {
          success = 0;
        }
      }

      if (success) {
        break;
      }
    }
  }

  serial_read(fd, &imubuf[4], 32);

  int i;
  for (i = 0; i < sizeof(imubuf); i++) {
    printf("%x ", imubuf[i]);
  }
  printf("\n");

  // Massive Bit Shifting Operation.
  // See the example imu_datagram_t in the comment at the top of this file
  *gram = (imu_datagram_t){
    .hd = (imubuf[0] << 24) | (imubuf[1] << 16) | (imubuf[2] << 8) | imubuf[3],
    .wx = ((union bytes_to_float) { .b = { imubuf[7], imubuf[6], imubuf[5], imubuf[4] } }).f,
    .wy = ((union bytes_to_float) { .b = { imubuf[11], imubuf[10], imubuf[9], imubuf[8] } }).f,
    .wz = ((union bytes_to_float) { .b = { imubuf[15], imubuf[14], imubuf[13], imubuf[12] } }).f,
    .x = ((union bytes_to_float) { .b = { imubuf[19], imubuf[18], imubuf[17], imubuf[16] } }).f,
    .y = ((union bytes_to_float) { .b = { imubuf[23], imubuf[22], imubuf[21], imubuf[20] } }).f,
    .z = ((union bytes_to_float) { .b = { imubuf[27], imubuf[26], imubuf[25], imubuf[24] } }).f,
    .status = imubuf[28],
    .sequence = imubuf[29],
    .temperature = (imubuf[30] << 8) | (imubuf[31]),
    .actual_crc = (imubuf[32] << 24) | (imubuf[33] << 16) | (imubuf[34] << 8) | (imubuf[35] << 0),
    .computed_crc = crc_calc(&imubuf[0], 32)
  };

  if(gram->status == 0x77) {
    return 0;
  } else {
    return -1;
  }
}


// Connect the serial device for the IMU
int imu_connect(const char * device) {
  // note("Connecting to IMU at: %s", device);
  if (access(device, F_OK) != 0) {
    // error_no("device '%s' does not exist", device);
    perror("Given IMU Device Does not exist");
    return -1;
  }

  int fd = open(device, O_RDWR);

  if (fd < 0) {
    perror("IMU connect failed");
    return -1;
  }

  struct termios opts;

  tcgetattr(fd, &opts);
  cfsetispeed(&opts, B921600);
  cfsetospeed(&opts, B921600);
  opts.c_cflag |= (CLOCAL | CREAD | CS8);
  tcsetattr(fd, TCSANOW, &opts);

  return fd;
}

// Connect the serial device for the IMU
int imu_disconnect(int fd) {
  if (fd < 0) {
    fprintf(stderr, "IMU fd invalid");
    return -1;
  }

  // TODO: Reset Settings?

  return close(fd);
}
