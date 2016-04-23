#include "IndexServer.h"
#include "Common.h"
#include "Rewriter.h"

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


char ** IncludeDirStrList = NULL;
char ** DefineStrList = NULL;

int * IncludeDirType = NULL;

int IncludeDirCounter = 0;
int DefineCounter = 0;

int * IsSource = NULL;







int SendStringSync(int socket_fd,char * buf,int length)
{
	struct MsgUniversal *mMsg;
	char mbuf[1024];
    int size = sizeof(struct MsgUniversal) + length + 1;



    mMsg = (struct MsgUniversal*)malloc(size);
    memset((char*)mMsg,0,size);

    mMsg->base.p1 = MSGTYPE_DEBUG_STRING;
    sprintf(&(mMsg->data),"%s",buf);

    write(socket_fd,(char*)mMsg, size);

                //TODO wait for futher command

    read(socket_fd, mbuf, 1024);
                //sleep(10000);     
    free(mMsg);
} 

int SendSourceFileName(int socket_fd,char * buf,int length)
{
	struct MsgUniversal *mMsg;
	char mbuf[1024];
    int size = sizeof(struct MsgUniversal) + length + 1;
    int *ptr = (int*)mbuf;


    mMsg = (struct MsgUniversal*)malloc(size);
    memset((char*)mMsg,0,size);

    mMsg->base.p1 = MSGTYPE_C2S_FILE_SOURCE;
    sprintf(&(mMsg->data),"%s",buf);

    write(socket_fd,(char*)mMsg, size);

                //TODO wait for futher command

    read(socket_fd, mbuf, 1024);


                //sleep(10000);     
    free(mMsg);

    return ptr[0]; // FileID
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

	
	IncludeDirStrList = (char**) malloc(sizeof(char*)*argc);
	DefineStrList = (char**) malloc(sizeof(char*)*argc);
	IncludeDirType = (int*) malloc(sizeof(char*)*argc);

	IsSource = (int*) malloc(sizeof(int) * argc);	
	memset((char*)IsSource,0,sizeof(int)*argc);


	// Start transaction with server
	for(int ind = 1; ind < argc; ind++)
	{
		int len;
		len = strlen(argv[ind]);
		
		SendStringSync(socket_fd, argv[ind], strlen(argv[ind]));
	

	
		if(state > 0)
		{

			sprintf(buf,"%s/%s",argv[1],argv[ind]);

			
			IncludeDirType[IncludeDirCounter] = state;
			
			IncludeDirStrList[IncludeDirCounter] = (char*)malloc(sizeof(char)*(len+strlen(argv[1])+2));
			sprintf(IncludeDirStrList[IncludeDirCounter],"%s",buf);
			
			IncludeDirCounter++;



			SendStringSync(socket_fd, buf, len + strlen(argv[1]));

			state = 0;
			continue;
		}
		
		if( len == 2)
		{
			if(argv[ind][0] == '-' && argv[ind][1] == 'I')
			{
				state = 2; // -I option, following is dir
				continue;
			}
			
		}

		if (len == 8)
		{
			if(strcmp(argv[ind],"-isystem")==0)
			{
				state = 1; // -isystem option
				continue;
			}

		}
		
		if(len > 2)
		{
			if(argv[ind][0] == '-' && argv[ind][1] == 'D')
			{
				SendStringSync(socket_fd, argv[ind], len);	

				DefineStrList[DefineCounter] = (char*)malloc(sizeof(char)*strlen(argv[ind]));
				sprintf(DefineStrList[DefineCounter],"%s",&(argv[ind][2]));
				
				DefineCounter ++;

				continue;
			}
		}	
		
		if(len>4)
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

				IsSource[ind] = 1;


				continue;
			}	

			


		}


//		if(ind!=1)	printf("%s ",argv[ind]);
	}



	for(int ind = 2; ind < argc; ind++)
    {
		if(IsSource[ind] == 1)
		{
			// Rewriting ...

			char* src = (char*)malloc(sizeof(char)*(strlen(argv[ind])+strlen(argv[1])+2));
			char* dest = NULL;

			int j;


			j = sprintf(src,"%s/%s",argv[1],argv[ind]);
			//dest = (char*)malloc(sizeof(char)*(j + strlen(TEMP_DIR) + 2));
			dest = (char*)malloc(sizeof(char)*(j + 8 + 2 + 4));

			j = sprintf(dest,"%s.cpp.cpp",src);
	
	//		printf("%d %d %s\n",j,strlen(TEMP_DIR),TEMP_DIR);

	//		for(int i = strlen(TEMP_DIR); i<j; i++)
	//		{
	//			if(dest[i] == '/') dest[i] = '_';
	//		}

			SendStringSync(socket_fd,dest,j);
			RewriterFileID = SendSourceFileName(socket_fd,src,j-8);

			// Key Function 
			int ret = Rewrite(src,dest,IncludeDirStrList,IncludeDirType, IncludeDirCounter, DefineStrList,DefineCounter);

			// User Dest 
			if(ret!= 0)
			{
				printf("%s ",argv[ind]);
				
			}
			else
			{
				printf("%s ",dest);
			}
			continue;
		}
		printf("%s ",argv[ind]);
	}






	close(socket_fd);
	return 0;
}
