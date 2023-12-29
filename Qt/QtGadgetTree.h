
#ifndef QTGADGETTREE_H
#define QTGADGETTREE_H

/** @file */

#include <QtWidgets/QTreeWidget>

class CStdGadgetTree;
class CWindow;

/**
 * Qt helper class that represents a tree widget.
 * This class is required to receive signals when events occur on the tree widget, such as the
 * user selecting an item, so that we can pass them onto the framework.
 */

class CQtTreeWidget : public QTreeWidget
{
	Q_OBJECT

private:

	CStdGadgetTree	*m_parentTree;	/**< Pointer to framework tree that uses this Qt tree */

public:

	CQtTreeWidget(CStdGadgetTree *a_parentTree) : m_parentTree(a_parentTree) { }

	void construct(const std::string &a_title);

	void setTitle(const std::string &a_title)
	{
		setHeaderLabel(a_title.c_str());
	}

public slots:

	void itemClicked();
};

#endif /* ! QTGADGETTREE_H */
