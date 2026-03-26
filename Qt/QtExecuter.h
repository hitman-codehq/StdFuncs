
#ifndef QTEXECUTER_H
#define QTEXECUTER_H

#include <QObject>
#include <QProcess>

/* Forward declaration to reduce the # of includes required */

class RStdExecuter;

class RQtExecuter : public QObject
{
	Q_OBJECT

private:

	QProcess		*m_process;			/**< QProcess object used for launching the command */
	RStdExecuter	&m_parentExecuter;	/**< Reference to framework executer that uses this Qt executer */

private slots:

	void onReadyRead();
	void onFinished(int exitCode, QProcess::ExitStatus status);

public:

	RQtExecuter(RStdExecuter &a_parentExecuter) : m_process(nullptr), m_parentExecuter(a_parentExecuter) { }

	int launchCommand(const char *a_commandName);
};

#endif /* ! QTEXECUTER_H */
