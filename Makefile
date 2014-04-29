# Это комментарий, который говорит, что переменная CC указывает компилятор, используемый для сборки
CC=g++
#Это еще один комментарий. Он поясняет, что в переменной CFLAGS лежат флаги, которые передаются компилятору
#CFLAGS=-c -Wall
CFLAGS=-c -std=c++11

all: server client

server: server_TCP.o util.o
	$(CC) server_TCP.o util.o -o server_TCP

client: client_TCP.o util.o
	$(CC) client_TCP.o util.o -o client_TCP

util.o: util.cpp
	$(CC) $(CFLAGS) util.cpp

server_TCP.o: server_TCP.cpp
	$(CC) $(CFLAGS) server_TCP.cpp

client_TCP.o: client_TCP.cpp
	$(CC) $(CFLAGS) client_TCP.cpp

clean:
	rm -rf *.o server_TCP client_TCP