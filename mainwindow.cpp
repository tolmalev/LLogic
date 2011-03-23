#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>

#include "mainwindow.h"
#include "document.h"
#include "controller.h"
#include "workpanel.h"
#include "complexelement.h"
#include "simpleelements.h"

MainWindow* MainWindow::wnd = 0;

void MainWindow::triggered(QAction *act)
{
    qWarning("se");
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
    hb->setSpacing(0);
    vb->setMargin(1);
    leftWidget->setLayout(vb);

    toolBar     = new QToolBar();
    aand        = new QAction("and", toolBar);
    asend       = new QAction("send", toolBar);
    arec        = new QAction("rec", toolBar);
    aor         = new QAction("or", toolBar);
    aselect     = new QAction("select", toolBar);
    aautoCalc   = new QAction("auto", toolBar);

    QActionGroup *ag = new QActionGroup(toolBar);

    aand->setCheckable(1);
    arec->setCheckable(1);
    asend->setCheckable(1);
    aor->setCheckable(1);
    aselect->setCheckable(1);
    aautoCalc->setCheckable(1);

    aselect->setChecked(1);
    aautoCalc->setChecked(1);

    ag->addAction(aand);
    ag->addAction(aor);
    ag->addAction(asend);
    ag->addAction(arec);
    ag->addAction(aselect);

    toolBar->addActions(ag->actions());
    toolBar->addAction(aautoCalc);
    toolBar->show();

    connect(toolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(toolBarAction(QAction*)));

    addToolBar((QToolBar*)toolBar);


    //vb->addWidget(toolBar);


    menuBar = QMainWindow::menuBar();
    QMenu *file = menuBar->addMenu("File");
    QAction *op = new QAction("Open", this);
    op->setShortcut(QKeySequence::Open);
    file->addAction(op);
    file->addSeparator();
    file->addAction("Save");
    file->addAction("Save As");




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
}

MainWindow::~MainWindow()
{
}

void MainWindow::doubleClicked(ElementWidget * ew)
{
    if(ew->element()->type() != COMPLEX)
         return;
    ComplexElement *ce = (ComplexElement*)(ew->element());
    if(widgets.find(ce->document()) == widgets.end())
    {
        widgets[ce->document()] = ce->document()->workPanel();
        documents[widgets[ce->document()]] = ce->document();
        tabWidget->addTab(widgets[ce->document()], "testnew");
        tabWidget->setCurrentWidget(widgets[ce->document()]);
    }
    else
    {
        tabWidget->setCurrentWidget(widgets[ce->document()]);
    }
}

void MainWindow::closeTab(int n)
{
    QWidget *w = tabWidget->widget(n);
    widgets.remove(documents[w]);
    documents.remove(w);
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
        connect(d, SIGNAL(instrumentChanged()), this, SLOT(instrumentChanged()));
        connect(d, SIGNAL(documentChanged(Document*)), this, SLOT(documentChanged(Document*)));
        tabWidget->setCurrentWidget(d->workPanel());

        documents[d->workPanel()] = d;
        widgets[d] = d->workPanel();
        d->setInstrument(Document::SELECT);
    }
}

void MainWindow::calculation_started()
{
    tabWidget->setEnabled(0);
}

void MainWindow::toolBarAction(QAction * act)
{
    if(act->text() == "and")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new AndElement());
    }
    else if(act->text() == "send")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new SendElement());
    }
    else if(act->text() == "rec")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new ReceiveElement());
    }
    else if(act->text() == "select")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::SELECT);
    }
    else if(act->text() == "auto")
    {
        if(act->isChecked())
            documents[tabWidget->currentWidget()]->auto_calculation = 1;
        else
            documents[tabWidget->currentWidget()]->auto_calculation = 0;
    }
}

void MainWindow::instrumentChanged()
{
    aselect->trigger();
}

void MainWindow::documentChanged(Document *d)
{
    if(widgets.find(d) != widgets.end())
    {
        QWidget *w = widgets[d];
        int ind = tabWidget->indexOf(w);
        tabWidget->setTabText(ind, d->name() + "*");
    }
}
