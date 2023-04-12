
#include "StdFuncs.h"
#include "StdSocket.h"

#if defined(__unix__) || defined(__amigaos__)

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define closesocket(socket) ::close(socket)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#else /* ! defined(__unix__) || defined(__amigaos__) */

#include <ws2tcpip.h>

#endif /* ! defined(__unix__) || defined(__amigaos__) */

/**
 * Typeinfo workaround.
 * This is a workaround for Amiga GCC 6.5.0b.  In order to catch an exception that is derived from the
 * std::runtime_error or std::exception class, some typeinfo needs to be present.  This can be forced
 * by having at least one concrete implementation of a virtual method.  Other compilers (including GCC
 * 6.5.0) on other platforms work without this, but not Amiga GCC 6.5.0b.
 *
 * Do not remove this or exception handling will break on Amiga OS!
 *
 * @date	Saturday 08-May-2021 7:47 am, Code HQ Bergmannstrasse
 */

RSocket::Error::~Error() noexcept
{
}

/**
 * RSocket constructor.
 * Initialises the socket to a state ready for connection.
 *
 * @date	Wednesday 29-Jan-2020 4:47 pm, Scoot flight TR 735 to Singapore
 */

RSocket::RSocket()
{
	m_serverSocket = m_socket = INVALID_SOCKET;
}

/**
 * Opens a socket in either client or server mode.
 * Depending on whether a host name is specified, this method will open a socket for use either as a client
 * or a server.  If a host is specified, it will be resolved and a connection made to the it.  The host can
 * be either a host name such as codehq.org or an IP address.  If no host is specified, the socket will be
 * opened in a state suitable for listening.
 *
 * @date	Saturday 11-Feb-2017 4:37 pm, Code HQ Habersaathstrasse
 * @param	a_host			The name of the host, an IP address or NULL
 * @param	a_port			The port to which to connect
 * @return	KErrNone if successful
 * @return	KErrGeneral if the socket could not be opened
 * @return	KErrHostNotFound if the host could not be resolved
 */

int RSocket::open(const char *a_host, unsigned short a_port)
{
	int retVal = KErrGeneral;

	struct hostent *hostEnt;
	struct in_addr *inAddr;
	struct sockaddr_in sockAddr;

#ifdef WIN32

	WSAData wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)

#endif /* WIN32 */

	{
		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
		{
			if (a_host)
			{
				if ((hostEnt = gethostbyname(a_host)) != nullptr)
				{
					inAddr = (struct in_addr *) hostEnt->h_addr_list[0];

					sockAddr.sin_family = hostEnt->h_addrtype;
					sockAddr.sin_port = htons(a_port);
					sockAddr.sin_addr = *inAddr;

					if (connect(m_socket, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) >= 0)
					{
						retVal = KErrNone;
					}
				}
				else
				{
					retVal = KErrHostNotFound;
				}
			}
			else
			{
				/* When running as a host, enable SO_LINGER to ensure that socket is cleanly closed and can thus be */
				/* immediately reopened for the next client connection */
				struct linger Linger = { 1, 0 };

				if (setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char *) &Linger, sizeof(Linger)) == 0)
				{
					retVal = KErrNone;
				}
			}
		}
	}

	return retVal;
}

/**
 * Closes any open connections.
 * Closes any currently open sockets, setting them back to a disconnected state.
 *
 * @date	Saturday 11-Feb-2017 5:41 pm, Code HQ Habersaathstrasse
 */

void RSocket::close()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	if (m_serverSocket != INVALID_SOCKET)
	{
		closesocket(m_serverSocket);
		m_serverSocket = INVALID_SOCKET;
	}

#ifdef WIN32

	WSACleanup();

#endif

}

/**
 * Listens for incoming connections.
 * Binds the socket to a local address and the given port number and listens for incoming connections
 * on it.  When a new connection is accepted, the connected socket will be put into use as the active
 * socket, and data can then be read from, and written to, the remote host.
 *
 * @pre		The socket has been opened with open()
 *
 * @date	Sunday 12-Feb-2017 7:53 am, Code HQ Habersaathstrasse
 * @param	a_port			The port on which to listen for connections
 * @return	KErrNone if successful, otherwise KErrGeneral
 */

