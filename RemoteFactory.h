
#ifndef REMOTEFACTORY_H
#define REMOTEFACTORY_H

#include "RemoteDir.h"
#include "RemoteFile.h"
#include "RemoteFileInfo.h"
#include "RemoteFileUtils.h"

class RRemoteFactory
{
	RDir				m_dir;
	RRemoteDir			m_remoteDir;
	RFile				m_file;
	RRemoteFile			m_remoteFile;
	RFileInfo			m_fileInfo;
	RRemoteFileInfo		m_remoteFileInfo;
	BaflUtils			m_fileUtils;
	RRemoteFileUtils	m_remoteFileUtils;
	std::string			m_serverName;
	int					m_serverPort;

public:

	RDirObject &getDirObject()
	{
		if (m_serverName.length() != 0)
		{
			m_remoteDir.setFactory(this);
			return m_remoteDir;
		}
		else
		{
			return m_dir;
		}
	}

	RFileObject &getFileObject()
	{
		if (m_serverName.length() != 0)
		{
			m_remoteFile.setFactory(this);
			return m_remoteFile;
		}
		else
		{
			return m_file;
		}
	}

	RFileInfoObject &getFileInfoObject()
	{
		if (m_serverName.length() != 0)
		{
			m_remoteFileInfo.setFactory(this);
			return m_remoteFileInfo;
		}
		else
		{
			return m_fileInfo;
		}
	}

	RFileUtilsObject &getFileUtilsObject()
	{
		if (m_serverName.length() != 0)
		{
			m_remoteFileUtils.setFactory(this);
			return m_remoteFileUtils;
		}
		else
		{
			return m_fileUtils;
		}
	}

	bool isRemote()
	{
		return m_serverName.length() != 0;
	}

	std::string &getServer() { return m_serverName; }

	int &getPort() { return m_serverPort; }

	void setConfig(const std::string &a_serverName, int a_port)
	{
		m_serverName = a_serverName;
		m_serverPort = a_port;
	}
};

#endif /* ! REMOTEFACTORY_H */
