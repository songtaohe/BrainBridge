extern "C" int Rewrite(char* src, char* dest, char** includeDirStrList,
			int * includeDirType, int includeDirCount,
			char ** defineStrList, int defineCount);

extern "C" int RewriterFileID;

extern "C" int SendModulePosition(int fileID, int moduleID, int s1, int s2, int e1, int e2);


