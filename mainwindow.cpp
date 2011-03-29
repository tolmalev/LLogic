#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolButton>
#include <QSplitter>

#include "mainwindow.h"
#include "document.h"
#include "controller.h"
#include "workpanel.h"
#include "complexelement.h"
#include "simpleelements.h"
#include "elementlibrary.h"

MainWindow* MainWindow::wnd = 0;

void MainWindow::triggered(QAction *act)
{
    //qDebug("se");
    if(act->text() == "New")
    {
        newDocument();
        return;
    }
    if(documents.find(tabWidget->currentWidget()) == documents.end())
        return;
    Document *d=documents[tabWidget->currentWidget()]->rootDocument();
    if(act->text() == "Open")
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File",
                                                        "",
                                                        "Logic files (*.lod)");
        if(fileName != "")
        {
            foreach(Document *d, documents)
            {
                if(d->fileName == fileName)
                {
                    tabWidget->setCurrentWidget(widgets[d]);
                    return;
                }
            }
	    Document *d = Document::fromFile(fileName);

	    if(d != 0)
		showDocument(d);
	    else
		QMessageBox::critical(this, "error", "Can't open file " + fileName);
        }
    }
    else if(act->text() == "Save As")
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Locic files (*.lod)");
        if(fileName != "")
        {
            d->saveToFile(fileName);
            int ind = tabWidget->indexOf(widgets[d]);
            tabWidget->setTabText(ind, d->name());
        }
    }
    else if(act->text() == "Save")
    {        
        if(d->fileName != "")
        {
            d->saveToFile();
            int ind = tabWidget->indexOf(widgets[d]);
            tabWidget->setTabText(ind, d->name());
        }
        else
        {
            QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Locic files (*.lod)");
            if(fileName != "")
            {
                d->saveToFile(fileName);
                int ind = tabWidget->indexOf(widgets[d]);
                tabWidget->setTabText(ind, d->name());
            }
        }
    }
    else if(act->text() == "Import library")
    {
	QString fileName = QFileDialog::getOpenFileName(this, "Open File",
							"",
							"Logic files (*.lod)");
	if(fileName != "")
	{
	    Document *lib = Document::fromFile(fileName);
	    if(lib)
	    {
		QStringList list;
		foreach(QString s, lib->libraryNames())
		{
		    d->addLibraryElement(s, (ComplexElement*)lib->getLibraryElement(s));
		}
	    }
	    else
	    {
		QMessageBox::critical(this, "error", "Can't read document from " + fileName);
	    }
	}
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    wnd = this;
    untitledN = 0;
    calculating_message = 0;
    setGeometry(100, 100, 800, 500);

    QSplitter * centralWidget = new QSplitter(this);
    QWidget * leftWidget = new QWidget();
    leftWidget->setMinimumWidth(100);
    leftWidget->setMaximumWidth(200);
    centralWidget->addWidget(leftWidget);
    //centralWidget->setSizes(l);

    QHBoxLayout *hb = new QHBoxLayout();
    QVBoxLayout *vb = new QVBoxLayout();
    hb->setSpacing(1);
    vb->setMargin(0);
    leftWidget->setLayout(vb);    

    toolBar     = new QToolBar();
    aand        = new QAction("and",    toolBar);
    asend       = new QAction("send",   toolBar);
    arec        = new QAction("rec",    toolBar);
    aor         = new QAction("or",     toolBar);
    anot        = new QAction("not",    toolBar);
    axor        = new QAction("xor",    toolBar);
    aornot      = new QAction("or-not", toolBar);
    aandnot     = new QAction("and-not",toolBar);
    apoint      = new QAction("point",	toolBar);
    a8bitsend	= new QAction("8bit->", toolBar);
    aselect     = new QAction("select", toolBar);
    aautoCalc   = new QAction("auto",   toolBar);

    QActionGroup *ag = new QActionGroup(toolBar);

    aand->setCheckable(1);
    arec->setCheckable(1);
    asend->setCheckable(1);
    aor->setCheckable(1);
    anot->setCheckable(1);
    axor->setCheckable(1);
    aornot->setCheckable(1);
    aandnot->setCheckable(1);
    aselect->setCheckable(1);
    aautoCalc->setCheckable(1);
    apoint->setCheckable(1);
    a8bitsend->setCheckable(1);

    aselect->setChecked(1);
    aautoCalc->setChecked(1);

    ag->addAction(aand);
    ag->addAction(aor);
    ag->addAction(anot);
    ag->addAction(axor);
    ag->addAction(aornot);
    ag->addAction(aandnot);
    ag->addAction(asend);
    ag->addAction(arec);
    ag->addAction(apoint);
    ag->addAction(aselect);
    ag->addAction(a8bitsend);

    toolBar->addActions(ag->actions());
    toolBar->addSeparator();
    toolBar->addAction(aautoCalc);
    toolBar->show();

    connect(toolBar, SIGNAL(actionTriggered(QAction*)), this, SLOT(toolBarAction(QAction*)));

    addToolBar((QToolBar*)toolBar);


    //vb->addWidget(toolBar);


    menuBar = QMainWindow::menuBar();
    QMenu *file = menuBar->addMenu("File");
    QAction *op = new QAction("Open", this);
    op->setShortcut(QKeySequence::Open);
    QAction *save = new QAction("Save", this);
    save->setShortcut(QKeySequence::Save);
    QAction *saveas = new QAction("Save As", this);
    saveas->setShortcut(QKeySequence("Ctrl+Shift+S"));
    QAction *newdoc = new QAction("New", this);
    newdoc->setShortcut(QKeySequence::New);

    file->addAction(op);
    file->addSeparator();
    file->addAction(save);
    file->addAction(saveas);
    file->addAction(newdoc);
    file->addAction("Import library");

    QAction *ctrlf4 = new QAction("Close tab", this);
    ctrlf4->setShortcut(QKeySequence("Ctrl+F4"));

    addAction(ctrlf4);
    connect(ctrlf4, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));

    connect(menuBar, SIGNAL(triggered(QAction*)), this, SLOT(triggered(QAction*)));

    adeleteLibrary = new QAction("Remove this element from the library", this);
    connect(adeleteLibrary, SIGNAL(triggered()), this, SLOT(removeFromLibrary()));



    tabWidget = new TabWidget();
    tabWidget->setMinimumSize(300, 200);
    centralWidget->addWidget(tabWidget);
    //hb->addWidget(leftWidget);
    //hb->addWidget(tabWidget);

    listWidget = new QListWidget();
    listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(libraryClicked(QListWidgetItem*)));
    connect(listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(listWidgetMenu(QPoint)));

    vb->addWidget(listWidget, 1);

    setMenuBar(menuBar);

    tabWidget->setMovable(1);
    tabWidget->setMainWindow(this);
    //tabWidget->setTabsClosable(1);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    centralWidget->setStretchFactor(0, 0);
    QList<int> l;
    l.append(100);
    l.append(1000);
    centralWidget->setSizes(l);
    setCentralWidget(centralWidget);
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
	QScrollArea *sa = new QScrollArea(tabWidget);
	sa->setWidget(ce->document()->workPanel());
	sa->setWidgetResizable(1);
	widgets[ce->document()] = sa;
        documents[widgets[ce->document()]] = ce->document();
        ce->updateDocumentName();
	tabWidget->addTab(sa, ce->document()->name());
        tabWidget->setCurrentWidget(widgets[ce->document()]);

        connect(ce->document(), SIGNAL(doubleClicked(ElementWidget*)), this, SLOT(doubleClicked(ElementWidget*)));
    }
    else
    {
        tabWidget->setCurrentWidget(widgets[ce->document()]);
    }
}

