#include <assert.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdint.h>
#include "imu.h"
#include "crc.h"

int serial_port;

#ifdef __linux__
static char *PORT_NAME = "/dev/ttyUSB0";
#else
static char *PORT_NAME = "/dev/cu.usbserial-A600DTJI";
#endif

#define SPINUP_ITER    10000
#define MAX_ITERATIONS 50000

uint64_t getTime() {
  struct timeval currentTime;

  assert(gettimeofday(&currentTime, NULL) == 0);

  return (currentTime.tv_sec * 1000000ULL) + currentTime.tv_usec;
}

void finish(int code, int fd) {
  imu_disconnect(fd);
  exit(code);
}

int main(int argc, char *argv[]) {
  int max_iterations = MAX_ITERATIONS;
  if (argc > 1) {
    PORT_NAME = argv[1];
    if (argc > 2) {
      max_iterations = atoi(argv[2]);
    }
  }

  printf("[TEST] Connecting to: %s\n", PORT_NAME);

  int fd = imu_connect(PORT_NAME);

  if (fd < 0) {
    exit(1);
  }

  int l = -1;
  int i = 0;

  crc_generate_table();

  while (i<max_iterations) {
    imu_datagram_t data = {0};
  	imu_read(fd, &data);

    printf("[TEST] hd: %X\tx: %f\ty: %f\tz: %f\twx: %f\twy: %f\twz: %f\t seq: %d\t stat: %X\t tp: %u\t crc: %X \t ckc: %X\t t: %llu\n", data.hd, data.x, data.y, data.z, data.wx, data.wy, data.wz, data.sequence, data.status, data.temperature, data.actual_crc, data.computed_crc, getTime());

    if (i > SPINUP_ITER) {
      if (data.status != 0x77) {
        printf("[FAIL] Bad Status! %X\n", data.status);
        exit(1);
      }

      if ((l + 1) % 128 != data.sequence) {
        printf("[FAIL] Skipped Sequence Number! %d not after %d\n", data.sequence, l);
        finish(1, fd);
      }

      if (data.temperature < 20 || data.temperature > 50) {
        printf("[FAIL] Danger temperature\n");
        finish(1, fd);
      }

      if (data.actual_crc != data.computed_crc) {
        printf("[FAIL] CRC error\n");
        finish(1, fd);
      }

      l = data.sequence;
      fflush(stdout);
    } else {
      if (i % 1000 == 0) {
        printf("[TEST] Spinning Up %d/%d\n", i, SPINUP_ITER);
      }
    }
    i++;
    l = data.sequence;

  }
  finish(0, fd);
  return 1;;
}

