
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include "StdSocket.h"
#include <string.h>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CDelete::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CDelete::sendRequest()
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
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CRename::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 11-Mar-2023 5:53 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CRename::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_oldName) + 1 + strlen(m_newName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_oldName, strlen(m_oldName) + 1); // TODO: CAW
	m_socket->write(m_newName, strlen(m_newName) + 1);
	readResponse();
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Wednesday 08-Mar-2023 7:02 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
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

		if (handler->getResponse()->m_result == KErrNone)
		{
			// readFromRemote(); // TODO: CAW
		}
		else
		{
			// TODO: CAW - These should not use Utils::Error()
			Utils::Error("deleteFile: Received invalid response %d", handler->getResponse()->m_result);
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
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Wednesday 08-Mar-2023 7:02 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RRemoteFileUtils::RenameFile(const char *a_oldFullName, const char *a_newFullName)
{
	RSocket m_socket;

	int retVal = m_socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		m_socket.write(g_signature, SIGNATURE_SIZE);

		CRename *handler = new CRename(&m_socket, a_newFullName, a_oldFullName); // TODO: CAW - Reversed parameters
		handler->sendRequest();

		if (handler->getResponse()->m_result == KErrNone)
		{
			// readFromRemote(); // TODO: CAW
		}
		else
		{
			Utils::Error("RenameFile: Received invalid response %d", handler->getResponse()->m_result);
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
