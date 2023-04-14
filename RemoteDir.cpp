
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include <memory>
#include <string.h>

/**
 * Sends a directory listing to the remote client.
 * This command is not required for the remote directory implementation, but must be present.
 *
 * @date	Sunday 22-Jan-2023 11:08 am, Code HQ Tokyo Tsukuda
 */

void CDir::execute()
{
}

/**
 * Requests a directory listing on the remote server.
 * Requests the remote server list the contents of the specified directory.  The name of the directory
 * to be listed is passed in as the payload.  If "" or "." is passed in, the contents of the current
 * directory will be listed.
 *
 * @date	Sunday 22-Jan-2023 11:07 am, Code HQ Tokyo Tsukuda
 */

void CDir::sendRequest()
{
	int32_t payloadSize = static_cast<int32_t>(strlen(m_directoryName) + 1);

	m_command.m_size = payloadSize;
	sendCommand();
	m_socket->write(m_directoryName, payloadSize);
	readResponse();
}

/**
 * Opens a remote object for scanning.
 * This function prepares to scan a file or directory.  The a_pattern parameter can refer to either a
 * directory name, a single filename, a wildcard pattern or a combination thereof.
 *
 * @date	Sunday 22-Jan-2023 10:41 am, Code HQ Tokyo Tsukuda
 * @param	a_pattern		OS specific path and wildcard to scan
 * @return	KErrNone if successful, otherwise one of the system errors
 */

int RRemoteDir::open(const char *a_pattern)
{
	(void) a_pattern;

	RSocket socket;
	int retVal = socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		socket.write(g_signature, SIGNATURE_SIZE);

		CDir *handler = new CDir(&socket, a_pattern);
		handler->sendRequest();

		if (handler->getResponse()->m_result == KErrNone)
		{
			const char *name;
			uint32_t size;
			TEntry *Entry;
			TTime Now;

			const unsigned char *payload = handler->getResponsePayload();
			const unsigned char *payloadEnd = payload + handler->getResponse()->m_size;

			Now.HomeTime();

			/* Iterate through the file information in the payload and extract its contents.  Provided the payload */
			/* is structured correctly, we could just check for it being ended by NULL terminator, but in the */
			/* interest of safety, we'll also check that we haven't overrun the end */
			while (payload < payloadEnd && *payload != '\0')
			{
				name = reinterpret_cast<const char *>(payload);
				payload += strlen(name) + 1;
				READ_INT(size, payload);
				payload += sizeof(size);

				if ((Entry = m_entries.Append(name)) != NULL)
				{
					Entry->Set(EFalse, EFalse, size, 0, Now.DateTime());
				}

				ASSERTM((payload < payloadEnd), "RRemoteDir::open => Payload contents do not match its size");
			}
		}
		else
		{
			Utils::Error("RemoteDir: Received invalid response %d", handler->getResponse()->m_result);

			retVal = handler->getResponse()->m_result;
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

/**
 * Scans a remote directory for file and directory entries.
 * Scans a directory that has been prepared with RRemoteDir::open() and populates a list with all of
 * the entries found.  This list is then returned to the calling client code.
 *
 * @date	Sunday 22-Jan-2023 10:42 am, Code HQ Tokyo Tsukuda
 * @param	a_entries		Reference to a ptr into which to place a ptr to the array of entries read
 *							by this function
 * @param	a_sortOrder		Enumeration specifying the order in which to sort the files.  EDirSortNone
 * 							is used by default
 * @return	KErrNone is always returned, as this method cannot fail
 */

int RRemoteDir::read(TEntryArray *&a_entries, enum TDirSortOrder a_sortOrder)
{
	(void) a_sortOrder;

	a_entries = &m_entries;

	return KErrNone;
}
