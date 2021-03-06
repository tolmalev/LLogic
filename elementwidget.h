#ifndef ELEMENTWIDGET_H
#define ELEMENTWIDGET_H

#include "classes.h"

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QLineEdit>


class ElementWidget : public QWidget
{
    Q_OBJECT

    friend class WorkPanel;

    protected:
        bool moving;
        int px, py;
        int drawType;

        Element * e;
	QWidget *l;
    public:
        explicit ElementWidget(QWidget *parent = 0);

        virtual void updateSize();

        void        setElement(Element*_e);
        Element*    element(){return e;}

        virtual void paintEvent ( QPaintEvent * event );
        void mouseDoubleClickEvent(QMouseEvent *);

        QPoint getPointPos(int type, int n);

        void setDrawType(int);

    signals:
        void doubleClicked(ElementWidget*);
        void needCalculation(Element*);
    public slots:
        void changeDrawType();
};

class SendElementWidget : public ElementWidget{
    Q_OBJECT

    public:
        explicit SendElementWidget(QWidget *parent = 0) : ElementWidget(parent){};
        virtual void updateSize();
        virtual void paintEvent ( QPaintEvent * event );

        void mouseDoubleClickEvent(QMouseEvent *);
};

class ReceiveElementWidget : public ElementWidget{
    Q_OBJECT

    public:
        explicit ReceiveElementWidget(QWidget *parent = 0) : ElementWidget(parent){};
        virtual void updateSize();
        virtual void paintEvent ( QPaintEvent * event );
};

class NumberSendElement8Widget : public ElementWidget{
    Q_OBJECT
    QLineEdit *lineEdit;
	void recalc(QString s);
    public:
	explicit NumberSendElement8Widget(QWidget *parent = 0) : ElementWidget(parent){lineEdit = 0;};
	virtual void updateSize();
	virtual void paintEvent ( QPaintEvent * event );

	void mouseDoubleClickEvent(QMouseEvent *);
    public slots:
	void numberChanged();
};

class IfElementWidget : public ElementWidget{
    Q_OBJECT

    QLineEdit *lineEdit;

    QString text;
	void recalc(QString s);
    public:
	explicit IfElementWidget(QWidget *parent = 0) : ElementWidget(parent){lineEdit = 0;text="";};
	virtual void paintEvent ( QPaintEvent * event );

	void mouseDoubleClickEvent(QMouseEvent *);
    public slots:
	void numberChanged();
};


class NumberRecieveElement8Widget : public ElementWidget{
    Q_OBJECT

    public:
	explicit NumberRecieveElement8Widget(QWidget *parent = 0) : ElementWidget(parent){};
	virtual void updateSize();
	virtual void paintEvent ( QPaintEvent * event );
};

class SegmentElementWidget : public ElementWidget{
    Q_OBJECT

    public:
	explicit SegmentElementWidget(QWidget *parent = 0) : ElementWidget(parent){};
	virtual void updateSize();
	virtual void paintEvent ( QPaintEvent * event );
};

#endif // ELEMENTWIDGET_H
