#include "tabwidget.h"
#include "mainwindow.h"
#include <QTabBar>
#include <QFileDialog>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setTabsClosable(1);
}

void TabWidget::setMainWindow(MainWindow *m)
{
    mw = m;
}
