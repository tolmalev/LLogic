#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>

#include "mainwindow.h"
#include "document.h"
#include "controller.h"
#include "workpanel.h"
#include "complexelement.h"

MainWindow* MainWindow::wnd = 0;

void MainWindow::triggered(QAction *act)
{
    if(act->text() == "Open")
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File",
                                                        "",
                                                        "Logic files (*.lod)");
        if(fileName != "")
            showDocument(Document::fromFile(fileName));
    }
    else if(act->text() == "Save As")
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Locic files (*.lod)");
        if(fileName != "")
            documents[tabWidget->currentWidget()]->saveToFile(fileName);
    }
    else if(act->text() == "Save")
    {
        documents[tabWidget->currentWidget()]->saveToFile();
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    wnd = this;
    calculating_message = 0;
    setGeometry(100, 100, 800, 500);

    QWidget * centralWidget = new QWidget();
    QWidget * leftWidget = new QWidget();
    leftWidget->setMinimumWidth(100);
    leftWidget->setMaximumWidth(100);

    QHBoxLayout *hb = new QHBoxLayout();
    QVBoxLayout *vb = new QVBoxLayout();
    QGridLayout *gl = new QGridLayout();
    hb->setSpacing(0);
    vb->setMargin(1);
    leftWidget->setLayout(vb);

    toolBar = new QWidget();
    toolBar->setLayout(gl);
    gl->addWidget(new QLabel("tets"));
    vb->addWidget(toolBar);

    QPushButton * and_btn = new QPushButton("and");
    QPushButton * or_btn = new QPushButton("or");
    gl->addWidget(and_btn);
    gl->addWidget(or_btn);


    menuBar = new QMenuBar();
    QMenu *file = new QMenu("File");
    file->addAction("Open");
    file->addSeparator();
    file->addAction("Save");
    file->addAction("Save As");
    menuBar->addMenu(file);

    connect(menuBar, SIGNAL(triggered(QAction*)), this, SLOT(triggered(QAction*)));



    tabWidget = new TabWidget();
    tabWidget->setMinimumSize(300, 200);
    hb->addWidget(leftWidget);
    hb->addWidget(tabWidget);

    listWidget = new QListWidget();

    vb->addWidget(listWidget, 1);

    centralWidget->setLayout(hb);
    setCentralWidget(centralWidget);
    setMenuBar(menuBar);

    tabWidget->setMovable(1);
    tabWidget->setMainWindow(this);
    //tabWidget->setTabsClosable(1);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    QPushButton * btn = new QPushButton(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton), "");


    connect(btn, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));

    tabWidget->setCornerWidget(btn);
}

MainWindow::~MainWindow()
{
}

#include <QFileDialog>

void MainWindow::doubleClicked(ElementWidget * ew)
{
    if(map.find(ew) == map.end())
    {        
        if(ew->element()->type() == COMPLEX)
        {
            map[ew] = ((ComplexElement*)ew->element())->document()->workPanel();
            rmap[map[ew]]=ew;
            tabWidget->addTab(map[ew], "testnew");
            tabWidget->setCurrentWidget(map[ew]);
        }
    }
    else
        tabWidget->setCurrentWidget(map[ew]);
}

void MainWindow::closeTab(int n)
{
    tabWidget->prepareClose(n);
    QWidget *w = tabWidget->widget(n);
    map.remove(rmap[w]);
    rmap.remove(w);
    tabWidget->removeTab(n);
    if(tabWidget->count() == 0)
        tabWidget->addTab(new WorkPanel, "Unnamed");
}

void MainWindow::closeCurrentTab()
{
    closeTab(tabWidget->currentIndex());
}

void MainWindow::calculation_timeout(Document*d)
{
    calculating_message = new QMessageBox(this);
    calculating_message->setIconPixmap(style()->standardPixmap(QStyle::SP_MessageBoxWarning));
    calculating_message->setText("Calculating in progress");
    calculating_message->addButton(QMessageBox::Cancel);
    int result = calculating_message->exec();
    if (result == QMessageBox::Cancel)
        d->stop_calculation();
}

void MainWindow::calculation_finished(int r)
{
    if(calculating_message)
    {
        calculating_message->close();
        delete calculating_message;
        calculating_message = 0;
    }
    tabWidget->setEnabled(1);
}

void MainWindow::showDocument(Document *d)
{
    if(d != 0)
    {
        tabWidget->addTab(d->workPanel(), d->name());
        connect(d, SIGNAL(timeout(Document*)), this, SLOT(calculation_timeout(Document*)));
        connect(d, SIGNAL(calculation_finished(int)), this, SLOT(calculation_finished(int)));
        connect(d, SIGNAL(doubleClicked(ElementWidget*)), this, SLOT(doubleClicked(ElementWidget*)));
        connect(d, SIGNAL(calculation_started()), this, SLOT(calculation_started()));
        tabWidget->setCurrentWidget(d->workPanel());

        documents[d->workPanel()] = d;
    }
}

void MainWindow::calculation_started()
{
    tabWidget->setEnabled(0);
}
