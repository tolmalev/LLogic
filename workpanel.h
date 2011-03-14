#ifndef WORKPANEL_H
#define WORKPANEL_H

#include "classes.h"

#include <QWidget>
#include <QMap>
#include <QMouseEvent>

class WorkPanel : public QWidget
{
    Q_OBJECT

    friend class Document;
    QMap<int, PointWidget*> points;
protected:
    Document *d;

public:
    explicit WorkPanel(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

    void addElement(Element* e);

    void mouseMoveEvent(QMouseEvent *);

    Document *document(){return d;}

signals:
    void doubleClicked(ElementWidget*);
    void needCalculation(Element*);
public slots:

};

#endif // WORKPANEL_H
