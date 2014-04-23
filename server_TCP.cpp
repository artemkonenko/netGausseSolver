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


using namespace std;

void handleError(string msg)
{
    cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
    exit(1);
}

vector<double> gausse_solve( vector<vector<double> >& matrix, vector<double>& k, const bool debug )
{
	vector<double> ret(k.size());
	
	for ( int i=0; i < matrix.size(); ++i )
	{
		// Делаем первый символ лидирующим
		for ( int j=i+1; j < matrix.size(); ++j )
		{
			if ( matrix[j][i] != 0 )
			{
				double koef = matrix[i][i] / matrix[j][i];
				if ( debug )
					cout << "k = " << koef << " (" << i << ", " << j << ") " << endl;
				for ( int l=i; l < matrix[0].size(); l++ )
				{
					if ( debug )
					{
						cout<<"M["<<j<<"]["<<l<<"] = M["<<j<<"]["<<l<<"]*M["<<i<<"]["<<i<<"]/M["<<j<<"]["<<i<<"]-M["<<i<<"]["<<l<<"] = ";
						cout<<matrix[j][l]<<" * "<<koef<<" - "<<matrix[i][l] << " = " << matrix[j][l] * koef - matrix[i][l] << endl;
					}
					matrix[j][l] = matrix[j][l] * koef - matrix[i][l];
				}
				k[j] = k[j] * koef - k[i];
			}
		}
	}
	
	for ( int i=matrix.size()-1; i >= 0; --i )
	{
		ret[i] = k[i];
		for ( int j=matrix[0].size()-1; j > i; --j )
		{
			ret[i] -= matrix[i][j] * ret[j];
		}
		ret[i] /= matrix[i][i];
	}

	if ( debug )	
		for ( int i=0; i < k.size(); ++i )
		{
			for ( int j=0; j < k.size(); ++j )
				cout << matrix[i][j] << " ";

			cout << "| " << k[i] << " -> " << ret[i];
			cout << endl;
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
    int err, maxlen = 512;  // код ошибки и размер буферов
    char* recvbuf=new char[maxlen+1];  // буфер приема
    char* result_string=new char[maxlen];  // буфер отправки

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
		
		vector<vector<double> > matrix(height, vector<double>(width));
		vector<double> k(height);
		
		if (debug)
			cout << "Serverside matrix: " << endl;

		for ( int i=0; i < height; ++i )
		{
			for ( int j=0; j < width; ++j )
			{
				recv(ClientSocket, &matrix[i][j], sizeof(matrix[i][j]), 0);
				if (debug)
					cout << matrix[i][j] << " ";
			}
			if ( debug )
				cout << endl;
		}
		if (debug)
			cout << "[";
		for ( int j=0; j < height; ++j )
		{
			recv(ClientSocket, &k[j], sizeof(k[j]), 0);
			if (debug)
				cout << k[j] << " ";
		}
		if (debug)
			cout << "]" << endl;
		
		
		vector<double> answer = gausse_solve(matrix, k, debug);
		
		if ( debug )
		{
			cout << "Answer: [";
			for (int i = 0; i < answer.size(); ++i)
			{
				cout << (i > 0 ? ", " : "") << answer[i];
			}
			cout << "]" << endl;

		}

		for ( int j=0; j < height; ++j )
		{
			send( ClientSocket,  &answer[j], sizeof(answer[j]), 0 );
		}
		
		if (debug)
        	cout << "Send answer complete" << endl;

        // закрываем соединение с клиентом
        close(ClientSocket);

        if ( debug )
        	cout << "============================================================";
    }
    return 0;
}

