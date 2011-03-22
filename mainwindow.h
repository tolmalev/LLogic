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

class MainWindow : public QMainWindow
{
    Q_OBJECT


    QMessageBox *calculating_message;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showDocument(Document*);
protected:
    QToolBar    *toolBar;
    TabWidget  *tabWidget;
    QMenuBar    *menuBar;
    QListWidget *listWidget;
    QMap<Document*, QWidget*>       widgets;
    QMap<QWidget*, Document*>       documents;

    static MainWindow *wnd;


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

    QAction *aand;
    QAction *aor;
    QAction *aselect;
    QAction *asend;
    QAction *arec;
    QAction *aautoCalc;
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
};

#endif // MAINWINDOW_H
