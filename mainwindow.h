#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "classes.h"

#include <QMainWindow>
#include <QToolBar>
#include <QTabWidget>
#include <QMenuBar>
#include "workpanel.h"
#include "elementwidget.h"
#include "tabwidget.h"
#include <QListWidget>
#include <QMap>
#include <QMessageBox>
#include <QStandardItemModel>

class ListItemModel : public QStandardItemModel {
    Q_OBJECT

    public:
	ListItemModel(QObject *p) : QStandardItemModel(0, 2, p){};
	QStringList mimeTypes();
	QMimeData *mimeData(const QModelIndexList &indexes) const;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT


    QMessageBox *calculating_message;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showDocument(Document*);
    void newDocument();
    static MainWindow *wnd;
    QMap<QString,QPixmap> pix;

    QPixmap pixmap(QString);
protected:
    QToolBar    *toolBar;
    TabWidget  *tabWidget;
    QMenuBar    *menuBar;
    QListView	*listWidget;
    ListItemModel * listModel;
    QMap<Document*, QWidget*>       widgets;
    QMap<QWidget*, Document*>       documents;


    int instrument;
    enum instruments{
        NONE,
        AND,
        NOT,
        OR,
        XOR,
        ANDNOT,
        ORNOT,
    };

    int untitledN;

    QAction *aand;
    QAction *aor;
    QAction *axor;
    QAction *anot;
    QAction *aornot;
    QAction *aandnot;
    QAction *aselect;
    QAction *asend;
    QAction *apoint;
    QAction *a8bitsend;
    QAction *a8bitrec;
    QAction *asegment;
    QAction *arec;
    QAction *aautoCalc;

    QAction *adeleteLibrary;


    void closeEvent(QCloseEvent *);

public slots:
    void doubleClicked(ElementWidget *);
    void closeTab(int);
    void closeCurrentTab();
    void calculation_timeout(Document*c);
    void calculation_finished(int);
    void calculation_started();
    void triggered(QAction*);
    void toolBarAction(QAction*);
    void instrumentChanged();
    void libraryClicked(QModelIndex);
    void documentChanged(Document *d);
    void libraryChanged();
    void listWidgetMenu(QPoint);
    void removeFromLibrary();
    void tabChanged(int);
};

#endif // MAINWINDOW_H
