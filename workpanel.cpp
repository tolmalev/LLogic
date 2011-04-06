#include <QPainter>
#include <QPixmap>
#include <QLayout>
#include <QMenu>
#include <QApplication>
#include <QKeyEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

#include "simpleelements.h"

#include <cmath>

using namespace std;


#include "workpanel.h"
#include "element.h"
#include "complexelement.h"
#include "elementwidget.h"
#include "mainwindow.h"
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
    setAcceptDrops(1);
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
            pw->point = ce->in_connections.at(i).second;
            points[ce->in_connections.at(i).second] = pw;
        }
        for(int i = 0; i < ce->out_cnt; i++)
        {
            PointWidget * pw = new PointWidget(this);
            pw->show();
            pw->installEventFilter(this);
            pw->panel = this;
            pw->move(40*grid_size, (i+1)*grid_size*2-3);
            pw->point = ce->out_connections.at(i).first;
            points[ce->out_connections.at(i).first] = pw;
        }
    }
    d = 0;

    acomplex = new QAction("Create complex element", this);
    acomplex->setShortcut(QKeySequence("Ctrl+Shift+C"));
    abuildtable = new QAction("Build the thuth table", this);

    alibrary = new QAction("Add this element to the library", this);
    connect(acomplex, SIGNAL(triggered()), this, SLOT(createComplex()));
    connect(alibrary, SIGNAL(triggered()), this, SLOT(addToLibrary()));
    connect(abuildtable, SIGNAL(triggered()), this, SLOT(buildTable()));
}

QPoint WorkPanel::toGrid(QPoint a)
{
    return QPoint((int)( (double)a.x() / grid_size + 0.5)*grid_size,
                  (int)( (double)a.y() / grid_size + 0.5)*grid_size);
}

void WorkPanel::paintEvent(QPaintEvent * ev)
{
    QPainter painter(this);
    QPixmap p = MainWindow::wnd->pixmap(":/images/background.png");
    int w = geometry().width();
    int h = geometry().height();
    painter.fillRect(0, 0, w, h, p);

    painter.setPen(Qt::black);

    drawLines(painter);

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
    foreach(PointWidget *pw, selectedFreePoints)
    {
        QRect rt = pw->geometry();
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
    ev->accept();
}

void WorkPanel::addElement(Element *e)
{
    ElementWidget *ew;
    switch(e->type())
    {
	case SEND:      ew = new SendElementWidget(this);	    break;
	case RECEIVE:   ew = new ReceiveElementWidget(this);	    break;
	case NUMSEND:	ew = new NumberSendElement8Widget(this);    break;
	default:        ew = new ElementWidget(this);		    break;
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
        pw->move(p.x()-6, p.y()-2);
        points[e->out.at(i)] = pw;
        pw->point = e->out.at(i);
    }
    if(tmpw)
        tmpw->raise();
    updateMinimumSize();
    calculateLines();
    update();
}

void WorkPanel::addPoint(int p, QPoint pos)
{
    PointWidget *pw = new PointWidget(this);
    pw->show();
    pw->installEventFilter(this);
    pw->panel = this;

    pw->move(pos.x()*grid_size-2, pos.y()*grid_size-2);
    points[p] = pw;
    pw->point = p;
    freePoints.insert(pw);
    if(tmpw)
        tmpw->raise();
    update();
}

void WorkPanel::mouseMoveEvent(QMouseEvent *ev)
{
    //qDebug("workpanel mouse move %d %d", ev->x(), ev->y());
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
        //qDebug("%s", w->metaObject()->className());
    }
    if(tmpw)
    {
        int minx = min(ev->x(), p1.x());
        int miny = min(ev->y(), p1.y());
        int maxx = max(ev->x(), p1.x());
        int maxy = max(ev->y(), p1.y());

        tmpw->setGeometry(minx, miny, maxx-minx, maxy-miny);

        //qDebug("%d %d %d %d", minx, miny, maxx, maxy);

    }
    ev->accept();
}

