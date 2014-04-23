#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace std;

void handleError(string msg)
{
    cerr << msg << " error code " << errno << " (" << strerror(errno) << ")\n";
    exit(1);
}

int main( int argc, char* argv[] )
{
	int port = 9753;
  string addr = "127.0.0.1";
  string inputfilesrc = "input.dat";
  string outputfilesrc = "output.dat";

  char c;
  while ((c = getopt (argc, argv, "p:a:i:o:")) != -1)
    switch (c)
    {
      case 'p':
        sscanf( optarg, "%d", &port );
        break;
      case 'a':
        addr = optarg;
        break;
      case 'i':
        inputfilesrc = optarg;
        break;
      case 'o':
        outputfilesrc = optarg;
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

	freopen(inputfilesrc.c_str(), "r", stdin);
	freopen(outputfilesrc.c_str(), "w", stdout);

    int ConnectSocket;  // сокет для общения с сервером
    sockaddr_in ServerAddr;  // это будет адрес сервера
    int err, maxlen = 512;  // код ошибки и размер буферов
    char* recvbuf = new char[maxlen+1];  // буфер приема

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    ServerAddr.sin_family = AF_INET;
    //используйте функцию gethostbyname для подключения к серверу по DNS-имени
    ServerAddr.sin_addr.s_addr = inet_addr(addr.c_str());
    ServerAddr.sin_port = htons(port);

	int width, height;
	cin >> width >> height;
	vector<vector<double> > matrix(height, vector<double>(width));
	vector<double> k(height);
	
	for ( int i=0; i < width; ++i )
		for ( int j=0; j < height; ++j )
			cin >> matrix[j][i];
	
	for ( int j=0; j < height; ++j )
			cin >> k[j];
	
    // Connect to server
    if( connect( ConnectSocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1)
        handleError("connect failed:");
		
    // отправляем запрос на сервер
	send( ConnectSocket,  &width, sizeof(width), 0 );
	send( ConnectSocket,  &height, sizeof(height), 0 );
	for ( int i=0; i < width; ++i )
		for ( int j=0; j < height; ++j )
			send( ConnectSocket,  &matrix[j][i], sizeof(matrix[j][i]), 0 );
    for ( int j=0; j < height; ++j )
		send( ConnectSocket,  &k[j], sizeof(k[j]), 0 );
	
    cout << "Sending complete" << endl;

	
	vector<double> answer(height);
	cout << "answer: [";
	for ( int j=0; j < height; ++j )
	{
		recv(ConnectSocket, &answer[j], sizeof(answer[j]), 0);
		cout << answer[j] << " ";
	}
	cout << "]" << endl;

    // shutdown the connection since we're done
    close(ConnectSocket);
    return 0;
}

