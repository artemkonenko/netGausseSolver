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

double* gausse_solve( double* matrix, int width, int height)
{
	double* ret = (double*)malloc( sizeof(double) * height );

	for ( int i=0; i < height; ++i )
	{
		// Делаем первый символ лидирующим
		for ( int j=i+1; j < height; ++j )
		{
			if ( matrix[ linearization(j, i, width+1) ] != 0 )
			{
				double koef = matrix[linearization(i, i, width+1)] / matrix[linearization(j, i, width+1)];

				for ( int l=i; l <= width; l++ )
					matrix[linearization(j, l, width+1)] = matrix[linearization(j, l, width+1)] * koef - matrix[linearization(i, l, width+1)];
			}
		}
	}
	
	for ( int i=height-1; i >= 0; --i )
	{
		ret[i] = matrix[linearization(i, width, width+1)];
		for ( int j=width-1; j > i; --j )
		{
			ret[i] -= matrix[linearization(i, j, width+1)] * ret[j];
		}
		ret[i] /= matrix[linearization(i, i, width+1)];
	}
	
	return ret;
}

int maxBufferWidth = 100;
int maxBufferHeight= 100;

void handle( BridgeWrapper bw )
{
	double* matrix = (double*)malloc( sizeof(double) * (maxBufferWidth * maxBufferHeight +1) );
	bw.recv( matrix, maxBufferWidth * maxBufferHeight +1 );

	double* answer = gausse_solve(matrix, reinterpret_cast<int*>(matrix)[0], reinterpret_cast<int*>(matrix)[1]);

	//if (debug)
		print_matrix(matrix, answer, reinterpret_cast<int*>(matrix)[0], reinterpret_cast<int*>(matrix)[1]);

	bw.send( answer, reinterpret_cast<int*>(matrix)[1] );
	
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

