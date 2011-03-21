#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include "workpanel.h"
#include "pointwidget.h"

PointWidget::PointWidget(QWidget *parent) :
    QWidget(parent)
{
    setGeometry(0, 0, 4, 7);
    setAutoFillBackground(0);
    setMouseTracking(1);
    drawType = 0;
}

void PointWidget::paintEvent(QPaintEvent * ev)
{
    QPainter p(this);
    p.setPen(QColor(132, 2, 4));
    p.setBrush(QColor(132, 2, 4));

    p.drawEllipse(0, 1, 3, 4);

    if(drawType == 1)
    {
        p.setPen(QColor(0, 255, 0, 200));
        p.setBrush(Qt::transparent);
        p.drawEllipse(0, 1, 3, 4);
    }
}

void PointWidget::enterEvent(QEvent *ev)
{
    drawType = 1;
    update();
}

void PointWidget::leaveEvent(QEvent *ev)
{
    drawType = 0;
    update();
}

void PointWidget::mouseMoveEvent(QMouseEvent *ev)
{
    ev->accept();
}

void PointWidget::mousePressEvent(QMouseEvent *ev)
{
    ev->accept();
    panel->grabMouse();
}
