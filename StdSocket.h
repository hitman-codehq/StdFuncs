
#ifndef STDSOCKET_H
#define STDSOCKET_H

/** @file */

#include <stdexcept>

#ifdef WIN32

#include <winsock2.h>

#else /* ! WIN32 */

typedef int SOCKET;

#endif /* ! WIN32 */

/**
 * A class providing synchronous socket communications.
 * This class provides basic synchronous socket communications, such as sending and receiving of data,
 * as well as automatic host name resolution when opening new sockets that reference a host name.  It
 * can be used in either client or server mode, where in server mode it can listen for and accept
 * incoming connections.
 */

class RSocket
{
public:

	class Error : public std::runtime_error
	{
	public:

		int	m_result;	/**< The error code that triggered the exception */

		Error(const char *a_message, int a_result) : std::runtime_error(a_message), m_result(a_result) { }

		~Error() noexcept;
	};

private:

#ifdef WIN32

	static int	m_useCount;		/**< Number of times the socket has been opened */

#endif /* WIN32 */
		
	SOCKET	m_serverSocket;		/**< The socket on which to listen for connections */

public:

	SOCKET	m_socket;			/**< The socket with which data to transfer data */

public:

	RSocket();

	~RSocket()
	{
		close();
	}

	int open(const char* a_host, unsigned short a_port);

	void close();

	int accept();

	bool isOpen();

	int listen(unsigned short a_port);

	int read(void* a_buffer, int a_size, bool a_readAll = true);

	int write(const void* a_buffer, int a_size);

	int write(const char* a_buffer);
};

#endif /* ! STDSOCKET_H */
