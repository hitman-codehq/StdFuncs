
#include "../StdFuncs.h"
#include "../StdFileWatcher.h"
#include "QtFileWatcher.h"
#include <QDir>

/**
 * Internal callback for Qt file watcher changes.
 * This method is called by a Qt signal when a change is detected. It will determine what type of change occurred,
 * and to what files, and will call the internal platform independent callback.
 *
 * @date	Monday 08-Sep-2025 6:05 am, Code HQ Tokyo Tsukuda
 * @param	a_directoryName	The directory in which the change occurred
 */

void RQtFileWatcher::changed(const QString &a_directoryName)
{
	QDir newDir(m_directoryName);

	/* Detect files that were deleted. If the file is in the old list but not in the new list, it has been deleted */
	auto it = m_files.begin();

	while (it != m_files.end())
	{
		if (!newDir.exists(it.key()))
		{
			this->m_parentWatcher.changed(a_directoryName.toStdString(), it.key().toStdString(), EChangeDeleted);

			/* Because we are modifying the files list while we are iterating through it, we have to carefully remove */
			/* the current node while moving to the next one */
			auto next = it;
			++next;
			m_files.remove(it.key());
			it = next;
		}
		else
		{
			++it;
		}
	}

	/* Detect files that were added. If the file is in the new list but not in the old list, it is new */
	for (auto &info : newDir.entryInfoList(QDir::Files))
	{
		if (!m_files.contains(info.fileName()))
		{
			this->m_parentWatcher.changed(a_directoryName.toStdString(), info.fileName().toStdString(), EChangeAdded);
			m_files[info.fileName()] = info.lastModified();
		}
		/* It's in both lists, so check its timestamp to see if it has been modified */
		else if (m_files[info.fileName()] != info.lastModified())
		{
			// TODO: CAW - Attribute and time changes don't work on Qt
			this->m_parentWatcher.changed(a_directoryName.toStdString(), info.fileName().toStdString(), EChangeModified);
		}
	}
}

/**
 * Pause file watching.
 * Removes the watched directory from the list of directories being watched.
 *
 * @date	Friday 29-Aug-2025 6:26 am, Code HQ Tokyo Tsukuda
 */

void RQtFileWatcher::pauseWatching()
{
	m_watcher.removePath(m_directoryName);
}

/**
 * Resumes file watching.
 * Adds the watched directory back onto the list of directories being watched.
 *
 * @date	Friday 29-Aug-2025 6:27 am, Code HQ Tokyo Tsukuda
 */

void RQtFileWatcher::resumeWatching()
{
	DEBUGCHECK(m_watcher.addPath(m_directoryName), "RQtFileWatcher::resumeWatching() => Could not restart file watching");
}

/**
 * Starts file watching.
 * Adds the given directory to the list of directories being watched and saves a list of files in that directory,
 * which can be used to determine what files have been changed when a directory change signal is received from Qt.
 *
 * @date	Monday 11-Aug-2025 1:20 pm, Sarutahiko Coffee Marunouchi
 * @param	a_directoryName	The name of the directory to be watched
 * @return	True if the watcher was started successfully, otherwise false
 */

bool RQtFileWatcher::startWatching(const std::string &a_directoryName)
{
	m_directoryName = QString::fromStdString(a_directoryName);

	if (m_watcher.addPath(m_directoryName))
	{
		/* The signal only indicates that something has changed, not what has changed. So we need to make a snapshot */
		/* of the contents of the directory under observation for later reference when we receive a signal */
		QDir dir(m_directoryName);

		for (auto &info : dir.entryInfoList(QDir::Files))
		{
			m_files[info.fileName()] = info.lastModified();
		}

		/* Connect to the directoryChanged signal so that we receive a callback when it is emitted */
		QObject::connect(&m_watcher, &QFileSystemWatcher::directoryChanged, [this](const QString &a_directoryName) {
			this->changed(a_directoryName);
		});

		return true;
	}

	return false;
}

/**
 * Stops file watching.
 * Removes the given directory from the list of directories watched by the Qt file watcher and disconnects
 * the signal that indicates changes to the watched directory.
 *
 * @date	Thursday 04-Sep-2025 6:12 am, Code HQ Tokyo Tsukuda
 */

void RQtFileWatcher::stopWatching()
{
	QObject::disconnect(m_connection);
	m_watcher.removePath(m_directoryName);
}
