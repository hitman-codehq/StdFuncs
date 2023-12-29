
#include "../StdFuncs.h"
#include "../StdApplication.h"
#include "QtLocalSocket.h"
#include <QtWidgets/QApplication>

/**
 * RLocalSocket constructor.
 * Initialises all internal variables to a known state and prepares the socket for use.
 *
 * @date	Saturday 25-Apr-2015 11:11 am, Code HQ Ehinger Tor
 */

RLocalSocket::RLocalSocket()
{
	m_bIsServer = false;
	m_poObserver = NULL;
	m_poLocalServer = NULL;
}

/**
 * RLocalSocket destructor.
 * Shuts down the socket connection and frees all resources associated with the socket.
 *
 * @date	Friday 24-Apr-2015 07:50 am, Code HQ Ehinger Tor
 */

RLocalSocket::~RLocalSocket()
{
	close();
}

/**
 * Callback method for incoming connections.
 * This function is used by server mode sockets and is called by Qt when a connection attempt is
 * made by a new client mode socket.  It will complete the connection, wait for a message from the
 * socket and will then notify any attached observer of the message and its contents.
 *
 * @date	Friday 24-Apr-2015 07:21 am, Code HQ Ehinger Tor
 */

void RLocalSocket::Connected()
{
	char *Buffer;
	qint64 Available, BytesRead;
	QLocalSocket *ClientSocket;

	/* Obtain a local socket for the incoming connection and wait for it to receive some data */

	if ((ClientSocket = m_poLocalServer->nextPendingConnection()) != NULL)
	{
		if (ClientSocket->waitForReadyRead())
		{
			/* Determine the amount of data waiting to be read and read it into a local buffer */

			if ((Available = ClientSocket->bytesAvailable()) > 0)
			{
				if ((Buffer = new char[Available]) != NULL)
				{
					if ((BytesRead = ClientSocket->read(Buffer, Available)) > 0)
					{
						/* If an observer is listening then notify it of the received data */

						if (m_poObserver)
						{
							m_poObserver->MessageReceived((unsigned char *) Buffer, Available);
						}
					}

					/* And free the temporary buffer */

					delete [] Buffer;
				}
			}
		}
		else
		{
			/* A connection was made, but no data was sent with it.  In this case, we still want to notify the */
			/* observer so that, for example, the client application listening on the socket can be brought */
			/* to the front */

			if (m_poObserver)
			{
				m_poObserver->MessageReceived(NULL, 0);
			}
		}

		delete ClientSocket;
	}
}

/**
 * Indicates whether the socket is a server or client socket.
 * Local sockets have two modes: server and client.  Whichever local socket of a given name is
 * created first is automatically the server and the second local socket to be created is then
 * the client.  The server will listen for connections from the client and after this, two way
 * communication is possible.  This function will indicate which of the two types the current
 * socket is.
 *
 * @date	Saturday 25-Apr-2015 11:30 am, Code HQ Ehinger Tor
 * @return	true if the socket is a server socket, else false if it a client socket
 */

bool RLocalSocket::IsServer()
{
	return(m_bIsServer);
}

/**
 * Opens a local socket in server or client mode.
 * This function will initialise an instance of a local socket in either server or client mode.
 * The socket that is created first will automatically be in server mode and the socket that is
 * created second will automatically be in client mode.  The socket must be given a unique name
 * and it is this name that is used for linking the server and client sockets together and for
 * determining which is the server and which is the client.
 *
 * @date	Friday 24-Apr-2015 07:28 am, Code HQ Ehinger Tor
 * @param	a_poApplication	Pointer to the parent application under which the program is running
 * @param	a_pccName		The name of the socket to be opened
 * @param	a_bServerMode	true to open as a listening server, else false to open as a client
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

int RLocalSocket::open(const char *a_pccName, RApplication *a_poApplication, bool a_bServerMode)
{
	int RetVal;

	/* Try to open the local socket.  This can fail if the program shut down without cleaning up, thus */
	/* leaving the local socket open.  In this case, an attempt to re-open it will fail, but during the */
	/* opening attempt the old local socket will be cleaned up, and trying a second time will work */

	RetVal = OpenSocket(a_pccName, a_poApplication, a_bServerMode);

	if (RetVal != KErrNone)
	{
		RetVal = OpenSocket(a_pccName, a_poApplication, a_bServerMode);
	}

	return(RetVal);
}

