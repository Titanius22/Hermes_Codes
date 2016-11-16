/*

COMMENTS
http://stackoverflow.com/questions/18108932/linux-c-serial-port-reading-writing

*/

#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions


//Open Port-----------------------------------------------------------------------
int USB = open( "/dev/ttyDNT1", O_RDWR| O_NOCTTY );

//Set paramenters-----------------------------------------------------------------
struct termios tty;
struct termios tty_old;
memset (&tty, 0, sizeof tty);

/* Error Handling */
if ( tcgetattr ( USB, &tty ) != 0 ) {
   std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
}

/* Save old tty parameters */
tty_old = tty;

/* Set Baud Rate */
cfsetospeed (&tty, (speed_t)B9600);
cfsetispeed (&tty, (speed_t)B9600);

/* Setting other Port Stuff */
tty.c_cflag     &=  ~PARENB;            // Make 8n1
tty.c_cflag     &=  ~CSTOPB;
tty.c_cflag     &=  ~CSIZE;
tty.c_cflag     |=  CS8;

tty.c_cflag     &=  ~CRTSCTS;           // no flow control
tty.c_cc[VMIN]   =  1;                  // read doesn't block
tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

/* Make raw */
cfmakeraw(&tty);

/* Flush Port, then applies attributes */
tcflush( USB, TCIFLUSH );
if ( tcsetattr ( USB, TCSANOW, &tty ) != 0) {
   std::cout << "Error " << errno << " from tcsetattr" << std::endl;
}

//Write-----------------------------------------------------------------------
unsigned char cmd[] = "INIT \r";
int n_written = 0,
    spot = 0;

do {
    n_written = write( USB, &cmd[spot], 1 );
    spot += n_written;
} while (cmd[spot-1] != '\r' && n_written > 0);

//Read--------------------------------------------------------------------------
int n = 0,
    spot = 0;
char buf = '\0';

/* Whole response*/
char response[1024];
memset(response, '\0', sizeof response);

do {
    n = read( USB, &buf, 1 );
    sprintf( &response[spot], "%c", buf );
    spot += n;
} while( buf != '\r' && n > 0);

if (n < 0) {
    std::cout << "Error reading: " << strerror(errno) << std::endl;
}
else if (n == 0) {
    std::cout << "Read nothing!" << std::endl;
}
else {
    std::cout << "Response: " << response << std::endl;
}