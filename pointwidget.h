#ifndef POINTWIDGET_H
#define POINTWIDGET_H

#include <QWidget>

#include "classes.h"

class PointWidget : public QWidget
{
    Q_OBJECT

    friend class WorkPanel;
    WorkPanel *panel;

protected:
    int drawType;
    int point;
public:
    explicit PointWidget(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);
    void setDrawType(int dt);
signals:

public slots:

};

#endif // POINTWIDGET_H
