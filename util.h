#include <string.h>

#ifndef __UTIL
#define __UTIL

enum ConnectionType {
	TCPconnect,
	UDPconnect
};

int linearization(int i, int j, int width);

void handleError(std::string msg);

void print_matrix(double* matrix, double* k, double* answer, int width, int height);

#endif