void WorkPanel::mousePressEvent(QMouseEvent *ev)
{
    setFocus();
    //qDebug("mouse pressed");
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
                selectedFreePoints.clear();
            }
        }
    }
    ev->accept();
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
        {
            selected.clear();
            selectedFreePoints.clear();
        }
        foreach(ElementWidget* ew, elementWidgets)
        {
            if(rt.intersect(ew->geometry()) == ew->geometry() || rt.intersect(ew->geometry()) == rt)
                selected.insert(ew);
        }
        foreach(PointWidget* ew, freePoints)
        {
            if(rt.intersect(ew->geometry()) == ew->geometry() || rt.intersect(ew->geometry()) == rt)
                selectedFreePoints.insert(ew);
        }
        update();
    }
    ev->accept();
}

void SelectWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    p.setPen(QColor(53, 103, 255, 255));
    p.fillRect(0, 0, width()-1, height()-1, QColor(176, 204, 241, 100));
    p.drawRect(0, 0, width()-1, height()-1);
    ev->accept();
}

bool WorkPanel::eventFilter(QObject *o, QEvent *e)
{
    //setFocus();
    if(e->type() == QEvent::MouseButtonDblClick)
    {
        if(o->inherits("PointWidget"))
            return 1;
    }
    else if(e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *me = (QMouseEvent*)e;
	if(me->button() == Qt::LeftButton || me->button() == Qt::MiddleButton)
        {
	    if(me->button() == Qt::MiddleButton)
		midButton = 1;
	    else
		midButton = 0;
            if(state == NONE)
            {
                if(o->inherits("ElementWidget"))
                {
                    ElementWidget *ew = (ElementWidget*)o;
                    if(selected.find(ew) == selected.end())
                    {
                        if(qApp->keyboardModifiers() != Qt::ControlModifier)
                        {
                            selected.clear();
                            selectedFreePoints.clear();
                        }
                        selected.insert(ew);
                    }
                    p2 = p1 = ew->mapTo(this, me->pos());
		    tmpw = new MovingWidget(this, this);
                    tmpw->show();
		    tmpw->setGeometry(0, 0, width(), height());
		    tmpw->setAttribute(Qt::WA_TransparentForMouseEvents);
		    state = MOVING;
                }
                else if(o->inherits("PointWidget"))
                {
                    bool move = 0;
                    if(!selectedFreePoints.empty())
                    {
                        PointWidget *pw = (PointWidget*)o;
                        if(selectedFreePoints.find(pw) != selectedFreePoints.end())
                        {
                            p2 = p1 = pw->mapTo(this, me->pos());
			    tmpw = new MovingWidget(this, this);
                            tmpw->show();
			    tmpw->setGeometry(0, 0, width(), height());
                            state = MOVING;
                            move=1;
                        }
                    }
                    if(!move)
                    {
                        PointWidget *pw = (PointWidget*)o;
			pw2 = pw1 = (PointWidget*)o;
                        state = LINING;
			tmpw = new LiningWidget(this, this);
                        tmpw->show();
                        p2 = p1 = toGrid(pw->mapTo(this, me->pos()));
			tmpw->setGeometry(0, 0, width(), height());
                    }
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
		PointWidget *pw3 = (PointWidget*)o;
		if(pw1 && pw1 != pw3)
		{
		    pw1->setDrawType(0);
		    pw1->update();
		}
		pw1=pw3;
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
            PointWidget *pw = 0;
            foreach(PointWidget*p, points)
            {
                QRect rt = p->geometry();
                rt.setTopLeft( p->mapTo(this, QPoint(0, 0)) );
                rt.setWidth(p->geometry().width());
		rt.setHeight(p->geometry().height());
                if(rt.contains(p2))
                {
                    pw = p;
                    break;
                }
            }

            if(pw)
	    {
		if(pw2)
		    pw2->setDrawType(0);
                pw2 = pw;
		if(d->canConnect(pw1->point, pw2->point) || midButton)
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
            if(o->inherits("QWidget"))
            {
                QWidget *ew = (QWidget*)o;
                p2 = ew->mapTo(this, me->pos());
                bool can = 1;
		bool shift = qApp->keyboardModifiers() == Qt::ShiftModifier || midButton;
                foreach(ElementWidget *ew, selected)
                {
		    if(!canMoveTo(ew, toGrid(ew->pos()+p2-p1), !shift))
                    {
                        can = 0;
                        break;
                    }
                }
                foreach(PointWidget *ew, selectedFreePoints)
                {
		    if(!canMoveTo(ew, toGrid(ew->pos()+p2-p1), !shift))
                    {
                        can = 0;
                        break;
                    }
                }

                if(can)
		{
		    QSet<Element*> els;
		    QSet<int> pts;
		    QPoint dr;
		    foreach(ElementWidget *ew, selected)
		    {
			els.insert(ew->e);
			dr = toGrid(  ew->geometry().topLeft() + p2 -p1) - ew->pos();
		    }
		    foreach(PointWidget *pw, selectedFreePoints)
		    {
			pts.insert(pw->point);
			dr = toGrid(  pw->geometry().topLeft() + QPoint(2,2)+ p2 -p1) - pw->pos() - QPoint(2,2);
		    }
		    dr /= grid_size;
		    if(!shift)
			d->move(dr, els, pts);
		    else
			d->clone(dr, els, pts);
		    updateMinimumSize();
                }
	    }

            if(tmpw)
		tmpw->deleteLater();
            tmpw=0;
	    update();
	    state = NONE;
	    setFocus();
        }
        else if(state == LINING)
	{
            if(pw1 && pw2)
	    {
		if(!midButton)
		    d->addConnection(pw1->point,pw2->point);
		else
		    d->removeConnection(pw1->point,pw2->point);
	    }
            if(pw1)
            {
                pw1->setDrawType(0);
		pw1->update();
                pw1=0;
            }
            if(tmpw)
		tmpw->deleteLater();
            tmpw=0;
            state = NONE;
        }
	if(pw1)
	{
	    pw1->setDrawType(0);
	    pw1=0;
	}
	if(pw2)
	{
	    pw2->setDrawType(0);
	    pw2=0;
	}
	calculateLines();

	return 1;
    }

    return QWidget::eventFilter(o, e);
}

bool WorkPanel::canMoveTo(QWidget *e, QPoint p, bool sel)
{
    if(p.x() < 0 || p.y() < 0)
        return 0;
    QRect rt = e->geometry();
    rt.moveTopLeft(p);
    foreach(ElementWidget *ew, elementWidgets)
    {
	if(selected.find(ew) == selected.end() || !sel)
        {
            if(!(rt.intersect(ew->geometry()).isNull()))
                return 0;
        }
    }
    foreach(PointWidget *ew, freePoints)
    {
        if(selectedFreePoints.find(ew) == selectedFreePoints.end())
        {
            if(!(rt.intersect(ew->geometry()).isNull()))
                return 0;
        }
    }
    return 1;
}

void MovingWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPoint dp = wp->p2-wp->p1;
    //qDebug("dp = %d %d", dp.x(), dp.y());
    if(wp->toGrid(dp).isNull())
        return;

    bool shift = qApp->keyboardModifiers() == Qt::ShiftModifier || qApp->mouseButtons() == Qt::MidButton;
    foreach(ElementWidget *ew, wp->selected)
    {
        QRect rt;
        rt.setTopLeft(wp->toGrid(ew->pos() + dp));
        rt.setWidth(ew->width());
        rt.setHeight(ew->height());

	if(wp->canMoveTo(ew, rt.topLeft(), !shift))
            p.fillRect(rt, QColor(0, 255, 0, 100));
        else
            p.fillRect(rt, QColor(255, 0, 0, 100));
        p.drawRect(rt);
    }
    foreach(PointWidget *ew, wp->selectedFreePoints)
    {
        QRect rt;
	rt.setTopLeft(wp->toGrid(ew->pos()+QPoint(2,2) + dp)-QPoint(2,2));
        rt.setWidth(ew->width());
        rt.setHeight(ew->height());

	if(wp->canMoveTo(ew, rt.topLeft(), !shift))
            p.setBrush(QColor(0, 255, 0, 100));
        else
            p.setBrush(QColor(255, 0, 0, 100));
        p.drawEllipse(rt);
    }
}

void WorkPanel::contextMenuEvent(QContextMenuEvent *ev)
{
    if(!selected.empty())
    {
        QWidget *w = childAt(ev->pos());
	QMenu mn;
        if(!w)
            return;
        if((w->inherits("ElementWidget") && selected.find((ElementWidget*)w) != selected.end()) ||
           (w->inherits("PointWidget") && selectedFreePoints.find((PointWidget*)w) != selectedFreePoints.end()))
        {
	    mn.addAction(acomplex);
	    if(w->inherits("ElementWidget") && selectedFreePoints.empty() && selected.count() == 1 && (*selected.begin())->e->type() == COMPLEX)
	    {
		mn.addAction(alibrary);
		mn.addAction(abuildtable);
	    }
        }

	mn.exec(ev->globalPos());
    }
}

void LiningWidget::paintEvent(QPaintEvent *ev)
{
    QPainter p(this);
    if(wp->midButton)
    {
	QPen pen;
	pen.setColor(QColor(255, 0, 0, 200));
	pen.setWidth(2);
	p.setPen(pen);
    }
    p.drawLine(wp->p1, wp->p2);
}

void WorkPanel::keyPressEvent(QKeyEvent * ev)
{
    if(ev->key() == Qt::Key_Delete)
    {
	QSet<Element*> els;
	QSet<int> pts;
	foreach(ElementWidget *ew, selected)
	    els.insert(ew->e);
	foreach(PointWidget *pw, selectedFreePoints)
	    pts.insert(pw->point);
	d->remove(els, pts);

	selected.clear();
	selectedFreePoints.clear();
    }
    else if(ev->key() == Qt::Key_C)
    {
	if(qApp->keyboardModifiers() == Qt::ControlModifier)
	{
	    QSet<Element*> elements;
	    QSet<int> _points;
	    foreach(ElementWidget *ew, selected)
		elements.insert(ew->e);
	    foreach(PointWidget *pw, selectedFreePoints)
		_points.insert(pw->point);
	    d->addToClipboard(elements, _points);
	}
    }
    else if(ev->key() == Qt::Key_V)
    {
	if(qApp->keyboardModifiers() == Qt::ControlModifier)
	{
	    d->addFromClipboard();
	    calculateLines();
	}
    }
}

void WorkPanel::setAddingElement(Element *e)
{
    selected.clear();
    selectedFreePoints.clear();
    adding = e;
    tmpw = new AddingWidget(this, this, e->_view.width*grid_size, e->_view.height*grid_size, e);
    tmpw->setGeometry(0, 0, width(), height());
    tmpw->show();
    connect(tmpw, SIGNAL(addElement(Element*)), d, SLOT(addElement(Element*)));
    connect(tmpw, SIGNAL(stopAdding()), this, SLOT(stopAdding()));
}

AddingWidget::AddingWidget(QWidget *parent, WorkPanel*wp, int wd, int h, Element*e, bool addp) : QWidget(parent), wp(wp), width(wd), height(h), addingPoint(addp)
{
    qWarning("Adding created");
    setMouseTracking(1);
    setAcceptDrops(1);
    mouseIn=0;

    if(addp)
    {
        tmp = new PointWidget();
    }
    else
    {
        tmp = new ElementWidget();
        ((ElementWidget*)tmp)->setElement(e);
        ((ElementWidget*)tmp)->updateSize();
    }
}

void AddingWidget::mouseMoveEvent(QMouseEvent *ev)
{
    setFocus();
    if(ev->button() != Qt::LeftButton)
    {
        pt = ev->pos();
        update();
    }
    else
    {
        pt = ev->pos();
    }
}

void AddingWidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() != Qt::LeftButton)
    {
        emit stopAdding();
    }
}

void AddingWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(!addingPoint)
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
    else
    {
        if(wp->canMoveTo(tmp, wp->toGrid(pt)-QPoint(3, 3) ))
        {
            QPoint tm = wp->toGrid(pt);
            emit addPoint(QPoint(tm.x()/grid_size, tm.y()/grid_size));
        }
    }
}

void AddingWidget::paintEvent(QPaintEvent *ev)
{
    if(!mouseIn)
	return;
    QPainter p(this);
    QRect rt = tmp->geometry();
    if(!addingPoint)
    {
        rt.moveTopLeft(wp->toGrid(pt - QPoint(20, 10)));
    }
    else
    {
        rt.moveTopLeft(wp->toGrid(pt) - QPoint(2,2));
    }
    if(wp->canMoveTo(tmp, rt.topLeft()))
        p.setBrush(QColor(0, 255, 0, 100));
    else
        p.setBrush(QColor(255, 0, 0, 100));
    if(!addingPoint)
        p.drawRect(rt);
    else
        p.drawEllipse(rt);
}

void WorkPanel::stopAdding(int type)
{
    tmpw->deleteLater();
    if(d->instrument == Document::ADDELEMENT)
    {
        delete adding;
        adding = 0;
    }
    tmpw=0;
    if(!type)
    {
        d->setInstrument(Document::SELECT);
        emit d->instrumentChanged();
    }
}

