
#ifndef REMOTEFACTORY_H
#define REMOTEFACTORY_H

/** @file */

#include "RemoteDir.h"
#include "RemoteFile.h"
#include "RemoteFileUtils.h"
#include "Yggdrasil/Commands.h"

/**
 * A class for management of file system related objects.
 * This helper class is useful for applications that need to have seamless access to both local and remote
 * file system objects.  Rather than using the file system related classes directly, applications should
 * declare an instance of this class, passing in the host name and port of the remote instance of RADRunner
 * to be used for remote access (if remote access is desired) or no host name (if local access is desired).
 * This enables the application to switch between remote and local file access dynamically at runtime.
 */

class RRemoteFactory
{
	bool				m_useLocal;			/**< True to ignore m_serverName, if set */
	RDir				m_dir;				/**< Class for local directory scanning */
	RRemoteDir			m_remoteDir;		/**< Class for remote directory scanning */
	RFile				m_file;				/**< Class for local file access */
	RRemoteFile			m_remoteFile;		/**< Class for remote file access */
	RFileUtils			m_fileUtils;		/**< Class for local file system manipulation */
	RRemoteFileUtils	m_remoteFileUtils;	/**< Class for remote file system manipulation */
	RSocket				m_socket;			/**< Socket for communicating with remote RADRunner */
	std::string			m_serverName;		/**< The host name of the instance of RADRunner to use */
	unsigned short		m_serverPort;		/**< The port on which RADRunner is listening */

public:

	RRemoteFactory() : m_useLocal(false) { }

	int openRemote();

	void close();

	int checkConnection();

	RDirObject &getDirObject();

	RFileObject &getFileObject();

	RFileUtilsObject &getFileUtilsObject();

	bool isRemote()
	{
		return m_serverName.length() != 0 && !m_useLocal;
	}

	std::string &getServer() { return m_serverName; }

	unsigned short &getPort() { return m_serverPort; }

	void setConfig(const std::string &a_serverName, unsigned short a_port)
	{
		m_serverName = a_serverName;
		m_serverPort = a_port;
	}

	/**< Set this to true when the RRemoteFactory instance is configured to be used for remote
	 * access, but there is a need to temporarily access the local file system
	 */
	void useLocal(bool a_useLocal)
	{
		m_useLocal = a_useLocal;
	}
};

#endif /* ! REMOTEFACTORY_H */
