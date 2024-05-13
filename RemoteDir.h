
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
	RSocket			*m_socket;			/**< Socket for communicating with remote RADRunner */

public:

	RRemoteDir() : m_remoteFactory(nullptr), m_socket(nullptr) { }

	int open(const char *a_pattern);

	int read(TEntryArray *&a_entries, enum TDirSortOrder a_sortOrder = EDirSortNone);

	void setFactory(RRemoteFactory *a_remoteFactory, RSocket *a_socket)
	{
		m_remoteFactory = a_remoteFactory;
		m_socket = a_socket;
	}
};

#endif /* ! REMOTEDIR_H */
