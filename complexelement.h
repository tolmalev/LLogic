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
        QList<QPair<int, int> > in_connections;
        QList<QPair<int, int> > out_connections;
    public:

        ComplexElement();
        void        recalc();
        Element*    clone();
        Document* document(){return d;}
        int         saveToFile(FILE *f);
};

#endif // COMPLEXELEMENT_H
