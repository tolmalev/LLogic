#include "elementwidget.h"
#include <QPainter>
#include "math.h"
#include "element.h"
#include "simpleelements.h"
#include "mainwindow.h"
#include <QPainter>
#include <QGraphicsTextItem>
#include <QLineEdit>

ElementWidget::ElementWidget(QWidget *parent) :
    QWidget(parent)
{
    drawType=0;
    setMouseTracking(1);
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
    QString toolT;
    switch(e->type())
    {
	case AND:	toolT=QString::fromAscii("And"); break;
	case NOT:	toolT=QString::fromAscii("Not"); break;
	case OR:	toolT=QString::fromAscii("Or"); break;
	case ANDNOT:    toolT=QString::fromAscii("And-Not"); break;
	case ORNOT:	toolT=QString::fromAscii("Or-Not"); break;
	case XOR:	toolT=QString::fromAscii("Xor"); break;
	case COMPLEX:	toolT=e->text; break;
    }
    setToolTip(toolT);
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

    //if(e->type() != COMPLEX)
    {
	QRect rt(grid_size, 2, w-1-grid_size*2, h-5);
	painter.setFont(QFont("Arial", 10));
	painter.drawText(rt, Qt::AlignCenter | Qt::TextWrapAnywhere, e->text);
    }
    event->accept();
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

void ElementWidget::mouseDoubleClickEvent(QMouseEvent * ev)
{
    emit doubleClicked(this);
    ev->accept();
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

    QPixmap p = MainWindow::wnd->pixmap(":/images/element_background.png");
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

    event->accept();
}

void SendElementWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ((SendElement*)e)->val ^= 1;
    emit needCalculation(e);
    update();
    ev->accept();
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

    QPixmap p = MainWindow::wnd->pixmap(":/images/element_background.png");
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

    event->accept();
}

void NumberSendElement8Widget::updateSize()
{
    setGeometry(e->view().x*grid_size, e->view().y*grid_size, (e->view().width + 1) * grid_size, e->view().height* grid_size);
}

void NumberSendElement8Widget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if(lineEdit == 0)
    {
	int num = ((NumberSendElement8*)e)->num;
	lineEdit = new QLineEdit(QString::number(num), this);
	lineEdit->selectAll();
	connect(lineEdit, SIGNAL(editingFinished ()), this, SLOT(numberChanged()));
	lineEdit->show();
    }
    lineEdit->setFocus();
    ev->accept();
}

void NumberSendElement8Widget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    NumberSendElement8* el = (NumberSendElement8*)e;

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

    painter.setFont(QFont("Arial", 12));
    painter.drawText(15, 47, QString::number(el->num));

    event->accept();
}

void NumberSendElement8Widget::numberChanged()
{
    QString n = lineEdit->text();
    bool ok = 0;
    int nn = n.toInt(&ok);
    if(!ok)
	return;
    if(nn < 0)
	nn = 0;
    if(nn > 255)
	nn = 255;
    ((NumberSendElement8*)e)->num = nn;
    e->text = ""+nn;
    emit needCalculation(e);
    lineEdit->deleteLater();
    lineEdit = 0;
}
