#include "tabwidget.h"
#include "mainwindow.h"
#include <QTabBar>
#include <QFileDialog>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(changed(int)));
    pb = new QPushButton(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton), "");
    num = 0;
}

void TabWidget::setMainWindow(MainWindow *m)
{
    mw = m;
    connect(pb, SIGNAL(clicked()), mw, SLOT(closeCurrentTab()));
}

void TabWidget::changed(int n)
{
    tabBar()->setTabButton(num, QTabBar::RightSide, 0);
    tabBar()->setTabButton(n, QTabBar::RightSide, pb);

    num = n;
}

void TabWidget::prepareClose(int n)
{
    tabBar()->setTabButton(num, QTabBar::RightSide, 0);
}
