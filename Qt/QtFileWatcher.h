
#ifndef QTFILEWATCHER_H
#define QTFILEWATCHER_H

#include <QtCore/QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QMap>

/* Forward declaration to reduce the # of includes required */

class RStdFileWatcher;

/**
 * Qt helper class to listen for file change events.
 * This class is internal to the framework and will listen for file change events coming from a Qt file system
 * watcher. It will then pass these events onto the generic framework file watcher that owns an instance of this
 * class.
 */
class RQtFileWatcher : public QObject
{
	Q_OBJECT

private:

	RStdFileWatcher				&m_parentWatcher;	/**< Reference to framework watcher that uses this Qt watcher */
	QFileSystemWatcher			m_watcher;			/**< Qt native file watcher class */
	QMap<QString, QDateTime>	m_files;			/**< List of files in the directory being watched */
	QMetaObject::Connection		m_connection;		/**< Connection set up by call to QObject::connect() */
	QString						m_directoryName;	/**< Name of the directory being watched */
	QString						m_fileName;			/**< Name of the file being watched, if any */

private:

	void changed();

public:

	RQtFileWatcher(RStdFileWatcher &a_parentWatcher) : m_parentWatcher(a_parentWatcher) { }

	void pauseWatching();

	void resumeWatching();

	bool startWatching(const std::string &a_directoryName, const std::string *a_fileName);

	void stopWatching();
};

#endif /* ! QTFILEWATCHER_H */
