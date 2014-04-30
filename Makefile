# Это комментарий, который говорит, что переменная CC указывает компилятор, используемый для сборки
CC=g++
#Это еще один комментарий. Он поясняет, что в переменной CFLAGS лежат флаги, которые передаются компилятору
#CFLAGS=-c -Wall
CFLAGS=-c -std=c++11

all: server client

server: server.o util.o
	$(CC) server.o util.o -o server

client: client.o util.o
	$(CC) client.o util.o -o client

util.o: util.h util.cpp
	$(CC) $(CFLAGS) util.cpp

server.o: server.cpp util.h util.cpp
	$(CC) $(CFLAGS) server.cpp

client.o: client.cpp util.h util.cpp
	$(CC) $(CFLAGS) client.cpp

clean:
	rm -rf *.o