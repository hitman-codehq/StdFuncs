
#include "../StdFuncs.h"
#include "QtFileListener.h"

void watch();

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 11-Aug-2025 1:30 pm, Sarutahiko Coffee Otemachi TODO: CAW
 * @param	Parameter		Description
 * @return	Return value
 */

void CQtFileListener::watch()
{
	// Watch a single file
	m_watcher.addPath("/Users/colinward/Source/Brunel/Editor.cpp");

	// Watch a directory
	m_watcher.addPath("/Users/colinward/Source/Brunel");

	QObject::connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
					 [](const QString &path) {
						//qDebug() << "Directory changed:" << path;
						printf("Directory changed: %s\n", path.toStdString().c_str());
						Utils::info("Directory changed: %s", path.toStdString().c_str());
					 });

	QObject::connect(&m_watcher, &QFileSystemWatcher::fileChanged,
					 [](const QString &path) {
						 //qDebug() << "File changed:" << path;
						printf("File changed: %s\n", path.toStdString().c_str());
						Utils::info("File changed: %s", path.toStdString().c_str());
					 });
}
