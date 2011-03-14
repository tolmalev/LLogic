#include <QPainter>
#include <QPixmap>

#include "workpanel.h"
#include "element.h"
#include "elementwidget.h"
#include "stdio.h"
#include "document.h"
#include "controller.h"
#include "pointwidget.h"

WorkPanel::WorkPanel(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(1);
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
                QPoint p1 = d->workPanel()->points[k]->mapTo(d->workPanel(), QPoint(3, 3));
                QPoint p2 = d->workPanel()->points[b]->mapTo(d->workPanel(), QPoint(3, 3));
                painter.drawLine(p1, p2);
            }
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
