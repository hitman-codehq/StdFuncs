
#include <StdFuncs.h>
#include "StdApplication.h"
#include "StdFileWatcher.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 23-Aug-2025 7:57 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::changed(const std::string &a_directoryName, const std::string &a_fileName, TChangeType a_changeType)
{
	ASSERTM((m_callback != nullptr), "CStdFileWatcher::changed() => File watcher callback must not be NULL");

	if (m_state == EStateWatching)
	{
		m_callback(a_directoryName, a_fileName, a_changeType);
	}
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 29-Aug-2025 7:11 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CALLBACK CStdFileWatcher::completionRoutine(DWORD a_errorCode, DWORD a_numberOfBytesTransfered, LPOVERLAPPED a_overlapped)
{
	auto self = reinterpret_cast<CStdFileWatcher *>(a_overlapped->hEvent);

	if (self->m_state == EStateWatching)
	{
		if (a_errorCode == ERROR_SUCCESS && a_numberOfBytesTransfered > 0)
		{
			char fileName[MAX_PATH];
			int length;
			DWORD offset = 0;
			FILE_NOTIFY_INFORMATION *info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(self->m_buffer);

			do
			{
				info = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(self->m_buffer + offset);
				length = WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength / sizeof(WCHAR), fileName, sizeof(fileName) - 1, NULL, NULL);
				fileName[length] = '\0';

				if (info->Action == FILE_ACTION_ADDED)
				{
					Utils::info("Added: %s", fileName);
					self->changed(self->m_directoryName, fileName, EChangeTypeAdded);
				}
				else if (info->Action == FILE_ACTION_REMOVED)
				{
					Utils::info("Removed: %s", fileName);
					self->changed(self->m_directoryName, fileName, EChangeTypeDeleted);
				}
				else if (info->Action == FILE_ACTION_MODIFIED)
				{
					Utils::info("Modified: %s", fileName);
					self->changed(self->m_directoryName, fileName, EChangeTypeModified);
				}
				else
				{
					Utils::info("Unknown action %lu on file: %s", info->Action, fileName);
				}

				offset += info->NextEntryOffset;
			}
			while (info->NextEntryOffset != 0);

			self->watchDirectory();
		}
	}
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 29-Aug-2025 5:54 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::pauseWatching()
{

#ifdef __amigaos__

	m_watcher.pauseWatching();

#elif defined(QT_GUI_LIB)

	m_watcher.pauseWatching();

#elif !defined(__unix__)

	DEBUGCHECK(CancelIo(m_changeHandle), "CStdFileWatcher::pauseWatching() => Could not cancel watch notifications");

#endif /* ! __unix__ */

	m_state = EStatePaused;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 29-Aug-2025 5:55 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::resumeWatching()
{

#ifdef __amigaos__

	m_watcher.resumeWatching();

#elif defined(QT_GUI_LIB)

	m_watcher.resumeWatching();

#elif !defined(__unix__)

	m_changeHandle = CreateFileA(
		m_directoryName.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);

	if (m_changeHandle == INVALID_HANDLE_VALUE) {
		//std::cerr << "Failed to open directory handle, error: " << GetLastError() << std::endl;
		return;
	}

	watchDirectory();

#endif /* ! __unix__ */

	m_state = EStateWatching;
}

/**
 * Short description.
 * Long multi line description.
 * Only one directory can be watched at a time. If this methid is called while another directory is being watched, the
 * watch on the old directory will be cancelled and a new watch will be started on the new directory.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 11-Aug-2025 1:25 pm, Sarutahiko Coffee Marunouchi
 * @param	Parameter		Description
 * @return	Return value
 */

bool CStdFileWatcher::startWatching(const std::string &a_directoryName, Callback a_callback)
{
	bool retVal;

	m_directoryName = a_directoryName;
	m_callback = a_callback;

	stopWatching();

#ifdef __amigaos__

	retVal = m_watcher.startWatching(m_directoryName);

#elif defined(QT_GUI_LIB)

	retVal = m_watcher.startWatching(m_directoryName);

#elif !defined(__unix__)

	m_changeHandle = CreateFileA(
		m_directoryName.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);

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
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 25-Aug-2025 6:21 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::stopWatching()
{
	// TODO: CAW - Implement this for others
	if (m_state == EStateWatching)
	{

#ifdef __amigaos__

		m_watcher.stopWatching();

#elif defined(QT_GUI_LIB)

		m_watcher.stopWatching();

#elif !defined(__unix__)

		if (m_changeHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_changeHandle);
			m_changeHandle = INVALID_HANDLE_VALUE;
		}

#endif /* ! __unix__ */

	}

	m_state = EStateStopped;
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 25-Aug-2025 6:15 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool CStdFileWatcher::watchDirectory()
{
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	// Trick: stash "this" in hEvent so we can get back inside completionRoutine
	m_overlapped.hEvent = this;

	BOOL retVal = ReadDirectoryChangesW(
		m_changeHandle,
		m_buffer,
		sizeof(m_buffer),
		FALSE,
		FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
		nullptr,
		&m_overlapped,
		completionRoutine);

	if (!retVal) {
		DWORD err = GetLastError();

		if (err != ERROR_IO_PENDING) {
			Utils::info("ReadDirectoryChangesW failed, error: %lx", err);
		}
	}

	return retVal;
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */
