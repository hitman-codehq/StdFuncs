
#include "../StdFuncs.h"
#include "../StdExecuter.h"
#include "QtExecuter.h"
#include <QDebug>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Mar-2026 6:13 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RQtExecuter::launchCommand(const char *a_commandName)
{
	// TODO: CAW - Handle being called when we are already running, for all operating systems
	m_process = new QProcess(this);

	// Connect signals for async reading
	connect(m_process, &QProcess::readyReadStandardOutput, this, &RQtExecuter::onReadyRead);
	connect(m_process, &QProcess::readyReadStandardError, this, &RQtExecuter::onReadyRead);
	// connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &RQtExecuter::onProcessFinished);
	connect(m_process, &QProcess::errorOccurred, this, &RQtExecuter::onProcessError);

	// Start the process asynchronously
	m_process->start(a_commandName);

	if (m_process->waitForStarted())
	{
		qDebug() << "Started successfully";
		return KErrNone;
	}
	else
	{
		qDebug() << "Could not start";
		return KErrNotFound;
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Mar-2026 8:05 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtExecuter::onProcessError(QProcess::ProcessError a_error)
{
	qDebug() << "Error occurred:" << a_error;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Mar-2026 6:45 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtExecuter::onReadyRead()
{
	QByteArray data = m_process->readAllStandardOutput();

	if (data.size() > 0)
	{
		qDebug() << "Got stdout" << data;
		m_parentExecuter.readComplete(data.constData(), data.size());
	}

	QByteArray stdErrData = m_process->readAllStandardError();

	if (stdErrData.size() > 0)
	{
		qDebug() << "Got stderr" << stdErrData;
		m_parentExecuter.readComplete(stdErrData.constData(), stdErrData.size());
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Mar-2026 7:23 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RQtExecuter::onReadyReadError()
{
	QByteArray data = m_process->readAllStandardError();
	// Process the output (parse errors, update UI, etc.)
	//processOutput(data);

	qDebug() << "Got output" << data;
	m_parentExecuter.readComplete(data.constData(), data.size());
}
