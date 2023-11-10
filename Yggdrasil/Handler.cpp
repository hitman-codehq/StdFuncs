
#include <StdFuncs.h>
#include <File.h>
#include <FileUtils.h>
#include <StdSocket.h>
#include "Commands.h"
#include <string.h>

#ifdef __unix__

#include <sys/stat.h>

#endif /* __unix__ */

/* Number of bytes to be transferred per call to read() or write() */
#define TRANSFER_SIZE 1024

/* String versions of the commands supported by RADRunner.  These can be indexed by the TCommand enumeration */
const char *g_commandNames[] =
{
	"version",
	"delete",
	"dir",
	"execute",
	"fileinfo",
	"get",
	"rename",
	"send",
	"shutdown"
};

/* Signature sent by the client when connecting, to identify it as a RADRunner client */
const char g_signature[] = "RADR";

/**
 * Obtains information about a given file or directory.
 * This is an internal method for obtaining directory listing information about a single file or
 * directory, to be used by the remote file access functionality.
 *
 * @date	Thursday 06-Apr-2023 6:30 am, Code HQ Tokyo Tsukuda
 * @param	a_fileName		Pointer to the name of the file for which to obtain information
 * @param	a_fileInfo		Reference to a pointer into which to allocate an instance of the
 *							@link SFileInfo @endlink structure that contains information about the file
 * KErrNone if successful, otherwise one of the errors returned by @link Utils::GetFileInfo() @endlink
 */

int CHandler::getFileInformation(const char *a_fileName, SFileInfo *&a_fileInfo)
{
	int retVal;
	TEntry entry;

	/* Determine if the file exists */
	retVal = Utils::GetFileInfo(a_fileName, &entry);

	/* If the file exists then send a response and a payload, containing the file's timestamp */
	if (retVal == KErrNone)
	{
		/* Include the size of just the filename in the payload size */
		int32_t payloadSize = static_cast<int32_t>(sizeof(SFileInfo) + strlen(entry.iName) + 1);

		/* Allocate an SFileInfo structure of a size large enough to hold the file's name */
		a_fileInfo = reinterpret_cast<SFileInfo*>(new unsigned char[payloadSize]);

		/* Initialise it with the file's name and timestamp */
		a_fileInfo->m_microseconds = entry.iModified.Int64();
		SWAP64(&a_fileInfo->m_microseconds);
		strcpy(a_fileInfo->m_fileName, entry.iName);

		/* And other fields of interest */
		a_fileInfo->m_isDir = entry.iIsDir;
		a_fileInfo->m_isLink = entry.iIsLink;

		a_fileInfo->m_size = entry.iSize;
		SWAP64(&a_fileInfo->m_size);
	}

	return retVal;
}

/**
 * Reads a file from a connected socket.
 * A convenience method used to read a file from a socket and to write its contents to a local file in
 * the file system.  This method assumes that the file has been requested and that upon reading the socket,
 * the entire file will be able to be read.
 *
 * @pre		A file has been requested from the connected endpoint
 *
 * @date	Sunday 17-Jan-2021 5:39 am, Code HQ Bergmannstrasse
 * @param	a_fileName		The name of the local file into which to write the file's contents
 * @return	KErrNone if successful, else one of the system errors
 */

