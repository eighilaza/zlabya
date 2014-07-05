#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#define SIZE_DATA 1454
#define SIZE_SEG 6
#define SIZE_BUF (SIZE_DATA + SIZE_SEG)
#define SIZE_FILE_NAME 100
#define REUSE 1
#define STATIC_RTT 500000

int init_socket(struct sockaddr_in, socklen_t, int, int);

int init_connect(int, struct sockaddr_in, socklen_t, int);

int file_Size(FILE*);

int estimateSRTT(struct timespec [], int, int, int);

void *timer_fc(void *);

/* Struct for Timer thread */
struct timer_args{		
	int *pwindow;
	int *pretransmit;
	sem_t *pwindow_sem;	
	struct timespec *rtt_table;
};

/* struct for ACK thread */
struct check_ACK_args{
	int sockdata;
	socklen_t len;
	struct sockaddr_in client_addr;
	int *plast_seg_acked;
	int *pwindow;
	int *pretransmit;
	int *pfast_recovery;
	sem_t *pwindow_sem;
	int total_nb_seg;
	struct timespec *rtt_table;
};