#include "myHeaders.h"
#define CWND_MIN 200

int sendData(FILE *fd, int sockdata, struct sockaddr_in client_addr, socklen_t len, int *window, int seg_start, sem_t *pwindow_sem, struct timespec rtt_table[]){

	char buf [SIZE_BUF];
	char ack_buf[11];
	strcpy(ack_buf,"ACK");
	int nb_bytes_read = SIZE_DATA;

		/* We reach the seg we want to send in the file */
	fseek(fd, seg_start*SIZE_DATA, SEEK_SET);

		/* We set the seq number */	
	int seq_number = (ftell(fd)/SIZE_DATA)+1;

		/* We send segs until the window size equals zero */
	while(1){
		sem_wait(pwindow_sem);
		if((*window) > 0){		
			memset(buf,0,sizeof(buf));
			sprintf(buf,"%.6d",seq_number);
			nb_bytes_read = fread (buf+SIZE_SEG, 1, SIZE_DATA, fd);
				/* after reading the file we check if we have a full buffer */
			if(nb_bytes_read == SIZE_DATA){
				sendto(sockdata, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr,len);				
				estimateSRTT(rtt_table, seq_number, 1, 0);
				(*window)--;
				sem_post(pwindow_sem);
				seq_number++;
			}
				/* else means we reached the eof, then we put the data in a smaller buffer */
			else if(nb_bytes_read > 0){
				char small_buf[SIZE_SEG + nb_bytes_read];
				memcpy(small_buf,buf,sizeof(small_buf));
				sendto(sockdata, small_buf, sizeof(small_buf), 0, (struct sockaddr *) &client_addr,len);
				estimateSRTT(rtt_table, seq_number, 1, 0);
				(*window) = 0;
				sem_post(pwindow_sem);
				return seq_number;
			}
			else{				
				sem_post(pwindow_sem);
				printf("error reading the file! sending canceled\n");
				return -1;
			}	
		}
		else{			
			sem_post(pwindow_sem);
			return seq_number-1;		
		}
	}
}

void *check_ACK(void *context){		
	int nb_same_ack = 0;
	char buf [SIZE_BUF];
	char ack_buf [SIZE_SEG];
	struct check_ACK_args *args = (struct check_ACK_args *) context;
	while(1){

			/* We start a Timer */
		pthread_t timer;
		struct timer_args timer_context;			
		timer_context.pwindow = args->pwindow;
		timer_context.pwindow_sem = args->pwindow_sem;
		timer_context.pretransmit = args->pretransmit;
		timer_context.rtt_table = args->rtt_table;
		if(pthread_create(&timer, NULL, &timer_fc, (void *)&timer_context) != 0){
			printf("error creating Timer thread\n");
			exit(-1);
		}
			/* We wait for ACKs */
		recvfrom(args->sockdata, buf,sizeof(buf), 0, (struct sockaddr *) &(args->client_addr), &(args->len));
			/* If we get one we kill the timer */ 
		pthread_cancel (timer);
			/* we check if the seg is actually an ACK */
		if(buf[0]=='A' && buf[1]=='C' && buf[2]=='K'){
				/* if it's an ACK we put its number in last_seg_acked and we incease the cwnd*/
			memcpy(ack_buf,buf+3,6);
			ack_buf[6]='\0';
			int ack_nb = atoi(ack_buf);
			estimateSRTT((args->rtt_table), ack_nb, 0, 1);
			if(*(args->plast_seg_acked) < ack_nb){
				*(args->plast_seg_acked) = ack_nb;	
				sem_wait(args->pwindow_sem);	
				(*(args->pwindow)) = (*(args->pwindow))+2;
				sem_post(args->pwindow_sem);
			}
			else if(*(args->plast_seg_acked) == ack_nb){
				nb_same_ack++;
				if(nb_same_ack == 4){		
					nb_same_ack = 0;								
					(*(args->pretransmit)) = 1;
					(*(args->pfast_recovery)) = 1;
				}				
			}
			else{					
				sem_wait(args->pwindow_sem);
				(*(args->pwindow)) = (*(args->pwindow))+2;
				sem_post(args->pwindow_sem);
			}
			if(*(args->plast_seg_acked) == args->total_nb_seg){				
				return NULL;
			}	
		}
	}
}

