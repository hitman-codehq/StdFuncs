
#include "StdFuncs.h"
#include "RemoteFactory.h"

#if defined(__unix__) || defined(__amigaos__)

#include <fcntl.h>
#include <sys/socket.h>

#else /* ! defined(__unix__) || defined(__amigaos__) */

#include <ws2tcpip.h>

#endif /* ! defined(__unix__) || defined(__amigaos__) */

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
			m_socket.write(g_signature, SIGNATURE_SIZE);
		}
		catch (RSocket::Error &a_exception)
		{
			Utils::info("RRemoteFactory::openRemote() => Unable to perform I/O on socket (Error = %d)", a_exception.m_result);
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

#ifdef WIN32

		u_long blocking = 1;

		if (ioctlsocket(m_socket.m_socket, FIONBIO, &blocking) == 0)
		{
			char buffer[1];
			result = recv(m_socket.m_socket, buffer, 1, 0);

			blocking = 0;

			if (ioctlsocket(m_socket.m_socket, FIONBIO, &blocking) != 0)
			{
				result = -1;
			}
		}

#else /* ! WIN32 */

		int flags = fcntl(m_socket.m_socket, F_GETFL, 0);

		if (flags != -1)
		{
			if (fcntl(m_socket.m_socket, F_SETFL, flags | O_NONBLOCK) != -1)
			{
				char buffer[1];
				result = recv(m_socket.m_socket, buffer, 1, 0);

				if (fcntl(m_socket.m_socket, F_SETFL, flags) == -1)
				{
					result = -1;
				}
			}
		}

#endif /* ! WIN32 */

		if (result == 0)
		{
			close();

			retVal = openRemote();
		}
	}
	else
	{
		retVal = openRemote();
	}

	return retVal;
}
