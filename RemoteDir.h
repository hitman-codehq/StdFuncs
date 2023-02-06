
#ifndef REMOTEDIR_H
#define REMOTEDIR_H

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
	RRemoteFactory	*m_remoteFactory;

public:

	int open(const char *a_pccPattern);

	void close() { }

	int read(TEntryArray *&a_rpoEntries, enum TDirSortOrder a_eSortOrder = EDirSortNone);

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEDIR_H */
