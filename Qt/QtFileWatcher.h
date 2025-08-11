
#ifndef QTFILEWATCHER_H
#define QTFILEWATCHER_H

#include <QtCore/QObject>
#include <QFileSystemWatcher>

/* Forward declaration to reduce the # of includes required */

class CStdFileWatcher;

// TODO: CAW - Should these all be R classes?
class CQtFileWatcher : public QObject
{
	Q_OBJECT

private:

	CStdFileWatcher		&m_parentWatcher;	/**< Reference to framework watcher that uses this Qt watcher */
	QFileSystemWatcher	m_watcher;			/**< Qt native file watcher class */

public:

	CQtFileWatcher(CStdFileWatcher &a_parentWatcher) : m_parentWatcher(a_parentWatcher) { }

	void watch(const std::string &a_directoryName);
};

#endif /* ! QTFILEWATCHER_H */
