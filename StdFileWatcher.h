
#ifndef STDFILEWATCHER_H
#define STDFILEWATCHER_H

#ifdef __amigaos__

#include <Amiga/AmiFileWatcher.h>

#elif defined(QT_GUI_LIB)

#include <Qt/QtFileWatcher.h>

#elif defined(WIN32) && !defined(QT_GUI_LIB)

#include <atomic>
#include <thread>

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

#include <functional>

/* Forward declaration to reduce the # of includes required */

class RApplication;

/**< Types of change events that can be reported by the file watcher */
enum TChangeType
{
	EChangeAdded,		/**< A file was added to the watched directory */
	EChangeDeleted,		/**< A file was removed from the watched directory */
	EChangeModified		/**< A file was modified in the watched directory */
};

/**
 * A class for watching for changes in directories.
 * Using an instance of this class, client software can be notified if changes are made to files within a specified
 * directory. A client notification callback will be called if a file is added to, deleted from or changed inside
 * the directory.
 */
class RStdFileWatcher
{
	/**
	 * File watcher states.
	 * This is mainly needed for Qt, which will output warnings if watching is stopped or paused when it is not
	 * started, but is also useful for ensuring that start/stop/pause/unpause tasks are not performed unecessarily
	 * on other versions as well
	 */
	enum TState
	{
		EStateStopped,	/**< No directory is being watched */
		EStatePaused,	/**< A directory is being watched, but watching is paused */
		EStateWatching	/**< Watching is active and client will be notified of changes */
	};

	using Callback = std::function<void(const std::string &, const std::string &, TChangeType)>;

	TState				m_state;			/**< Current state of the file watcher */
	Callback			m_callback;			/**< Client callback to invoke when a change is detected */

#ifdef __amigaos__

	RAmiFileWatcher		m_watcher;			/**< Amiga-specific helper class for file system monitoring */

#elif defined(QT_GUI_LIB)

	RQtFileWatcher		m_watcher;			/**< Qt-specific helper class for file system monitoring */

#elif defined(WIN32)

	BYTE				m_buffer[8192];		/**< Buffer for file change notification information */
	HANDLE				m_changeHandle = INVALID_HANDLE_VALUE;	/**< Handle to the directory being watched */
	OVERLAPPED			m_overlapped;		/**< OVERLAPPED structure passed to ReadDirectoryChangesW() */
	std::string			m_directoryName;	/**< Name of the directory being watched */
	std::string			m_fileName;			/**< Name of the file being watched */

#endif /* defined(WIN32) */

private:

	void changed(const std::string &a_directoryName, const std::string &a_fileName, TChangeType a_changeType);

#if defined(WIN32) && !defined(QT_GUI_LIB)

	static void CALLBACK completionRoutine(DWORD a_errorCode, DWORD a_bytesTransfered, LPOVERLAPPED a_overlapped);

	bool watchDirectory();

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

public:

#if defined __amigaos__ || defined(QT_GUI_LIB)

	RStdFileWatcher() : m_watcher(*this)
	{
		m_state = EStateStopped;
		m_callback = nullptr;
	}

#endif /* defined __amigaos__ || defined(QT_GUI_LIB) */

	~RStdFileWatcher()
	{
		stopWatching();
	}

	void pauseWatching();

	void resumeWatching();

	bool startWatching(const std::string &a_directoryName, const std::string *a_fileName, Callback a_callback);

	void stopWatching();

	friend class RAmiFileWatcher;

	friend class RApplication;

	friend class RQtFileWatcher;
};

#endif /* ! STDFILEWATCHER_H */
