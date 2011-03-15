#include <QPainter>
#include <QPixmap>

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
    setMouseTracking(1);
    panel_type = DOCUMENT;
    if(ce != 0)
    {
        panel_type = ELEMENT;
        for(int i = 0; i < ce->in_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->move(0, (i+1)*grid_size*2-3);
            input_points[i] = pw;
        }
        for(int i = 0; i < ce->out_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->move(40*grid_size, (i+1)*grid_size*2-3);
            output_points[i] = pw;
        }

    }
    d = 0;
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
    ew->setElement(e);
    connect(ew, SIGNAL(doubleClicked(ElementWidget*)), this, SIGNAL(doubleClicked(ElementWidget*)));
    connect(ew, SIGNAL(needCalculation(Element*)), this, SIGNAL(needCalculation(Element*)));
    for(int i = 0; i < e->in_cnt; i++)
    {
        PointWidget *pw = new PointWidget(ew);
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
}