AddingWidget::~AddingWidget()
{
    releaseKeyboard();
    delete tmp;
}

void AddingWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
        emit stopAdding();
}

void WorkPanel::startAddingPoint()
{
    selected.clear();
    selectedFreePoints.clear();
    tmpw = new AddingWidget(this, this, 0, 0, 0, 1);
    tmpw->setGeometry(0, 0, width(), height());
    tmpw->show();
    connect(tmpw, SIGNAL(addPoint(QPoint)), d, SLOT(addPoint(QPoint)));
    connect(tmpw, SIGNAL(stopAdding()), this, SLOT(stopAdding()), Qt::QueuedConnection);
}

void WorkPanel::createComplex()
{
    QSet<Element*> elements;
    QList<int>      _points;
    foreach(ElementWidget* ew, selected)
    {
        elements.insert(ew->element());
        foreach(int i, ew->element()->in)
            points.remove(i);
        foreach(int i, ew->element()->out)
            points.remove(i);
        elementWidgets.remove(ew);
        ew->deleteLater();
    }
    foreach(PointWidget* ew, selectedFreePoints)
    {
        _points.push_back(ew->point);
        freePoints.remove(points[ew->point]);
        points.remove(ew->point);
        ew->deleteLater();
    }

    selected.clear();
    selectedFreePoints.clear();
    d->createComplex(elements, _points);
}

