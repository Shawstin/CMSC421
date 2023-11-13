CC = gcc

.PHONY: all clean

all: LikesServer ParentProcess PrimaryLikesServer

LikesServer: LikesServer.o helpers.o log.o

PrimaryLikesServer: PrimaryLikesServer.o helpers.o log.o

ParentProcess: ParentProcess.o log.o

clean: 
	rm -f *.o LikesServer ParentProcess PrimaryLikesServer
