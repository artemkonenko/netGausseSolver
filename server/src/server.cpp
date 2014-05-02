#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unistd.h>
#include <vector>

#include "util.h"

using namespace std;

double* gausse_solve( double* matrix, double* k, int width, int height)
{
	double* ret = (double*)malloc( sizeof(double) * height );

	for ( int i=0; i < height; ++i )
	{
		// Делаем первый символ лидирующим
		for ( int j=i+1; j < width; ++j )
		{
			if ( matrix[ linearization(i, j, width) ] != 0 )
			{
				double koef = matrix[linearization(i, i, width)] / matrix[linearization(j, i, width)];

				for ( int l=i; l < width; l++ )
					matrix[linearization(j, l, width)] = matrix[linearization(j, l, width)] * koef - matrix[linearization(i, l, width)];
				k[j] = k[j] * koef - k[i];
			}
		}
	}
	
	for ( int i=height-1; i >= 0; --i )
	{
		ret[i] = k[i];
		for ( int j=width-1; j > i; --j )
		{
			ret[i] -= matrix[linearization(i, j, width)] * ret[j];
		}
		ret[i] /= matrix[linearization(i, i, width)];
	}
	
	return ret;
}

void handle( BridgeWrapper bw )
{
	int width, height;
	bw.recv( &width, 1 );
	bw.recv( &height, 1 );

	//if (debug)
		cout << "Serverside: " << width << "x" << height << endl;
	
	double* matrix = (double*)malloc( sizeof(double) * height*width );
	double* k	   = (double*)malloc( sizeof(double) * height );

	bw.recv( matrix, width*height );
	bw.recv( k, height );
	
	double* answer = gausse_solve(matrix, k, width, height);

	//if (debug)
		print_matrix(matrix, k, answer, width, height);

	bw.send( answer, height );
	
	//if (debug)
    	cout << "Send answer complete." << endl;
}

int main( int argc, char* argv[] )
{
	Gate gate;
 	bool debug = false;

	char c;
  	while ((c = getopt (argc, argv, "tup:a:d")) != -1)
	    switch (c)
	    {
	      case 'd':
	      	debug = true;
	      	break;
	      case 'p':
	        int port;
	        sscanf( optarg, "%d", &port );
	        gate.setPort(port);
	        break;
	      case 'a':
	        gate.setAddr(optarg);
	        break;
	      case 't':
	        gate.setTCP();
	        break;
	      case 'u':
	        gate.setUDP();
	        break;
	      case '?':
	        if ( optopt == 'p' || optopt == 'a')
	          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	        else if (isprint (optopt))
	          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	        else
	          fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
	        return 1;
	      default:
	        abort ();
	    }

    gate.bind( handle );

    return 0;
}

