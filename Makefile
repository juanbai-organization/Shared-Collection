CC  = gcc
CXX = g++

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES)


server: server.o handle.o
	$(CC) -g server.o handle.o -o server

server.o: server.c handle.h
	$(CC) -c $(CFLAGS) server.c

handle.o: handle.c handle.h
	$(CC) -c $(CFLAGS) handle.c


.PHONY: clean
clean:
	rm -f *.o *~ a.out server
