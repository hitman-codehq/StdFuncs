
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include "StdSocket.h"
#include <string.h>

/**
 * Deletes a file.
 * This command is not required for the remote delete implementation, but must be present.
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 */

void CDelete::execute()
{
}

/**
 * Requests the deletion of a file.
 * Requests the remote server to delete the specified file.
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 */

TResult CDelete::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_fileName, payloadSize);
	readResponse();

	return TResult{};
}

/**
 * Determines detailed file information and returns it to the client.
 * This command is not required for the remote delete implementation, but must be present.
 *
 * @date	Friday 24-Feb-2023 6:49 am, Code HQ Tokyo Tsukuda
 */

void CFileInfo::execute()
{
}

/**
 * Requests detailed file information from the server.
 * Requests the remote server to query information about the given file, and to return it in the
 * payload in a @link SFileInfo @endlink structure.
 *
 * @date	Friday 24-Feb-2023 6:50 am, Code HQ Tokyo Tsukuda
 */

TResult CFileInfo::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_fileName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_fileName, payloadSize);
	readResponse();

	return TResult{};
}

/**
 * Renames a file.
 * This command is not required for the remote rename implementation, but must be present.
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 */

void CRename::execute()
{
}

/**
 * Requests the renaming of a file.
 * Requests the remote server to rename the specified file to the specified new name.
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 */

TResult CRename::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_oldName) + 1 + strlen(m_newName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_oldName, static_cast<int>(strlen(m_oldName) + 1));
	m_socket->write(m_newName, static_cast<int>(strlen(m_newName) + 1));
	readResponse();

	return TResult{};
}

/**
 * Deletes a file from the file system.
 * Requests the remote server to delete the specified file.
 *
 * @date	Wednesday 01-Jul-2009 7:54 pm
 * @param	a_fileName	Name of the file to be deleted
 * @return	KErrNone if successful, otherwise one of the system errors
 */

int RRemoteFileUtils::deleteFile(const char *a_fileName)
{
	RSocket m_socket;

	int retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		m_socket.write(g_signature, SIGNATURE_SIZE);

		CDelete *handler = new CDelete(&m_socket, a_fileName);
		handler->sendRequest();

		if (handler->getResponse()->m_result != KErrNone)
		{
			Utils::info("RRemoteFileUtils::deleteFile() => Received invalid response %d", handler->getResponse()->m_result);

			retVal = handler->getResponse()->m_result;
		}

		delete handler;
		m_socket.close();
	}
	else
	{
		Utils::info("RRemoteFileUtils::deleteFile() => Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}

/**
 * Obtains information about a given remote file or directory.
 * Wrapper method that enables remote operation of the @link Utils::GetFileInfo() @endlink method.
 * See @link Utils::GetFileInfo() @endlink for more detailed information.
 *
 * @pre		Pointer to filename passed in must not be NULL
 * @pre		Pointer to TEntry structure passed in must not be NULL
 *
 * @date	Friday 24-Feb-2023 6:47 am, Code HQ Tokyo Tsukuda
 * @param	a_fileName	Pointer to the name of the file for which to obtain information
 * @param	a_entry		Pointer to the TEntry structure into which to place the information
 * @return	KErrNone if successful, otherwise one of the system errors
 */

int RRemoteFileUtils::getFileInfo(const char *a_fileName, TEntry *a_entry)
{
	ASSERTM((a_entry != nullptr), "RRemoteFileUtils::getFileInfo() => Pointer to filename passed in must not be NULL");
	ASSERTM((a_entry != nullptr), "RRemoteFileUtils::getFileInfo() => TEntry structure passed in must not be NULL");

	RSocket socket;
	int retVal = socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		socket.write(g_signature, SIGNATURE_SIZE);

		CFileInfo* handler = new CFileInfo(&socket, a_fileName);
		handler->sendRequest();

		if (handler->getResponse()->m_result == KErrNone)
		{
			SFileInfo *fileInfo = reinterpret_cast<SFileInfo *>(handler->getResponsePayload());
			SWAP64(&fileInfo->m_microseconds);
			SWAP64(&fileInfo->m_size);

			TDateTime dateTime(fileInfo->m_microseconds);

			a_entry->Set(fileInfo->m_isDir, fileInfo->m_isLink, fileInfo->m_size, 0, dateTime);
		}
		else
		{
			Utils::info("RRemoteFileUtils::getFileInfo() => Received invalid response %d", handler->getResponse()->m_result);

			retVal = handler->getResponse()->m_result;
		}

		delete handler;
		socket.close();
	}
	else
	{
		Utils::info("RRemoteFileUtils::getFileInfo() => Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}

/**
  * Renames a file.
  * Requests the remote server to rename the specified file to the specified new name.
  *
  * @date	Wednesday 08-Mar-2023 7:02 am, Code HQ Tokyo Tsukuda
  * @param	a_oldFullName	The current name of the file to be renamed
  * @param	a_newFullName	The new name to which to rename the file
  * @return	KErrNone if successful, else KErrGeneral
  */

int RRemoteFileUtils::renameFile(const char *a_oldFullName, const char *a_newFullName)
{
	RSocket socket;

	int retVal = socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		socket.write(g_signature, SIGNATURE_SIZE);

		CRename *handler = new CRename(&socket, a_oldFullName, a_newFullName);
		handler->sendRequest();

		if (handler->getResponse()->m_result != KErrNone)
		{
			Utils::info("RRemoteFileUtils::renameFile() => Received invalid response %d", handler->getResponse()->m_result);

			retVal = handler->getResponse()->m_result;
		}

		delete handler;
		socket.close();
	}
	else
	{
		Utils::info("RRemoteFileUtils::renameFile() => Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}
