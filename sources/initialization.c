#include "myHeaders.h"

int init_socket(struct sockaddr_in my_addr, socklen_t len, int reuse, int port){
		/*Socket UDP Creation*/
	int sockfd;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0){
		perror("ERROR opening socket UDP");
		exit(-1);
	}
		/* Allow to reuse the socket */
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
		/* Initialize server address structure */
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(port);
		/*Bind de la structure UDP*/
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0){
		perror("ERROR on binding UDP");
		exit(-1);
	}
		/* return socket description */
	return sockfd;
}

int init_connect(int sockfd, struct sockaddr_in client_addr, socklen_t len, int port){	
	char buf[SIZE_BUF];
	char *ack = "ACK";
	char *syn = "SYN";
	char syn_ack [12];
	strcpy(syn_ack,"SYN-ACK");
	char new_port[5];	
		/* clean mem of client adress*/
	memset(&client_addr,0,sizeof(client_addr));
		/* We wait for a SYN */
	memset(buf,0,sizeof(buf));
	recvfrom(sockfd, buf,sizeof(buf), 0, (struct sockaddr *) &client_addr,&len);
		/* check if what we received is a SYN */
	if(strcmp(buf,syn) == 0){			
		printf("syn recu \n");
			/* if we did, we send SYN-ACKNewPort */
		sprintf(new_port,"%d",port);
		strcat(syn_ack,new_port);
		sendto(sockfd, syn_ack, sizeof(syn_ack), 0, (struct sockaddr *) &client_addr,len);
		printf("I sent %s\n",syn_ack);
			/* We wait for an ACK */
		memset(buf,0,sizeof(buf));			
		recvfrom(sockfd, buf,sizeof(buf), 0, (struct sockaddr *) &client_addr,&len);			
			/* We check if what we received is an ACK */
		if(strcmp(buf,ack) == 0){
			printf("ack recu\n");
			return 1;
		}
		else{
			return -1;
		}	
	}
	else{
		return -1;
	}
}

int file_Size(FILE* fd){
	int fileSize;
	fseek(fd, 0, SEEK_END);
	fileSize = ftell(fd); 
	fseek(fd, 0, SEEK_SET);
	return fileSize;
}