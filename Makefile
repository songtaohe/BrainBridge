CC = gcc
CFLAG = -pthread
CPPFLAG = -pthread
SOCKET_NAME = "-D INDEX_SERVER_NAME = \"$(shell pwd)/socket/index_server_socket\""

IndexServerSRC = src/IndexServer.c 
CompilerWrapperSRC = src/CompilerClient.c

all : indexServer compilerWrapper
	mkdir -p socket

indexServer : $(IndexServerSRC) src/*.h
	mkdir -p bin
	$(CC) -o bin/indexServer $(CFLAG) $(SOCKET_NAME) $(IndexServerSRC)

compilerWrapper : $(CompilerWrapperSRC) src/*.h
	mkdir -p bin
	$(CC) -o bin/compilerWrapper $(CFLAG) $(SOCKET_NAME) $(CompilerWrapperSRC)

install : 
	rm -f /usr/local/bin/compilerWrapper
	rm -f /usr/local/bin/indexServer
	ln -s `pwd`/bin/compilerWrapper  /usr/local/bin/compilerWrapper
	ln -s `pwd`/bin/indexServer  /usr/local/bin/indexServer
	
clean :
	touch src/*.c src/*.h
	rm -f -r bin
	rm -f -r socket
