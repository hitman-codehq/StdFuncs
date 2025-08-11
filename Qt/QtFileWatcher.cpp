
#include "../StdFuncs.h"
#include "../StdFileWatcher.h"
#include "QtFileWatcher.h"
#include <QDebug>
#include <QDir>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 08-Sep-2025 6:05 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtFileWatcher::changed(const QString &a_directoryName)
{
	QDir newDir(m_directoryName);

	// Detect files that were added. If the file is in the new list but not in the old list, it is new
	for (auto &info : newDir.entryInfoList(QDir::Files))
	{
		if (!m_files.contains(info.fileName()))
		{
			this->m_parentWatcher.changed(a_directoryName.toStdString(), info.fileName().toStdString(), EChangeTypeAdded);
			m_files[info.fileName()] = info.lastModified();
		}
		// It's in both lists, so check its timestamp to see if it has been modified
		else if (m_files[info.fileName()] != info.lastModified())
		{
			// TODO: CAW - Add tests for time and attributes? Test on all ports
			this->m_parentWatcher.changed(a_directoryName.toStdString(), info.fileName().toStdString(), EChangeTypeModified);
		}
	}

	// Detect files that were deleted. If the file is in the old list but not in the new list, it has been deleted
	for (auto it = m_files.begin(); it != m_files.end(); ++it)
	{
		if (!newDir.exists(it.key()))
		{
			this->m_parentWatcher.changed(a_directoryName.toStdString(), it.key().toStdString(), EChangeTypeDeleted);
			m_files.remove(it.key());

			break; // TODO: CAW - Test handling multiple files, for all ports. This is currently crashing
		}
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 29-Aug-2025 6:26 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtFileWatcher::pauseWatching()
{
	m_watcher.removePath(m_directoryName);
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 29-Aug-2025 6:27 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtFileWatcher::resumeWatching()
{
	m_watcher.addPath(m_directoryName);
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 11-Aug-2025 1:20 pm, Sarutahiko Coffee Marunouchi
 * @param	Parameter		Description
 * @return	Return value
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
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 04-Sep-2025 6:12 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtFileWatcher::stopWatching()
{
	m_watcher.removePath(m_directoryName);
}
