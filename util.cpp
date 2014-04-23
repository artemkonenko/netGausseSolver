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

int linearization(int i, int j, int width)
{
  return i*width + j;
}

void handleError(std::string msg)
{
    cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
    exit(1);
}

void print_matrix(double* matrix, double* k, double* answer, int width, int height)
{
	for (int i = 0; i < height; ++i)
	{
		cout << "( ";
		for (int j = 0; j < width; ++j)
		{
			cout << (j > 0 ? " " : "") << matrix[linearization(i, j, width)];
		}
		cout << " | " << k[i] << " ) " << (i == height / 2 ? "=" : " ") << "( " << answer[i] << " )" << endl;
	}
}