int main(int argc, char* argv[]){

	if(argc != 2 || !strcmp(argv[1],"help")){
		printf("usage: %s [Port number]\n", argv[0]);
		exit(-1);
	}

	else{
			/* Set control port */
		int port = atoi(argv[1]);
			/* Init first data port */
		int data_port = port + 1;
			/* Init variables */
		int sockfd;		
		struct sockaddr_in my_addr;
		struct sockaddr_in client_addr;
		socklen_t len = sizeof(struct sockaddr_in);
		

			/* Creat socket: Server_control - Client*/			
		sockfd = init_socket(my_addr, len, REUSE, port);	
		while(1){
			int sockdata;
			struct sockaddr_in data_addr;
			char file_name[SIZE_FILE_NAME];	
			int total_nb_seg = 0;
			int last_seg_acked = 0;
			int last_seg_sent = 0;
			int window = CWND_MIN;
			int retransmit = 0;
			int fast_recovery = 0;
				/* init un mutex sur le cwnd */
			sem_t window_sem;
			if(sem_init(&window_sem, 0, 1) ==-1){
				printf("error creating window semaphore\n");
				exit(-1);
			}

				/* Creat socket: Server_data - Client*/				
			sockdata = init_socket(data_addr, len, REUSE, data_port);

				/* initialize the connectiion */
			init_connect(sockfd, client_addr, len, data_port);
			printf("waiting for the file name\n");

				/*Wait for the file name */
			printf("waiting for the file name\n");
			recvfrom(sockdata, file_name, sizeof(file_name), 0, (struct sockaddr *) &client_addr, &len);			
			file_name[SIZE_DATA-1]='\0';
			printf("got this file name : %s\n",file_name);

				/* Open the file */
			FILE* fd = fopen(file_name,"r");
			if (fd == NULL)
			{
				printf("Error opening file\n");
				exit(-1);
			}

				/* Calculating the total number of seg to send */
			total_nb_seg = (file_Size(fd)/SIZE_DATA)+1;

				/* creating table of RTT */				
			struct timespec rtt_table[total_nb_seg];
			rtt_table[0].tv_nsec = 0.05;

				/* Start the thread to handle ACKs */			
			pthread_t check_ACK_thread;
			struct check_ACK_args context;			
			context.len = len;
			context.sockdata = sockdata;
			context.client_addr = client_addr;
			context.plast_seg_acked = &last_seg_acked;
			context.pwindow = &window;
			context.pwindow_sem = &window_sem;
			context.pretransmit = &retransmit;
			context.pfast_recovery = &fast_recovery;
			context.total_nb_seg = total_nb_seg;
			context.rtt_table = rtt_table;
			if(pthread_create(&check_ACK_thread, NULL, &check_ACK, (void *)&context) != 0){
				printf("error creating check_ACK thread \n");
				exit(-1);
			}
				/* Send the file */
			while(last_seg_acked != total_nb_seg){
				if(retransmit == 1){
					retransmit = 0;
					if(fast_recovery == 1){	
						fast_recovery = 0;						
						sem_wait(&window_sem);
						window = last_seg_sent - last_seg_acked + CWND_MIN;
						sem_post(&window_sem);
					}				
					last_seg_sent = sendData(fd, sockdata, client_addr, len, &window, last_seg_acked, &window_sem, rtt_table);
				}					
				else if(last_seg_sent != total_nb_seg){
					last_seg_sent = sendData(fd, sockdata, client_addr, len, &window, last_seg_sent, &window_sem, rtt_table);
				}	
			}			
				/* after sending the last seg, we wait for the ACK and then we send FIN */
			char fin[4] = "FIN";
			sendto(sockdata, fin, sizeof(fin), 0, (struct sockaddr *) &client_addr,len);				
			printf("last_calculated_ack = %d\n and last_seg_acked = %d\n",(file_Size(fd)/SIZE_DATA)+1,last_seg_acked);
				/* We close the file and terminate the process */
			fclose(fd);
				/* set data_port for next client */
			data_port++;			
		}
	}
}
