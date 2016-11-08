#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include "imu.h"
#include "crc.h"

int serial_port;

#ifdef __linux__
static const char *PORT_NAME = "/dev/ttyUSB0";
#else
static const char *PORT_NAME = "/dev/cu.usbserial-A600DTJI";
#endif

#define SPINUP_ITER 10000

uint64_t getTime() {
  struct timeval currentTime;

  assert(gettimeofday(&currentTime, NULL) == 0);

  return (currentTime.tv_sec * 1000000ULL) + currentTime.tv_usec;
}

int main() {
  int fd = imu_connect(PORT_NAME);

  if (fd < 0) {
    exit(1);
  }

  int l = -1;
  int i = 0;

  crc_generate_table();

  while (1) {
    imu_datagram_t data = {0};
  	imu_read(fd, &data);

    printf("hd: %X\tx: %f\ty: %f\tz: %f\twx: %f\twy: %f\twz: %f\t seq: %d\t stat: %X\t tp: %u\t crc: %X \t ckc: %X\t t: %llu\n", data.hd, data.x, data.y, data.z, data.wx, data.wy, data.wz, data.sequence, data.status, data.temperature, data.actual_crc, data.computed_crc, getTime());
    #define CONFIG_CMD "=config,1\n"


    if (i > SPINUP_ITER) {
      if (data.status != 0x77) {
        printf("[FAIL] Bad Status! %X\n", data.status);
        exit(1);
      }

      if ((l + 1) % 128 != data.sequence) {
        printf("[FAIL] Skipped Sequence Number! %d not after %d\n", data.sequence, l);
        exit(1);
      }

      if (data.temperature < 20 || data.temperature > 50) {
        printf("[FAIL] Danger temperature\n");
        exit(1);
      }

      if (data.actual_crc != data.computed_crc) {
        printf("[FAIL] CRC error\n");
        exit(1);
      }

      l = data.sequence;
      fflush(stdout);
    } else {
      if (i % 1000 == 0) {
        printf("Spinning Up %d/%d\n", i, SPINUP_ITER);
      }
      i++;
    }
    l = data.sequence;

  }
  imu_disconnect(fd);
}
