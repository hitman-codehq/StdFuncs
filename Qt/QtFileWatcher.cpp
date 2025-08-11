
#include "../StdFuncs.h"
#include "../StdFileWatcher.h"
#include "QtFileWatcher.h"
#include <QDebug>

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

void CQtFileWatcher::watch(const std::string &a_directoryName)
{
	m_watcher.addPath(QString::fromStdString(a_directoryName));

	QObject::connect(&m_watcher, &QFileSystemWatcher::directoryChanged, [this](const QString &a_directoryName) {
		this->m_parentWatcher.changed(a_directoryName.toStdString());
	});
}
