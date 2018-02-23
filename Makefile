CC  = gcc
CXX = g++

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)


server: server.o
	$(CC) -g server.o -o server

server.o: server.c
	$(CC) -c $(CFLAGS) server.c


.PHONY: clean
clean:
	rm -f *.o *~ a.out server
