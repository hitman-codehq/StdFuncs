
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include <memory>
#include <string.h>

#define TRANSFER_SIZE 1024

/**
 * Sends a file to the remote client.
 * This command is not required for the remote get implementation, but must be present.
 *
 * @date	Sunday 19-Feb-2023 9:15 am, Code HQ Tokyo Tsukuda
 */

void CGet::execute()
{
}

/**
 * Requests a file from the remote server.
 * This method does not perform the whole command, but just sends the request and puts the connection
 * in a state where it can be used for transferring the file.
 *
 * @date	Sunday 19-Feb-2023 9:15 am, Code HQ Tokyo Tsukuda
 */

TResult CGet::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_fileName, payloadSize);
	readResponse();

	return TResult{};
}

/**
 * Receives a file from the remote client.
 * This command is not required for the remote send implementation, but must be present.
 *
 * @date	Saturday 25-Feb-2023 7:57 am, Code HQ Tokyo Tsukuda
 */

void CSend::execute()
{
}

/**
 * Sends a file to the remote server.
 * This method does not perform the whole command, but just sends the request and puts the connection
 * in a state where it can be used for transferring the file.
 *
 * @date	Saturday 25-Feb-2023 7:57 am, Code HQ Tokyo Tsukuda
 */

TResult CSend::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(sizeof(SFileInfo) + strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();

	/* Allocate a SFileInfo structure of a size large enough to hold the file's name */
	SFileInfo *fileInfo = reinterpret_cast<SFileInfo *>(new unsigned char [payloadSize]);

	/* Initialise it with the file's name.  We won't include a timestamp in the packet, but will instead */
	/* let the server just leave the timestamp at the file's creation time.  It doesn't really make sense */
	/* to set a timestamp when saving a file from memory */
	strcpy(fileInfo->m_fileName, m_fileName);

	/* And send the payload */
	m_socket->write(fileInfo, payloadSize);

	delete [] reinterpret_cast<unsigned char *>(fileInfo);

	return TResult{};
}

/**
 * Creates a new remote file for writing.
 * This method creates a memory buffer that allows a client to write to the file, as though the file
 * exists locally.  Operations such as read(), write() and seek() can be performed, as though they were
 * being performed on a local file, but they affect only the contents of the memory buffer.
 *
 * No remote file is actually created at this point.  The file will be created and its contents written
 * to when @link RFile::close() @endlink is called.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_fileName		Pointer to the name of the file to be created
 * @param	a_fileMode		Mode in which to create the file; one of the @link TFileMode @endlink values
 * @return	KErrNone is always returned, as this method cannot fail
 */

int RRemoteFile::create(const char *a_fileName, TUint a_fileMode)
{
	(void) a_fileMode;

	m_fileName = a_fileName;
	m_fileOffset = 0;

	return KErrNone;
}

/**
 * Creates a new remote file for writing, deleting any that previously exists.
 * Creates a new file that can subsequently be used for writing operations.  If a file
 * already exists with the same name then the function will replace it.  This function is
 * a convenience wrapper around RRemoteFile::create();  see that function for further details.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_fileName		Pointer to the name of the file to be created
 * @param	a_fileMode		Mode in which to create the file; one of the @link TFileMode @endlink values
 * @return	KErrNone is always returned, as this method cannot fail
 */

int RRemoteFile::replace(const char *a_fileName, TUint a_fileMode)
{
	/* The remote server will always delete the file before creating it, so we can just directly call create() */
	return create(a_fileName, a_fileMode);
}

/**
 * Opens a remote existing file for reading and/or writing.
 * Opens an existing file that can subsequently be used for reading or writing operations, or both.
 * The file can be opened using the file mode flags EFileRead, EFileWrite, or a logical combination
 * of them both to elicit the desired behaviour.  If the file mode EFileWrite is specified then the
 * file will also be writeable.  The a_fileName parameter can optionally include a path to the file.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_fileName		Pointer to the name of the file to be opened
 * @param	a_fileMode		Mode in which to open the file; one of the @link TFileMode @endlink values
 * @return	KErrNone if successful, otherwise one of the system errors
 */

