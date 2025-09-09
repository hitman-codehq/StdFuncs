
#include "StdFuncs.h"
#include "StdSocket.h"

#if defined(__unix__) || defined(__amigaos__)

#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define closesocket(socket) ::close(socket)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#else /* ! defined(__unix__) || defined(__amigaos__) */

#include <ws2tcpip.h>

int RSocket::m_useCount;

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
 * Connect to a remote host, with timeout.
 * This method attempts to connect() to a remote host, but with a timeout.  If the connection cannot be made within
 * the specified timeout period, the operation will fail and an error indicating this will be returned.
 *
 * @date	Tuesday 09-Sep-2025 9:39 am, WbT Cafeteria
 * @param	a_address		The address of the host to which to connect
 * @param	a_addressLength	The length of the address structure
 * @param	timeout			The timeout in seconds
 * @return	KErrNone if successful
 * @return	KErrTimeOut if the connection timed out
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RSocket::connect(const struct sockaddr *a_address, socklen_t a_addressLength, int timeout)
{
	int retVal = KErrGeneral;

	/* Set to non-blocking so that we can use select() to wait for the connection, rather than waiting for connect() */
	if (setBlocking(false) == KErrNone)
	{
		/* Try to connect and ensure that the connection either succeeded immediately or would block */
		if (::connect(m_socket, a_address, a_addressLength) == 0)
		{
			retVal = KErrNone;
		}
		else
		{

#if defined(__unix__) || defined(__amigaos__)

			if (errno == EINPROGRESS)

#else /* ! defined(__unix__) || defined(__amigaos__) */

			int lastError = WSAGetLastError();

			if (lastError == WSAEWOULDBLOCK || lastError == WSAEINPROGRESS)

#endif /* ! defined(__unix__) || defined(__amigaos__) */

			{
				fd_set descriptors;
				struct timeval timeVal;

				/* Wait for the connection to succeed, or for the attempt to timeout */
				FD_ZERO(&descriptors);
				FD_SET(m_socket, &descriptors);
				timeVal.tv_sec = timeout;
				timeVal.tv_usec = 0;

				int result = select(static_cast<int>(m_socket + 1), nullptr, &descriptors, nullptr, &timeVal);

				if (result > 0)
				{
					int error = 0;
					socklen_t error_len = sizeof(error);

					/* Select could have returned due to an error on the socket, so check for that */
					result = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char *) &error, &error_len);

					if (result == 0 && error == 0)
					{
						retVal = KErrNone;
					}
				}
				else if (result == 0)
				{
					retVal = KErrTimeOut;
				}
			}
		}

		/* And put the socket back to its original blocking mode */
		setBlocking(true);
	}

	return retVal;
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
 * @return	KErrNotOpen if a connection to the remote server could not be made
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
	{
		++m_useCount;
	}
	else
	{
		return KErrGeneral;
	}

#endif /* WIN32 */

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

				if (RSocket::connect((struct sockaddr *) &sockAddr, sizeof(sockAddr), 5) == KErrNone)
				{
					retVal = KErrNone;
				}
				else
				{
					retVal = KErrNotOpen;

					close();
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

	if (retVal != KErrNone)
	{
		close();
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

#ifdef WIN32

	bool socketUsed = m_socket != INVALID_SOCKET || m_serverSocket != INVALID_SOCKET;

#endif /* WIN32 */

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

	if (socketUsed)
	{
		if (--m_useCount == 0)
		{
			DEBUGCHECK((WSACleanup() == 0), "RSocket::close() => Unable to shut down Winsock");
		}
	}

#endif /* WIN32 */

}

/**
 * Accepts an incoming connection.
 * When a new connection is accepted, the connected socket will be put into use as the active socket,
 * and data can then be read from, and written to, the remote host.
 *
 * @pre		The socket has been put in a listening state with listen()
 *
 * @date	Saturday 08-Apr-2023 8:22 am, Code HQ Tokyo Tsukuda
 * @return	KErrNone if successful
 * @return	KErrBreak if interrupted by ctrl+c signal
 * @return 	KErrGeneral if any other error occurred
 */

int RSocket::accept()
{
	int retVal;
	socklen_t clientSize;
	struct sockaddr_in client;

	ASSERTM((m_serverSocket != INVALID_SOCKET), "RSocket::accept() => Socket is not in listening state")

	clientSize = sizeof(client);
	m_socket = ::accept(m_serverSocket, (struct sockaddr*) &client, &clientSize);

	if (m_socket != INVALID_SOCKET)
	{
		retVal = KErrNone;
	}
	else
	{
		retVal = (errno == EINTR) ? KErrBreak : KErrGeneral;
	}

	return retVal;
}

/**
 * Indicates whether the socket is open.
 * Returns a boolean indicating whether the socket is open and connected to a remote server.
 *
 * @date	Tuesday 14-May-2024 6:41 am, Code HQ Tokyo Tsukuda
 * @return	true if the socket is open, else false
 */

bool RSocket::isOpen()
{
	return m_socket != INVALID_SOCKET;
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

	struct sockaddr_in server;

	ASSERTM((m_socket != INVALID_SOCKET), "RSocket::listen() => Socket is not open")

	server.sin_family = AF_INET;
	server.sin_port = htons(a_port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_socket, (struct sockaddr *) &server, sizeof(server)) != SOCKET_ERROR)
	{
		if (::listen(m_socket, 1) == 0)
		{
			retVal = KErrNone;

			m_serverSocket = m_socket;
			m_socket = INVALID_SOCKET;
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

	ASSERTM((m_socket != INVALID_SOCKET), "RSocket::read() => Socket is not open")

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
 * Set the socket to blocking or non-blocking mode.
 * Sets the socket to the requested mode.
 *
 * @date	Thursday 11-Sep-2025 6:14 am, Code HQ Tokyo Tsukuda
 * @param	a_blocking		True for blocking mode, else false for non-blocking
 * @return	KErrNone if successful
 * @return	KErrGeneral if an error occurred
 */

int RSocket::setBlocking(bool a_blocking)
{
	int retVal = KErrGeneral;

#if defined(__unix__) || defined(__amigaos__)

	int flags = fcntl(m_socket, F_GETFL, 0);

	if (flags != -1)
	{
		if (fcntl(m_socket, F_SETFL, a_blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK)) != -1)
		{
			retVal = KErrNone;
		}
	}

#else /* ! defined(__unix__) || defined(__amigaos__) */

	u_long mode = a_blocking ? 0 : 1;

	if (ioctlsocket(m_socket, FIONBIO, &mode) == 0)
	{
		retVal = KErrNone;
	}

#endif /* ! defined(__unix__) || defined(__amigaos__) */

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

	ASSERTM((m_socket != INVALID_SOCKET), "RSocket::write() => Socket is not open")

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
