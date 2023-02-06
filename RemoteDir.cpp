
#include "StdFuncs.h"
#include "RemoteFactory.h"
#include "Yggdrasil/Commands.h"
#include <memory>
#include <string.h>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Sunday 22-Jan-2023 11:08 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CDir::execute()
{
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Sunday 22-Jan-2023 11:07 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
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
 * Opens an object for scanning.
 * This function prepares to scan a file or directory.  The a_pccPattern parameter can
 * refer to either a directory name, a single filename, a wildcard pattern or a combination
 * thereof.  Examples are:
 *
 * ""\n
 * "."\n
 * "SomeDir"\n
 * "SomeDir/"\n
 * "*"\n
 * "*.cpp"\n
 * "SomeFile"\n
 * "SomeDir/SomeFile.txt"\n
 * "PROGDIR:"\n
 *
 * @date	Sunday 22-Jan-2023 10:41 am, Code HQ Tokyo Tsukuda
 * @param	a_pccPattern	OS specific path and wildcard to scan
 * @return	KErrNone if directory was opened successfully
 * @return	KErrNotFound if the directory or file could not be opened for scanning
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

int RRemoteDir::open(const char *a_pccPattern)
{
	RSocket socket;
	int retVal = socket.open(m_remoteFactory->getServer().c_str(), m_remoteFactory->getPort());

	if (retVal == KErrNone)
	{
		socket.write(g_signature, SIGNATURE_SIZE);

		CDir *handler = new CDir(&socket, ".");
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
				STREAM_INT(size, payload);
				payload += sizeof(size);

				if ((Entry = iEntries.Append(name)) != NULL)
				{
					Entry->Set(EFalse, EFalse, size, 0, Now.DateTime());
					//Entry->iPlatformDate = iSingleEntry.iPlatformDate;
				}

				ASSERTM((payload < payloadEnd), "RRemoteDir::open => Payload contents do not match its size");
			}
		}
		else
		{
			Utils::Error("RemoteDir: Received invalid response %d", handler->getResponse()->m_result);
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
 * Scans a directory for file and directory entries.
 * Scans a directory that has been prepared with RRemoteDir::open() and populates a list
 * with all of the entries found.  This list is then returned to the calling client
 * code.
 *
 * @date	Sunday 22-Jan-2023 10:42 am, Code HQ Tokyo Tsukuda
 * @param	a_rpoEntries	Reference to a ptr into which to place a ptr to the
 *							array of entries read by this function
 * @param	a_eSortOrder	Enumeration specifying the order in which to sort the files.
 *							EDirSortNone is used by default
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

int RRemoteDir::read(TEntryArray *&a_rpoEntries, enum TDirSortOrder a_eSortOrder)
{
	a_rpoEntries = &iEntries;

	return KErrNone;
}