void MainWindow::closeTab(int n)
{
    if(tabWidget->count() == 1)
    {
        Document *d = documents[tabWidget->currentWidget()];
        if(d->fileName == "" && d->_changed == 0)
            return;
    }
    QWidget *w = tabWidget->widget(n);
    Document *d = documents[w];
    bool close = 1;
    if(d->_changed && !d->ce)
    {
	int res = QMessageBox::question(this, "Save?", "Document " + d->fileName + " isn\'t save. \nDo you want to save it?", QMessageBox::Save, QMessageBox::Close, QMessageBox::Cancel);
	if(res == QMessageBox::Cancel)
	    return;
	if(res == QMessageBox::Close)
	    close = 1;
	if(res == QMessageBox::Save)
	{
	    close = 1;
	    if(d->fileName != "")
		d->saveToFile();
	    else
	    {
		QString fileName = QFileDialog::getSaveFileName(this, "Save file", "", "Locic files (*.lod)");
		if(fileName != "")
		    d->saveToFile(fileName);
		else
		    close = 0;
	    }
	}
    }

    if(close)
    {
	widgets.remove(documents[w]);
	documents.remove(w);
	tabWidget->removeTab(n);
	if(tabWidget->count() == 0)
	    newDocument();
    }
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
	QScrollArea *sa = new QScrollArea(tabWidget);
	sa->setWidget(d->workPanel());
	//d->workPanel()->setGeometry(0, 0,  1000, 1000);
	sa->setWidgetResizable(1);
	//tabWidget->addTab(d->workPanel(), d->name());
	tabWidget->addTab(sa, d->name());
        connect(d, SIGNAL(timeout(Document*)), this, SLOT(calculation_timeout(Document*)));
        connect(d, SIGNAL(calculation_finished(int)), this, SLOT(calculation_finished(int)));
        connect(d, SIGNAL(doubleClicked(ElementWidget*)), this, SLOT(doubleClicked(ElementWidget*)));
        connect(d, SIGNAL(calculation_started()), this, SLOT(calculation_started()));
        connect(d, SIGNAL(instrumentChanged()), this, SLOT(instrumentChanged()));
        connect(d, SIGNAL(documentChanged(Document*)), this, SLOT(documentChanged(Document*)));
	connect(d, SIGNAL(libraryChanged()), this, SLOT(libraryChanged()));
	tabWidget->setCurrentWidget(sa);
	//documents[d->workPanel()] = d;
	//widgets[d] = d->workPanel();
	documents[sa] = d;
	widgets[d] = sa;
        d->setInstrument(Document::SELECT);

	libraryChanged();
    }
}

