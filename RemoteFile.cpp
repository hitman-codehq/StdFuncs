
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include <memory>
#include <string.h>

#define TRANSFER_SIZE 1024

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Sunday 19-Feb-2023 9:15 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CGet::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Sunday 19-Feb-2023 9:15 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CGet::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_fileName, payloadSize);
	readResponse();
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Feb-2023 7:57 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CSend::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Feb-2023 7:57 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CSend::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(sizeof(SFileInfo) + strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();

	SFileInfo *fileInfo = reinterpret_cast<SFileInfo *>(new unsigned char [payloadSize]);

	/* Initialise it with the file's name.  We won't include a timestamp in the packet, but will instead */
	/* let the server just leave the timestamp at the file's creation time.  It doesn't really make sense */
	/* to set a timestamp when saving a file from memory */
	strcpy(fileInfo->m_fileName, m_fileName);

	/* And finally send the payload and the file itself */
	m_socket->write(fileInfo, payloadSize);

	delete [] reinterpret_cast<unsigned char *>(fileInfo);
}

/**
 * Creates a new file for writing.
 * Creates a new file that can subsequently be used for writing operations.  If a file already
 * exists with the same name then the function will fail.  The a_pccFileName parameter can additionally
 * include a path to the file but the path must already exist.  It will not be created if it does
 * not already exist.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_pccFileName	Ptr to the name of the file to be created
 * @param	a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian
 *							API and is ignored (but should be EFileWrite for consistency); one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RRemoteFile::Create(const char *a_fileName, TUint a_fileMode)
{
	m_fileName = a_fileName;
	m_readOffset = 0;

	return KErrNone;
}

/**
 * Creates a new file for writing, deleting any that previously exists.
 * Creates a new file that can subsequently be used for writing operations.  If a file
 * already exists with the same name then the function will replace it.  This function is
 * a convenience wrapper around RRemoteFile::Create();  see that function for further details.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_pccFileName	Ptr to the name of the file to be created
 * @param	a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian
 *							API and is ignored (but should be EFileWrite for consistency); one of
 *							the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RRemoteFile::Replace(const char *a_fileName, TUint a_fileMode)
{
	/* The remote server will always delete the file before creating it, so we can just directly call Create() */
	return Create(a_fileName, a_fileMode);
}

/**
 * Opens an existing file for reading and/or writing.
 * Opens an existing file that can subsequently be used for reading or writing operations,
 * or both.  The file can be opened using the file mode flags EFileRead, EFileWrite, or a
 * logical combination of them both to elicit the desired behaviour.  If the file mode
 * EFileWrite is specified then the file will also be writeable.  The a_pccFileName parameter can
 * optionally specify a path to the file and can also be prefixed with an Amiga OS style "PROGDIR:"
 * prefix.
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_pccFileName	Ptr to the name of the file to be opened
 * @param	a_uiFileMode	Mode in which to open the file; one of the @link TFileMode @endlink values
 * @return	KErrNone if successful
 * @return	KErrInUse if the file is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RRemoteFile::open(const char *a_fileName, TUint a_fileMode)
{
	int retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		m_socket.write(g_signature, SIGNATURE_SIZE);

		CGet *handler = new CGet(&m_socket, a_fileName);
		handler->sendRequest();

		if (handler->getResponse()->m_result == KErrNone)
		{
			readFromRemote();
		}
		else
		{
			Utils::Error("RemoteFile: Received invalid response %d", handler->getResponse()->m_result);
		}

		delete handler;
		m_socket.close();
	}
	else
	{
		Utils::Error("Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}

/**
 * Reads a number of bytes from the file.
 * Reads a number of bytes from the file.  There must be sufficient data in the file to be
 * able to satisfy the read request, or the function will fail.  It is safe to try and write
 * 0 bytes.  In this case 0 will be returned.
 *
 * @pre		The file must be open
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_pucBuffer		Ptr to the buffer to read the data into
 * @param	a_iLength		Number of bytes in the buffer to be read
 * @return	Number of bytes read, if successful, otherwise KErrGeneral
 */

