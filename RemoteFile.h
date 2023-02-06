
#ifndef REMOTEFILE_H
#define REMOTEFILE_H

/** @file */

#include "File.h"
#include "StdSocket.h"
#include <string>
#include <vector>

class CGet;
class CSend;
class RRemoteFactory;

/**
 * A class for reading from or writing to remote files.
 * This class enables the remote creation, reading and writing of files in a platform independent manner.
 */

class RRemoteFile : public RFileObject
{
	bool						m_dirty;			/**< True if a write has been performed */
	int							m_fileOffset;		/**< The current read/write offset into the file */
	RRemoteFactory				*m_remoteFactory;	/**< Pointer to the remote factory that owns this instance */
	RSocket						m_socket;			/**< Socket for communicating with remote RADRunner */
	std::string					m_fileName;			/**< The name of the file on the remote server */
	std::vector<unsigned char>	m_fileBuffer;		/**< Buffer containing the contents of the file */

	void readFromRemote();

public:

	RRemoteFile() : m_dirty(false), m_fileOffset(0), m_remoteFactory(nullptr) { }

	int create(const char *a_fileName, TUint a_fileMode);

	int replace(const char *a_fileName, TUint a_fileMode);

	int open(const char *a_fileName, TUint a_fileMode);

	int read(unsigned char *a_buffer, int a_length);

	int seek(int a_bytes);

	int write(const unsigned char *a_buffer, int a_length);

	void close();

	void setFactory(RRemoteFactory *a_remoteFactory) { m_remoteFactory = a_remoteFactory; }
};

#endif /* ! REMOTEFILE_H */
