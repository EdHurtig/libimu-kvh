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
#include <zlib.h>

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

unsigned long bytesToInt_2(unsigned char b0, unsigned char b1)
{
    unsigned long output;

    *((unsigned char*)(&output) + 1) = b0;
    *((unsigned char*)(&output) + 0) = b1;

    return output;
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

int byteParser(unsigned char *hexData_in, float *IMUData_in)
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
  uint16_t temp = bytesToInt_2(hexData_in[i], hexData_in[i+1]);
  i = i + 2;
  int crc = bytesToFloat_4(hexData_in[i], hexData_in[i+1], hexData_in[i+2], hexData_in[i+3]);


  if(status == 0x77)
  {

//    if(crc ^ 0xFFFFFFFF)
  //  {
	printf("x: %f\ty: %f\tz: %f\twx: %f\twy: %f\twz: %f\t tp: %lu\t %llu\n", xAcc, yAcc, zAcc, xRot, yRot, zRot, temp, getTime());
        IMUData_in[0] = xRot;
        IMUData_in[1] = yRot;
        IMUData_in[2] = zRot;
        IMUData_in[3] = xAcc;
        IMUData_in[4] = yAcc;
        IMUData_in[5] = zAcc;

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

  IMUData_in[0] = xRot;
  IMUData_in[1] = yRot;
  IMUData_in[2] = zRot;
  IMUData_in[3] = xAcc;
  IMUData_in[4] = yAcc;
  IMUData_in[5] = zAcc;
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
          chars_read += read(serial_port, &read_buffer[chars_read], max_chars_to_read - chars_read);
          // TODO: When Non-Blocking is truly implemented this needs to pass and
          //       the caller needs to handle the case when fewer bytes are read
          if (chars_read < 0) {
            perror("===Read Failed===");
            return -1;
          }
        }
        return chars_read;
}

int serial_port_open(void)
{
  struct termios options;

  serial_port = open(PORT_NAME, O_RDWR | O_NOCTTY);

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

int identifyHeader(){
  int c = 0;
	while(true){
		unsigned char firstByte[1] = {0};
		serial_port_read(firstByte, 1);
    c ++;
//		printf("%x\n", *firstByte);

		if(*firstByte == 0xFE){
			unsigned char restOfHeader[3] = {0};
			serial_port_read(restOfHeader, 3);
      c += 3;
			unsigned char ideal[3] = {0x81, 0xFF,0x55};
			int i;
			int success = 1;
			for (i = 0; i < 3; i++) {
//	printf("%x\n", restOfHeader[i]);
				if (restOfHeader[i] != ideal[i]) {
					success = 0;
				}
			}
//	printf("\n");
			if (success) {
        if (c != 4) {
          printf("Header Read: %d.  %llu\n", c, getTime());
        }
				return 0;
			}
		}
    printf("Not In Sync. %llu\n", getTime());
	}

}

void readIMU(unsigned char *hexData_in){
	//read from a serial port with a specific header

	identifyHeader();
	serial_port_read(hexData_in, 32);
	return;
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

  unsigned char data[36] = {
    0xFE,
    0x81,
    0xFF,
    0x55,
    0x37,
    0xA9,
    0x6A,
    0x6E,
    0x38,
    0x58,
    0x6C,
    0x1F,
    0xB7,
    0x5B,
    0xF8,
    0x62,
    0xBF,
    0x80,
    0x3E,
    0x78,
    0xBB,
    0x65,
    0x0D,
    0x28,
    0x3B,
    0x0A,
    0x37,
    0xAC,
    0x77,
    0x3D,
    0x00,
    0x28,
    0x4B,
    0xFA,
    0x34,
    0xD8};

  unsigned long crc = crc32(0L, Z_NULL, 0);
  printf("%lu\n", crc);
  printf("%X\n", crc32(crc, &data[0], 32));

	unsigned char *hexData = malloc(sizeof(char) * 32);
  serial_port_open();
  int i = 0;
  while (1) {
  	readIMU(hexData);

  	float IMUData[6] = { 0 };
  	byteParser(hexData, &IMUData[0]);
    fflush(stdout);
    i++;
  }
	serial_port_close();
}
