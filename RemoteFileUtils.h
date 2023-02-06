
#ifndef REMOTEFILEUTILS_H
#define REMOTEFILEUTILS_H

#include "BaUtils.h"

class RRemoteFactory;

class RRemoteFileUtils : public RFileUtilsObject
{
	RRemoteFactory	*m_remoteFactory;

public:

	int deleteFile(const char *a_fileName);

	int RenameFile(const char *a_oldFullName, const char *a_newFullName);

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEFILEUTILS_H */
