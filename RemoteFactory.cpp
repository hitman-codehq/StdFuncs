
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include <memory>

#if defined(__unix__) || defined(__amigaos__)

#include <fcntl.h>
#include <sys/socket.h>

#else /* ! defined(__unix__) || defined(__amigaos__) */

#include <ws2tcpip.h>

#endif /* ! defined(__unix__) || defined(__amigaos__) */

#ifdef __amigaos__

#include <proto/socket.h>
#include <sys/ioctl.h>

#define ioctlsocket(s, f, v) IoctlSocket(s, f, v)

#endif /* __amigaos__ */

/**
 * Gets an object for directory manipulation.
 * Returns an instance of a class derived from RDirObject which can be used to manipulate directories, either
 * locally or remotely, depending on how the factory was configured.
 *
 * @date	Saturday 25-May-2024 8:27 am, Code HQ Tokyo Tsukuda
 * @return	A reference to an RDirObject derived object
 */

RDirObject &RRemoteFactory::getDirObject()
{
	if (isRemote())
	{
		checkConnection();
		m_remoteDir.setFactory(this, &m_socket);
		return m_remoteDir;
	}
	else
	{
		return m_dir;
	}
}

/**
 * Gets an object for file manipulation.
 * Returns an instance of a class derived from RFileObject which can be used to manipulate files, either
 * locally or remotely, depending on how the factory was configured.
 *
 * @date	Saturday 25-May-2024 8:27 am, Code HQ Tokyo Tsukuda
 * @return	A reference to an RFileObject derived object
 */

RFileObject &RRemoteFactory::getFileObject()
{
	if (isRemote())
	{
		checkConnection();
		m_remoteFile.setFactory(this, &m_socket);
		return m_remoteFile;
	}
	else
	{
		return m_file;
	}
}

/**
 * Gets an object for file system manipulation.
 * Returns an instance of a class derived from RFileUtilsObject which can be used to manipulate file system
 * objects, either locally or remotely, depending on how the factory was configured.
 *
 * @date	Saturday 25-May-2024 8:27 am, Code HQ Tokyo Tsukuda
 * @return	A reference to an RFileUtilsObject derived object
 */

RFileUtilsObject &RRemoteFactory::getFileUtilsObject()
{
	if (isRemote())
	{
		checkConnection();
		m_remoteFileUtils.setFactory(this, &m_socket);
		return m_remoteFileUtils;
	}
	else
	{
		return m_fileUtils;
	}
}

/**
 * Gets an object for file system watching.
 * Returns an instance of a class derived from RFileWatcherObject which can be used to watch file system
 * objects, either locally or remotely, depending on how the factory was configured.
 *
 * @date	Thursday 23-Oct-2025 5:45 am, Code HQ Tokyo Tsukuda
 * @return	A reference to an RFileWatcherObject derived object
 */

RFileWatcherObject &RRemoteFactory::getFileWatcherObject()
{
	if (isRemote())
	{
		checkConnection();
		m_remoteWatcher.setFactory(this, &m_socket);
		return m_remoteWatcher;
	}
	else
	{
		return m_watcher;
	}
}

/**
 * Sends the server's protocol version.
 * This command is not required for the remote factory implementation, but must be present.
 *
 * @date	Saturday 08-Feb-2025 5:56 am, Code HQ Tokyo Tsukuda
 */

void CVersion::execute()
{
}

/**
 * Verifies the client's protocol version.
 * Requests the remote server's protocol version, and throws an exception if it is not supported.
 *
 * @date	Saturday 08-Feb-2025 5:53 am, Code HQ Tokyo Tsukuda
 */

TResult CVersion::sendRequest()
{
	sendCommand();

	uint32_t serverVersion, version = ((PROTOCOL_MAJOR << 16) | PROTOCOL_MINOR);
	m_socket->read(&serverVersion, sizeof(serverVersion));
	SWAP(&serverVersion);

	if (serverVersion != version)
	{
		throw std::runtime_error("Incompatible server version detected, remote editing functionality not available");
	}

	return TResult{};
}

/**
 * Open a connection to a remote RADRunner instance.
 * This function can be called any time a connection to RADRunner needs to be made. It can be called either
 * by client code at startup, or The Framework will do it automatically if a file request is made and it is
 * detected that the connection has been lost.
 *
 * @pre		Server and port must have already been set
 *
 * @date	Saturday 11-May-2024 9:14 am, Enoshima holiday apartment
 * @return	KErrNone if successful, otherwise one of the errors returned by RSocket::open()
 */

