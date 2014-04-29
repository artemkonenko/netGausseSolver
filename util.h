// For somethings
#include <string.h>
#include <algorithm>

// For network
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef __UTIL
#define __UTIL

enum ConnectionType {
	TCPconnect,
	UDPconnect
};

int linearization(int i, int j, int width);

void handleError(std::string msg);

void print_matrix(const double* matrix, const double* k, const double* answer, const int width, const int height);

class NetworkChatHelper
{
	int m_port = 9753; // порт
	std::string m_addr = "127.0.0.1"; // адрес

	ConnectionType m_contype = TCPconnect; // Тип соединения
	int m_UDP_counter;
public:
	NetworkChatHelper(int port = 9753,
					  std::string addr = "127.0.0.1",
					  ConnectionType contype = TCPconnect):m_port(port),
														   m_addr(addr),
														   m_contype(TCPconnect),
														   m_UDP_counter(0)
	{}

	void setPort( int port )
	{
		m_port = port;
	}

	int getPort()
	{
		return m_port;
	}

	void setAddr( std::string& addr )
	{
		m_addr = addr;
	}

	void setAddr( char* addr )
	{
		m_addr = std::string(addr);
	}

	std::string& getAddr()
	{
		return m_addr;
	}

	void setTCP()
	{
		m_contype = TCPconnect;
	}

	void setUDP()
	{
		m_contype = UDPconnect;
	}

	ConnectionType getConntype()
	{
		return m_contype;
	}

	int getIncUPDcounter()
	{
		return ++m_UDP_counter;
	}
};

class Connector: public NetworkChatHelper
{
	
	int m_ConnectSocket;  // сокет для общения с сервером
	sockaddr_in m_ServerAddr;  // это будет адрес сервера
public:
	Connector(int port = 9753,
			  std::string addr = "127.0.0.1",
			  ConnectionType contype = TCPconnect):NetworkChatHelper(port, addr, TCPconnect)
	{}

	~Connector()
	{
		// shutdown the connection since we're done
  		close(m_ConnectSocket);
	}

	void connect()
	{
		if ( getConntype() == TCPconnect)
		{
		  	m_ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		else
		{
		  	m_ConnectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}

		m_ServerAddr.sin_family = AF_INET;
		//используйте функцию gethostbyname для подключения к серверу по DNS-имени
		m_ServerAddr.sin_addr.s_addr = inet_addr(getAddr().c_str());
		m_ServerAddr.sin_port = htons(getPort());
		// Connect to server
		if( ::connect( m_ConnectSocket, (sockaddr *) &m_ServerAddr, sizeof(m_ServerAddr)) == -1)
			handleError("connect failed:");
	}

	template<typename T>
	void send( T* data, size_t datalength )
	{
		if ( getConntype() == TCPconnect)
		{
		  	::send( m_ConnectSocket, data, sizeof(T) * datalength, 0 );
		}
		else
		{
			T* buffer = (T*)(malloc( sizeof(T) * (datalength + 1) ));

			int cur_package = getIncUPDcounter();
			std::cout << "Sender(UDP) send package with seq: " << cur_package << "..." << std::endl;
			//(const_cast<int*>(buffer))[0] = cur_package;
			((int*)buffer)[0] = cur_package;
			std::copy( data, data + datalength, buffer + 1 ); // Копируем со сдвигом.
			int err = 0;
		    while (err == 0)
		    {
		        // отправляем запрос на сервер
		        sendto( m_ConnectSocket, data, sizeof(T) * (datalength+1), 0, (sockaddr *)&m_ServerAddr, sizeof(m_ServerAddr));

		        // проверяем, получен ли результат
		        struct timeval timeToWaitAnswer; 
		        timeToWaitAnswer.tv_sec = 1; timeToWaitAnswer.tv_usec = 0; //ждем максимум 1 секунду
		        fd_set fds; FD_ZERO(&fds); FD_SET(m_ConnectSocket, &fds); //file descriptor set
		        err = select(m_ConnectSocket+1, &fds, 0, 0, &timeToWaitAnswer);
		        if (err < 0)
		        	handleError("select failed:");
		        if (err == 0)
		            std::cout << "Packet was lost. Another attempt.\n";
		    }
		    int ack;
		    err = recvfrom(m_ConnectSocket, &ack, sizeof(int), 0, 0, 0);
		    std::cout << "Sender(UDP) recv ack: " << ack << "!" << std::endl;

		}
	}

	template<typename T>
	void recv( T* data, size_t datalength )
	{
		if ( getConntype() == TCPconnect )
		{
		  	::recv( m_ConnectSocket, data, sizeof(T) * datalength, 0 );
		}
		else
		{
			int err = recvfrom( m_ConnectSocket, data, sizeof(T) * datalength, 0, 0, 0 );
		    if (err == 0) 
		    	handleError("recv failed:");
		}
	}
};

class Gate: public NetworkChatHelper
{

public:
	Gate(int port = 9753,
		  std::string addr = "127.0.0.1",
		  ConnectionType contype = TCPconnect):NetworkChatHelper(port, addr, TCPconnect)
	{}

	void bind( void (*handler)(int)  )
	{
		int ListenSocket, ClientSocket;  // впускающий сокет и сокет для клиентов
	    sockaddr_in ServerAddr;  // это будет адрес сервера
	    /*int err, maxlen 	= 	512;  // код ошибки и размер буферов
	    char* recvbuf 		=	new char[maxlen+1];  // буфер приема
	    char* result_string = 	new char[maxlen];  // буфер отправки*/

	    // Create a SOCKET for connecting to server
	    if ( getConntype() == TCPconnect )
	    {
	    	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    }
	    else
	    {
	    	ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	    }
	    int on = 1;
	    if (setsockopt(SendRecvSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
	        handleError("setsockopt failed:");

	    // Setup the TCP listening socket
	    ServerAddr.sin_family = AF_INET;
	    ServerAddr.sin_addr.s_addr = inet_addr(getAddr().c_str());
	    ServerAddr.sin_port = htons(getPort());
	    
	    if (::bind( ListenSocket, (sockaddr *) &ServerAddr, sizeof(ServerAddr)) == -1) 
	        handleError("bind failed:");
	    if (listen(ListenSocket, 50) == -1) 
	        handleError("listen failed:");
	    while (true) 
	    {
	        // Accept a client socket
	        ClientSocket = accept(ListenSocket, NULL, NULL);
			
			handler( ClientSocket );

	        // закрываем соединение с клиентом
	        close(ClientSocket);

        	std::cout << "============================================================" << std::endl;
	    }
	}
};

#endif