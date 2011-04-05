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
    friend class WorkPanel;
    friend class ElementLibrary;
    friend class Document;

    protected:
        Document*d;
        QVector<QPair<int, int> > in_connections;
        QVector<QPair<int, int> > out_connections;
    public:

        ComplexElement(int _in_cnt = -1, int _out_cnt = -1);
        ~ComplexElement();
        void        recalc();
        Element*    clone();
        Document* document(){return d;}

        static ComplexElement *fromXml(QDomElement d_el);
        QDomElement toXml(QDomDocument);
        QDomElement inputConnectionsToXml(QDomDocument doc);
        QDomElement outputConnectionsToXml(QDomDocument doc);

        bool parseInputConnections(QDomElement d_el);
        bool parseOutputConnections(QDomElement d_el);

	void buildTable(QString fileName);

        void updateDocumentName();
};

#endif // COMPLEXELEMENT_H
