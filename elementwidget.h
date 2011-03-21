#ifndef ELEMENTWIDGET_H
#define ELEMENTWIDGET_H

#include "classes.h"

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>


class ElementWidget : public QWidget
{
    Q_OBJECT

    protected:
        bool moving;
        int px, py;
        int drawType;

        Element * e;
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

#endif // ELEMENTWIDGET_H