/**
 * Attempts to open a local socket.
 * Internal implementation of RLocalSocket::open().
 *
 * @date	Thursday 28-Nov-2019 6:43 am, Code HQ Bergmannstrasse
 * @param	a_poApplication	Pointer to the parent application under which the program is running
 * @param	a_pccName		The name of the socket to be opened
 * @param	a_bServerMode	true to open as a listening server, else false to open as a client
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

int RLocalSocket::OpenSocket(const char *a_pccName, RApplication *a_poApplication, bool a_bServerMode)
{
	TInt RetVal;

	/* Assume failure */

	RetVal = KErrGeneral;

	if (a_bServerMode)
	{
		/* Try to connect to the server.  If successful then an instance is already running, so return */
		/* without doing anything */

		m_oLocalSocket.connectToServer(a_pccName);

		if (m_oLocalSocket.waitForConnected(-1))
		{
			RetVal = KErrNone;
			m_oLocalSocket.abort();
		}

		/* Otherwise this is the first instance to start, so fire up a new server */

		else
		{
			/* Create an instance of a local server that can be used for listening for new connections */

			if ((m_poLocalServer = new QLocalServer(a_poApplication->Application())) != NULL)
			{
				/* If the connection failed then it is likely that the program using the local socket did */
				/* not shut down cleanly, so delete the local socket now so that a later attempt to */
				/* connect will work */

				QLocalServer::removeServer(a_pccName);

				/* Try to listen for a connection on the given socket.  If this succeeds then we are the */
				/* server.  If it fails then there is already a server listening and we are the client */

				if (m_poLocalServer->listen(a_pccName))
				{
					RetVal = KErrNone;
					m_bIsServer = true;

					connect(m_poLocalServer, SIGNAL(newConnection()), this, SLOT(Connected()));
				}
				else
				{
					/* Free the unneeded local server */

					delete m_poLocalServer;
					m_poLocalServer = NULL;
				}
			}
			else
			{
				RetVal = KErrNoMemory;
			}
		}
	}
	else
	{
		/* Connect to the server.  It is safe to use an infinite wait time as the server is local */
		/* and if the the connection fails, it will fail immediately */

		m_oLocalSocket.connectToServer(a_pccName);

		if (m_oLocalSocket.waitForConnected(-1))
		{
			RetVal = KErrNone;
		}
	}

	return(RetVal);
}

/**
 * Frees any resources associated with the class.
 * This method performs the same functions as the destructor, but allows the user to call it when
 * manual deinitialisation of the class is required.  After completion, the class instance can be
 * reused by calling RLocalSocket::open() again.
 *
 * @date	Thursday 02-Feb-2017 7:21 am, Code HQ Habersaathstraße
 */

void RLocalSocket::close()
{
	m_oLocalSocket.abort();

	m_bIsServer = false;
	delete m_poLocalServer;
	m_poLocalServer = NULL;
}

/**
 * Registers a function to be called when a message is received.
 * If a client program is interested in receiving a notification when a message is received
 * then it should call this function at startup, passing in a pointer to the function to be
 * called when the message is received.
 *
 * @date	Saturday 25-Apr-2015 10:53 am, Code HQ Ehinger Tor
 * @param	a_poObserver	Pointer to client function to be called when a message is received
 */

void RLocalSocket::SetObserver(MLocalSocketObserver *a_poObserver)
{
	m_poObserver = a_poObserver;
}

/**
 * Writes data to a connected socket.
 * This function will write the given data to the destination socket to which the socket is
 * currently connected.
 *
 * @date	Friday 24-Apr-2015 07:40 am, Code HQ Ehinger Tor
 * @param	a_pcucData		Pointer to the buffer to be written
 * @param	a_iDataSize		Number of bytes in the buffer to be written
 * @return	KErrNone if successful
 * @return	ErrWrite if the data could not be written
 */

TInt RLocalSocket::write(const unsigned char *a_pcucData, TInt a_iDataSize)
{
	TInt RetVal;

	/* Write the given data to the socket and flush it or it will stay in the write buffer */

	if (m_oLocalSocket.write((const char *) a_pcucData, a_iDataSize) == a_iDataSize)
	{
		RetVal = KErrNone;

		m_oLocalSocket.flush();
	}
	else
	{
		RetVal = KErrWrite;
	}

	return(RetVal);
}
