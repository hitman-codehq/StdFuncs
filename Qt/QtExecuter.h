
#ifndef QTEXECUTER_H
#define QTEXECUTER_H

// TODO: CAW - Inconsistent
#include <QtCore/QObject>
#include <QProcess>

/* Forward declaration to reduce the # of includes required */

class RStdExecuter;

class RQtExecuter : public QObject
{
	Q_OBJECT

private:

	QProcess		*m_process;
	RStdExecuter	&m_parentExecuter;	/**< Reference to framework executer that uses this Qt executer */

private slots:

	void onReadyRead();
	void onReadyReadError();
	// void onReadyReadStandardError();
	// void onProcessFinished(int exitCode, QProcess::ExitStatus status);
	void onProcessError(QProcess::ProcessError a_error);

public:

	RQtExecuter(RStdExecuter &a_parentExecuter) : m_process(nullptr), m_parentExecuter(a_parentExecuter) { }

	int launchCommand(const char *a_commandName);
};

#endif /* ! QTEXECUTER_H */
