#include <cstdio>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

#include <pthread.h>

struct Transaction
{
	std::vector<std::string> InputList;
	std::string OutputString;
	std::vector<std::string> AllOptions;
	std::string FolderPath;
	std::string CommandName;
};


class InfoManager
{
	struct CodeSegment
	{
		int fileID;
		int moduleID;
		int s1;
		int s2;
		int e1;
		int e2;
	};





public:
	InfoManager()
	{
		SourceFileList.clear();
		ModuleList.clear();
		TransactionList.clear();
		ScriptGenerationList.clear();
		pthread_mutex_init(&mLock,NULL);
		LoadAll();
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

	// Add Module Description to database
	void AddModuleDescription(int fileID, int moduleID, int s1, int s2, int e1, int e2)
	{
		pthread_mutex_lock(&mLock);
		struct CodeSegment mCS;
		int id = -1;
		for(int i = 0; i<ModuleList.size(); i++)
		{
			if(fileID == ModuleList.at(i).fileID && moduleID == ModuleList.at(i).moduleID)
			{
				id = i;
				ModuleList.at(i).s1 = s1;
				ModuleList.at(i).s2 = s2;
				ModuleList.at(i).e1 = e1;
				ModuleList.at(i).e2 = e2;

				break;
			}
		}

		if(id == -1)
		{
			mCS.fileID = fileID;
			mCS.moduleID = moduleID;
			mCS.s1 = s1;
			mCS.s2 = s2;
			mCS.e1 = e1;
			mCS.e2 = e2;
			ModuleList.push_back(mCS);
		}
		
		pthread_mutex_unlock(&mLock);
	}


	// Save all information 
	void SaveAll()
	{
		pthread_mutex_lock(&mLock);

		std::ofstream sourcelist;
		sourcelist.open(SOURCELISTNAME, std::ios::out);
		for(int i = 0; i<SourceFileList.size(); i++)
		{
			sourcelist << SourceFileList.at(i) << "\n";
		}
		sourcelist.close();

		std::ofstream modulelist;
		modulelist.open(MODULELISTNAME, std::ios::out);
		for(int i = 0; i <ModuleList.size();i++)
		{
			struct CodeSegment mCS;

			mCS = ModuleList.at(i);

			modulelist << mCS.fileID << " " << mCS.moduleID << " ";
			modulelist << mCS.s1 << " " << mCS.s2 << " ";
			modulelist << mCS.e1 << " " << mCS.e2 << "\n";
		}
		modulelist.close();

		pthread_mutex_unlock(&mLock);
	}

	// Load all information from file
	void LoadAll()
	{
		pthread_mutex_lock(&mLock);

		std::fstream sourcelist;
		sourcelist.open(SOURCELISTNAME, std::ios::in);
		int fileLoaded = 0;
		if(sourcelist.is_open())
		{
			std::string name;
			while(sourcelist >> name)
			{
				SourceFileList.push_back(name);
				fileLoaded ++;
			}
			sourcelist.close();
		}
		

		std::fstream modulelist;
		modulelist.open(MODULELISTNAME, std::ios::in);
		int moduleLoaded = 0;
		if(modulelist.is_open())
		{
			struct CodeSegment mCS;
			while(modulelist >> mCS.fileID)
			{
				modulelist >> mCS.moduleID;
				modulelist >> mCS.s1;
				modulelist >> mCS.s2;
				modulelist >> mCS.e1;
				modulelist >> mCS.e2;

				ModuleList.push_back(mCS);
				moduleLoaded ++;

			}
			modulelist.close();
		}

		printf("%d file(s) loaded\n", fileLoaded);
		printf("%d module description(s) loaded\n", moduleLoaded);


		pthread_mutex_unlock(&mLock);
	}

	// Add Transaction and check whether we can generate a script
	void AddTransaction(struct Transaction mTransaction)
	{
		pthread_mutex_lock(&mLock);

		TransactionList.push_back(mTransaction);
		//Start Search
		printf("Start Search %d\n",ScriptGenerationList.size());
		for(int i = 0; i< ScriptGenerationList.size(); i++)
		{
			GenerateScript(ScriptGenerationList.at(i));
		}


		pthread_mutex_unlock(&mLock);
	}

	// Add file id to the waiting list for script generation
	void AddFileToScriptGenList(int fileID)
	{
		pthread_mutex_lock(&mLock);

		ScriptGenerationList.push_back(fileID);

		pthread_mutex_unlock(&mLock);
	}

	void GenerateScript(int fileID)
	{
		std::stringstream scriptStr;
		bool valid = false;

		std::string currentTarget = SourceFileList.at(fileID);
		while(1)
		{
			bool found = false;
			struct Transaction mTransaction;
			printf("Target %s \n",currentTarget.c_str());

			for(int i = 0; i < TransactionList.size(); i++)
			{
				mTransaction = TransactionList.at(i);
				for(int j = 0; j<mTransaction.InputList.size(); j++)
				{
					//printf("InputList%d %s \n",j, mTransaction.InputList.at(j).c_str());
					if(mTransaction.InputList.at(j) == currentTarget)
					{
						found = true;
						currentTarget = mTransaction.OutputString;
						break;
					}
				}
				if(found) break;

			}

			int size = currentTarget.size();
			if(currentTarget[size-1] == 'o' &&
				currentTarget[size-2] == 's' &&
				currentTarget[size-3] == '.')
			{
				valid = true;
			}


			if(found)
			{
				scriptStr << "cd " << mTransaction.FolderPath << "\n";
				scriptStr << mTransaction.CommandName ;
				for(int i = 0; i<mTransaction.AllOptions.size();i++)
				{
					if(mTransaction.AllOptions.at(i) == SourceFileList.at(fileID))
					{
						std::string tmp = mTransaction.AllOptions.at(i);
						tmp = tmp + ".cpp.cpp";
						scriptStr << " " << tmp;
					}
					else if( mTransaction.AllOptions.at(i) == mTransaction.OutputString && valid)
					{
						std::string tmp(SHAREDOBJFOLDER);
						std::stringstream tmp2;
						tmp2 << tmp << fileID << ".so";
						scriptStr << " " << tmp2.str();
					}
					else
					{
						scriptStr << " " << mTransaction.AllOptions.at(i);
					}
				}
				scriptStr << "\n";
			}
			else
			{
				//printf("Generate Script Failed \nTrace:\n%s\n",scriptStr.str().c_str());
				break;
			}

			if(valid)
			{
				printf("Generate Script(%d) Successfully\n",fileID);
				std::stringstream fileName;
				fileName << SCRIPTFOLDER << fileID << ".sh";
				std::ofstream fscript;
				fscript.open(fileName.str().c_str(), std::ios::out);
				fscript << scriptStr.str();
				fscript.close();

				break;
			}






		}



	}

public:
	std::vector<int> ScriptGenerationList;

private:
	pthread_mutex_t mLock;
	std::vector<std::string> SourceFileList;
	std::vector<struct CodeSegment> ModuleList;
	std::vector<struct Transaction> TransactionList;

};
