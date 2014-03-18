/*
 * client.c - simple terminal client
 *
 * Copyright 2013 Edward V. Emelianoff <eddy@sao.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include <termios.h>		// tcsetattr
#include <unistd.h>			// tcsetattr, close, read, write
#include <sys/ioctl.h>		// ioctl
#include <stdio.h>			// printf, getchar, fopen, perror
#include <stdlib.h>			// exit
#include <sys/stat.h>		// read
#include <fcntl.h>			// read
#include <signal.h>			// signal
#include <time.h>			// time
#include <string.h>			// memcpy
#include <stdint.h>			// int types
#include <sys/time.h>		// gettimeofday

#include <netdb.h> // addrinfo
#include <arpa/inet.h> // inet_ntop
#include <assert.h>

size_t BUFSZ = 255;
#define TGT_HOST "192.168.99.2"
#define PORT "12345"

int sockfd = 0;
int connected = 0;

int zmon_period = 30; // monitor temperature each 30s
double t0; // start time

FILE *fout = NULL; // file for messages duplicating
char *comdev = "/dev/ttyACM0";
int BAUD_RATE = B115200;
struct termio oldtty, tty; // TTY flags
struct termios oldt, newt; // terminal flags
int comfd; // TTY fd

/**
 * function for different purposes that need to know time intervals
 * @return double value: time in seconds
 */
double dtime(){
	double t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = tv.tv_sec + ((double)tv.tv_usec)/1e6;
	return t;
}

/**
 * Exit & return terminal to old state
 * @param ex_stat - status (return code)
 */
void quit(int ex_stat){
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // return terminal to previous state
	ioctl(comfd, TCSANOW, &oldtty ); // return TTY to previous state
	close(comfd);
	if(fout) fclose(fout);
	printf("Exit! (%d)\n", ex_stat);
	exit(ex_stat);
}

/**
 * Open & setup TTY, terminal
 */
void tty_init(){
	// terminal without echo
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	if(tcsetattr(STDIN_FILENO, TCSANOW, &newt) < 0) quit(-2);
	printf("\nOpen port...\n");
	if ((comfd = open(comdev,O_RDWR|O_NOCTTY|O_NONBLOCK)) < 0){
		fprintf(stderr,"Can't use port %s\n",comdev);
		quit(1);
	}
	printf(" OK\nGet current settings...\n");
	if(ioctl(comfd,TCGETA,&oldtty) < 0) exit(-1); // Get settings
	tty = oldtty;
	tty.c_lflag     = 0; // ~(ICANON | ECHO | ECHOE | ISIG)
	tty.c_oflag     = 0;
	tty.c_cflag     = BAUD_RATE|CS8|CREAD|CLOCAL; // 9.6k, 8N1, RW, ignore line ctrl
	tty.c_cc[VMIN]  = 0;  // non-canonical mode
	tty.c_cc[VTIME] = 5;
	if(ioctl(comfd,TCSETA,&tty) < 0) exit(-1); // set new mode
	printf(" OK\n");
}

/**
 * Read character from console without echo
 * @return char readed
 */
int read_console(){
	int rb;
	struct timeval tv;
	int retval;
	fd_set rfds;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	tv.tv_sec = 0; tv.tv_usec = 1000;
	retval = select(1, &rfds, NULL, NULL, &tv);
	if(!retval) rb = 0;
	else {
		if(FD_ISSET(STDIN_FILENO, &rfds)) rb = getchar();
		else rb = 0;
	}
	//tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return rb;
}

/**
 * getchar() without echo
 * wait until at least one character pressed
 * @return character readed
 */
int mygetchar(){ // аналог getchar() без необходимости жать Enter
	int ret;
	do ret = read_console();
	while(ret == 0);
	return ret;
}

/**
 * Read data from TTY
 * @param buff (o) - buffer for data read
 * @param length   - buffer len
 * @return amount of readed bytes
 */
size_t read_tty(uint8_t *buff, size_t length){
	ssize_t L = 0;
	fd_set rfds;
	struct timeval tv;
	int retval;
	FD_ZERO(&rfds);
	FD_SET(comfd, &rfds);
	tv.tv_sec = 0; tv.tv_usec = 1000;
	retval = select(comfd + 1, &rfds, NULL, NULL, &tv);
	if (!retval) return 0;
	if(FD_ISSET(comfd, &rfds)){
		if((L = read(comfd, buff, length)) < 1) return 0;
	}
	return (size_t)L;
}

void help(){
	printf("Use this commands:\n"
		"T\tSend test message: a long text string\n"
		"O\tToggle output between eth & usb\n"
		"h\tShow this help\n"
		"q\tQuit\n"
		"t\tMonitor temperature on ZakWire\n"
		"v\tNon-verbose\n"
		"z\tGet ZakWire data\n"
		"V\tVerbose\n"
	);
}

#define dup_pr(...) do{printf(__VA_ARGS__); if(fout) fprintf(fout, __VA_ARGS__);}while(0)

