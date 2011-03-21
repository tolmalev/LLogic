#ifndef WORKPANEL_H
#define WORKPANEL_H

#include "classes.h"

#include <QWidget>
#include <QMap>
#include <QMouseEvent>

class SelectWidget : public QWidget{
    Q_OBJECT

    public:
        SelectWidget(QWidget *parent) : QWidget(parent){}
        void paintEvent(QPaintEvent *);
};

class MovingWidget : public QWidget{
    Q_OBJECT

    WorkPanel *wp;

    public:
        MovingWidget(QWidget *parent, WorkPanel*wp) : QWidget(parent), wp(wp){}
        void paintEvent(QPaintEvent *);
};

class WorkPanel : public QWidget
{
    Q_OBJECT

    friend class Document;
    friend class MovingWidget;
    QMap<int, PointWidget*> points;
    QMap<int, PointWidget*> input_points;
    QMap<int, PointWidget*> output_points;
protected:
    Document *d;
    ComplexElement *ce;

    int panel_type;
    enum types{
        DOCUMENT,
        ELEMENT,
    };

    QWidget * tmpw;
    QPoint p1, p2;
    QList<ElementWidget*> elementWidgets;
    QSet<ElementWidget*> selected;

public:
    explicit WorkPanel(ComplexElement*ce = 0, QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

    void addElement(Element* e);

    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent * ev);

    bool eventFilter(QObject *o, QEvent *e);

    bool canMoveTo(ElementWidget *ew, QPoint p);

    static QPoint toGrid(QPoint a);

    Document *document(){return d;}

signals:
    void doubleClicked(ElementWidget*);
    void needCalculation(Element*);
public slots:

};

#endif // WORKPANEL_H