int CHandler::readFile(const char *a_fileName)
{
	TInt64 fileSize;
	RFile file;

	m_socket->read(&fileSize, sizeof(fileSize));
	SWAP64(&fileSize);

	printf("%s: Transferring file \"%s\"\n", g_commandNames[m_command.m_command], a_fileName);

	/* The Framework doesn't truncate a file if it already exists, so delete any existing file before continuing */
	/* to avoid creating non working executables */
	RFileUtils FileUtils;
	FileUtils.deleteFile(a_fileName);

	/* Now create a new empty file into which to write */
	int retVal = file.create(a_fileName, EFileWrite);

	if (retVal == KErrNone)
	{
		int result;
		uint32_t bytesToRead, size;
		TInt64 bytesRead = 0;
		unsigned char *buffer = new unsigned char[TRANSFER_SIZE];

		/* Determine the start time so that it can be used to calculate the amount of time the transfer took */
		TTime now;
		now.HomeTime();
		TInt64 startTime = now.Int64();

		do
		{
			bytesToRead = ((fileSize - bytesRead) >= TRANSFER_SIZE) ? TRANSFER_SIZE : static_cast<uint32_t>(fileSize - bytesRead);
			size = m_socket->read(buffer, bytesToRead);

			/* Write to write the received information to the target file.  If this fails, we have to continue with */
			/* the transfer to prevent getting out of sync with the remote host.  Save the error for later reporting */
			if ((result = file.write(reinterpret_cast<unsigned char *>(buffer), size)) < KErrNone)
			{
				retVal = result;
			}

			bytesRead += size;
		}
		while (bytesRead < fileSize);

		/* Determine the end time and the number of milliseconds taken to perform the transfer */
		now.HomeTime();
		TInt64 endTime = now.Int64();
		TInt64 total = ((endTime - startTime) / 1000);

		if (retVal == KErrNone)
		{
			/* Cast the time results to integers when printing as Amiga OS doesn't support 64 bit format specifiers */
			printf("%s: Wrote %d.%d Kilobytes to file \"%s\" in %d.%d seconds\n", g_commandNames[m_command.m_command],
				static_cast<int>(bytesRead / 1024), static_cast<int>(bytesRead % 1024),
				a_fileName, static_cast<int>(total / 1000), static_cast<int>(total % 1000));
		}
		else
		{
			Utils::Error("Unable to write to file \"%s\" (Error %d)", a_fileName, retVal);
		}

		delete [] buffer;
		file.close();
	}
	else
	{
		Utils::Error("Unable to open file \"%s\" for writing (Error %d)", a_fileName, retVal);
	}

	return retVal;
}

/**
 * Reads the command's payload.
 * A convenience method that will allocate memory for the command's payload, and read the payload into that
 * memory.  If the command has no payload, no action will be performed but success will still be returned.
 *
 * @pre		A command has been received from the connected client
 * @pre		The socket is open
 *
 * @date	Sunday 10-Jan-2021 6:39 am, Code HQ Bergmannstrasse
 */

void CHandler::readPayload()
{
	/* If the command has a payload, allocate an appropriately sized buffer and read it */
	if (m_command.m_size > 0)
	{
		m_payload = new unsigned char[m_command.m_size];
		m_socket->read(m_payload, m_command.m_size);
	}
}

/**
 * Reads the response's payload.
 * A convenience method that will allocate memory for the response's payload, and read the payload into that
 * memory.  If the response has no payload, no action will be performed but success will still be returned.
 *
 * @pre		A command has been sent to the connected server
 * @pre		The socket is open
 *
 * @date	Saturday 27-Feb-2021 2:34 pm, Code HQ Bergmannstrasse
 */

void CHandler::readResponse()
{
	/* Read the response to the request */
	m_socket->read(&m_response, sizeof(m_response));

	SWAP(&m_response.m_result);
	SWAP(&m_response.m_size);

	/* If the response has a payload, allocate an appropriately sized buffer and read it */
	if (m_response.m_size > 0)
	{
		m_responsePayload = new unsigned char[m_response.m_size];
		m_socket->read(m_responsePayload, m_response.m_size);
	}
}

/**
 * Sends a command, without payload.
 * A convenience method to send a command, ensuring that its members are in network format before sending.
 *
 * @date	Sunday 29-Nov-2020 12:17 pm, Code HQ Bergmannstrasse
 */

TResult CHandler::sendCommand()
{
	SCommand command = m_command;

	SWAP(&command.m_command);
	SWAP(&command.m_size);

	m_socket->write(&command, sizeof(command));

	return TResult{};
}

