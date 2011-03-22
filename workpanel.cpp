#include <QPainter>
#include <QPixmap>
#include <QLayout>
#include <QMenu>
#include <QApplication>
#include <QKeyEvent>

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
    pw1=pw2=0;
    state = NONE;
    setMouseTracking(1);
    panel_type = DOCUMENT;
    //grabKeyboard();
    if(ce != 0)
    {
        panel_type = ELEMENT;
        for(int i = 0; i < ce->in_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->show();
            pw->installEventFilter(this);
            pw->panel = this;
            pw->move(0, (i+1)*grid_size*2-3);
            input_points[i] = pw;
        }
        for(int i = 0; i < ce->out_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->show();
            pw->installEventFilter(this);
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
        rt.setTopLeft(rt.topLeft() - QPoint(3, 3));
        rt.setBottomRight(rt.bottomRight() + QPoint(3, 3));
        //painter.setPen(Qt::DashLine);
        //painter.drawRect(rt);
        QPoint tl = rt.topLeft(), tr = rt.topRight(), bl = rt.bottomLeft(), br = rt.bottomRight();
        QPoint dx(rt.width()/5, 0);
        QPoint dy(0, rt.height()/5);

        painter.setPen(QColor(53, 53, 255, 255));

        painter.drawLine(tl, tl + dx);
        painter.drawLine(tr, tr - dx);
        painter.drawLine(tl, tl + dy);
        painter.drawLine(bl, bl - dy);
        painter.drawLine(bl, bl + dx);
        painter.drawLine(br, br - dy);
        painter.drawLine(br, br - dx);
        painter.drawLine(tr, tr + dy);
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
    ew->show();
    elementWidgets.insert(ew);
    ew->setElement(e);
    ew->installEventFilter(this);
    connect(ew, SIGNAL(doubleClicked(ElementWidget*)), this, SIGNAL(doubleClicked(ElementWidget*)));
    connect(ew, SIGNAL(needCalculation(Element*)), this, SIGNAL(needCalculation(Element*)));
    for(int i = 0; i < e->in_cnt; i++)
    {
        PointWidget *pw = new PointWidget(ew);
        pw->show();
        pw->installEventFilter(this);
        pw->panel = this;
        QPoint p = ew->getPointPos(0, i);
        pw->move(p.x(), p.y()-2);
        points[e->in.at(i)] = pw;
        pw->point = e->in.at(i);
    }
    for(int i = 0; i < e->out_cnt; i++)
    {
        PointWidget *pw = new PointWidget(ew);
        pw->show();
        pw->installEventFilter(this);
        QPoint p = ew->getPointPos(1, i);
        pw->move(p.x()-4, p.y()-2);
        points[e->out.at(i)] = pw;
        pw->point = e->out.at(i);
    }
    if(tmpw)
        tmpw->raise();
    update();
}

void WorkPanel::mouseMoveEvent(QMouseEvent *ev)
{
    qDebug("workpanel mouse move %d %d", ev->x(), ev->y());
    if(state == NONE)
    {
        if(pw1)
        {
            pw1->setDrawType(0);
            pw1->update();
            pw1 = 0;
        }
    }
    QWidget*w = childAt(ev->x(), ev->y());
    if(w)
    {
        qDebug("%s", w->metaObject()->className());
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
    setFocus();
    qWarning("mouse pressed");
    if(d->instrument == Document::SELECT)
    {
        if(ev->button() == Qt::LeftButton)
        {
            tmpw = new SelectWidget((QWidget*)this);
            tmpw->setGeometry(ev->x(), ev->y(), 0, 0);
            p1 = QPoint(ev->pos());
            tmpw->setAutoFillBackground(0);
            tmpw->show();
        }
        else
        {
            if(tmpw)
            {
                tmpw->deleteLater();
                tmpw=0;
                selected.clear();
            }
        }
    }
}

void WorkPanel::mouseReleaseEvent(QMouseEvent *ev)
{
    setFocus();
    releaseMouse();

    if(tmpw)
    {
        tmpw->deleteLater();
        QRect rt = tmpw->geometry();
        tmpw=0;
        if(qApp->keyboardModifiers() != Qt::ControlModifier)
            selected.clear();
        foreach(ElementWidget* ew, elementWidgets)
        {
            if(rt.intersect(ew->geometry()) == ew->geometry() || rt.intersect(ew->geometry()) == rt)
                selected.insert(ew);
        }
        update();
    }
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
    setFocus();
    if(e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = (QMouseEvent*)e;
        if(me->button() == Qt::LeftButton)
        {
            if(state == NONE)
            {
                if(o->inherits("ElementWidget"))
                {
                    ElementWidget *ew = (ElementWidget*)o;
                    if(selected.find(ew) == selected.end())
                    {
                        if(qApp->keyboardModifiers() != Qt::ControlModifier)
                            selected.clear();
                        selected.insert(ew);
                    }
                    p2 = p1 = ew->mapTo(this, me->pos());
                    tmpw = new MovingWidget(this, this);
                    tmpw->show();
                    tmpw->setGeometry(geometry());
                    state = MOVING;
                }
                else if(o->inherits("PointWidget"))
                {
                    PointWidget *pw = (PointWidget*)o;
                    pw1 = (PointWidget*)o;
                    state = LINING;
                    tmpw = new LiningWidget(this, this);
                    tmpw->show();
                    p2 = p1 = toGrid(pw->mapTo(this, me->pos()));
                    tmpw->setGeometry(geometry());
                }
            }
        }
        return 1;
    }
    else if(e->type() == QEvent::MouseMove)
    {
        QMouseEvent *me = (QMouseEvent*)e;
        if(state == MOVING || state == LINING)
        {
            if(o->inherits("QWidget"))
            {
                QWidget *ew = (QWidget*)o;
                p2 = ew->mapTo(this, me->pos());
                if(tmpw)
                    tmpw->update();
            }
        }
        else if(state == NONE)
        {
            if(o->inherits("PointWidget"))
            {
                pw1 = (PointWidget*)o;
                pw1->setDrawType(1);
                pw1->update();
            }
            else
            {
                if(pw1)
                {
                    pw1->setDrawType(0);
                    pw1->update();
                    pw1=0;
                }
            }
        }

        if(state == LINING)
        {
            qWarning("lining %d %d", p2.x(), p2.y());
            PointWidget *pw = 0;
            foreach(PointWidget*p, points)
            {
                QRect rt = p->geometry();
                rt.setTopLeft( p->mapTo(this, QPoint(0, 0)) );
                rt.setWidth(p->geometry().width());
                rt.setHeight(p->geometry().height());
                qWarning("%d %d %d %d", rt.left(), rt.top(), rt.right(), rt.bottom());
                if(rt.contains(p2))
                {
                    pw = p;
                    break;
                }
            }

            if(pw)
            {
                qWarning("pwpwwp");
                pw2 = pw;
                if(d->canConnect(pw1->point, pw2->point))
                {
                    pw2->setDrawType(1);
                    pw2->update();
                }
            }
            else
            {
                if(pw2)
                {
                    if(pw1 != pw2)
                        pw2->setDrawType(0);
                    pw2=0;
                }
            }

        }
        return 1;
    }
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *me = (QMouseEvent*)e;
        if(state == MOVING)
        {
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

            state = NONE;
        }
        else if(state == LINING)
        {
            if(pw1 && pw2)
                d->addConnection(pw1->point,pw2->point);
            if(pw1)
            {
                pw1->setDrawType(0);
                pw1=0;
            }
            if(tmpw)
                tmpw->deleteLater();
            tmpw=0;
            state = NONE;
        }

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
    if(!wp->toGrid(dp).isNull())
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

void WorkPanel::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu mn;
    mn.addAction("saf");
    mn.exec(ev->globalPos());
}

void LiningWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.drawLine(wp->p1, wp->p2);
}

void WorkPanel::keyPressEvent(QKeyEvent * ev)
{
    if(ev->key() == Qt::Key_Delete)
    {
        foreach(ElementWidget *ew, selected)
        {
            Element *e = ew->e;
            foreach(int i, e->in)
            {
                d->removePoint(i);
                points[i]->deleteLater();
                points.remove(i);
            }
            foreach(int i, e->out)
            {
                d->removePoint(i);
                points[i]->deleteLater();
                points.remove(i);
            }

            d->elements.remove(e);
            d->c->removeFromQueue(e);
            elementWidgets.remove(ew);
            ew->deleteLater();
            delete e;
        }

        selected.clear();
        d->calcIfNeed();

        update();
    }
}

void WorkPanel::setAddingElement(Element *e)
{
    selected.clear();
    adding = e;
    tmpw = new AddingWidget(this, this, e->_view.width*grid_size, e->_view.height*grid_size, e);
    tmpw->setGeometry(0, 0, width(), height());
    tmpw->show();
    connect(tmpw, SIGNAL(addElement(Element*)), d, SLOT(addElement(Element*)));
}

AddingWidget::AddingWidget(QWidget *parent, WorkPanel*wp, int wd, int h, Element*e) : QWidget(parent), wp(wp), width(wd), height(h)
{
    setMouseTracking(1);
    tmp = new ElementWidget();
    tmp->setElement(e);
    tmp->updateSize();
    grabKeyboard();
    //setFocusPolicy(Qt::ClickFocus);
}

void AddingWidget::mouseMoveEvent(QMouseEvent *ev)
{
    setFocus();
    if(ev->button() == Qt::NoButton)
    {
        pt = ev->pos();
        update();
    }
}

void AddingWidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() != Qt::LeftButton)
    {
        wp->stopAdding();
    }
    else
    {
        if(wp->canMoveTo(tmp, wp->toGrid(pt-QPoint(20, 10)) ))
        {
            QPoint tm = wp->toGrid(pt - QPoint(20, 10));
            wp->adding->_view.x = tm.x()/grid_size;
            wp->adding->_view.y = tm.y()/grid_size;
            emit addElement(wp->adding);
            wp->adding = wp->adding->clone();
        }
    }
}

void AddingWidget::mouseReleaseEvent(QMouseEvent *ev)
{

}

void AddingWidget::paintEvent(QPaintEvent *)
{
    if(!mouseIn)
        return;
    QPainter p(this);
    QRect rt = tmp->geometry();
    rt.moveTopLeft(wp->toGrid(pt - QPoint(20, 10)));

    if(wp->canMoveTo(tmp, rt.topLeft()))
        p.setBrush(QColor(0, 255, 0, 100));
    else
        p.setBrush(QColor(255, 0, 0, 100));

    p.drawRect(rt);
}

void WorkPanel::stopAdding(int type)
{
    tmpw->deleteLater();
    tmpw=0;
    if(!type)
    {
        d->setInstrument(Document::SELECT);
        emit d->instrumentChanged();
    }
    delete adding;
    adding = 0;
}

AddingWidget::~AddingWidget()
{
    releaseKeyboard();
    delete tmp;
}

void AddingWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
        wp->stopAdding();
}
