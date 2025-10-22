
#ifndef REMOTEFILEUTILS_H
#define REMOTEFILEUTILS_H

/** @file */

#include "FileUtils.h"
#include "StdSocket.h"

/* Forward declaration to reduce the # of includes required */
class RRemoteFactory;

/**
 * A class for performing miscellaneous remote file operations.
 * Instances of this class can be used to delete, rename and query files and directories on the remote file system.
 */

class RRemoteFileUtils : public RFileUtilsObject
{
	RRemoteFactory	*m_remoteFactory;	/**< Pointer to the remote factory that owns this instance */
	RSocket			*m_socket;			/**< Socket for communicating with remote RADRunner */

public:

	RRemoteFileUtils() : m_remoteFactory(nullptr),m_socket(nullptr) { }

	int deleteFile(const char *a_fileName);

	int getFileInfo(const char *a_fileName, TEntry *a_entry);

	int renameFile(const char *a_oldFullName, const char *a_newFullName);

	void setFactory(RRemoteFactory *a_remoteFactory, RSocket *a_socket)
	{
		m_remoteFactory = a_remoteFactory;
		m_socket = a_socket;
	}
};

#endif /* ! REMOTEFILEUTILS_H */
