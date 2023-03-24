
#ifndef FILEUTILS_H
#define FILEUTILS_H

class RFileUtils
{
public:

	int deleteFile(const char *a_pccFileName);

	int renameFile(const char *a_pccOldFullName, const char *a_pccNewFullName);
};

#endif /* ! FILEUTILS_H */