int RRemoteFile::open(const char *a_fileName, TUint a_fileMode)
{
	(void) a_fileMode;

	int retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		CGet *handler = nullptr;

		try
		{
			m_socket.write(g_signature, SIGNATURE_SIZE);

			handler = new CGet(&m_socket, a_fileName);
			handler->sendRequest();

			if (handler->getResponse()->m_result == KErrNone)
			{
				readFromRemote();
			}
			else
			{
				Utils::info("RRemoteFile::open() => Received invalid response %d", handler->getResponse()->m_result);

				retVal = handler->getResponse()->m_result;
			}
		}
		catch (RSocket::Error &a_exception)
		{
			Utils::info("RRemoteFile::open() => Unable to perform I/O on socket (Error = %d)", a_exception.m_result);

			retVal = KErrNotFound;
		}

		delete handler;
		m_socket.close();
	}
	else
	{
		Utils::info("RRemoteFile::open() => Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}

/**
 * Reads a number of bytes from the file.
 * Reads a number of bytes from the file.  There must be sufficient data in the file to be
 * able to satisfy the read request, or the function will fail.  It is safe to try and read
 * 0 bytes.  In this case 0 will be returned.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_buffer		Pointer to the buffer to read the data into
 * @param	a_length		Number of bytes in the buffer to be read
 * @return	Number of bytes read, if successful
 * @return	KErrGeneral if the read could not be performed
 */

int RRemoteFile::read(unsigned char *a_buffer, int a_length)
{
	int retVal = KErrGeneral;

	if (m_fileOffset + a_length < static_cast<int>(m_fileBuffer.size()))
	{
		memcpy(a_buffer, m_fileBuffer.data() + m_fileOffset, a_length);
		m_fileOffset += a_length;
		retVal = a_length;
	}
	else
	{
		memcpy(a_buffer, m_fileBuffer.data() + m_fileOffset, m_fileBuffer.size() - m_fileOffset);
		retVal = static_cast<int>(m_fileBuffer.size() - m_fileOffset);
		m_fileOffset += static_cast<int>(m_fileBuffer.size() - m_fileOffset);

		if (retVal == 0)
		{
			retVal = KErrEof;
		}
	}

	return retVal;
}

/**
 * Reads a file from a connected socket.
 * Read a file from a socket and writes its contents to a buffer in memory.  This method assumes that the
 * file has been requested and that upon reading the socket, the entire file will be able to be read.
 *
 * @pre		A file has been requested from the connected endpoint
 *
 * @date	Thursday 05-Jan-2023 7:59 am, MK290 holiday apartment, Naha, Okinawa
 */

void RRemoteFile::readFromRemote()
{
	uint32_t bytesRead = 0, bytesToRead, fileSize, size;

	m_socket.read(&fileSize, sizeof(fileSize));
	SWAP(&fileSize);

	m_fileBuffer.resize(fileSize);

	do
	{
		bytesToRead = ((fileSize - bytesRead) >= TRANSFER_SIZE) ? TRANSFER_SIZE : (fileSize - bytesRead);
		size = m_socket.read(m_fileBuffer.data() + bytesRead, bytesToRead);
		bytesRead += size;
	}
	while (bytesRead < fileSize);

	m_fileOffset = 0;
}

/**
 * Seeks to a specified position in the file.
 * This is a basic seek function that will seek from the beginning of a file to the position
 * passed in, that position being a number of bytes from the beginning of the file.
 *
 * @date	Thursday 05-Jan-2023 9:12 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_bytes			The number of bytes from the start of the file to which to seek
 * @return	KErrNone if successful
 * @return	KErrGeneral if the seek could not be performed
 */

int RRemoteFile::seek(int a_bytes)
{
	int retVal = KErrGeneral;

	if (a_bytes >= 0 && a_bytes < static_cast<int>(m_fileBuffer.size()))
	{
		retVal = KErrNone;
		m_fileOffset = a_bytes;
	}

	return retVal;
}

/**
 * Writes a number of bytes to the file.
 * Writes a number of bytes to the file.  The file must have been opened in a writeable mode,
 * either by using RRemoteFile::open(EFileWrite), RRemoteFile::replace() or RRemoteFile::create().
 * In the latter two cases, files are always opened as writeable, regardless of the file mode
 * passed in.  It is safe to try and write 0 bytes.  In this case 0 will be returned
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_buffer		Pointer to the buffer to be written to the file
 * @param	a_length		Number of bytes in the buffer to be written
 * @return	Number of bytes written, if successful
 * @return	KErrGeneral if the write could not be performed
 */

int RRemoteFile::write(const unsigned char *a_buffer, int a_length)
{
	int retVal = KErrGeneral;
	size_t endOffset = m_fileOffset + a_length;

	if (endOffset > m_fileBuffer.size())
	{
		m_fileBuffer.resize(endOffset);
	}

	if (endOffset <= m_fileBuffer.size())
	{
		m_dirty = true;
		memcpy(m_fileBuffer.data() + m_fileOffset, a_buffer, a_length);
		m_fileOffset += a_length;
		retVal = a_length;
	}

	return retVal;
}

/**
 * Closes a file when access is no longer required.
 * Closes a file that has been created for reading & writing, or opened for reading.  If changes have
 * been made to the contents of the memory buffer that represent the remote file, those changes will
 * be sent to the server for writing to the remote file.
 *
 * @date	Thursday 05-Jan-2023 6:51 am, MK290 holiday apartment, Naha, Okinawa
 * @return	KErrNone if successful, otherwise one of the errors from RSocket::open()
 */

int RRemoteFile::close()
{
	int retVal = KErrNone;

	if (m_dirty && m_fileBuffer.size() > 0)
	{
		retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

		if (retVal == KErrNone)
		{
			CSend *handler = nullptr;

			try
			{
				m_socket.write(g_signature, SIGNATURE_SIZE);

				handler = new CSend(&m_socket, m_fileName.c_str());
				handler->sendRequest();

				int fileSize = static_cast<int>(m_fileBuffer.size());
				SWAP(&fileSize);

				m_socket.write(&fileSize, sizeof(fileSize));
				m_socket.write(m_fileBuffer.data(), static_cast<int>(m_fileBuffer.size()));

				delete handler;
				m_dirty = false;
			}
			catch (RSocket::Error &a_exception)
			{
				Utils::info("RRemoteFile::close() => Unable to perform I/O on socket (Error = %d)", a_exception.m_result);

				retVal = KErrNotFound;
			}

			delete handler;
			m_socket.close();
		}
		else
		{
			Utils::info("RRemoteFile::close() => Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
		}
	}

	return retVal;
}
