
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include <string.h>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 24-Feb-2023 6:49 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CFileInfo::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 24-Feb-2023 6:50 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CFileInfo::sendRequest()
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
 * @date	Friday 24-Feb-2023 6:47 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RRemoteFileInfo::open(const char *a_fileName)
{
	RSocket socket;
	int retVal = socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		socket.write(g_signature, SIGNATURE_SIZE);

		CFileInfo *handler = new CFileInfo(&socket, a_fileName);
		handler->sendRequest();

		if (handler->getResponse()->m_result == KErrNone)
		{
			/*const*/ SFileInfo* fileInfo = reinterpret_cast</*const*/ SFileInfo*>(handler->getResponsePayload());
			SWAP64(&fileInfo->m_microseconds);
			SWAP(&fileInfo->m_size);

			TDateTime dateTime(fileInfo->m_microseconds);

			m_fileInfo.Set(fileInfo->m_isDir, fileInfo->m_isLink, fileInfo->m_size, 0, dateTime);
		}
		else
		{
			Utils::Error("RemoteFileInfo: Received invalid response %d", handler->getResponse()->m_result);
		}

		delete handler;
		socket.close();
	}
	else
	{
		Utils::Error("Cannot connect to %s (%d)", m_remoteFactory->getServer().c_str(), retVal);
	}

	return retVal;
}