int RRemoteFactory::openRemote()
{
	int retVal = m_socket.open(getServer().c_str(), getPort());

	if (retVal == KErrNone)
	{
		try
		{
			/* Start by sending a signature, to identify us as a RADRunner client */
			m_socket.write(g_signature, SIGNATURE_SIZE);

			/* Handlers are stored in a shared_ptr so that they are freed if an exception */
			/* occurs, but we allocate them with new (rather than std::make_shared()) as we */
			/* need them to be zero initialised by MungWall */
			std::shared_ptr<CHandler> handler(new CVersion(&m_socket));

			/* Check whether the server's protocol version is supported.  The handler will display an error if it is not */
			handler->sendRequest();
			handler = nullptr;
		}
		catch (RSocket::Error &a_exception)
		{
			Utils::info("RRemoteFactory::openRemote() => Unable to perform I/O on socket (Error = %d)", a_exception.m_result);
			retVal = KErrNotOpen;
		}
		catch (std::runtime_error &a_exception)
		{
			Utils::info("RRemoteFactory::openRemote() => %s", a_exception.what());
			retVal = KErrInvalidVersion;
		}

		/* If an error occurred, close the socket to prevent the client from trying to use it. Otherwise, the protocol version */
		/* check would be easily bypassed */
		if (retVal != KErrNone)
		{
			m_socket.close();
		}
	}

	return retVal;
}

/**
 * Close the remote factory.
 * Closing the remote factory will also close any connected sockets.
 *
 * @date	Saturday 11-May-2024 9:16 am, Enoshima holiday apartment
 */

void RRemoteFactory::close()
{
	m_blocking = true;

	m_socket.close();
}

/**
* Check connection and reopen if necessary.
* This method is a part of the self-healing functionality of The Framework's socket system. It can be called at
* any time, whether the socket has been connected previously or not. If the socket is closed, it will be opened.
* If the socket is open, it will be checked to see if it is still connected. If it is not, it will be reconnected.
*
* @pre		Server and port must have already been set
*
* @date	Tuesday 14-May-2024 6:02 am, Code HQ Tokyo Tsukuda
* @return	KErrNone if successful, otherwise one of the errors returned by RRemoteFactory::openRemote()
*/

int RRemoteFactory::checkConnection()
{
	int retVal = KErrNone;

	/* If the socket is open, it could be that the connection has been lost, so we'll check that by setting */
	/* the socket to non-blocking and trying to read a byte. If it fails, it means the connection is closed. */
	/* This trick depends on the remote server not sending data without being requested to */
	if (m_socket.isOpen())
	{
		int result = 0;

		/* Error handling is difficult here, as it is unlikely to fail unless an invalid socket is used, and if */
		/* it does fail, there is not much we can do as this is supposed to be a transparent self-healing routine. */
		/* So we'll check for errors and handle but not report them, and if the final call to set the socket back */
		/* to blocking mode fails, all hell will break loose, but so be it */

		// TODO: CAW - This is working well, but should be moved into RSocket so that it can be used by code completion
		//             routines as well
		ULONG nonBlocking = 1;

		if (m_blocking)
		{
			DEBUGCHECK((ioctlsocket(m_socket.m_socket, FIONBIO, &nonBlocking) == 0),
				"RRemoteFactory::checkConnection() => Unable to set socket to non blocking mode");
		}

		char buffer[1];
		result = recv(m_socket.m_socket, buffer, 1, MSG_PEEK);

		if (m_blocking)
		{
			nonBlocking = 0;

			DEBUGCHECK((ioctlsocket(m_socket.m_socket, FIONBIO, &nonBlocking) == 0),
				"RRemoteFactory::checkConnection() => Unable to set socket to blocking mode");
		}

		/* If read() returned 0, the server has closed the connection or the connection has been lost somehow, so we */
		/* have to reopen it */
		if (result == 0)
		{
			close();

			retVal = openRemote();
		}

#ifdef __amigaos__

		/* If read() returned -1, there is no data available but the socket is alive. But just to be absolutely */
		/* sure, check that the error is that the socket would block and not some other error, which would indicate */
		/* a network error */
		else if (result == -1)
		{
			LONG error = Errno();

			if (error == EWOULDBLOCK || error == EAGAIN)
			{
				/* The connection is alive - there is just no data waiting! */

				retVal = KErrNone;
			}
			else
			{
				/* An actual network fault occurred */
				close();
				retVal = openRemote();
			}
		}

#endif /* __amigaos__ */

	}
	else
	{
		retVal = openRemote();
	}

	return retVal;
}
