
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

// TODO: CAW - Naming standard, here and below
enum TChangeType
{
	EChangeTypeAdded,
	EChangeTypeDeleted,
	EChangeTypeModified
};

class CStdFileWatcher
{
	enum TState
	{
		EStateStopped,
		EStatePaused,
		EStateWatching
	};

	using Callback = std::function<void(const std::string &, const std::string &, TChangeType)>;

	//bool				m_watching = true;
	TState				m_state = EStateStopped;
	Callback			m_callback;
	std::string			m_directoryName;

#ifdef __amigaos__

	RAmiFileWatcher		m_watcher;	/**< Amiga-specific helper class for file system monitoring */

#elif defined(QT_GUI_LIB)

	RQtFileWatcher		m_watcher;	/**< Qt-specific helper class for file system monitoring */

#elif defined(WIN32)

	HANDLE				m_changeHandle = INVALID_HANDLE_VALUE;
	OVERLAPPED			m_overlapped;
	BYTE				m_buffer[8192];

#endif /* defined(WIN32) */

private:

	void changed(const std::string &a_directoryName, const std::string &a_fileName, TChangeType a_changeType);

public:

#if defined __amigaos__ || defined(QT_GUI_LIB)

	CStdFileWatcher() : m_watcher(*this) { }

#endif /* defined __amigaos__ || defined(QT_GUI_LIB) */

	~CStdFileWatcher()
	{
		stopWatching();
	}

#if defined(WIN32) && !defined(QT_GUI_LIB)

	static void CALLBACK CStdFileWatcher::completionRoutine(DWORD a_errorCode, DWORD a_numberOfBytesTransfered, LPOVERLAPPED a_overlapped);

	bool watchDirectory();

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	void pauseWatching();

	void resumeWatching();

	bool startWatching(const std::string &a_directoryName, Callback a_callback);

	void stopWatching();

	friend class RAmiFileWatcher;

	friend class RApplication;

	friend class RQtFileWatcher;
};

#endif /* ! STDFILEWATCHER_H */