void WorkPanel::addToLibrary()
{
    if(selected.count() > 1 || selectedFreePoints.count() > 0)
	return;
    Element *e = (*selected.begin())->e;
    if(e->type() != COMPLEX)
	return;
    ComplexElement *ce = (ComplexElement*)e;
    QString s = QInputDialog::getText(this, "Name", "Input the name of the element", QLineEdit::Normal, ce->text);
    if(s != "")
    {
	if(d->addLibraryElement(s, ce))
	    QMessageBox::critical(this, "error", "There id alreqdy elements with name " + s + " in the library");
	else
	{
	    ce->text = s;
	    (*selected.begin())->setToolTip(s);
	}
    }
}


void WorkPanel::resizeEvent(QResizeEvent *ev)
{
    if(tmpw)
	tmpw->setGeometry(0, 0, ev->size().width(), ev->size().height());

    if(ce != 0)
    {
	panel_type = ELEMENT;
	for(int i = 0; i < ce->out_cnt; i++)
	{
	    PointWidget * pw =  points[ce->out_connections.at(i).first];
	    pw->move(ev->size().width()/grid_size*grid_size-2, (i+1)*grid_size*2-2);
	}
    }
}

void WorkPanel::updateMinimumSize()
{
    int mw = 0;
    int mh = 0;
    foreach(ElementWidget*ew, elementWidgets)
    {
	mw = std::max(mw, ew->geometry().right()+2*grid_size);
	mh = std::max(mh, ew->geometry().bottom()+2*grid_size);
    }
    foreach(PointWidget*ew, freePoints)
    {
	mw = std::max(mw, ew->geometry().right()+2*grid_size);
	mh = std::max(mh, ew->geometry().bottom()+2*grid_size);
    }
    if(ce)
    {
	mh = max(mh, (ce->in_cnt+1)*3*grid_size);
    }
    setMinimumWidth(mw);
    setMinimumHeight(mh);
}

void WorkPanel::setSelection(QSet<Element*> els, QSet<int> pts)
{
    selected.clear();
    selectedFreePoints.clear();

    foreach(ElementWidget * ew, elementWidgets)
	if(els.find(ew->e) != els.end())
	    selected.insert(ew);
    foreach(PointWidget *pw, points)
	if(pts.find(pw->point) != pts.end())
	    selectedFreePoints.insert(pw);

    update();
    setFocus();
}

