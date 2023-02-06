
#ifndef REMOTEFILEINFO_H
#define REMOTEFILEINFO_H

#include "FileInfo.h"
#include "StdSocket.h"

class CFileInfo;
class RRemoteFactory;

class RRemoteFileInfo : public RFileInfoObject
{
	RRemoteFactory	*m_remoteFactory;

public:

	int open(const char *a_fileName);

	void close() { };

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEFILEINFO_H */
