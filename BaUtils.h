
#ifndef BAUTILS_H
#define BAUTILS_H

class BaflUtils
{
public:

	static TInt DeleteFile(const char *a_pccFileName);

	static TInt RenameFile(const char *a_pccOldFullName, const char *a_pccNewFullName);
};

#endif /* ! BAUTILS_H */
