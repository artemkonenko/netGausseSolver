#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

using namespace std;

int main( int argc, char* argv[] )
{
	int port = 9753;
  string addr = "127.0.0.1";

  char c;
  while ((c = getopt (argc, argv, "p:a:i:o:")) != -1)
    switch (c)
    {
      case 'p':
        sscanf( optarg, "%d", &port );
        break;
      case 'a':
        addr = optarg;
        break;
      case 'i':
        freopen(optarg, "r", stdin);
        break;
      case 'o':
        freopen(optarg, "w", stdout);
        break;
      case '?':
        if ( optopt == 'p' || optopt == 'a' || optopt == 'i' || optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        return 1;
      default:
        abort ();
    }

  int ConnectSocket;  // сокет для общения с сервером
  sockaddr_in ServerAddr;  // это будет адрес сервера
  int err, maxlen = 512;  // код ошибки и размер буферов
  char* recvbuf = new char[maxlen+1];  // буфер приема

  ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  ServerAddr.sin_family = AF_INET;
  //используйте функцию gethostbyname для подключения к серверу по DNS-имени
  ServerAddr.sin_addr.s_addr = inet_addr(addr.c_str());
  ServerAddr.sin_port = htons(port);

	int width, height;
  scanf("%d %d", &width, &height);
	double* matrix = (double*)malloc( sizeof(double) * height*width );
	double* k      = (double*)malloc( sizeof(double) * height );
	
	for ( int i=0; i < width; ++i )
		for ( int j=0; j < height; ++j )
			scanf("%lf", &matrix[ linearization(i, j, width) ]);
	
	for ( int j=0; j < height; ++j )
			scanf("%lf", &k[j]);
	
    // Connect to server
  if( connect( ConnectSocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1)
    handleError("connect failed:");
		
    // отправляем запрос на сервер
	send( ConnectSocket, &width,  sizeof(width), 0 );
	send( ConnectSocket, &height, sizeof(height), 0 );
	send( ConnectSocket, matrix,  sizeof(double) * height*width, 0 );
	send( ConnectSocket, k,       sizeof(double) * height, 0 );
	
	double* answer = (double*)malloc( sizeof(double) * height );
  recv(ConnectSocket, answer, sizeof(double) * height, 0);
	for ( int j=0; j < height; ++j )
	{
		printf( "%s%lf", j > 0 ? " " : "", answer[j]);
	}
  printf("\n");


  // shutdown the connection since we're done
  close(ConnectSocket);
  return 0;
}
