#include <cstdio>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include <pthread.h>

class InfoManager
{
public:
	InfoManager()
	{
		SourceFileList.clear();
		pthread_mutex_init(&mLock,NULL);
	}

	// Add source file to database, return a unique id;
	int AddSourceFile(char * name)
	{
		pthread_mutex_lock(&mLock);
		std::string str(name);
		int id = -1;

		for(int i = 0; i<SourceFileList.size(); i++)
		{
			if(str == SourceFileList.at(i))
			{
				id = i;
				break;
			}
		}

		if(id == -1)
		{
			id = SourceFileList.size();
			SourceFileList.push_back(str);
		}

		pthread_mutex_unlock(&mLock);

		return id;
	}

	// Save all information 
	void SaveAll(char *name)
	{
		//TODO
	}

	// Load all information from file
	void LoadAll(char *name)
	{
		//TODO
	}


private:

	pthread_mutex_t mLock;
	std::vector<std::string> SourceFileList;
};
