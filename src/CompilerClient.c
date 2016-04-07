#include "IndexServer.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>


int connectToServer()
{
	struct sockaddr_un address;
    int socket_fd;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);

    if(socket_fd < 0)
    {
        printf("socket init failed\n");
        return -1;
    } 

    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,strlen(INDEX_SERVER_NAME)+1,INDEX_SERVER_NAME);

    if(connect(socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) !=0)
    {
        printf("connect failed\n");
        return -1;
    }

    printf("Connect Successfully\n");

	return socket_fd;
}



int main(int argc, char** argv)
{
	int socket_fd;
	// TODO	
	socket_fd = connectToServer();

	if(socket_fd == -1)
	{
		printf("Connection Error\n");
		return 0;
	}

	// Start transaction with server
	for(int ind = 1; ind < argc; ind++)
	{
		printf("%s\n",argv[ind]);

	}










	close(socket_fd);
	return 0;
}
