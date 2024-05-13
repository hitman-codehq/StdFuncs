
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
 * Short description.
 * Long multi line description.
 *
 * @pre		Server and port must have already been set
 *
 * @date	Saturday 11-May-2024 9:14 am, Enoshima holiday apartment
 * @param	Parameter		Description
 * @return	Return value
 */

int RRemoteFactory::openRemote()
{
	int retVal = m_socket.open(getServer().c_str(), getPort());

	if (retVal == KErrNone)
	{
		m_socket.write(g_signature, SIGNATURE_SIZE); // TODO: CAW - Exception handling
	}

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 11-May-2024 9:16 am, Enoshima holiday apartment
 * @param	Parameter		Description
 * @return	Return value
 */

void RRemoteFactory::close()
{
	m_socket.close();
}

/**
* Short description.
* Long multi line description.
*
* @pre		Some precondition here
*
* @date	Tuesday 14-May-2024 6:02 am, Code HQ Tokyo Tsukuda
* @param	Parameter		Description
* @return	Return value
*/

int RRemoteFactory::checkConnection()
{
	int retVal = KErrNone;

	if (m_socket.isOpen())
	{

// TODO: CAW - Move this stuff into RSocket
#ifdef WIN32

		u_long blocking = 1;

		// TODO: CAW - Returns 0 on success
		ioctlsocket(m_socket.m_socket, FIONBIO, &blocking);

		char buffer[1];
		int result = recv(m_socket.m_socket, buffer, 1, 0);

		blocking = 0;
		ioctlsocket(m_socket.m_socket, FIONBIO, &blocking);

#else /* ! WIN32 */

		// TODO: CAW - Returns -1 on failure
		int flags = fcntl(m_socket.m_socket, F_GETFL, 0);

		fcntl(m_socket.m_socket, F_SETFL, flags | O_NONBLOCK);

		char buffer[1];
		int result = recv(m_socket.m_socket, buffer, 1, 0);

		fcntl(m_socket.m_socket, F_SETFL, flags);

#endif /* ! WIN32 */

		if (result == 0)
		{
			printf("*** Socket closed!\n");
			close();

			retVal = openRemote();
			printf("*** openRemote() returned %d\n", retVal);
		}
	}
	else
	{
		retVal = openRemote();
	}

	return retVal;
}
