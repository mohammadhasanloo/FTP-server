all: server_side.out

# Server Side
server_side.out: main.o server.o FTP.o JsonParser.o user.o LoggerHandler.o
	g++  main.o server.o FTP.o JsonParser.o user.o LoggerHandler.o -o server_side.out

# Main
main.o: main.cpp server.h
	g++ -c main.cpp -o main.o

# Server
server.o: server.cpp server.h FTP.h JsonParser.h user.h LoggerHandler.h
	g++ -c server.cpp -o server.o

# JsonParser
JsonParser.o: JsonParser.cpp JsonParser.h
	g++ -c JsonParser.cpp -o JsonParser.o

# FTP
FTP.o: FTP.cpp FTP.h user.h LoggerHandler.h
	g++ -c FTP.cpp -o FTP.o

# LoggerHandler
LoggerHandler.o: LoggerHandler.cpp LoggerHandler.h
	g++ -c LoggerHandler.cpp LoggerHandler.h

# user
user.o: user.cpp user.h
	g++ -c user.cpp -o user.o

clean:
	rm -r -f *.out
	rm -r -f *.o