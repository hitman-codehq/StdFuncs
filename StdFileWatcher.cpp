
#include <StdFuncs.h>
#include "StdApplication.h"
#include "StdFileWatcher.h"

/**
 * Internal callback for file watcher changes.
 * Called by the platform specific watcher implementation when a change is detected. It will check the current watcher
 * state and if watching is active, will call the client callback.
 *
 * @date	Saturday 23-Aug-2025 7:57 am, Code HQ Tokyo Tsukuda
 * @param	a_directoryName	The directory in which the change occurred
 * @param	a_fileName		The name of the file that changed
 * @param	a_changeType	The type of change that occurred
 */

void RStdFileWatcher::changed(const std::string &a_directoryName, const std::string &a_fileName, TChangeType a_changeType)
{
	ASSERTM((m_callback != nullptr), "RStdFileWatcher::changed() => File watcher callback must not be nullptr");

	if (m_state == EStateWatching)
	{
		m_callback(a_directoryName, a_fileName, a_changeType);
	}
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Internal callback for Windows file watcher changes.
 * This function is called by ReadDirectoryChanges() when a change is detected. It will determine what type of change
 * occurred and call the internal platform independent callback.
 *
 * @date	Friday 29-Aug-2025 7:11 am, Code HQ Tokyo Tsukuda
 * @param	a_errorCode			The result of the change operation
 * @param	a_bytesTransfered	The number of bytes placed into the m_buffer array
 * @param	a_overlapped		Pointer to the OVERLAPPED structure used for this asynchronous operation
 */

void CALLBACK RStdFileWatcher::completionRoutine(DWORD a_errorCode, DWORD a_bytesTransfered, LPOVERLAPPED a_overlapped)
{
	/* The Windows file watching API does not handle the situation of the buffer being too small very well. If it is */
	/* too small, notifications will be lost and there is no way to recover. So just check that some data was actually */
	/* received and handle whatever we get */
	if (a_errorCode == ERROR_SUCCESS && a_bytesTransfered > 0)
	{
		char fileName[MAX_PATH];
		int length;
		DWORD offset = 0;
		RStdFileWatcher *self = reinterpret_cast<RStdFileWatcher *>(a_overlapped->hEvent);
		FILE_NOTIFY_INFORMATION *info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(self->m_buffer);

		/* Iterate through the notifications that were received, check their type and call the generic watcher callback */
		do
		{
			info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(self->m_buffer + offset);
			length = WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength / sizeof(WCHAR), fileName,
				sizeof(fileName) - 1, nullptr, nullptr);
			fileName[length] = '\0';

			if (info->Action == FILE_ACTION_ADDED || info->Action == FILE_ACTION_RENAMED_NEW_NAME)
			{
				self->changed(self->m_directoryName, fileName, EChangeAdded);
			}
			else if (info->Action == FILE_ACTION_REMOVED || info->Action == FILE_ACTION_RENAMED_OLD_NAME)
			{
				self->changed(self->m_directoryName, fileName, EChangeDeleted);
			}
			else if (info->Action == FILE_ACTION_MODIFIED)
			{
				self->changed(self->m_directoryName, fileName, EChangeModified);
			}

			offset += info->NextEntryOffset;
		}
		while (info->NextEntryOffset != 0);

		DEBUGCHECK(self->watchDirectory(), "RStdFileWatcher::completionRoutine() => Could not restart file watching");
	}
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/**
 * Pause file watching.
 * If file watching is underway, it will be paused until unpauseWatching() is called. Calling this method when
 * watching is not underway has no effect.
 *
 * Note that this only has an effect when directories are being watched. Calling pauseWatching() on a file will
 * have no effect.
 *
 * @date	Friday 29-Aug-2025 5:50 am, Code HQ Tokyo Tsukuda
 */

void RStdFileWatcher::pauseWatching()
{
	if (m_state == EStateWatching)
	{

#if defined(__amigaos__) || defined(QT_GUI_LIB)

		m_watcher.pauseWatching();

#elif !defined(__unix__)

		DEBUGCHECK(CancelIo(m_changeHandle), "RStdFileWatcher::pauseWatching() => Could not cancel watch notifications");

#endif /* ! __unix__ */

		m_state = EStatePaused;
	}
}

/**
 * Unpause file watching.
 * If file watching is paused, it will be restarted. Calling this method when watching is not paused has no effect.
 *
 * Note that this only has an effect when directories are being watched. Calling pauseWatching() on a file will
 * have no effect.
 *
 * @date	Friday 29-Aug-2025 5:55 am, Code HQ Tokyo Tsukuda
 */

void RStdFileWatcher::resumeWatching()
{
	if (m_state == EStatePaused)
	{

#if defined(__amigaos__) || defined(QT_GUI_LIB)

		m_watcher.resumeWatching();

#elif !defined(__unix__)

		m_changeHandle = CreateFile(m_directoryName.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

		ASSERTM((m_changeHandle != INVALID_HANDLE_VALUE), "RStdFileWatcher::resumeWatching() => Could not re-open directory for watching");

		if (m_changeHandle != INVALID_HANDLE_VALUE)
		{
			DEBUGCHECK(watchDirectory(), "RStdFileWatcher::resumeWatching() => Could not restart file watching");
		}

#endif /* ! __unix__ */

		m_state = EStateWatching;
	}
}

/**
 * Starts the file watcher.
 * Calls the underlying platform specific file watcher to start watching the specified directory. The callback passed
 * in will be called if a change is detected in the given directory. If a file watch is already underway, it will be
 * stopped and a new one started.
 *
 * An optional filename can be passed in, in which case only changes to that specific file will be reported. If no
 * filename is passed in, all changes to files in the directory will be reported. For monitoring a single file in a
 * directory (such as a preferences file), this is more efficient than monitoring the entire directory.
 *
 * @date	Monday 11-Aug-2025 1:25 pm, Sarutahiko Coffee Marunouchi
 * @param	a_directoryName	The name of the directory to watch
 * @param	a_fileName		Optional name of a specific file to watch
 * @param	a_callback		The callback to invoke on file changes
 * @return	True if the watcher was started successfully, otherwise false
 */

bool RStdFileWatcher::startWatching(const std::string &a_directoryName, const std::string *a_fileName, Callback a_callback)
{
	bool retVal;

#if defined(WIN32) && !defined(QT_GUI_LIB)

	m_directoryName = a_directoryName;

	if (a_fileName != nullptr)
	{
		m_fileName = *a_fileName;
	}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	m_callback = a_callback;

	stopWatching();

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	retVal = m_watcher.startWatching(a_directoryName, a_fileName);

#elif defined(__unix__)

	(void) a_directoryName;
	(void) a_fileName;

	retVal = false;

#else /* ! __unix__ */

	m_changeHandle = CreateFile(m_directoryName.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

	if (m_changeHandle != INVALID_HANDLE_VALUE)
	{
		retVal = watchDirectory();
	}
	else
	{
		retVal = false;
	}

#endif /* ! __unix__ */

	if (retVal)
	{
		m_state = EStateWatching;
	}

	return retVal;
};

/**
 * Stops the file watcher.
 * Calls the underlying platform specific file watcher to stop the currently active watcher, if any. If no watcher is
 * currently active, this method has no effect.

 * @date	Monday 25-Aug-2025 6:21 am, Code HQ Tokyo Tsukuda
 */

void RStdFileWatcher::stopWatching()
{
	if (m_state == EStateWatching)
	{

#if defined(__amigaos__) || defined(QT_GUI_LIB)

		m_watcher.stopWatching();

#elif defined(QT_GUI_LIB)

		m_watcher.stopWatching();

#elif !defined(__unix__)

		if (m_changeHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_changeHandle);
			m_changeHandle = INVALID_HANDLE_VALUE;
		}

		m_directoryName.clear();
		m_fileName.clear();

#endif /* ! __unix__ */

	}

	m_state = EStateStopped;
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Start the Windows-specific file watcher.
 * This method is used on Windows to call the asynchronous ReadDirectoryChangesW() call to start watching the
 * requested directory. It is called internally when starting the watcher, and again each time a change to the
 * file system is detected.
 *
 * @date	Monday 25-Aug-2025 6:15 am, Code HQ Tokyo Tsukuda
 * @return	True if the watcher was started successfully, otherwise false
 */

bool RStdFileWatcher::watchDirectory()
{
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	/* Stash "this" in hEvent so we can retrieve it inside the completion routine */
	m_overlapped.hEvent = this;

	/* We want to watch for adds, deletes, renames and modifications */
	return ReadDirectoryChangesW(m_changeHandle, m_buffer, sizeof(m_buffer), FALSE,
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
		nullptr, &m_overlapped, completionRoutine);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */
