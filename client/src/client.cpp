#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"

using namespace std;

void read_matrix ( Size& size, double* &matrix)
{
	// Width Height
	if ( !scanf("%d %d", &size.width, &size.height) ) handleError("File reading error.");
	matrix = (double*)malloc( sizeof(double) * ((size.width+1) * size.height + 1));
	// Засовываем размер в первый дабл массива.
	reinterpret_cast<int*>(matrix)[0] = size.width;
	reinterpret_cast<int*>(matrix)[1] = size.height;

	for ( int i=0; i < size.height; ++i )
		for ( int j=0; j < size.width; ++j )
			if ( !scanf("%lf", &matrix[linearization(i, j, size.width + 1)]) ) handleError("File reading error.");

	for ( int i=0; i < size.height; ++i )
		if ( !scanf("%lf", &matrix[linearization(i, size.width, size.width + 1)]) ) handleError("File reading error.");
}

void print_matrix( Size& size, double* &matrix )
{
	for ( int i=0; i < size.height; ++i )
	{
		for ( int j=0; j < size.width; ++j )
		{
			printf( "%s%lf", j > 0 ? " " : "", matrix[ linearization(i, j, size.width+1)]);
		}
		printf("\n");
	}
}

int main( int argc, char* argv[] )
{
	Connector conn;
	bool debug = false;

	char c;
	while ((c = (char)getopt (argc, argv, "tuhp:a:i:o:")) != -1)
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
				if (!freopen(optarg, "r", stdin))
				{
					fprintf (stderr, "File %s not exist.\n", optarg);
					return -1;
				}
				break;
			case 'o':
				if ( !freopen(optarg, "w", stdout) ) handleError("File open error.");
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

	Size size;
	double *matrix;
	read_matrix(size, matrix);

	// отправляем запрос на сервер
	conn.send( matrix,	size.height*(size.width+1) + 1 );

	double* answer = (double*)malloc( sizeof(double) * size.height );

	conn.recv( answer, size.height );

	print_matrix( matrix, answer, size.width, size.height);

	return 0;
}
