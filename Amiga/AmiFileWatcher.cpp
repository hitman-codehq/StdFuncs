
#include "../StdFuncs.h"
#include "../StdApplication.h"
#include "../StdFileWatcher.h"
#include "../StdWindow.h"
#include <string.h>

/**
 * Internal callback for Amiga file watcher changes.
 * This method is called by the RApplication::Main() message loop when a change is detected. It will determine what
 * type of change occurred, and to what files, and will call the internal platform independent callback.
 *
 * @date	Monday 01-Sep-2025 6:05 am, Code HQ Tokyo Tsukuda
 */

void RAmiFileWatcher::changed()
{
	int result;
	RDir newDir;
	TEntry *newEntry;

	/* Scan the requested directory and build a list of files that it contains */
	if ((result = newDir.open(reinterpret_cast<const char *>(m_request->nr_Name))) == KErrNone)
	{
		if ((result = newDir.read(EDirSortNameAscending)) == KErrNone)
		{
			/* Detect files that were deleted. If the file is in the old list but not in the new list, it has been deleted */
			const TEntry *oldFile = m_files.getEntries()->getHead();

			while (oldFile != nullptr)
			{
				const TEntry *newFile = findEntry(*newDir.getEntries(), oldFile->iName);

				if (!newFile)
				{
					m_parentWatcher.changed(reinterpret_cast<const char *>(m_request->nr_Name), oldFile->iName, EChangeDeleted);

					/* Because we are modifying the files list while we are iterating through it, we have to carefully */
					/* remove the current node while moving to the next one */
					const TEntry *next = m_files.getEntries()->getSucc(oldFile);
					m_files.getEntries()->remove(oldFile, ETrue);
					oldFile = next;
				}
				else
				{
					oldFile = m_files.getEntries()->getSucc(oldFile);
				}
			}

			/* Detect files that were added. If the file is in the new list but not in the old list, it is new */
			for (const TEntry *newFile = newDir.getEntries()->getHead(); newFile; newFile = newDir.getEntries()->getSucc(newFile))
			{
				const TEntry *oldFile = findEntry(*m_files.getEntries(), newFile->iName);

				if (!oldFile)
				{
					m_parentWatcher.changed(reinterpret_cast<const char *>(m_request->nr_Name), newFile->iName, EChangeAdded);

					if ((newEntry = m_files.getEntries()->Append(newFile->iName)) != nullptr)
					{
						newEntry->Set(*newFile);
					}
				}
				/* It's in both lists, so check its timestamp and attributes to see if it has been modified */
				else if (oldFile->iModified != newFile->iModified || oldFile->iAttributes != newFile->iAttributes)
				{
					m_parentWatcher.changed(reinterpret_cast<const char *>(m_request->nr_Name), oldFile->iName, EChangeModified);
				}
			}

			newDir.close();
		}
	}
}

/**
 * Search for an instance of a file in a TEntryArray.
 * Iterates through the passed in entry array and searches for a file with the given name
 *
 * @date	Monday 01-Sep-2025 6:14 am, Code HQ Tokyo Tsukuda
 * @param	a_entries		The entry array in which to search
 * @param	a_name			The name of the file for which to search
 * @return	A pointer to the entry that was found, or nullptr if not found
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
 * Pause file watching.
 * Cancels notifications being received from DOS, for the directory being watched.
 *
 * @date	Monday 01-Sep-2025 7:23 am, Code HQ Tokyo Tsukuda
 */

void RAmiFileWatcher::pauseWatching()
{
	if (m_request != nullptr)
	{
		EndNotify(m_request);
	}
}

/**
 * Resumes file watching.
 * Restarts notifications from DOS, after they have been paused for the directory being watched.
 *
 * @date	Monday 01-Sep-2025 7:23 am, Code HQ Tokyo Tsukuda
 */

void RAmiFileWatcher::resumeWatching()
{
	if (m_request != nullptr)
	{
		DEBUGCHECK(StartNotify(m_request), "RAmiFileWatcher::resumeWatching() => Could not restart file watching");
	}
}

/**
 * Starts file watching.
 * Watches the given directory for changes, and adds the watcher to the list of watchers known by the RApplication
 * class, so that it can be notified when a change occurs. It also saves a list of files in the directory, which
 * can be used to determine what files have been changed when a directory change message is received from DOS.
 *
 * This is an internal class used by RStdFileWatcher. The name of the class passed in must remain persistent during
 * the use of this class, and must not be deleted.
 *
 * @date	Monday 01-Sep-2025 6:00 am, Code HQ Tokyo Tsukuda
 * @param	a_directoryName	The name of the directory to be watched. The string must be in persistent memory
 * @return	True if the watcher was started successfully, otherwise false
 */

bool RAmiFileWatcher::startWatching(const std::string &a_directoryName)
{
	/* Create a message port */
	if ((m_port = CreatePort(nullptr, 0)) != nullptr)
	{
		m_signal = 1L << m_port->mp_SigBit;

		/* Allocate notify request */
		m_request = (struct NotifyRequest *) AllocMem(sizeof(struct NotifyRequest), MEMF_CLEAR | MEMF_PUBLIC);

		if (m_request != nullptr)
		{
			m_request->nr_Name = a_directoryName.c_str();
			m_request->nr_FullName = a_directoryName.c_str();
			m_request->nr_Flags = NRF_SEND_MESSAGE;
			m_request->nr_stuff.nr_Msg.nr_Port = m_port;

			if (StartNotify(m_request))
			{
				/* The signal only indicates that something has changed, not what has changed. So we need to make a */
				/* snapshot of the contents of the directory under observation for later reference when we receive a */
				/* signal */
				if (m_files.open(reinterpret_cast<const char *>(m_request->nr_Name)) == KErrNone)
				{
					if (m_files.read(EDirSortNameAscending) == KErrNone)
					{
						CWindow::GetRootWindow()->GetApplication()->AddWatcher(this);

						return true;
					}
				}
			}
		}
	}

	/* Something failed, so cleanup whatever resources were allocated */
	stopWatching();

	return false;
}

/**
 * Stops file watching.
 * Cancels notifications being received from DOS, for the directory being watched, and cleans up DOS's watcher-related
 * resources that were created by startWatching().
 *
 * @date	Monday 01-Sep-2025 6:17 am, Code HQ Tokyo Tsukuda
 */

void RAmiFileWatcher::stopWatching()
{
	CWindow::GetRootWindow()->GetApplication()->RemoveWatcher(this);

	/* Purge the list of files in the watched directory so it can be reused for a different directory later */
	m_files.close();

	/* And free DOS's resources */
	if (m_request != nullptr)
	{
		EndNotify(m_request);
		FreeMem(m_request, sizeof(struct NotifyRequest));
		m_request = nullptr;
	}

	if (m_port != nullptr)
	{
		DeletePort(m_port);
		m_port = nullptr;
	}

	m_signal = 0;
}
