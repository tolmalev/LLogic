#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QString>
#include <QSet>
#include <QtXml/QDomElement>

#include "classes.h"


class Document : public QObject
{
    Q_OBJECT

    friend class WorkPanel;
    friend class ComplexElement;
    friend class LibraryElement;
protected:
    Controller * c;
    ComplexElement *ce;
    ElementLibrary *library;
    QSet<Element*> elements;
    WorkPanel *panel;
    void createPanel();
    QString _name;
    QString fileName;
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

public:
    explicit Document(int _type=FULL, ComplexElement* el = 0, QObject *parent = 0);
    ~Document();
    static   Document* fromFile(QString filename);

    static   Document* fromXml(QDomElement d_el);
    QDomElement toXml(QDomDocument doc);
    QDomElement elementsToXml(QDomDocument doc);
    QDomElement connectionsToXml(QDomDocument doc);

    bool    parseElements(QDomElement d_el);
    bool    parseConnections(QDomElement d_el);

    int     saveToFile(QString _filename = "");

    int     document_type(){return _document_type;}

    virtual Document * clone();

    void addElement(Element* e);
    int addConnection(int id1, int id2);
    bool canConnect(int id1, int id2);
    void removePoint(int id);

    void calcIfNeed();

    WorkPanel *workPanel();
    QString name(){return _name;}

    void stop_calculation();

signals:
    void timeout(Document*);
    void calculation_finished(int);
    void calculation_started();
    void doubleClicked(ElementWidget*);
public slots:
    void timeout(Controller*c);
    void needCalculation(Element*);
};

#endif // DOCUMENT_H
