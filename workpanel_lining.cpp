#include "workpanel.h"
#include "document.h"
#include "element.h"
#include "elementwidget.h"
#include "pointwidget.h"
#include "controller.h"

#include <QPainter>

void WorkPanel::calculateLines()
{

}

void WorkPanel::drawLines(QPainter &painter)
{
    if(d != 0)
    foreach(int k, d->c->connections.keys())
    {
	foreach(int b, *d->c->connections[k])
	{
	    if(k < b)
	    {
		if(points.find(k) != points.end() && points.find(b) != points.end())
		{
		    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
		    QPoint p2 = toGrid(points[b]->mapTo(this, QPoint(3, 3)));
		    painter.drawLine(p1, p2);
		}
	    }
	}
    }
}
