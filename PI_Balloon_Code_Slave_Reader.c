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
#define ADDRESS 0x08

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
static const char *devName = "/dev/i2c-1";


char buf[40];
struct timespec req={0},rem={0};

//argc: the number of argments sent to main()
//argv: the actual arguments
int main(int argc, char* argv[]) {

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
	
	req.tv_nsec = 500000000; //500ms
	
	while (1){
		read(file, buf, 36);
		printf("%s\n", buf);
		nanosleep(&req,&rem);
	}

	close(file);
	return (EXIT_SUCCESS);
}
