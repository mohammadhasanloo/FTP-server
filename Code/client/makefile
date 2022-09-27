all: client.out

# Out
client.out: main.o client.o JsonParser.o
	g++  	main.o client.o JsonParser.o -o client.out

# Main
main.o: main.cpp client.h
	g++ -c main.cpp -o main.o

# JsonParser
JsonParser.o: JsonParser.cpp JsonParser.h
	g++ -c JsonParser.cpp -o JsonParser.o

# Client
client.o: client.cpp client.h JsonParser.h
	g++ -c client.cpp -o client.o

clean:
	rm -r -f *.out
	rm -r -f *.o
