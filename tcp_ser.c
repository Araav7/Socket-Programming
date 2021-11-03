/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd, int errorProbability);                                                        // transmitting and receiving function

int main(int argc, char **argv)//argc refers to the number of command line arguments including program name passed into the program and argv refers to the error probability
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;
	int errorProbability;

	if (argc != 2) {
		printf("parameters not match");
	}

	errorProbability = atoi(argv[1]); //change char to int

//	char *buf;
	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create socket
	if (sockfd <0)
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");
	bzero(&(my_addr.sin_zero), 8); //append zeros at the back to satisfy requirement
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket
	if (ret <0)
	{
		printf("error in binding");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);                              //listen, informs the OS that socket is ready for requests
	if (ret <0) {
		printf("error in listening");x
		exit(1);
	}

	while (1)
	{
		printf("waiting for data\n");
		sin_size = sizeof (struct sockaddr_in);
		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
		if (con_fd <0)
		{
			printf("error in accept\n");
			exit(1);
		}

		if ((pid = fork())==0)                                         // creat acception process
		{
			close(sockfd);
			str_ser(con_fd, errorProbability);                                          //receive packet and response
			close(con_fd);
			exit(0);
		}
		else close(con_fd);                                         //parent process
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd, int errorProbability)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
    time_t t;
	end = 0;
    srand((unsigned) time(&t)); // Initialize random number generator
	printf("receiving data!\n");

	while(!end) //for packet transmission
	{

		if ((n = recv(sockfd, &recvs, DATALEN, 0)) == -1)        //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
		if (recvs[n-1] == '\0')									//if it is the end of the file
		{
			end = 1;
			n --;
		}
		memcpy((buf+lseek), recvs, n);
		ack.len = 0;
        //artificial error generator. if the error probability is low then then the chances of rand() being lower than error probability is low and it will enter the else loop to move on to the next data packet and send positive acknowledgment
        if ((rand() % 100) < errorProbability) {
            ack.num = 0;
        } else {
            lseek += n;
            ack.num = 1;
        }
        if ((n = send(sockfd, &ack, 2, 0))==-1)//error when sending packet to the server (locally detected errors are returned by -1)
	    {
			printf("send error!");								//send the ack
			exit(1);
	    }
	}
	ack.num = 1; //ack end of transmission
	ack.len = 0;
	if ((n = send(sockfd, &ack, 2, 0))==-1)
	{
			printf("send error!");								//send the ack
			exit(1);
	}
	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);					//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
