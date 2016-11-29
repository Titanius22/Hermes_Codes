/*

COMMANDS:

COMMAND: 1 latitude longitude elevation
RETURN: (printed successful)
Sets the ground station's GPS location. It is usually only called once before tracking begins but has no limitation on number of calls. It is used in determining the pointing direction of the antenna during tracking. This command call has the integer command number and 3 double arguments. The arguments of latitude and longitude are in decimal degrees and elevation is in meters. Returns 1 if sucessful, -1 if not.

COMMAND: 2 latitude longitude elevation
RETURN: (printed successful)
Sets the tracking object's GPS location. This command it called with a high frequency but no more than 1 Hz. This command call has the integer command number and 3 double arguments. The arguments of latitude and longitude are in decimal degrees and elevation is in meters. Returns 1 if sucessful, -1 if not.

COMMAND: 3
RETURN: (printed azimuth elevation)
This command tells the tracking system to return the rotor's current azimuth and elevation. The values are then sent as a string of azimuth and elevation angles seperated by a space. The angles are in integer degrees.
*/


#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
 
// The PiWeather board i2c address
#define ADDRESS 0x04
 
// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";


//argc: the number of argments sent to main()
//argv: the actual arguments
int main(int argc, char* argv[]) { 
 
	if (argc == 1) {
		printf("Supply one or more commands to send to the Arduino\n");
		exit(1);
	}
 
	printf("I2C: Connecting\n");
	int file;
 
	if ((file = open(devName, O_RDWR)) < 0) {
		fprintf(stderr, "I2C: Failed to access %d\n", devName);
		exit(1);
	}

	printf("I2C: acquiring buss to 0x%x\n", ADDRESS);

	if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
		fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
		exit(1);
	}

	int commandNum;
	int longitude;
	int latitude;
	int elevation;
	int azimuthAngle;
	int elevationAngle;
	double numBuf;
	
	unsigned char cmd[40];
	char buf[10];
	struct timespec req={0},rem={0};
	char *ptr;
	char *ptr_e;
		
	switch (strtol(argv[1], &ptr_e, 10)){
		case 1:
			if (argc != 5) {
				fprintf(stderr, "Invalid number of parameters for case 1\n");
				exit(1);
			}
			sscanf(argv[2], "%d", &longitude);
			sscanf(argv[3], "%d", &latitude);
			sscanf(argv[4], "%d", &elevation);
			
			printf("Sending 1 %d %d %d\n", longitude, latitude, elevation);
			
			sprintf(cmd, "1 %d %d %d", longitude, latitude, elevation);
			if (write(file, cmd, strlen(cmd)) > 0) {
		 
				// As we are not talking to direct hardware but a microcontroller we
				// need to wait a short while so that it can respond.
				//
				// 1ms seems to be enough but it depends on what workload it has
				req.tv_nsec = 10000000; //10ms
				nanosleep(req,rem);

				if (read(file, buf, 1) == 1) {
					if ((signed int) buf[0] == 1){
						printf("Success, ground Station location set\n");
					} 
					else {
						printf("Failed, received error from rotor, ground Station location not set\n");
					}
				}
				else {
						printf("Failed, can't read received signal from from rotor, new balloon location not set\n");
				}
			}
			break;

		case 2:
			if (argc != 5) {
				fprintf(stderr, "Invalid number of parameters for case 2\n");
				exit(1);
			}
			sscanf(argv[2], "%d", &longitude);
			sscanf(argv[3], "%d", &latitude);
			sscanf(argv[4], "%d", &elevation);
			
			printf("Sending 2 %d %d %d\n", longitude, latitude, elevation);
			
			sprintf(cmd, "2 %d %d %d", longitude, latitude, elevation);
			if (write(file, cmd, strlen(cmd)) > 0) {
		 
				// As we are not talking to direct hardware but a microcontroller we
				// need to wait a short while so that it can respond.
				//
				// 1ms seems to be enough but it depends on what workload it has
				req.tv_nsec = 10000000; //10ms
				nanosleep(req,rem);

				if (read(file, buf, 1) == 1) {
					if ((signed int) buf[0] == 1){
						printf("Success, new balloon location set\n");
					} 
					else {
						printf("Failed, received error from rotor, new balloon location not set\n");
					}
				}
				else {
						printf("Failed, can't read received signal from from rotor, new balloon location not set\n");
				}
			}
			break;
			
		case 3:
			if (argc != 2) {
				fprintf(stderr, "No parameters allowed for case 3\n");
				exit(1);
			}
			printf("Sending 3\n");
			
			sprintf(cmd, "3");
			if (write(file, cmd, 1) == 1) {
		 
				// As we are not talking to direct hardware but a microcontroller we
				// need to wait a short while so that it can respond.
				//
				// 1ms seems to be enough but it depends on what workload it has
				req.tv_nsec = 10000000; //10ms
				nanosleep(req,rem);

				if (read(file, buf, 7) == 7) {
					
					numBuf = strtol(buf, &ptr, 10);
					if (numBuf != 0){
						azimuthAngle = numBuf; //Converts first part to double then forces it to an int. Returns pointer to rest of the string.
					}
					else {
						printf("Either, Azimuth angle is zero or it....\n");
						printf("Failed, error reading azimuth angle, pointing angles not received\n");
					}
					numBuf = strtol(buf, &ptr, 10);
					if (numBuf != 0){
						elevationAngle = numBuf; //Converts first part to double then forces it to an int. Returns pointer to rest of the string.
					}
					else {
						printf("Either, Elevation angle is zero or it....\n");
						printf("Failed, error reading elevation angle, pointing angles not received\n");
					}
				}else {
					printf("Failed, can't read received signal from from rotor, pointing angles not received\n");
				}
			}
			break;
			
		default:
			
			printf("Failed, unknown command\n");
			exit(1);
	}
 
    // Now wait else you could crash the arduino by sending requests too fast
    req.tv_nsec = 10000000; //10ms
	nanosleep(req,rem); 
 
	close(file);
	return (EXIT_SUCCESS);
}