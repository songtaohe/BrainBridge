TMP_PREFIX = hst__

CC = gcc
CFLAG = -pthread -g
#CPPFLAG = -pthread
SOCKET_NAME = "-D INDEX_SERVER_NAME = \"$(shell pwd)/socket/index_server_socket\""
CFLAG += "-DTEMP_DIR = \"$(shell pwd)/tmp/$(TMP_PREFIX)\""



IndexServerSRC = src/IndexServer.c 
CompilerWrapperSRC = src/CompilerClient.c

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

indexServer : $(IndexServerSRC) src/*.h
	mkdir -p bin
	$(CC) -o bin/indexServer $(CFLAG) $(SOCKET_NAME) $(IndexServerSRC)

compilerWrapper : compilerWrapper.o rewriter.o src/*.h
	mkdir -p bin
	$(CXX) -o bin/compilerWrapper compilerWrapper.o rewriter.o $(CFLAG) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANGLIBS) $(LLVM_LDFLAGS)

compilerWrapper.o: src/CompilerClient.c
	$(CC) -c -o compilerWrapper.o $(CFLAG) $(SOCKET_NAME) $(CompilerWrapperSRC)

rewriter.o : src/Rewriter.cpp
	$(CXX) src/Rewriter.cpp $(CXXFLAGS) $(LLVM_CXXFLAGS) -c -o rewriter.o $(CLANG_BUILD_FLAGS)	





install : 
	rm -f /usr/local/bin/compilerWrapper
	rm -f /usr/local/bin/indexServer
	ln -s `pwd`/bin/compilerWrapper  /usr/local/bin/compilerWrapper
	ln -s `pwd`/bin/indexServer  /usr/local/bin/indexServer
	
clean :
	touch src/*.c src/*.h
	rm -f -r bin
	rm -f -r socket
	#rm -f -r tmp
	rm -f *.o
