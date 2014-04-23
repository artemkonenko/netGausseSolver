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

double* gausse_solve( double* matrix, double* k, int width, int height, const bool debug )
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
				if ( debug )
					cout << "k = " << koef << " (" << i << ", " << j << ") " << endl;
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

	if ( debug )	
		for ( int i=0; i < height; ++i )
		{
			for ( int j=0; j < height; ++j )
				cout << matrix[linearization(i, j, width)] << " ";

			cout << "| " << k[i] << " -> " << ret[i] << endl;
		}
	
	return ret;
}

int main( int argc, char* argv[] )
{
	int port = 9753;
  	string addr = "127.0.0.1";
 	bool debug = false;

	char c;
  	while ((c = getopt (argc, argv, "p:a:d")) != -1)
	    switch (c)
	    {
	      case 'p':
	        sscanf( optarg, "%d", &port );
	        break;
	      case 'a':
	        addr = optarg;
	        break;
	      case 'd':
	      	debug = true;
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

    int ListenSocket, ClientSocket;  // впускающий сокет и сокет для клиентов
    sockaddr_in ServerAddr;  // это будет адрес сервера
    int err, maxlen 	= 	512;  // код ошибки и размер буферов
    char* recvbuf 		=	new char[maxlen+1];  // буфер приема
    char* result_string = 	new char[maxlen];  // буфер отправки

    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int on = 1;
    if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        handleError("setsockopt failed:");

    // Setup the TCP listening socket
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = inet_addr(addr.c_str());
    ServerAddr.sin_port = htons(port);
    
    if (bind( ListenSocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1) 
        handleError("bind failed:");
    if (listen(ListenSocket, 50) == -1) 
        handleError("listen failed:");
    while (true) 
    {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
		
		int width, height;
		recv(ClientSocket, &width, sizeof(width), 0);
		recv(ClientSocket, &height, sizeof(height), 0);
		if (debug)
			cout << "Serverside: " << width << "x" << height << endl;
		
		double* matrix = (double*)malloc( sizeof(double) * height*width );
		double* k	   = (double*)malloc( sizeof(double) * height );

		recv(ClientSocket, matrix, sizeof(double) * width*height, 0);
		recv(ClientSocket, k, sizeof(double) * height, 0);
		
		double* answer = gausse_solve(matrix, k, width, height, debug);

		if (debug)
			print_matrix(matrix, k, answer, width, height);

		for ( int j=0; j < height; ++j )
		{
			send( ClientSocket,  answer, sizeof(double) * height, 0 );
		}
		
		if (debug)
        	cout << "Send answer complete." << endl;

        // закрываем соединение с клиентом
        close(ClientSocket);

        if ( debug )
        	cout << "============================================================" << endl;
    }
    return 0;
}

