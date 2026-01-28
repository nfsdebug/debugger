/*
 * Mostly ripped off of console-tools' writevt.c
 */

///// this code is from stacloverflow : 
// https://stackoverflow.com/questions/37401944/ioctl-failed-operation-not-permitted-block-layout-of-my-files
// I don't have the first creator of this content

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

char *progname;

static int usage() {
	printf("Usage: %s ttydev text\n", progname);
	return 2;
}

int main(int argc, char **argv) {
	int fd, argi;
	char *term = NULL;
	char *text = NULL;

	progname = argv[0];

	argi = 1;

	if (argi < argc)
		term = argv[argi++];
	else {
		fprintf(stderr, "%s: no tty specified\n", progname);
		return usage();
	}

	if (argi < argc)
		text = argv[argi++];
	else {
		fprintf(stderr, "%s: no text specified\n", progname);
		return usage();
	}

	if (argi != argc) {
		fprintf(stderr, "%s: too many arguments\n", progname);
		return usage();
	}

	fd = open(term, O_RDONLY);
	if (fd < 0) {
		perror(term);
		fprintf(stderr, "%s: could not open tty\n", progname);
		return 1;
	}

	while (*text) {
		if (ioctl(fd, TIOCSTI, text)) {
			perror("ioctl");
			return 1;
		}
		text++;
	}

	return 0;
}