int zflag = 0; // monitor zakwire
void con_sig(int rb){
	uint8_t cmd;
	static int outflag = 0;
	if(rb < 1) return;
	if(rb == 'q') quit(0); // q == exit
	switch(rb){
		case 'h':
			help();
		break;
		case 's':
			dup_pr("Stop ZakWire\n");
			zflag = 0;
		break;
		case 't':
			dup_pr("ZakWire thermal monitoring\n");
			zflag = 1;
			write(comfd, "v", 1);
			t0 = dtime() - zmon_period;
		break;
		case 'T':
			dup_pr("Test mode\n");
			zflag = 0;
			char *teststr = "TXTTST\n";
			size_t L = strlen(teststr);
			write(comfd, teststr, L);
			if(connected){
				if(send(sockfd, teststr, L, 0) != L) perror("send");
			}
		break;
		case 'O': // toggle output
			outflag = !outflag;
		break;
		default:
			cmd = (uint8_t) rb;
			if(outflag && connected){
				if(send(sockfd, (char*)&rb, 1, 0) != 1) perror("send");
			}else{
				write(comfd, &cmd, 1);
			}
	}
}

/**
 * Get integer value from buffer
 * @param buff (i) - buffer with int
 * @param len      - length of data in buffer (could be 2 or 4)
 * @return
 */
uint32_t get_int(uint8_t *buff, size_t len){
	int i;
	printf("read %zd bytes: ", len);
	for(i = 0; i < len; i++) printf("0x%x ", buff[i]);
	printf("\n");
	if(len != 2 && len != 4){
		fprintf(stdout, "Bad data length!\n");
		return 0xffffffff;
	}
	uint32_t data = 0;
	uint8_t *i8 = (uint8_t*) &data;
	if(len == 2) memcpy(i8, buff, 2);
	else memcpy(i8, buff, 4);
	return data;
}

/**
 * wait for answer from server
 * @param sock - socket fd
 * @return 0 in case of error or timeout, 1 in case of socket ready
 */
int waittoread(int sock){
	fd_set fds;
	struct timeval timeout;
	int rc;
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	rc = select(sock+1, &fds, NULL, NULL, &timeout);
	if(rc < 0){
		perror("select failed");
		return 0;
	}
	if(rc > 0 && FD_ISSET(sock, &fds)) return 1;
	return 0;
}

int main(int argc, char *argv[]){
	int rb;
	uint8_t buff[128];
	size_t L;
	struct addrinfo h, *r, *p;
	char *recvBuff = calloc(BUFSZ, 1);
	assert(recvBuff);
	if(argc == 2){
		fout = fopen(argv[1], "a");
		if(!fout){
			perror("Can't open output file");
			exit(-1);
		}
		setbuf(fout, NULL);
	}
	tty_init();
	signal(SIGTERM, quit);		// kill (-15)
	signal(SIGINT, quit);		// ctrl+C
	signal(SIGQUIT, SIG_IGN);	// ctrl+\   .
	signal(SIGTSTP, SIG_IGN);	// ctrl+Z
	setbuf(stdout, NULL);
	memset(&h, 0, sizeof(h));
	h.ai_family = AF_INET;
	h.ai_socktype = SOCK_STREAM;
	h.ai_flags = AI_CANONNAME;
	char *host = TGT_HOST;
	char *port = PORT;
	if(getaddrinfo(host, port, &h, &r)){perror("getaddrinfo"); return -1;}
	struct sockaddr_in *ia = (struct sockaddr_in*)r->ai_addr;
	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(ia->sin_addr), str, INET_ADDRSTRLEN);
	for(p = r; p; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("connect");
			continue;
		}
		break; // if we get here, we must have connected successfully
	}
	if(p == NULL){
		// looped off the end of the list with no connection
		fprintf(stderr, "failed to connect\n");
	}else{
		connected = 1;
	}
	freeaddrinfo(r);
	t0 = dtime();
	while(1){
		rb = read_console();
		if(rb > 0) con_sig(rb);
		L = read_tty(buff, 127);
		if(L){
			buff[L] = 0;
			printf("TTY: %s\n", buff);
			if(fout) fprintf(fout, "%zd\t%s\n", time(NULL), buff);
		}
		if(zflag && dtime() - t0 > zmon_period){ // thermal monitoring
			t0 += zmon_period;
			char NUM[] = {'6', '5', '3'};
			char obuf[] = {'N', 0, 'z'};
			int i;
			double temper[3];
			for(i = 0; i < 3; i++){
				obuf[1] = NUM[i];
				if(3 != write(comfd, &obuf, 3)){
					perror("Can't write");
					quit(-1);
				}
				L = read_tty(buff, 127);
				//for(j = 0; j < 10; j++) // 10 tries to read data from USB
				//	if((L = read_tty(buff, 127)) < 1) continue;
				uint32_t ans = get_int(buff, L);
				if(ans == 0xffffffff) temper[i] = -274.;
				else temper[i] = ((double)ans)/2047.*70. - 10.;
			}
			dup_pr("%zd\t%.2f\t%.2f\t%.2f\n", time(NULL), temper[0], temper[1], temper[2]);
		}
		if(connected && waittoread(sockfd)){
			int offset = 0, n = 0;
			do{
				if(offset >= BUFSZ){
					BUFSZ += 1024;
					recvBuff = realloc(recvBuff, BUFSZ);
					assert(recvBuff);
					fprintf(stderr, "Buffer reallocated, new size: %zd\n", BUFSZ);
				}
				n = read(sockfd, &recvBuff[offset], BUFSZ - offset);
				if(!n) break;
				if(n < 0){
					perror("read");
					return -1;
				}
				offset += n;
			}while(waittoread(sockfd));
			if(!offset){
				fprintf(stderr, "Socket closed\n");
				return 0;
			}
			printf("read %d bytes\n", offset);
			recvBuff[offset] = 0;
			printf("ETH: %s\n", recvBuff);
			fflush(stdout);
		}
	}
}