void WorkPanel::buildTable()
{
    if(selected.count() > 1 || selectedFreePoints.count() > 0)
	return;
    Element *e = (*selected.begin())->e;
    if(e->type() != COMPLEX)
	return;
    ComplexElement *ce = (ComplexElement*)e;
    if(ce->in_cnt > 10)
    {
	QMessageBox::critical(this, "error", "This element has too many input points to build the truth table");
	return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Locic files (*.lod)");
    if(fileName != "")
    {
	ce->buildTable(fileName);
    }
}

void WorkPanel::dragEnterEvent(QDragEnterEvent * ev)
{
    qDebug() << ev->mimeData()->data("text/uri-list");
    if(ev->mimeData()->hasFormat("LLogic/element"))
    {
	QDomDocument doc;
	QByteArray ba = ev->mimeData()->data("LLogic/element");
	if(doc.setContent(ba))
	{
	    adding = Element::fromXml(doc.firstChildElement());
	    if(!adding)
		return;
	    if(tmpw)
	    {
		tmpw->deleteLater();
		tmpw = 0;
	    }
	    tmpw = new AddingWidget(this, this, 3, 3, adding);
	    tmpw->setGeometry(0, 0, width(), height());
	    tmpw->show();
	    tmpw->setAttribute(Qt::WA_TransparentForMouseEvents);
	    tmpw->raise();
	    ev->acceptProposedAction();
	}
    }
}

void WorkPanel::dragLeaveEvent(QDragLeaveEvent *ev)
{
    if(adding)
    {
	delete adding;
	adding = 0;
    }
    if(tmpw)
    {
	tmpw->deleteLater();
	tmpw = 0;
    }
    qWarning("drag leave");
    ev->accept();
}

void WorkPanel::dragMoveEvent(QDragMoveEvent *ev)
{
    if(tmpw)
	if(tmpw->inherits("AddingWidget"))
	{
	    ((AddingWidget*)tmpw)->pt = ev->pos();
	    ((AddingWidget*)tmpw)->mouseIn = 1;
	    tmpw->update();
	    update();
	}
    ev->acceptProposedAction();
}

void WorkPanel::dropEvent(QDropEvent *ev)
{
    if(tmpw)
    {
	qWarning("drop");
	if(!tmpw->inherits("AddingWidget"))
	{
	    qWarning("bred");
	    return;
	}
	AddingWidget* aw =  (AddingWidget*)tmpw;
	if(canMoveTo(aw->tmp, toGrid(aw->pt-QPoint(20, 10)) ))
	{
	    QPoint tm = toGrid(aw->pt - QPoint(20, 10));
	    adding->_view.x = tm.x()/grid_size;
	    adding->_view.y = tm.y()/grid_size;
	    d->addElement(adding);
	}
	else
	{
	    delete adding;
	}
	adding = 0;


	tmpw->deleteLater();
	tmpw = 0;
    }
    ev->acceptProposedAction();
}

void AddingWidget::dragEnterEvent(QDragEnterEvent *ev)
{
    qWarning("adding drag");
    ev->ignore();
}

void WorkPanel::move(QPoint dr, QSet<Element *> els, QSet<int> pts)
{
    foreach(int i, pts)
	points[i]->move(dr*grid_size + points[i]->pos());
    foreach(ElementWidget *ew, elementWidgets)
	if(els.find(ew->e) != els.end())
	    ew->move(ew->pos() + dr*grid_size);
}

void WorkPanel::remove(QSet<Element *> els, QSet<int> pts)
{
    foreach(int i, pts)
    {
	selectedFreePoints.remove(points[i]);
	PointWidget *pw = points[i];
	points.remove(i);
	freePoints.remove(pw);
	pw->deleteLater();
    }
    foreach(ElementWidget *ew, elementWidgets)
	if(els.find(ew->e) != els.end())
	{
	    selected.remove(ew);
	    Element *e = ew->e;
	    foreach(int i, e->in)
	    {
		points[i]->deleteLater();
		points.remove(i);
	    }
	    foreach(int i, e->out)
	    {
		points[i]->deleteLater();
		points.remove(i);
	    }
	    elementWidgets.remove(ew);
	    ew->deleteLater();
	}

    calculateLines();
    update();
}
