#include "myHeaders.h"
#define CWND_MIN 100

int estimateSRTT(struct timespec rtt_table[], int seg_number, int sending, int receiving){
	if(sending == 1 && receiving == 0){
		clock_gettime(CLOCK_REALTIME,&(rtt_table[seg_number]));
		return 1;
	}
	else if(sending == 0 && receiving == 1){
		struct timespec tmp;
		clock_gettime(CLOCK_REALTIME,&tmp);
		rtt_table[seg_number].tv_nsec = (tmp.tv_nsec - rtt_table[seg_number].tv_nsec);
		long gardefou = 0.6*(rtt_table[0].tv_nsec) + 0.4*(rtt_table[seg_number].tv_nsec);
		if(gardefou > 0 && gardefou< 2*STATIC_RTT){
			rtt_table[0].tv_nsec = gardefou;
		}
		return 1;
	}
	else{
		printf("error! estimateSRTT;");
		return -1;
	}
}

void *timer_fc(void *context){
	struct timer_args *args = (struct timer_args *) context;
	struct timespec start, finish;
	while(1){
		long time_out;
		clock_gettime(CLOCK_REALTIME, &start);
		time_out = start.tv_nsec + STATIC_RTT;//(args->rtt_table[0].tv_nsec);
		clock_gettime(CLOCK_REALTIME, &finish);
		while(finish.tv_nsec < time_out){
			clock_gettime(CLOCK_REALTIME, &finish);
		}
		sem_wait(args->pwindow_sem);
		*(args->pwindow) = CWND_MIN;
		sem_post(args->pwindow_sem);
		*(args->pretransmit) = 1;
		usleep(500000);
	}
}