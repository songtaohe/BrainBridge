#include "IndexServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void * TransactionThread(void * confd)
{
	int mfd = *((int*)confd);

	printf("This is a new transaction\n");
	




	return NULL;
}






int main(void)
{
	struct sockaddr_un address;
	int socket_fd;
	socklen_t address_length;
	

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket init failed\n");
		return 1;
	}

	unlink(INDEX_SERVER_NAME);
	
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path,strlen(INDEX_SERVER_NAME)+1,INDEX_SERVER_NAME);

	if(bind(socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind failed\n");
		return 1;
	}

	if(listen(socket_fd, 5)  != 0)
	{
		printf("listen failed\n");
		return 1;
	}

	while(1)
	{
	// Accept and create thread 
		int * confd = NULL;
		confd = (int*)malloc(sizeof(int)*1);
		*confd = accept(socket_fd, (struct sockaddr *)&address, &address_length);

		if(*confd > -1)
		{
			// Create a pthread
			pthread_t * t = NULL;
			int ret = 0;
			t  = (pthread_t*)malloc(sizeof(pthread_t)*1);
			if(t!= NULL)
			{
				ret = pthread_create(t, NULL, TransactionThread, (void*)confd);
			}
			

			if(t == NULL || ret != 0)
			{
				printf("Thread create error\n");
			}
			

		}
		else
		{
			printf("Something wrong happen! :-( \n");

		}	


	}


	close(socket_fd);
	unlink(INDEX_SERVER_NAME);
	return 0;

}
