#include <QPainter>
#include <QPixmap>
#include <QLayout>

#include <cmath>

using namespace std;


#include "workpanel.h"
#include "element.h"
#include "complexelement.h"
#include "elementwidget.h"
#include "stdio.h"
#include "document.h"
#include "controller.h"
#include "pointwidget.h"

WorkPanel::WorkPanel(ComplexElement *ce, QWidget *parent) :
    QWidget(parent), ce(ce)
{
    tmpw = 0;
    setMouseTracking(1);
    panel_type = DOCUMENT;
    if(ce != 0)
    {
        panel_type = ELEMENT;
        for(int i = 0; i < ce->in_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->panel = this;
            pw->move(0, (i+1)*grid_size*2-3);
            input_points[i] = pw;
        }
        for(int i = 0; i < ce->out_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->panel = this;
            pw->move(40*grid_size, (i+1)*grid_size*2-3);
            output_points[i] = pw;
        }

    }
    d = 0;
}

QPoint WorkPanel::toGrid(QPoint a)
{
    return QPoint((int)( (double)a.x() / grid_size + 0.5)*grid_size,
                  (int)( (double)a.y() / grid_size + 0.5)*grid_size);
}

void WorkPanel::paintEvent(QPaintEvent * ev)
{
    QPainter painter(this);
    QPixmap p(":/images/background.png");
    int w = geometry().width();
    int h = geometry().height();
    painter.fillRect(0, 0, w, h, p);

    painter.setPen(Qt::black);

    if(d != 0)
    foreach(int k, d->c->connections.keys())
    {
        foreach(int b, *d->c->connections[k])
        {
            if(k < b)
            {
                QPoint p1 = points[k]->mapTo(this, QPoint(3, 3));
                QPoint p2 = points[b]->mapTo(this, QPoint(3, 3));
                painter.drawLine(p1, p2);
            }
        }
    }
    if(ce != 0)
    {
        QPair<int, int> p;
        foreach(p, ce->in_connections)
        {
            QPoint p1 = input_points[p.first]->mapTo(this, QPoint(3,3));
            QPoint p2 = points[p.second]->mapTo(this, QPoint(3,3));
            painter.drawLine(p1, p2);
        }
        foreach(p, ce->out_connections)
        {
            QPoint p1 = points[p.first]->mapTo(this, QPoint(3,3));
            QPoint p2 = output_points[p.second]->mapTo(this, QPoint(3,3));
            painter.drawLine(p1, p2);
        }

    }

    foreach(ElementWidget *ew, selected)
    {
        QRect rt = ew->geometry();
        painter.drawRect(rt.x()-1, rt.y()-1, rt.width()+1, rt.height()+1);
    }
}

void WorkPanel::addElement(Element *e)
{
    ElementWidget *ew;
    switch(e->type())
    {
        case SEND:      ew = new SendElementWidget(this); break;
        case RECEIVE:   ew = new ReceiveElementWidget(this); break;
        default:        ew = new ElementWidget(this); break;
    }
    elementWidgets.push_back(ew);
    ew->setElement(e);
    ew->installEventFilter(this);
    connect(ew, SIGNAL(doubleClicked(ElementWidget*)), this, SIGNAL(doubleClicked(ElementWidget*)));
    connect(ew, SIGNAL(needCalculation(Element*)), this, SIGNAL(needCalculation(Element*)));
    for(int i = 0; i < e->in_cnt; i++)
    {
        PointWidget *pw = new PointWidget(ew);
        pw->panel = this;
        QPoint p = ew->getPointPos(0, i);
        pw->move(p.x(), p.y()-3);
        points[e->in.at(i)] = pw;
        pw->point = e->in.at(i);
    }
    for(int i = 0; i < e->out_cnt; i++)
    {
        PointWidget *pw = new PointWidget(ew);
        QPoint p = ew->getPointPos(1, i);
        pw->move(p.x()-4, p.y()-3);
        points[e->out.at(i)] = pw;
        pw->point = e->out.at(i);
    }
}

