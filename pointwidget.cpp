#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include "workpanel.h"
#include "pointwidget.h"

PointWidget::PointWidget(QWidget *parent) :
    QWidget(parent)
{
    setGeometry(0, 0, 4, 5);
    setAutoFillBackground(0);
    setMouseTracking(1);
    drawType = 0;
}

void PointWidget::paintEvent(QPaintEvent * ev)
{
    QPainter p(this);
    p.setPen(QColor(132, 2, 4));
    p.setBrush(QColor(132, 2, 4));

    //p.drawEllipse(0, 1, 3, 4);
    p.fillRect(0, 0, 100, 100, Qt::black);

    if(drawType == 1)
    {
        p.setPen(QColor(0, 255, 0, 200));
        p.setBrush(Qt::transparent);
        p.drawEllipse(0, 1, 3, 4);
    }
}

void PointWidget::setDrawType(int dt)
{
    drawType = dt;
}
