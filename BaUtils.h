
#ifndef BAUTILS_H
#define BAUTILS_H

class RFileUtilsObject
{
public:

	virtual int deleteFile(const char *a_pccFileName) = 0;

	virtual int RenameFile(const char *a_pccOldFullName, const char *a_pccNewFullName) = 0;
};

class BaflUtils : public RFileUtilsObject
{
public:

	int deleteFile(const char *a_pccFileName);

	int RenameFile(const char *a_pccOldFullName, const char *a_pccNewFullName);
};

#endif /* ! BAUTILS_H */
