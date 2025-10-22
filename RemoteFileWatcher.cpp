
#include <StdFuncs.h>
#include "RemoteFileWatcher.h"

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

void RRemoteFileWatcher::pauseWatching()
{
	if (m_state == EStateWatching)
	{
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

void RRemoteFileWatcher::resumeWatching()
{
	if (m_state == EStatePaused)
	{
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

bool RRemoteFileWatcher::startWatching(const std::string &a_directoryName, const std::string *a_fileName, Callback a_callback)
{
	(void) a_directoryName;
	(void) a_fileName;
	(void) a_callback;

	m_state = EStateWatching;

	return true;
};

/**
 * Stops the file watcher.
 * Calls the underlying platform specific file watcher to stop the currently active watcher, if any. If no watcher is
 * currently active, this method has no effect.

 * @date	Monday 25-Aug-2025 6:21 am, Code HQ Tokyo Tsukuda
 */

void RRemoteFileWatcher::stopWatching()
{
	if (m_state == EStateWatching)
	{
	}

	m_state = EStateStopped;
}
