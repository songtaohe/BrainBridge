//To be inserted:  static void WYSIWYG_Init(); (At the begining)
//To be defined : WYSIWYG_FILEID and WYSIWYG_CMDFILE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
static void WYSIWYG_Update(const char* soname, const char* funcname);
static pthread_t WYSIWYG_Thread;

static void * WYSIWYG_Daemon(void* param)
{
	static int ticket = 0;
	
	

	while( (param == NULL) ? 1:0)
	{
		FILE *fp = NULL;
		fp = fopen("/data/WYSIWYG/cmd", "rt");
		ALOGE("WYSIWYG_Daemon%d On\n",WYSIWYG_FILEID);
		if(fp != NULL)
		{
			char n1[512];
			char n2[512];
			int i1,i2;
			fscanf(fp, "%d %d %s %s",&i1,&i2,n1,n2);
			if(i1 != ticket)
			{
				ticket = i1;
				if(i2 == WYSIWYG_FILEID)
				{
					ALOGE("WYSIWYG_Update%d %s %s\n",WYSIWYG_FILEID,n1,n2);
					WYSIWYG_Update(n1,n2);
				}
			}
			fclose(fp);
		}
		sleep(1); // 1 seconds

	}

	return NULL;
}
static void WYSIWYG_Init()
{
	static int init = 0;
	if((init++) == 0) // May have concurrency issues
	{
		sleep(1);
		init ++;
		sleep(1);
		init ++;
		sleep(1);
		if(init == 3)
			pthread_create(&WYSIWYG_Thread, NULL, WYSIWYG_Daemon, NULL);
		else
			init = 0;
	}
}
static void WYSIWYG_Update(const char* soname, const char* funcname)
{
	void * handle = NULL;
	void * func = NULL;

	handle = dlopen(soname,RTLD_LOCAL);
	if(handle !=NULL)
	{
		func = (void*)dlsym(handle,funcname);
        if(func == NULL)
        {
            ALOGE("WYSIWYG_Update Cannot find %s\n",funcname);
        }
	}
    else
    {
        ALOGE("WYSIWYG_Update Cannot open %s\n",soname);
    }



	if(func!=NULL)
	{
		ALOGE("WYSIWYG_Update Here %d %s %s %p\n",WYSIWYG_FILEID, soname,funcname,func);
		//Automatically Generated Part