void MainWindow::calculation_started()
{
    tabWidget->setEnabled(0);
}

void MainWindow::toolBarAction(QAction * act)
{
    if(documents.find(tabWidget->currentWidget()) == documents.end())
        return;
    if(act->text() == "and")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new AndElement());
    }
    else if(act->text() == "or")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new OrElement());
    }
    else if(act->text() == "8bit->")
    {
	documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
	documents[tabWidget->currentWidget()]->setAddingElement(new NumberSendElement8);
    }
    else if(act->text() == "xor")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new XorElement());
    }
    else if(act->text() == "not")
    {
	documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
	documents[tabWidget->currentWidget()]->setAddingElement(new NotElement());
    }
    else if(act->text() == "or-not")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new OrNotElement());
    }
    else if(act->text() == "and-not")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDELEMENT);
        documents[tabWidget->currentWidget()]->setAddingElement(new AndNotElement());
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
    else if(act->text() == "point")
    {
        documents[tabWidget->currentWidget()]->setInstrument(Document::ADDPOINT);
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

void MainWindow::newDocument()
{
    Document *d = new Document();
    d->_name = "Untitled_" + QString::number(untitledN++) + ".lod";
    d->fileName = "";
    d->library = new ElementLibrary();
    showDocument(d);
}

void MainWindow::libraryClicked(QListWidgetItem *lwi)
{
    if(documents.find(tabWidget->currentWidget()) == documents.end())
	return;

    Document *d = documents[tabWidget->currentWidget()];
    Element *e = d->getLibraryElement(lwi->text());
    if(e)
    {
	d->setInstrument(Document::ADDELEMENT);
	d->setAddingElement(e->clone());
    }
}

void MainWindow::libraryChanged()
{
    if(documents.find(tabWidget->currentWidget()) == documents.end())
	return;
    Document *d = documents[tabWidget->currentWidget()];

    listWidget->clear();
    listWidget->addItems(d->libraryNames());
}

void MainWindow::listWidgetMenu(QPoint pt)
{
    QMenu mn;
    if(listWidget->currentIndex().column() < 0)
	return;
    mn.addAction(adeleteLibrary);
    mn.exec(listWidget->mapToGlobal(pt));
}

void MainWindow::removeFromLibrary()
{
    if(documents.find(tabWidget->currentWidget()) == documents.end())
	return;
    if(!listWidget->currentItem())
	return;
    Document *d = documents[tabWidget->currentWidget()];
    d->removeLibraryElement(listWidget->currentItem()->text());
}

void MainWindow::tabChanged(int)
{
    libraryChanged();
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    bool have = 0;
    foreach(Document*d, documents)
    {
	have |= d->_changed;
    }
    if(have)
    {
	int res = QMessageBox::question(this, "Close?", "You have unsaved document, exit anyway?", QMessageBox::Cancel, QMessageBox::Close);
	if(res == QMessageBox::Cancel)
	    ev->ignore();
    }
}
