#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
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

void print_matrix( const int width, const int height, const double* matrix )
{
  for ( int i=0; i < height; ++i )
  {
    for ( int j=0; j < width; ++j )
    {
      printf( "%s%lf", j > 0 ? " " : "", matrix[ linearization(i, j, width) ]);
    }
    printf("\n");
  }
}

int main( int argc, char* argv[] )
{
  Connector conn;
  bool debug = false;

  char c;
  while ((c = getopt (argc, argv, "tuhp:a:i:o:")) != -1)
    switch (c)
    {
      case 'p':
        int port;
        sscanf( optarg, "%d", &port );
        conn.setPort(port);
        break;
      case 'a':
        conn.setAddr(optarg);
        break;
      case 'i':
        freopen(optarg, "r", stdin);
        break;
      case 'o':
        freopen(optarg, "w", stdout);
        break;
      case 't':
        conn.setTCP();
        break;
      case 'u':
        conn.setUDP();
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
  conn.connect();

  int width, height;
  double *matrix, *k;
  read_matrix(width, height, matrix, k);

  // отправляем запрос на сервер
  conn.send( &width,  1 );
  conn.send( &height, 1 );
  conn.send( matrix,  height*width );
  conn.send( k,       height );

  double* answer = (double*)malloc( sizeof(double) * height );

  conn.recv( answer, height );

  print_matrix( matrix, k, answer, width, height);  

  return 0;
}
