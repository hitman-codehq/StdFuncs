
#ifndef REMOTEFILEUTILS_H
#define REMOTEFILEUTILS_H

/** @file */

#include "FileUtils.h"

class RRemoteFactory;

/**
 * A class for performing miscellaneous remote file operations.
 * Instances of this class can be used to delete, rename and query files and directories on the remote file system.
 */

class RRemoteFileUtils : public RFileUtilsObject
{
	RRemoteFactory	*m_remoteFactory;	/**< Pointer to the remote factory that owns this instance */

public:

	RRemoteFileUtils() : m_remoteFactory(nullptr) { }

	int deleteFile(const char *a_fileName);

	int getFileInfo(const char *a_fileName, TEntry *a_entry);

	int renameFile(const char *a_oldFullName, const char *a_newFullName);

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEFILEUTILS_H */