/**
 * Writes a file to a connected socket.
 * A convenience method used to read a file and to write its contents to a connected socket.  This method
 * assumes that the remote endpoint is expecting the file.
 *
 * @pre		The connected endpoint has requested a file
 *
 * @date	Sunday 17-Jan-2021 5:50 am, Code HQ Bergmannstrasse
 * @param	a_fileName		The name of the local file from which to read the file's contents
 * @return	KErrNone if successful, else one of the system errors
 */

int CHandler::sendFile(const char *a_fileName)
{
	int retVal;
	TEntry entry;

	printf("%s: Transferring file \"%s\"\n", g_commandNames[m_command.m_command], a_fileName);

	if ((retVal = Utils::GetFileInfo(a_fileName, &entry)) == KErrNone)
	{
		TInt64 fileSize = entry.iSize;
		SWAP64(&fileSize);

		m_socket->write(&fileSize, sizeof(fileSize));

		RFile file;

		if ((retVal = file.open(a_fileName, EFileRead)) == KErrNone)
		{
			unsigned char *buffer = new unsigned char[TRANSFER_SIZE];
			size_t size;

			/* Determine the start time so that it can be used to calculate the amount of time the transfer took */
			TTime now;
			now.HomeTime();
			TInt64 startTime = now.Int64();

			while ((size = file.read(buffer, TRANSFER_SIZE)) > 0)
			{
				m_socket->write(buffer, static_cast<int>(size));
			}

			file.close();

			/* Determine the end time and the number of milliseconds taken to perform the transfer */
			now.HomeTime();
			TInt64 endTime = now.Int64();
			TInt64 total = ((endTime - startTime) / 1000);

			/* Cast 64 bit results to integers when printing as Amiga OS doesn't support 64 bit format specifiers */
			printf("%s: Transferred %d.%d Kilobytes in %d.%d seconds\n", g_commandNames[m_command.m_command],
				static_cast<int>(entry.iSize / 1024), static_cast<int>(entry.iSize % 1024),
				static_cast<int>(total / 1000), static_cast<int>(total % 1000));

			delete [] buffer;
		}
		else
		{
			Utils::Error("Unable to open file \"%s\" for reading (Error %d)", a_fileName, retVal);
		}
	}
	else
	{
		Utils::Error("Unable to query file \"%s\" (Error %d)", a_fileName, retVal);
	}

	return retVal;
}

/**
 * Sets the datestamp and protection bits for a file.
 * Using the information in the SFileInfo structure passed in, this method will update the datestamp and
 * protection bits on a file, whose name is also stored in said structure.
 *
 * @date	Saturday 27-Feb-2021 7:25 am, Code HQ Bergmannstrasse
 * @param	a_fileInfo		Structure containing information about the file
 */

void CHandler::setFileInformation(const SFileInfo &a_fileInfo)
{
	int result;

	/* It is legal to pass in a microsecond count of 0, in which case the time of the file will not be changed */
	if (a_fileInfo.m_microseconds != 0)
	{
		/* Create a TEntry instance and use the given microseconds value to initialise its timestamp */
		/* related members, so that it can be used to set the timestamp of the file just received */
		TEntry entry(TDateTime(a_fileInfo.m_microseconds));

		if ((result = Utils::setFileDate(a_fileInfo.m_fileName, entry)) != KErrNone)
		{
			Utils::Error("Unable to set datestamp on file \"%s\" (Error %d)", a_fileInfo.m_fileName, result);
		}
	}

#ifdef __amigaos__

	result = Utils::setProtection(a_fileInfo.m_fileName, 0);

#elif defined(__unix__)

	result = Utils::setProtection(a_fileInfo.m_fileName, (S_IXUSR | S_IXGRP | S_IXOTH | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR));

#else /* ! __unix__ */

	result = KErrNone;

#endif /* ! __unix__ */

	if (result != KErrNone)
	{
		Utils::Error("Unable to set protection bits for file \"%s\" (Error %d)", a_fileInfo.m_fileName, result);
	}
}
