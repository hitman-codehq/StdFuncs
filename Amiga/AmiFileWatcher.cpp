
#include "../StdFuncs.h"
#include "../FileWatcher.h"
#include "../StdApplication.h"
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

	if (!m_fileName.empty())
	{
		this->m_parentWatcher.changed(m_directoryName, m_fileName, EChangeModified);
	}
	else
	{
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
	if (m_fileName.empty())
	{
		if (m_request != nullptr)
		{
			EndNotify(m_request);
		}
	}
}

/**
 * Read the list of files in the watched directory.
 * This function will fill or update the cached list of files in the watched directory. It can be called either
 * when starting to watch a directory, or when resuming watching after a pause.
 *
 * @date	Friday 03-Oct-2025 6:22 am, Code HQ Tokyo Tsukuda
 * @return	true if the directory was scanned successfully, else false
 */

bool RAmiFileWatcher::readFileList()
{
	bool retVal = false;

	/* Purge the old list of files in the watched directory */
	m_files.close();

	if (m_files.open(reinterpret_cast<const char *>(m_request->nr_Name)) == KErrNone)
	{
		if (m_files.read(EDirSortNameAscending) == KErrNone)
		{
			retVal = true;
		}
	}

	return retVal;
}

/**
 * Resumes file watching.
 * Restarts notifications from DOS, after they have been paused for the directory being watched.
 *
 * @date	Monday 01-Sep-2025 7:23 am, Code HQ Tokyo Tsukuda
 */

void RAmiFileWatcher::resumeWatching()
{
	/* Only read the list of files if we are watching a directory, rather than an individual file */
	if (m_fileName.empty())
	{
		/* Refresh the contents of the cached file list, in case something changed while we were paused */
		readFileList();

		if (m_request != nullptr)
		{
			DEBUGCHECK(StartNotify(m_request), "RAmiFileWatcher::resumeWatching() => Could not restart file watching");
		}
	}
}

/**
 * Starts file watching.
 * Watches the given directory for changes, and adds the watcher to the list of watchers known by the RApplication
 * class, so that it can be notified when a change occurs. It also saves a list of files in the directory, which
 * can be used to determine what files have been changed when a directory change message is received from DOS.
 * Alternatively, will set up single file watch if the optional a_fileName parameter is passed in.
 *
 * @date	Monday 01-Sep-2025 6:00 am, Code HQ Tokyo Tsukuda
 * @param	a_directoryName	The name of the directory to be watched
 * @param	a_fileName		Optional name of a specific file to watch
 * @return	True if the watcher was started successfully, otherwise false
 */

bool RAmiFileWatcher::startWatching(const std::string &a_directoryName, const std::string *a_fileName)
{
	bool retVal = false;
	m_directoryName = a_directoryName;

	if (a_fileName != nullptr && !a_fileName->empty())
	{
		m_fileName = a_directoryName;

		if (m_fileName.length() > 0)
		{
			char lastChar = m_fileName[m_fileName.length() - 1];

			if  (lastChar != '/' && lastChar != ':')
			{
				m_fileName += '/';
			}

			m_fileName += *a_fileName;
		}
	}

	/* Create a message port */
	if ((m_port = CreatePort(nullptr, 0)) != nullptr)
	{
		m_signal = 1L << m_port->mp_SigBit;

		/* Allocate notify request */
		m_request = (struct NotifyRequest *) AllocMem(sizeof(struct NotifyRequest), MEMF_CLEAR | MEMF_PUBLIC);

		if (m_request != nullptr)
		{
			const char *objectToWatch = m_fileName.empty() ? m_directoryName.c_str() : m_fileName.c_str();

			m_request->nr_Name = objectToWatch;
			m_request->nr_Flags = NRF_SEND_MESSAGE;
			m_request->nr_stuff.nr_Msg.nr_Port = m_port;

			if (StartNotify(m_request))
			{
				if (m_fileName.empty())
				{
					/* The signal only indicates that something has changed, not what has changed. So we need to make a */
					/* snapshot of the contents of the directory under observation for later reference when we receive a */
					/* signal */
					if (readFileList())
					{
						retVal = true;
						CWindow::GetRootWindow()->GetApplication()->AddWatcher(this);
					}
				}
				else
				{
					retVal = true;
					CWindow::GetRootWindow()->GetApplication()->AddWatcher(this);
				}
			}
		}
	}

	if (!retVal)
	{
		/* Something failed, so cleanup whatever resources were allocated */
		stopWatching();
	}

	return retVal;
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

	/* Free DOS's resources */
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

	/* Purge the list of files in the watched directory so it can be reused for a different directory later */
	m_files.close();

	m_directoryName.clear();
	m_fileName.clear();
}
