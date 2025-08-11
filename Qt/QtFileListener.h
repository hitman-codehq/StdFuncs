
#ifndef QTFILELISTENER_H
#define QTFILELISTENER_H

/** @file */ // TODO: CAW

#include <QtCore/QObject>
#include <QFileSystemWatcher>

// TODO: CAW - Should these all be R classes?
class CQtFileListener : public QObject
{
	Q_OBJECT

private:

	QFileSystemWatcher	m_watcher;

public: // TODO: CAW - Order?

	void watch();
};

#endif /* ! QTFILELISTENER_H */
