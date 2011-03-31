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


class Document : public QObject
{
    Q_OBJECT

    friend class WorkPanel;
    friend class ComplexElement;
    friend class LibraryElement;
    friend class MainWindow;
protected:
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

    int     addConnection(int id1, int id2);
    void    removeConnection(int id1, int id2);
    bool    canConnect(int id1, int id2);
    void    removePoint(int id);
    void    moveElement(Element *e, QPoint pos);
    void    moveFreePoint(int p, QPoint pos);

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

    void addToClipboard(QSet<Element*> elements, QSet<int> points);
    void addFromClipboard();

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
    void addElement(Element* e);
    int	 addPoint(QPoint pos, int p=-1);
};

#endif // DOCUMENT_H
