
#ifndef QTACTION_H
#define QTACTION_H

/** @file */

#include <QAction>
#include <QtWidgets/QMainWindow>
#include "QtWindow.h"

class CQtWindow;

/**
 * Qt helper class that represents an action associated with a menu item.
 * This custom class is required to keep track of Qt menu selection events and send them to
 * the framework and its client classes
 */

class CQtAction : public QAction
{
	Q_OBJECT

private:

	int			m_iCommand;			/**< ID of this action/menu item */
	CQtWindow	*m_poWindow;		/**< Ptr to framework window to which to send command updates */

public:

	CQtAction(int a_iCommand, const QString &a_roText, CQtWindow *a_poWindow) : QAction(a_roText, a_poWindow)
	{
		m_iCommand = a_iCommand;
		m_poWindow = a_poWindow;

		/* Connect the action's triggered() slot to our actionTriggered() signal so that we know */
		/* when the menu item is selected */

		QObject::connect(this, SIGNAL(triggered()), this, SLOT(actionTriggered()));
	}

	int Command()
	{
		return(m_iCommand);
	}

public slots:

	/* Slot for picking up menu selections */

	void actionTriggered();
};

#endif /* ! QTACTION_H */
