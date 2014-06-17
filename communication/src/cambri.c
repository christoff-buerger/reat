#include <string.h>
#include <error.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "cambri.h"



static int cambri_fd;

void cambri_init(void) {
	cambri_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (cambri_fd < 0) error(1, 0, "cambri");

	struct termios tty = {};
	tcgetattr(cambri_fd, &tty);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_iflag = 0;
	tty.c_cflag = CS8 | CLOCAL | CREAD;
	tty.c_cc[VTIME] = 1; // 0.1 s read timeout
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);
	tcsetattr(cambri_fd, TCSANOW, &tty);
}


void cambri_kill(void) {
	close(cambri_fd);
}


void cambri_write(char* cmd) {
	write(cambri_fd, cmd, strlen(cmd));
	write(cambri_fd, "\r\n", 2);
}


void cambri_read(char* buf) {
	int i;
	for (i = 0; i < 1000; i++) { // safety
		int p = strlen(buf);
		if (p >= 5 && strcmp(buf + p - 5, "\r\n>> ") == 0) break;
		read(cambri_fd, buf + p, sizeof(buf) - p);
	}
}
