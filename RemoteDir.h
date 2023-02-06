
#ifndef REMOTEDIR_H
#define REMOTEDIR_H

/** @file */

#include "Dir.h"
#include "StdSocket.h"

class CDir;
class RRemoteFactory;

/**
 * A class for scanning directories for remote directory and file entries.
 * Instances of this class can be used to scan for file information on the remote file system.
 */

class RRemoteDir : public RDirObject
{
	RRemoteFactory	*m_remoteFactory;	/**< Pointer to the remote factory that owns this instance */

public:

	RRemoteDir() : m_remoteFactory(nullptr) { }

	int open(const char *a_pattern);

	void close() { }

	int read(TEntryArray *&a_entries, enum TDirSortOrder a_sortOrder = EDirSortNone);

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEDIR_H */
