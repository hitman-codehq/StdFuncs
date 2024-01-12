
#ifndef QTGADGETTEXTPANE_H
#define QTGADGETTEXTPANE_H

/** @file */

#include <QtWidgets/QTextEdit>

/**
 * Qt helper class that represents a text pane.
 * This class is required to receive signals when events occur on the text pane, such as the
 * user selecting an item, so that we can pass them onto the framework. // TODO: CAW
 */

class CQtGadgetTextPane : public QTextEdit
{
	Q_OBJECT

public:

	CQtGadgetTextPane() { }

	void construct(); //const std::string &a_title);
};

#endif /* ! QTGADGETTEXTPANE_H */
