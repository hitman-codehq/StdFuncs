
#ifndef FILEINFO_H
#define FILEINFO_H

class RFileInfoObject
{
protected:

	TEntry			m_fileInfo;		/**< Information about the remote file, in standard TEntry format */

public:

	virtual int open(const char *a_fileName) = 0;

	TEntry *getFileInfo() { return &m_fileInfo; }

	virtual void close() = 0;
};

class RFileInfo : public RFileInfoObject
{
public:

	int open(const char *a_fileName);

	void close() { };
};

#endif /* ! FILEINFO_H */
