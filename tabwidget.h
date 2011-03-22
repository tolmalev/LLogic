#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "classes.h"


#include <QTabWidget>
#include <QPushButton>


class TabWidget : public QTabWidget
{
    Q_OBJECT

    int num;
    MainWindow * mw;
    QPushButton *pb;
public:
    explicit TabWidget(QWidget *parent = 0);
    void setMainWindow(MainWindow *m);

signals:

public slots:

};

#endif // TABWIDGET_H
