#include "elementwidget.h"
#include <QPainter>
#include "math.h"
#include "element.h"
#include "simpleelements.h"
#include <QPainter>

ElementWidget::ElementWidget(QWidget *parent) :
    QWidget(parent)
{
    //setAutoFillBackground(1);
    setGeometry(0, 0, 50, 50);

    setMouseTracking(1);
    drawType=0;

    moving = 0;

    timer.setInterval(500);
    timer.setSingleShot(1);
    timer.stop();
    connect(&timer, SIGNAL(timeout()), this, SLOT(changeDrawType()));
}

QPoint ElementWidget::getPointPos(int type, int n)
{
    int x = (type) * geometry().width();
    int y = (n+1)*grid_size;
    return QPoint(x, y);
}

void ElementWidget::updateSize()
{
    setGeometry(e->view().x*grid_size, e->view().y*grid_size, (e->view().width + 2) * grid_size, e->view().height* grid_size);
}

void ElementWidget::setElement(Element *_e)
{
    e = _e;
    updateSize();
}

void ElementWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    int w = geometry().width();
    int h = geometry().height();

    QPixmap p(":/images/element_background.png");
    painter.setPen(QColor(132, 2, 4));
    painter.fillRect(grid_size, 0, w-1-grid_size*2, h-1, p);
    painter.drawRect(grid_size, 0, w-1-grid_size*2, h-1);
    for(int i = 0; i < e->in_cnt; i++)
    {
        int y = (i+1)*grid_size;
        painter.drawLine(0, y, grid_size, y);
    }
    for(int i = 0; i < e->out_cnt; i++)
    {
        int y = (i+1)*grid_size;
        painter.drawLine(w-1-grid_size, y, w-1, y);
    }
}

void ElementWidget::mousePressEvent(QMouseEvent * ev)
{
    timer.stop();
    setDrawType(0);
    px = ev->x();
    py = ev->y();
    moving = 1;
}

void ElementWidget::mouseReleaseEvent(QMouseEvent * ev)
{
    timer.stop();
    setDrawType(0);
    moving = 0;
}

int min(int a, int b)
{
    if(a<b)
        return a;
    return b;
}

int max(int a, int b)
{
    if(a>b)
        return a;
    return b;
}

void ElementWidget::mouseMoveEvent(QMouseEvent * ev)
{
    if(moving)
    {
        e->_view.x = (int)( (double)max(0, x()+ev->x()-px) / grid_size + 0.5);
        e->_view.y = (int)( (double)max(0, y()+ev->y()-py) / grid_size + 0.5);

        move(e->_view.x*grid_size, e->_view.y*grid_size);

        parentWidget()->update();
    }
}

void ElementWidget::mouseDoubleClickEvent(QMouseEvent * ev)
{
    emit doubleClicked(this);
}

void ElementWidget::enterEvent(QEvent *)
{
    timer.start();
}

void ElementWidget::leaveEvent(QEvent *)
{
    timer.stop();
    setDrawType(0);
}

void ElementWidget::changeDrawType()
{
    setDrawType(1);
}

void ElementWidget::setDrawType(int dt)
{
    if(drawType != dt)
    {
        drawType = dt;
        update();
    }
}




void SendElementWidget::updateSize()
{
    setGeometry(e->view().x*grid_size, e->view().y*grid_size, (e->view().width + 1) * grid_size, e->view().height* grid_size);
}

void SendElementWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    SendElement* el = (SendElement*)e;
    int val = el->val;

    int w = geometry().width();
    int h = geometry().height();

    QPixmap p(":/images/element_background.png");
    painter.setPen(QColor(132, 2, 4));
    painter.fillRect(0, 0, w-1-grid_size, h-1, p);
    painter.drawRect(0, 0, w-1-grid_size, h-1);
    for(int i = 0; i < el->out_cnt; i++)
    {
        int y = (i+1)*grid_size;
        painter.drawLine(w-1-grid_size, y, w-1, y);
    }

    if(val == 1)
        painter.setBrush(QColor(0, 255, 0, 150));
    else
        painter.setBrush(QColor(255, 0, 0, 150));

    painter.drawEllipse(2, 2, w-1-grid_size-4, h-1-4);
}

void SendElementWidget::mouseDoubleClickEvent(QMouseEvent *)
{
    ((SendElement*)e)->val ^= 1;
    qWarning("%d", ((SendElement*)e)->val);
    emit needCalculation(e);
}

void ReceiveElementWidget::updateSize()
{
    setGeometry(e->view().x*grid_size, e->view().y*grid_size, (e->view().width + 1) * grid_size, e->view().height* grid_size);
}

void ReceiveElementWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    ReceiveElement* el = (ReceiveElement*)e;
    int val = el->val;

    int w = geometry().width();
    int h = geometry().height();

    QPixmap p(":/images/element_background.png");
    painter.setPen(QColor(132, 2, 4));
    painter.fillRect(grid_size, 0, w-1-grid_size, h-1, p);
    painter.drawRect(grid_size, 0, w-1-grid_size, h-1);
    for(int i = 0; i < el->in_cnt; i++)
    {
        int y = (i+1)*grid_size;
        painter.drawLine(0, y, grid_size, y);
    }

    if(val == 1)
        painter.setBrush(QColor(0, 255, 0, 150));
    else
        painter.setBrush(QColor(255, 0, 0, 150));

    painter.drawEllipse(grid_size + 2, 2, w-1-grid_size-4, h-1-4);
}