void WorkPanel::mouseMoveEvent(QMouseEvent *ev)
{
    qWarning("workpanel mouse move");
    QWidget*w = childAt(ev->x(), ev->y());
    if(w)
    {
        qWarning("%s", w->metaObject()->className());
    }
    if(tmpw)
    {
        int minx = min(ev->x(), p1.x());
        int miny = min(ev->y(), p1.y());
        int maxx = max(ev->x(), p1.x());
        int maxy = max(ev->y(), p1.y());

        tmpw->setGeometry(minx, miny, maxx-minx, maxy-miny);

        qWarning("%d %d %d %d", minx, miny, maxx, maxy);

    }

}

void WorkPanel::mousePressEvent(QMouseEvent *ev)
{
    tmpw = new SelectWidget((QWidget*)this);
    tmpw->setGeometry(ev->x(), ev->y(), 0, 0);
    p1 = QPoint(ev->pos());
    tmpw->setAutoFillBackground(0);
    tmpw->show();
}



void WorkPanel::mouseReleaseEvent(QMouseEvent *ev)
{
    releaseMouse();
    tmpw->deleteLater();
    QRect rt = tmpw->geometry();
    tmpw=0;
    selected.clear();
    foreach(ElementWidget* ew, elementWidgets)
    {
        if(rt.intersect(ew->geometry()) == ew->geometry() || rt.intersect(ew->geometry()) == rt)
            selected.insert(ew);
    }
    update();
}

void SelectWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(QColor(53, 103, 255, 255));
    p.fillRect(0, 0, width()-1, height()-1, QColor(176, 204, 241, 100));
    p.drawRect(0, 0, width()-1, height()-1);
}

bool WorkPanel::eventFilter(QObject *o, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = (QMouseEvent*)e;
        if(o->inherits("ElementWidget"))
        {
            ElementWidget *ew = (ElementWidget*)o;
            if(selected.find(ew) == selected.end())
            {
                selected.clear();
                selected.insert(ew);
            }
            p2 = p1 = ew->mapTo(this, me->pos());
            tmpw = new MovingWidget(this, this);
            tmpw->show();
            tmpw->setGeometry(geometry());
        }
        return 1;
    }
    else if(e->type() == QEvent::MouseMove)
    {
        QMouseEvent *me = (QMouseEvent*)e;
        if(o->inherits("ElementWidget"))
        {
            ElementWidget *ew = (ElementWidget*)o;
            p2 = ew->mapTo(this, me->pos());
            if(tmpw)
                tmpw->update();
        }
        return 1;
    }
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        qWarning("asd");
        QMouseEvent *me = (QMouseEvent*)e;
        if(o->inherits("ElementWidget"))
        {
            ElementWidget *ew = (ElementWidget*)o;
            p2 = ew->mapTo(this, me->pos());
            bool can = 1;
            foreach(ElementWidget *ew, selected)
            {
                if(!canMoveTo(ew, toGrid(ew->pos()+p2-p1)))
                {
                    can = 0;
                    break;
                }
            }

            if(can)
                foreach(ElementWidget *ew, selected)
                {
                    ew->move(toGrid(ew->pos() + p2 - p1));
                }
        }
        update();
        if(tmpw)
            tmpw->deleteLater();
        tmpw=0;

        return 1;
    }
    else
        return QWidget::eventFilter(o, e);
}

bool WorkPanel::canMoveTo(ElementWidget *e, QPoint p)
{
    if(p.x() < 0 || p.y() < 0)
        return 0;
    QRect rt = e->geometry();
    rt.moveTopLeft(p);
    foreach(ElementWidget *ew, elementWidgets)
    {
        if(e != ew)
        {
            if(!(rt.intersect(ew->geometry()).isNull()))
            {
                qWarning("intersect %d %d %d %d    %d %d %d %d", rt.left(), rt.top(), rt.right(), rt.bottom(),
                         ew->geometry().left(), ew->geometry().top(), ew->geometry().right(), ew->geometry().bottom());
                return 0;
            }
        }
    }
    return 1;
}

void MovingWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPoint dp = wp->p2-wp->p1;
    qWarning("dp = %d %d", dp.x(), dp.y());
    foreach(ElementWidget *ew, wp->selected)
    {
        QRect rt;
        rt.setTopLeft(wp->toGrid(ew->pos() + dp));
        rt.setWidth(ew->width());
        rt.setHeight(ew->height());

        if(wp->canMoveTo(ew, rt.topLeft()))
            p.fillRect(rt, QColor(0, 255, 0, 100));
        else
            p.fillRect(rt, QColor(255, 0, 0, 100));
        p.drawRect(rt);
    }
}
