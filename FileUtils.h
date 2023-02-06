
#ifndef FILEUTILS_H
#define FILEUTILS_H

/** @file */

/**
 * Interface for all file object manipulation classes.
 * This pure virtual base class defines the interface that all file object manipulation classes will adhere to.
 * Instances of these classes can either be used directly, or obtained from RRemoteFactory::getFileUtilsObject().
 */

class RFileUtilsObject
{
public:

	virtual int deleteFile(const char *a_fileName) = 0;

	virtual int getFileInfo(const char *a_fileName, TEntry *a_entry) = 0;

	virtual int renameFile(const char *a_oldFullName, const char *a_newFullName) = 0;
};

/**
 * A class for performing miscellaneous local file operations.
 * Instances of this class can be used to delete, rename and query files and directories on the local file system.
 */

class RFileUtils : public RFileUtilsObject
{
public:

	int deleteFile(const char *a_fileName);

	int getFileInfo(const char *a_fileName, TEntry *a_entry);

	int renameFile(const char *a_oldFullName, const char *a_newFullName);
};

#endif /* ! FILEUTILS_H */
