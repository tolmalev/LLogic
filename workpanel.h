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

class LiningWidget : public QWidget{
    Q_OBJECT

    WorkPanel *wp;

    public:
        LiningWidget(QWidget *parent, WorkPanel*wp) : QWidget(parent), wp(wp){}
        void paintEvent(QPaintEvent *);
};

class AddingWidget : public QWidget{
    Q_OBJECT

    WorkPanel *wp;
    ElementWidget *tmp;
    int width, height;
    bool mouseIn;
    QPoint pt;

    public:
        AddingWidget(QWidget *parent, WorkPanel*wp, int wd, int h, Element*e);
        ~AddingWidget();
        void paintEvent(QPaintEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);

        void enterEvent(QEvent *){mouseIn=1;}
        void leaveEvent(QEvent *){mouseIn=0;update();}

        void keyPressEvent(QKeyEvent *);

    signals:
        void addElement(Element *e);
};

class WorkPanel : public QWidget
{
    Q_OBJECT

    friend class Document;
    friend class MovingWidget;
    friend class LiningWidget;
    friend class AddingWidget;
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
    Element *adding;
    PointWidget* pw1, *pw2;
    QSet<ElementWidget*> elementWidgets;
    QSet<ElementWidget*> selected;

    int state;
    enum states{
        NONE,
        MOVING,
        LINING,
    };

public:
    explicit WorkPanel(ComplexElement*ce = 0, QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

    void addElement(Element* e);

    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent * ev);

    void keyPressEvent(QKeyEvent *);

    bool eventFilter(QObject *o, QEvent *e);

    bool canMoveTo(ElementWidget *ew, QPoint p);

    void contextMenuEvent(QContextMenuEvent *);

    static QPoint toGrid(QPoint a);

    Document *document(){return d;}

    void setAddingElement(Element *e);
    void stopAdding(int type = 0);

signals:
    void doubleClicked(ElementWidget*);
    void needCalculation(Element*);
public slots:

};

#endif // WORKPANEL_H
