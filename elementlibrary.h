#ifndef ELEMENTLIBRARY_H
#define ELEMENTLIBRARY_H

#include <QObject>
#include <QMap>
#include <QtXml/QDomDocument>

#include "classes.h"
#include "element.h"
#include "complexelement.h"

class ElementLibrary : public QObject
{
    Q_OBJECT

    QMap<QString, ComplexElement*> elements;
public:
    explicit ElementLibrary(QObject *parent = 0);
            ~ElementLibrary();
    ComplexElement *getElement(QString name);
    bool hasElement(QString name);
    int addElement(QString name, ComplexElement *e);

    ElementLibrary * clone();

    static ElementLibrary *fromXml(QDomElement d_el);
    ComplexElement *elementFromXml(QDomElement d_el);
    QDomElement toXml(QDomDocument doc);
    QDomElement elementToXml(QDomDocument doc, QString name);

signals:

public slots:

};

#endif // ELEMENTLIBRARY_H