int RRemoteFile::read(unsigned char *a_buffer, int a_length)
{
	int retVal = KErrGeneral;

	ASSERTM(m_fileBuffer, "RRemoteFile::read() => File is not open");

	if (m_readOffset + a_length < m_fileBufferSize)
	{
		memcpy(a_buffer, m_fileBuffer + m_readOffset, a_length);
		m_readOffset += a_length;
		retVal = a_length;
	}
	else
	{
		memcpy(a_buffer, m_fileBuffer + m_readOffset, m_fileBufferSize - m_readOffset);
		retVal = m_fileBufferSize - m_readOffset;
		m_readOffset += m_fileBufferSize - m_readOffset;

		if (retVal == 0)
		{
			retVal = KErrEof;
		}
	}

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 05-Jan-2023 7:59 am, MK290 holiday apartment, Naha, Okinawa
 * @param	Parameter		Description
 * @return	Return value
 */

void RRemoteFile::readFromRemote()
{
	uint32_t bytesRead = 0, bytesToRead, fileSize, size;

	m_socket.read(&fileSize, sizeof(fileSize));
	SWAP(&fileSize);

	m_fileBuffer = new unsigned char[fileSize];

	do
	{
		bytesToRead = ((fileSize - bytesRead) >= TRANSFER_SIZE) ? TRANSFER_SIZE : (fileSize - bytesRead);
		size = m_socket.read(m_fileBuffer + bytesRead, bytesToRead);
		bytesRead += size;
	}
	while (bytesRead < fileSize);

	m_readOffset = 0;
	m_fileBufferSize = fileSize;
}

/**
 * Seeks to a specified position in the file.
 * This is a basic seek function that will seek from the beginning of a file to the position
 * passed in, that position being a number of bytes from the beginning of the file.
 *
 * @pre		The file must be open
 *
 * @date	Thursday 05-Jan-2023 9:12 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_iBytes		The number of bytes from the start of the file to which to seek
 * @return	KErrNone if successful
 * @return	KErrGeneral if the seek could not be performed
 */

int RRemoteFile::seek(int a_bytes)
{
	int retVal = KErrGeneral;

	ASSERTM(m_fileBuffer, "RRemoteFile::seek() => File is not open");

	if (a_bytes >= 0 && a_bytes < m_fileBufferSize)
	{
		retVal = KErrNone;
		m_readOffset = a_bytes;
	}

	return retVal;
}

/**
 * Writes a number of bytes to the file.
 * Writes a number of bytes to the file.  The file must have been opened in a writeable mode,
 * either by using RRemoteFile::open(EFileWrite), RRemoteFile::Replace() or RRemoteFile::Create().  In the
 * latter two cases, files are always opened as writeable, regardless of the file mode passed in.
 * It is safe to try and write 0 bytes.  In this case 0 will be returned
 *
 * @pre		The file must be open
 *
 * @date	Thursday 05-Jan-2023 6:50 am, MK290 holiday apartment, Naha, Okinawa
 * @param	a_pcucBuffer	Ptr to the buffer to be written to the file
 * @param	a_iLength		Number of bytes in the buffer to be written
 * @return	Number of bytes written, if successful, otherwise KErrGeneral
 */

int RRemoteFile::write(const unsigned char *a_buffer, int a_length)
{
	int retVal = KErrGeneral;
	size_t endOffset = m_readOffset + a_length;

	//ASSERTM((m_writeBuffer.size() > 0), "RRemoteFile::write() => File is not open");

	if (endOffset > m_writeBuffer.size())
	{
		m_writeBuffer.resize(endOffset);
	}

	if (endOffset <= m_writeBuffer.size())
	{
		memcpy(m_writeBuffer.data() + m_readOffset, a_buffer, a_length);
		m_readOffset += a_length; // TODO: CAW - Rename these
		retVal = a_length;
	}

	return retVal;
}

/**
 * Closes a file when access is no longer required.
 * Closes a file that has been created for reading & writing, or opened for reading.
 *
 * @date	Thursday 05-Jan-2023 6:51 am, MK290 holiday apartment, Naha, Okinawa
 */

void RRemoteFile::close()
{
	delete [] m_fileBuffer;
	m_fileBuffer = nullptr;

	if (m_writeBuffer.size() > 0)
	{
		int retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

		if (retVal == KErrNone)
		{
			m_socket.write(g_signature, SIGNATURE_SIZE);

			CSend *handler = new CSend(&m_socket, m_fileName.c_str());
			handler->sendRequest();

			int fileSize = static_cast<int>(m_writeBuffer.size());
			SWAP(&fileSize);

			m_socket.write(&fileSize, sizeof(fileSize));
			m_socket.write(m_writeBuffer.data(), m_writeBuffer.size());

			delete handler;
		}
		else
		{
			Utils::Error("Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
		}
	}

	m_socket.close();
}
