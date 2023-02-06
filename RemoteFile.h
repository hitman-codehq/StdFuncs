
#ifndef REMOTEFILE_H
#define REMOTEFILE_H

#include "File.h"
#include "StdSocket.h"
#include <string>
#include <vector>

class CGet;
class CSend;
class RRemoteFactory;

/**
 * A class for reading from or writing to remote files.
 * This class enables the remote creation, reading and writing of file in a platform independent manner.
 */

class RRemoteFile : public RFileObject
{
	int				m_readOffset;
	int				m_fileBufferSize;
	unsigned char	*m_fileBuffer;
	RRemoteFactory	*m_remoteFactory;
	RSocket			m_socket;		/* Socket for communicating with remote RADRunner */
	std::string		m_fileName;
	std::vector<unsigned char>	m_writeBuffer;

	void readFromRemote();

public:

	RRemoteFile() : m_readOffset(0), m_fileBufferSize(0), m_fileBuffer(nullptr) { }

	int Create(const char *a_fileName, TUint a_fileMode);

	int Replace(const char *a_fileName, TUint a_fileMode);

	int open(const char *a_fileName, TUint a_fileMode);

	int read(unsigned char *a_buffer, int a_length);

	int seek(int a_bytes);

	int write(const unsigned char *a_buffer, int a_length);

	void close();

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEFILE_H */
