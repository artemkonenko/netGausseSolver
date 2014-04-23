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

void read_matrix ( int &width, int &height, double* &matrix, double* &k )
{
  scanf("%d %d", &width, &height);
  matrix = (double*)malloc( sizeof(double) * height*width );
  k = (double*)malloc( sizeof(double) * height );
  for ( int i=0; i < width; ++i )
    for ( int j=0; j < height; ++j )
      scanf("%lf", &matrix[ linearization(i, j, width) ]);
  
  for ( int j=0; j < height; ++j )
      scanf("%lf", &k[j]);
}

void send_task_and_get_answer(int ConnectSocket, int width, int height, double* matrix, double* k, double* answer )
{
  // отправляем запрос на сервер
  send( ConnectSocket, &width,  sizeof(int), 0 );
  send( ConnectSocket, &height, sizeof(int), 0 );
  send( ConnectSocket, matrix,  sizeof(double) * height*width, 0 );
  send( ConnectSocket, k,       sizeof(double) * height, 0 );
  
  recv( ConnectSocket, answer, sizeof(double) * height, 0);
}

int main( int argc, char* argv[] )
{
  int port = 9753; // порт сервера
  string addr = "127.0.0.1"; // адрес сервера
  ConnectionType contype = TCPconnect; // Тип соединения с сервером
  int ConnectSocket;  // сокет для общения с сервером
  sockaddr_in ServerAddr;  // это будет адрес сервера
  int err;  // код ошибки

  char c;
  while ((c = getopt (argc, argv, "tuhp:a:i:o:")) != -1)
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
      case 't':
        contype = TCPconnect;
        break;
      case 'u':
        contype = UDPconnect;
        break;
      case 'h':
        cout << "Usage: client [-p port] [-a ip] [-i input_filepath] [-o output_filepath]"<< endl;
        cout << "-t to conection over TCP"<< endl;
        cout << "-u to connection over UDP"<< endl;
        cout << "-h for this help."<< endl;
        return 0;
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

  switch( contype )
  {
    case TCPconnect:
      ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      break;
    case UDPconnect:
      ConnectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      break;
  }

  ServerAddr.sin_family = AF_INET;
  //используйте функцию gethostbyname для подключения к серверу по DNS-имени
  ServerAddr.sin_addr.s_addr = inet_addr(addr.c_str());
  ServerAddr.sin_port = htons(port);
  // Connect to server
  if( connect( ConnectSocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1)
    handleError("connect failed:");

  int width, height;
  double *matrix, *k;
  read_matrix(width, height, matrix, k);
  
  double* answer = (double*)malloc( sizeof(double) * height );

  send_task_and_get_answer(ConnectSocket, width, height, matrix, k, answer );
  
  for ( int j=0; j < height; ++j )
  {
    printf( "%s%lf", j > 0 ? " " : "", answer[j]);
  }
  printf("\n");


  // shutdown the connection since we're done
  close(ConnectSocket);
  return 0;
}
