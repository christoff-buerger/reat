#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <error.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "cambri.h"
#include "event.h"



static int cambri_fd;
static FILE* cambri_log_file;


static int cambri_enabled = 0;


void cambri_init(void) {
	cambri_fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (cambri_fd < 0) error(1, 0, "cambri_init");

	struct termios tty = {};
	tcgetattr(cambri_fd, &tty);
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_iflag = 0;
	tty.c_cflag = CS8 | CLOCAL | CREAD;
	tty.c_cc[VTIME] = 1; // 0.1 s read timeout
	tty.c_cc[VMIN] = 0; // non-blocking
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);
	tcsetattr(cambri_fd, TCSANOW, &tty);


	cambri_log_file = fopen("cambri.log", "w");
	if (!cambri_log_file) error(1, 0, "cambri_init");
	fprintf(cambri_log_file, " time      ");
	int i;
	for (i = 1; i <= 8; i++) fprintf(cambri_log_file, " | %4d", 1000 + i);
	fprintf(cambri_log_file, "\n");
	fprintf(cambri_log_file, "-----------");
	for (i = 1; i <= 8; i++) fprintf(cambri_log_file, "-+-----");
	fprintf(cambri_log_file, "\n");
	fflush(cambri_log_file);

	cambri_enabled = 1;
}


void cambri_kill(void) {
	if (!cambri_enabled) return;
	close(cambri_fd);
	fclose(cambri_log_file);
}


static void cambri_write(const char* fmt, ...) {
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int len = vsprintf(buf, fmt, args);
	va_end(args);
	write(cambri_fd, buf, len);
	write(cambri_fd, "\r\n", 2);
}


static int cambri_read(char* buf, int len) {
	int i;
	int p = 0;
	for (i = 0; i < 200; i++) { // safety
		if (p >= 5 && strcmp(buf + p - 5, "\r\n>> ") == 0) break;
		p += read(cambri_fd, buf + p, len - p);
	}
	return p;
}


void cambri_log_current(const char* time) {
	if (!cambri_enabled) return;
	cambri_write("state");
	char buf[1024] = {};
	int ret = cambri_read(buf, sizeof(buf));
	if (ret == 0) error(1, 0, "cambri_log_current");

	fprintf(cambri_log_file, "%s", time);

	char* p = buf;
	int i;
	for (i = 0; i < 8; i++) {
		p = strchr(p, '\n') + 5;
		int current = atoi(p);
		fprintf(cambri_log_file, " | %4d", current);
	}
	fprintf(cambri_log_file, "\n");
	fflush(cambri_log_file);
}


void cambri_set_mode(int id, int mode) {
	if (!cambri_enabled) return;
	cambri_write("mode %c %d 4", mode, id % 10);
	char buf[1024] = {};
	cambri_read(buf, sizeof(buf));
}

