#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include "workpanel.h"
#include "pointwidget.h"

PointWidget::PointWidget(QWidget *parent) :
    QWidget(parent)
{
    setGeometry(0, 0, 6, 6);
    setAutoFillBackground(0);
    setMouseTracking(1);
    drawType = 0;
}

void PointWidget::paintEvent(QPaintEvent * ev)
{
    QPainter p(this);
    p.setPen(QColor(132, 2, 4));
    p.setBrush(QColor(132, 2, 4));

    p.drawEllipse(0, 0, width()-2, height()-2);

    if(drawType == 1)
    {
        p.setPen(QColor(0, 255, 0, 200));
        p.setBrush(QColor(0, 255, 0, 200));//Qt::transparent);
        p.drawEllipse(0, 0, width()-1, height()-1);
    }
    if(drawType == 2)
    {
	p.setPen(QColor(255, 0, 0, 255));
	p.setBrush(QColor(0, 255, 0, 200));//Qt::transparent);
	p.drawEllipse(0, 0, width()-1, height()-1);
    }
    ev->accept();
}

void PointWidget::setDrawType(int dt)
{
    drawType = dt;
}
