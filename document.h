#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QString>
#include <QSet>
#include <QMap>
#include <QPoint>
#include <QtXml/QDomElement>
#include <QClipboard>
#include <QMimeData>

#include "classes.h"

class DocumentChange{
    protected:
	Document *d;
    public:
	DocumentChange(Document *d) : d(d){}
	virtual void undo() = 0;
	virtual void redo() = 0;
};

class ConnectionsChange : public DocumentChange{
    friend class ElementsChange;
    protected:
	QList<QPair<int, int> > con;
	bool add;
	void addConnections();
	void removeConnections();
    public:
	ConnectionsChange(Document *d, QList<QPair<int, int> > con, bool add) : DocumentChange(d), con(con), add(add){};
	ConnectionsChange(Document *d, int from, int to, bool add);

	void undo();
	void redo();
};

class ElementsChange : public DocumentChange{
	QSet<Element*>	els;
	QMap<int, QPoint> pts;
	ConnectionsChange *con_ch;
	bool add;

	void removeAll();
	void addAll();
    public:
	ElementsChange(Document *d, QSet<Element*> els, QMap<int, QPoint> pts, QList<QPair<int, int> > con, bool add);

	void undo();
	void redo();
};

class MovingChange : public DocumentChange{
	QPoint dr;
	QSet<Element*> els;
	QSet<int> pts;
    public:
	MovingChange(Document *d, QPoint dr, QSet<Element*> els, QSet<int> pts) : DocumentChange(d), dr(dr), els(els), pts(pts){};
	void undo();
	void redo();
};


class Document : public QObject
{
    Q_OBJECT

    friend class WorkPanel;
    friend class ComplexElement;
    friend class LibraryElement;
    friend class MainWindow;
protected:
    QList<DocumentChange*> changes;
    QList<DocumentChange*>::iterator now_change;

    Controller          *c;
    ComplexElement      *ce;
    ElementLibrary      *library;
    QSet<Element*>      elements;
    QMap<int, QPoint>   freePoints;

    WorkPanel *panel;
    void createPanel();

    QString _name;
    QString fileName;

    bool _changed;
    bool auto_calculation;
    int _document_type;

    enum types{
        FULL,
        ELEMENT
    };

    int instrument;
    enum inst{
        SELECT,
        CONNECT,
        ADDELEMENT,
        ADDPOINT,
    };

private slots:

    void changed();

public:
    explicit Document(int _type=FULL, ComplexElement* el = 0, QObject *parent = 0);
    ~Document();

    static   Document* fromFile(QString filename);
    static   Document* fromXml(QDomElement d_el);
    QDomElement toXml(QDomDocument doc);
    QDomElement elementsToXml(QDomDocument doc);
    QDomElement connectionsToXml(QDomDocument doc);
    QDomElement freePointsToXml(QDomDocument doc);

    QDomElement selectionToXml(QDomDocument doc, QSet<Element*> elements, QSet<int> points);
    int     saveToFile(QString _filename = "");

    bool    parseElements(QDomElement d_el);
    bool    parseConnections(QDomElement d_el);
    bool    parseFreePoints(QDomElement d_el);

    int     document_type(){return _document_type;}
    Document* rootDocument();

    virtual Document * clone();

    int     addConnection(int id1, int id2, bool save = 1);
    void    removeConnection(int id1, int id2, bool save = 1);
    bool    canConnect(int id1, int id2);
    void    removePoint(int id);
    void    moveElement(Element *e, QPoint pos);
    void    moveFreePoint(int p, QPoint pos);
    void    move(QPoint dr, QSet<Element*> els, QSet<int> pts, bool save = 1);
    void    clone(QPoint dr, QSet<Element*> els, QSet<int> pts, bool save = 1);
    void    remove(QSet<Element*>els, QSet<int> pts, bool save = 1);
    void    add(QSet<Element*>els, QMap<int, QPoint> pts, bool save =1);

    void    calcIfNeed();

    WorkPanel *workPanel();
    QString name(){return _name;}

    void setInstrument(int in);
    void setAddingElement(Element *el);

    void stop_calculation();

    void createComplex(QSet<Element*> elements, QList<int> points);

    Element * getLibraryElement(QString name);
    QList<QString> libraryNames();

    int addLibraryElement(QString name, ComplexElement* e);
    int removeLibraryElement(QString name);

    void setUnchanged();

    QMimeData *toMimeData(QSet<Element *> elements, QSet<int> points);
    void addToClipboard(QSet<Element*> elements, QSet<int> points);
    void addFromClipboard();

    bool canUndo();
    bool canRedo();
    void undo();
    void redo();
    void addChange(DocumentChange *ch);

    int newPoint();
    void updateElement(Element* el);

signals:
    void timeout(Document*);
    void calculation_finished(int);
    void calculation_started();
    void doubleClicked(ElementWidget*);
    void instrumentChanged();
    void documentChanged(Document*);
    void libraryChanged();
public slots:
    void timeout(Controller*c);
    void needCalculation(Element*);
    void addElement(Element* e, bool save = 1);
    int	 addPoint(QPoint pos, int p=-1, bool save = 1);
};

#endif // DOCUMENT_H
