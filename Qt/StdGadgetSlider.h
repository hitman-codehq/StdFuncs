
#ifndef QTSTDGADGETSLIDER_H
#define QTSTDGADGETSLIDER_H

#include <QtCore/QObject>

/* Forward declaration to reduce the # of includes required */

class CStdGadgetSlider;

/**
 * Qt helper class to listen for Qt slider events.
 * This class is internal to the framework and will listen for slider events coming
 * from a Qt slider.  It will then pass these events onto the generic framework
 * slider that owns an instance of this class.
 */

class CQtSlider : public QObject
{
	Q_OBJECT;

private:

	CStdGadgetSlider	*m_poParentSlider;	/**< Ptr to framework slider that uses this Qt slider */

public:

	CQtSlider(CStdGadgetSlider *a_poParentSlider) : m_poParentSlider(a_poParentSlider)
	{
		m_poParentSlider = a_poParentSlider;
	}

public slots:

	void valueChanged(int a_iValue);
};

#endif /* ! QTSTDGADGETSLIDER_H */
