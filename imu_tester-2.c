#include <stdbool.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>

// Darwin's termios.h does not go this high
#ifndef B921600
#define B921600 921600
#endif

unsigned char serial_buffer[40960];

int serial_port;
struct termios options_original;

#ifdef __linux__
static const char *PORT_NAME = "/dev/ttyUSB0";
#else
static const char *PORT_NAME = "/dev/cu.usbserial-A600DTJI";
#endif

unsigned long long getTime() {
  struct timeval currentTime;

  assert(gettimeofday(&currentTime, NULL) == 0);

  return (currentTime.tv_sec * 1000000ULL) + currentTime.tv_usec;
}

float bytesToFloat_4(unsigned char b0, unsigned char b1, unsigned char b2, unsigned char b3)
{
    float output;

    *((unsigned char*)(&output) + 3) = b0;
    *((unsigned char*)(&output) + 2) = b1;
    *((unsigned char*)(&output) + 1) = b2;
    *((unsigned char*)(&output) + 0) = b3;

    return output;
}

float bytesToFloat_2(unsigned char b0, unsigned char b1)
{
    float output;

    *((unsigned char*)(&output) + 3) = b0;
    *((unsigned char*)(&output) + 2) = b1;

    return output;
}

int byteParser(unsigned char *hexData_in)
{
  int i = 0;

 //Declare & intitialize
  float xRot = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  float yRot = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  float zRot = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  float xAcc = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  float yAcc = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  float zAcc = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);
  i = i + 4;
  unsigned char status = hexData_in[i];
  i = i + 1;
  unsigned int seqnum = hexData_in[i];
  i = i + 1;
  // REVIEW: temp is an int16, not a float
  float temp = bytesToFloat_2(hexData_in[i], hexData_in[i+1]);
  i = i + 2;
  int crc = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);


  if(status == 0x77)
  {

//    if(crc ^ 0xFFFFFFFF)
  //  {
	printf("x: %f\ty: %f\tz: %f\twx: %f\twy: %f\twz: %f\t t: %llu\n", xAcc, yAcc, zAcc, xRot, yRot, zRot, getTime());

        return 0;
   // }
   // else
   // {
        printf("Warning: Possible Error %llu\n", getTime());
   // }
  }
  else
  {
    printf("Bad Status: %llu\n", getTime());
  }

  return -1;
}

void serial_port_close()
{
        tcsetattr(serial_port,TCSANOW,&options_original);
        close(serial_port);
}

int serial_port_read(unsigned char *read_buffer, size_t max_chars_to_read)
{
        int chars_read = 0;
        while (chars_read < max_chars_to_read) {
          int c = read(serial_port, &read_buffer[chars_read], max_chars_to_read - chars_read);
          // TODO: When Non-Blocking is truly implemented this needs to pass and
          //       the caller needs to handle the case when fewer bytes are read
          if (c < 0) {
            if (errno == EAGAIN) {
              printf(".");
              fflush(stdout);
              usleep(50000);
            } else {
              perror("===Read Failed===");
              return -1;
            }
          } else {
            chars_read += c;
          }
        }
        return chars_read;
}

int serial_port_open(void)
{
  struct termios options;

  serial_port = open(PORT_NAME, O_RDWR | O_NONBLOCK | O_NOCTTY);

  if (serial_port != -1)
  {
          tcgetattr(serial_port,&options_original);
          tcgetattr(serial_port, &options);
          cfsetispeed(&options, B921600);
          cfsetospeed(&options, B921600);
          options.c_cflag |= (CLOCAL | CREAD | CS8);
          tcsetattr(serial_port, TCSANOW, &options);
  }
  else
          printf("Unable to open /dev/ttyUSB0\n");
  return (serial_port);
}

unsigned char * identifyHeader(unsigned char *buffer){
	serial_port_read(buffer, 72); // Read in 2 times the packet size
//		printf("%x\n", *firstByte);
  // scan the entire buffer for a header
  int i;
  unsigned const char ideal[4] = {0xFE, 0x81, 0xFF,0x55};

  for (i=0;i<37;i++) {
  		int j;
  		int success = 1;
  		for (j = 0; j < 4; j++) {
  			if (buffer[i + j] != ideal[j]) {
  				success = 0;
          break;
  			}
  		}

  		if (success) {
  			return &buffer[i];
  		}

  }
  printf("Not In Sync. %llu\n", getTime());

  return NULL;
}

unsigned char * readIMU(unsigned char *hexData_in){
	//read from a serial port with a specific header

	return identifyHeader(hexData_in);
}

// unsigned char[] byteParser(unsigned char* hexData){
// 	//split the hexdata into an array for different data values
// 	unsigned char xRot, yRot, zRot, yAcc, zAcc, status, seqnum, temp, crc;
// 	unsigned char data[10];
// 	xRot = hexData[0:4];
// 	yRot = hexData[4:8];
// 	zRot = hexData[8:12];
// 	xAcc = hexData[12:16];
// 	yAcc = hexData[16:20];
// 	zAcc = hexData[20:24];
// 	status = hexData[24];
// 	seqnum = hexData[25];
// 	temp = hexData[26:28];
// 	crc = hexData[28:32] ;
// 	IMUData = [xRot,yRot,zRot,xAcc,yAcc,zAcc,status,seqnum,temp,crc];
// 	return IMUData

// }

int main() {
	unsigned char hexData[72];
  serial_port_open();

  while (1) {
  	unsigned char * packet = readIMU(hexData);
    if (packet == NULL) {
      printf("=========Failed\n=======");
      continue;
    }

    int i;
    for (i=0; i<36; i++) {
      printf("%x ", packet[i]);
    }

    printf("\n");

    byteParser(&packet[4]);

    fflush(stdout);
  }
	serial_port_close();
}
