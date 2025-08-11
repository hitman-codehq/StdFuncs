
#include "../StdFuncs.h"
#include "../StdApplication.h"
#include "../StdFileWatcher.h"
#include "../StdWindow.h"
#include <string.h>

void RAmiFileWatcher::changed()
{
	int result;
	RDir newDir;
	TEntry *newEntry;

	/* Scan the requested directory and build a list of files that it contains */

	if ((result = newDir.open(reinterpret_cast<char *>(m_request->nr_Name))) == KErrNone)
	{
		if ((result = newDir.read(EDirSortNameAscending)) == KErrNone)
		{
			// Detect files that were added. If the file is in the new list but not in the old list, it is new
			for (const TEntry *newFile = newDir.getEntries()->getHead(); newFile; newFile = newDir.getEntries()->getSucc(newFile))
			{
				if (!findEntry(*m_dir.getEntries(), newFile->iName))
				{
					m_parentWatcher.changed(reinterpret_cast<char *>(m_request->nr_Name), newFile->iName, EChangeTypeAdded);

					if ((newEntry = m_dir.getEntries()->Append(newFile->iName)) != nullptr)
					{
						newEntry->Set(*newFile);
					}

					break;
				}
			}

			// Detect files that were deleted. If the file is in the old list but not in the new list, it has been deleted
			for (const TEntry *oldFile = m_dir.getEntries()->getHead(); oldFile; oldFile = m_dir.getEntries()->getSucc(oldFile))
			{
				const TEntry *newFile = findEntry(*newDir.getEntries(), oldFile->iName);

				if (!newFile)
				{
					m_parentWatcher.changed(reinterpret_cast<char *>(m_request->nr_Name), oldFile->iName, EChangeTypeDeleted);
					m_dir.getEntries()->remove(oldFile, ETrue);

					break;
				}
				// It's in both lists, so check its timestamp and attributes to see if it has been modified
				else if (oldFile->iModified != newFile->iModified || oldFile->iAttributes != newFile->iAttributes)
				{
					m_parentWatcher.changed(reinterpret_cast<char *>(m_request->nr_Name), oldFile->iName, EChangeTypeModified);
				}
			}

			newDir.close();
		}
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 01-Sep-2025 6:14 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

const TEntry *RAmiFileWatcher::findEntry(const TEntryArray &a_entries, const char *a_name)
{
	for (const TEntry *entry = a_entries.getHead(); entry; entry = a_entries.getSucc(entry))
	{
		if (!strcmp(a_name, entry->iName))
		{
			return entry;
		}
	}

	return nullptr;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 01-Sep-2025 7:23 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RAmiFileWatcher::pauseWatching()
{
	if (m_request)
	{
		EndNotify(m_request);
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 01-Sep-2025 7:23 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RAmiFileWatcher::resumeWatching()
{
	if (m_request)
	{
		if (!StartNotify(m_request))
		{
			Utils::info("RAmiFileWatcher::resumeWatching() => Unable to restart file watching");
		}
	}
}

/**
 * Start watching a directory.
 * Watches the given directory for changes, and adds the watcher to the list of watchers known by the RApplication
 * class, so that it can be notified when a change occurs.
 *
 * This is an internal class used by CStdFileWatcher. The name of the class passed in must remain persistent during
 * the use of this class, and must not be deleted.
 *
 * @date	Monday 01-Sep-2025 6:00 am, Code HQ Tokyo Tsukuda
 * @param	a_directoryName	The name of the directory to be watched. The string must be in persistent memory
 * @param	a_application	Application to be notified when the directory changes
 */

bool RAmiFileWatcher::startWatching(const std::string &a_directoryName)
{
	// Create a message port
	if ((m_port = CreatePort(NULL, 0)) != nullptr)
	{
		m_signal = 1L << m_port->mp_SigBit;

		// Allocate notify request
		m_request = (struct NotifyRequest *) AllocMem(sizeof(struct NotifyRequest), MEMF_CLEAR | MEMF_PUBLIC);

		if (m_request != nullptr)
		{
			m_request->nr_Name = a_directoryName.c_str();
			m_request->nr_FullName = a_directoryName.c_str();
			m_request->nr_Flags = NRF_SEND_MESSAGE;
			m_request->nr_stuff.nr_Msg.nr_Port = m_port;

			if (StartNotify(m_request))
			{
				if (m_dir.open(reinterpret_cast<char *>(m_request->nr_Name)) == KErrNone)
				{
					if (m_dir.read(EDirSortNameAscending) == KErrNone)
					{
						CWindow::GetRootWindow()->GetApplication()->AddWatcher(this);

						return true;
					}
				}
			}
		}
	}

	stopWatching();

	return false;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 01-Sep-2025 6:17 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RAmiFileWatcher::stopWatching()
{
	CWindow::GetRootWindow()->GetApplication()->RemoveWatcher(this);

	m_dir.close();

	if (m_request)
	{
		EndNotify(m_request);
		FreeMem(m_request, sizeof(struct NotifyRequest));
		m_request = nullptr;
	}

	if (m_port)
	{
		DeletePort(m_port);
		m_port = nullptr;
	}

	m_signal = 0;
}
