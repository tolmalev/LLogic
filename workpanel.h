#ifndef WORKPANEL_H
#define WORKPANEL_H

#include "classes.h"

#include <QWidget>
#include <QMap>
#include <QMouseEvent>

class SelectWidget : public QWidget{
    Q_OBJECT

    public:
        SelectWidget(QWidget *parent) : QWidget(parent){}
        void paintEvent(QPaintEvent *);
};

class MovingWidget : public QWidget{
    Q_OBJECT

    WorkPanel *wp;

    public:
        MovingWidget(QWidget *parent, WorkPanel*wp) : QWidget(parent), wp(wp){}
        void paintEvent(QPaintEvent *);
};

class LiningWidget : public QWidget{
    Q_OBJECT

    WorkPanel *wp;

    public:
        LiningWidget(QWidget *parent, WorkPanel*wp) : QWidget(parent), wp(wp){}
        void paintEvent(QPaintEvent *);
};

class AddingWidget : public QWidget{
    Q_OBJECT

    friend class WorkPanel;

    WorkPanel *wp;
    QWidget *tmp;
    int width, height;
    bool mouseIn;
    QPoint pt;

    bool addingPoint;

    public:
        AddingWidget(QWidget *parent, WorkPanel*wp, int wd, int h, Element*e, bool addp=0);
        ~AddingWidget();
        void paintEvent(QPaintEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);

	void dragEnterEvent(QDragEnterEvent *);

        void enterEvent(QEvent *){mouseIn=1;}
        void leaveEvent(QEvent *){mouseIn=0;update();}

        void keyPressEvent(QKeyEvent *);

    signals:
        void addElement(Element *e);
        void addPoint(QPoint p);
        void stopAdding();
};

class WorkPanel : public QWidget
{
    Q_OBJECT

    friend class Document;
    friend class MovingWidget;
    friend class LiningWidget;
    friend class AddingWidget;
    QMap<int, PointWidget*> points;


protected:
    Document *d;
	ComplexElement *ce;

	int panel_type;
	enum types{
	    DOCUMENT,
	    ELEMENT,
	};

	QAction*    acomplex;
	QAction*    alibrary;
	QAction*    abuildtable;
	QAction*    aCopy;
	QAction*    aPaste;
	QAction*    aDelete;

	QWidget * tmpw;
	QPoint p1, p2;
	Element *adding;
	PointWidget     * pw1, *pw2;
	bool    midButton;

	QSet<ElementWidget*>    elementWidgets;
	QSet<ElementWidget*>    selected;
	QSet<PointWidget*>      freePoints;
	QSet<PointWidget*>      selectedFreePoints;

	struct tp{
		int x,y,t;
		bool operator ==(tp t2) {
		    return t==t2.t;
		}
		bool operator !=(tp t2) {
		    return t!=t2.t;
		}
	    };
	struct step{
	    int x,y,t,wait;
	};
	struct tlines{
	    int x1,y1,x2,y2;
	    tp t;
	    int light,error;
	};
	struct field{
	    int point,k1,k2,k,re;
	    tp t1,t2;
	};

	field s[200][200];
	int xsize,ysize;
	int kstep1,kstep2,firstpoint,n;
	step step1[10000],step2[10000],list[1000];
	tlines lines[10000];
	int klines;
	int component[1000],maxpoint;

	int state;
	enum states{
	    NONE,
	    MOVING,
	    LINING,
	};

	int getx(int x);
	int getk(int x);
	int min(int x, int y);
	int max(int x, int y);
	int bfs(tp tt);
	int dfs(int k, int t);
	void calculateLines();
	void drawLines(QPainter &p);
	int dist(int x1, int y1, int x2, int y2, int x, int y);

public:
    explicit WorkPanel(ComplexElement*ce = 0, QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

    void addElement(Element* e);
    void addPoint(int p, QPoint pos);

    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent * ev);
    void keyPressEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *);

    void dragEnterEvent(QDragEnterEvent *);
    void dragLeaveEvent(QDragLeaveEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);

    void setSelection(QSet<Element*>, QSet<int> points);
    void move(QPoint dr, QSet<Element*> els, QSet<int> pts);
    void remove(QSet<Element *> els, QSet<int> pts);

    bool eventFilter(QObject *o, QEvent *e);

    bool canMoveTo(QWidget *ew, QPoint p, bool sel=1);

    void contextMenuEvent(QContextMenuEvent *);

    static QPoint toGrid(QPoint a);

    Document *document(){return d;}

    void setAddingElement(Element *e);
    void startAddingPoint();
    void updateMinimumSize();

signals:
    void doubleClicked(ElementWidget*);
    void needCalculation(Element*);
public slots:
    void stopAdding(int type = 0);
    void createComplex();
    void addToLibrary();
    void buildTable();
    void copy();
    void paste();
    void adelete();
};

#endif // WORKPANEL_H
