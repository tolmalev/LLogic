#ifndef COMPLEXELEMENT_H
#define COMPLEXELEMENT_H

#include <QList>
#include <QPair>
#include "stdio.h"

#include "element.h"
#include "document.h"
#include "classes.h"


class ComplexElement : public Element
{
    Q_OBJECT

    friend class ComplexElementDocument;

    protected:
        Document*d;
        QVector<QPair<int, int> > in_connections;
        QVector<QPair<int, int> > out_connections;
    public:

        ComplexElement();
        void        recalc();
        Element*    clone();
        Document* document(){return d;}
        int         saveToFile(FILE *f);

        static ComplexElement *fromXml(QDomElement d_el);

        bool parseInputConnections(QDomElement d_el);
        bool parseOutputConnections(QDomElement d_el);
};

#endif // COMPLEXELEMENT_H
