TMP_PREFIX = hst__

CC = gcc
CFLAG = -pthread -g
#CPPFLAG = -pthread
SOCKET_NAME = "-D INDEX_SERVER_NAME = \"$(shell pwd)/socket/index_server_socket\""
CFLAG += "-DTEMP_DIR = \"$(shell pwd)/tmp/$(TMP_PREFIX)\""
DATASET_NAME = "-D SOURCELISTNAME = \"$(shell pwd)/data/sourcelist\"" 
DATASET_NAME += "-D MODULELISTNAME = \"$(shell pwd)/data/modulelist\"" 
DATASET_NAME += "-D SCRIPTFOLDER = \"$(shell pwd)/data/script/\"" 
DATASET_NAME += "-D SHAREDOBJFOLDER = \"$(shell pwd)/data/lib/\"" 
DATASET_NAME += "-D DATAFOLDER = \"$(shell pwd)/data/\""
DATASET_NAME += "-D RUNTIMENAME = \"$(shell pwd)/runtime/update.c\""
DATASET_NAME += "-D WYSIWYGCMDFILE = \"undefined\""


IndexServerSRC = src/IndexServer.cpp src/InfoManager.cpp
CompilerWrapperSRC = src/CompilerClient.cpp

CXX = g++
CXXFLAGS = -fno-rtti -std=c++11 -g

LLVM_SRC_PATH = /ssd/CodeProject/llvm
LLVM_BUILD_PATH = /ssd/CodeProject/build

LLVM_BIN_PATH = $(LLVM_BUILD_PATH)/bin
LLVM_LIBS=core mc

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`


CLANG_BUILD_FLAGS = -I$(LLVM_SRC_PATH)/tools/clang/include \
                                      -I$(LLVM_BUILD_PATH)/tools/clang/include

CLANGLIBS = \
  -lclangFrontendTool -lclangFrontend -lclangDriver \
  -lclangSerialization -lclangCodeGen -lclangParse \
  -lclangSema -lclangStaticAnalyzerFrontend \
  -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore \
  -lclangAnalysis -lclangARCMigrate -lclangRewrite \
  -lclangEdit -lclangAST -lclangLex -lclangBasic


all : indexServer compilerWrapper
	mkdir -p socket
	mkdir -p tmp
	mkdir -p data
	mkdir -p data/script
	mkdir -p data/lib

indexServer : $(IndexServerSRC) src/*.h
	mkdir -p bin
	$(CXX) -o bin/indexServer $(CFLAG) $(SOCKET_NAME) $(DATASET_NAME) $(IndexServerSRC)

compilerWrapper : compilerWrapper.o rewriter.o
	mkdir -p bin
	$(CXX) -o bin/compilerWrapper compilerWrapper.o rewriter.o $(CFLAG) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANGLIBS) $(LLVM_LDFLAGS)

compilerWrapper.o: src/CompilerClient.cpp
	$(CXX) -c -o compilerWrapper.o $(CFLAG) $(SOCKET_NAME) $(CompilerWrapperSRC)

rewriter.o : src/Rewriter.cpp
	$(CXX) src/Rewriter.cpp $(DATASET_NAME) $(CXXFLAGS) $(LLVM_CXXFLAGS) -c -o rewriter.o $(CLANG_BUILD_FLAGS)	





install : 
	rm -f /usr/local/bin/compilerWrapper
	rm -f /usr/local/bin/indexServer
	rm -f /home/songtao/.config/sublime-text-3/Packages/User/android_wysiwyg.py
	ln -s `pwd`/bin/compilerWrapper  /usr/local/bin/compilerWrapper
	ln -s `pwd`/bin/indexServer  /usr/local/bin/indexServer
	ln -s `pwd`/plugin/android_wysiwyg.py /home/songtao/.config/sublime-text-3/Packages/User/android_wysiwyg.py
	
clean :
	touch src/*.c src/*.h
	rm -f -r bin
	rm -f -r socket
	rm -f -r data
	#rm -f -r tmp
	rm -f *.o
