
#ifndef FILEUTILS_H
#define FILEUTILS_H

class RFileUtils
{
public:

	int deleteFile(const char *a_fileName);

	int getFileInfo(const char *a_fileName, TEntry *a_entry);

	int renameFile(const char *a_oldFullName, const char *a_newFullName);
};

#endif /* ! FILEUTILS_H */