int RSocket::listen(unsigned short a_port)
{
	int retVal = KErrGeneral;

	socklen_t clientSize;
	SOCKET socket;
	struct sockaddr_in server, client;

	server.sin_family = AF_INET;
	server.sin_port = htons(a_port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr *) &server, sizeof(server)) != SOCKET_ERROR)
	{
		if (::listen(m_socket, 1) == 0)
		{
			clientSize = sizeof(client);
			socket = accept(m_socket, (struct sockaddr *) &client, &clientSize);

			if (socket != INVALID_SOCKET)
			{
				retVal = KErrNone;

				m_serverSocket = m_socket;
				m_socket = socket;
			}
		}
	}

	return retVal;
}

/**
 * Reads data from the socket.
 * Reads the number of bytes requested from the socket.  This method can operated in a "read all" mode and
 * a "read waiting" mode, as specified by the a_bReadAll parameter.  If this is specified as true then the
 * entire number of bytes specified by a_size will be read, even if this involves blocking.  If this is
 * false then only the number of bytes waiting on the socket will be read.
 *
 * @pre		The socket has been opened with open()
 *
 * @date	Saturday 11-Feb-2017 5:59 pm, Code HQ Habersaathstrasse
 * @param	a_buffer		Pointer to the buffer into which to read the data
 * @param	a_size			The number of bytes to be read
 * @param	a_readAll		true to read entire number of bytes specified
 * @throw	Error			If the socket could not be read from or was closed
 * @return	The number of bytes received
 */

int RSocket::read(void *a_buffer, int a_size, bool a_readAll)
{
	char *buffer = (char *) a_buffer;
	int bytesToRead, retVal = 0, size;

	if (a_readAll)
	{
		/* Loop around, trying to read however many bytes are left to be read, starting with the total number */
		/* of bytes and gradually reducing the size until all have been read */
		do
		{
			bytesToRead = (a_size - retVal);

			if ((size = recv(m_socket, (buffer + retVal), bytesToRead, 0)) > 0)
			{
				retVal += size;
			}
			else
			{
				retVal = size;
				break;
			}
		}
		while (retVal < a_size);
	}
	else
	{
		retVal = recv(m_socket, buffer, a_size, 0);
	}

	if (retVal < 0)
	{
		throw Error("Unable to read from socket", retVal);
	}
	else if (retVal == 0)
	{
		throw Error("Socket closed by remote host", retVal);
	}

	return retVal;
}

/**
 * Writes data to the socket.
 * Writes the number of bytes requested to the socket.  The entire number of bytes specified by a_size
 * will be written, even if this involves blocking.
 *
 * @pre		The socket has been opened with open()
 *
 * @date	Saturday 11-Feb-2017 5:55 pm, Code HQ Habersaathstrasse
 * @param	a_buffer		Pointer to the buffer from which to write the data
 * @param	a_size			The number of bytes to be written
 * @throw	Error			If the socket could not be written to or was closed
 * @return	The number of bytes sent
 */

int RSocket::write(const void *a_buffer, int a_size)
{
	const char *buffer = (const char *) a_buffer;
	int bytesToWrite, retVal = 0, size;

	/* Loop around, trying to write however many bytes are left to be written, starting with the total number */
	/* of bytes and gradually reducing the size until all have been written */
	do
	{
		bytesToWrite = (a_size - retVal);

		if ((size = send(m_socket, (buffer + retVal), bytesToWrite, 0)) > 0)
		{
			retVal += size;
		}
		else
		{
			retVal = size;
			break;
		}
	}
	while (retVal < a_size);

	if (retVal <= 0)
	{
		throw Error("Unable to write to socket", retVal);
	}
	else if (retVal == 0)
	{
		throw Error("Socket closed by remote host", retVal);
	}

	return retVal;
}

/**
 * Writes a string to the socket.
 * A convenience method that writes a NULL terminated string to the socket, including the NULL
 * terminator itself.
 *
 * @date	Sunday 10-Jan-2021 7:30 am, Code HQ Bergmannstrasse
 * @param	a_buffer		A pointer to the NULL terminated string to be written
 * @return	The number of bytes written to the socket
 */

int RSocket::write(const char *a_buffer)
{
	return write((const void *) a_buffer, (int) (strlen(a_buffer) + 1));
}
