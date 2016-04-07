#include "IndexServer.h"
#include "Common.h"


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

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

    //printf("Connect Successfully\n");

	return socket_fd;
}



int main(int argc, char** argv)
{
	int socket_fd;
	int state = 0;
	char buf[1024];

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
		int len;
		if((len = strlen(argv[ind]))>4)
		{
			if(argv[ind][len-1] == 'p' &&
				argv[ind][len-2] == 'p' &&
				argv[ind][len-3] == 'c' &&
				argv[ind][len-4] == '.')
			{
				// This is a cpp file, we are interested in it
				//char * fullpath = 0;
				int lendir = strlen(argv[1]);
				int lenfile = len;
				
				//fullpath = (char*)malloc(sizeof(char)*(lendir+lenfile+1+1));
				//sprintf(fullpath,"%s/%s",argc[1],argc[ind]);
				//fullpath[lendir+lenfile+1] = 0;
				//fullpath is the full path to the source code

				struct MsgUniversal *mMsg;

				int size = sizeof(struct MsgUniversal) + lendir + lenfile + 2;

				mMsg = (struct MsgUniversal*)malloc(size);
				memset((char*)mMsg,0,size);

				mMsg->base.p1 = MSGTYPE_C2S_FILE_SOURCE;
				sprintf(&(mMsg->data),"%s/%s",argv[1],argv[ind]);		
				
				write(socket_fd,(char*)mMsg, size);

				//TODO wait for futher command

				read(socket_fd, buf, 1024);
				//sleep(10000);		
				free(mMsg);
			}	


		}


		if(ind!=1)	printf("%s ",argv[ind]);
	}










	close(socket_fd);
	return 0;
}
