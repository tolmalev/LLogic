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
    QMap<int, PointWidget*> input_points;
    QMap<int, PointWidget*> output_points;
protected:
    Document *d;
    ComplexElement *ce;

    int panel_type;
    enum types{
        DOCUMENT,
        ELEMENT,
    };

public:
    explicit WorkPanel(ComplexElement*ce = 0, QWidget *parent = 0);
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
