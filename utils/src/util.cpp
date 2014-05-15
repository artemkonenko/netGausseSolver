// For somethings
#include <errno.h>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// For network
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "util.h"

using namespace std;

// (y, x)
int linearization(int i, int j, int width)
{
  return i*width + j + 1;
}

void handleError(std::string msg)
{
    cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
    exit(1);
}

void print_matrix(const double* matrix, const double* answer, const int width, const int height)
{
	for (int i = 0; i < height; ++i)
	{
		cout << "( ";
		for (int j = 0; j < width; ++j)
		{
			cout << (j > 0 ? " " : "") << matrix[linearization(i, j, width+1)];
		}
		cout << " | " << matrix[linearization(i, width, width+1)] << " ) " << (i == height / 2 ? "=" : " ") << "( " << answer[i] << " )" << endl;
	}
}