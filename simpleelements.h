#ifndef SIMPLEELEMENTS_H
#define SIMPLEELEMENTS_H

#include "classes.h"
#include "element.h"

Element* SimpleElement(int type);


class SendElement : public Element
{
    friend class SendElementWidget;
        int val;
    public:
        SendElement() : Element(0, 0, 1, SEND), val(0){};
        virtual void recalc();
        virtual Element*clone();
};

class ReceiveElement : public Element
{
    friend class ReceiveElementWidget;
        int val;
    public:
        ReceiveElement() : Element(0, 1, 0, RECEIVE), val(0){};
        virtual void recalc();
        virtual Element*clone();
};

class XorElement : public Element
{
    public:
        XorElement() : Element(0, 2, 1, XOR) {};
        virtual void recalc();
        virtual Element*clone();
};

class AndElement : public Element
{
    public:
        AndElement() : Element(0, 2, 1, AND) {};
        virtual void recalc();
        virtual Element*clone();
};

class AndNotElement : public Element
{
    public:
        AndNotElement() : Element(0, 2, 1, ANDNOT) {};
        virtual void recalc();
        virtual Element*clone();
};

class OrElement : public Element
{
    public:
        OrElement() : Element(0, 2, 1, OR) {};
        virtual void recalc();
        virtual Element*clone();
};

class OrNotElement : public Element
{
    public:
        OrNotElement() : Element(0, 2, 1, ORNOT) {};
        virtual void recalc();
        virtual Element*clone();
};

class NotElement : public Element
{
    public:
        NotElement() : Element(0, 1, 1, NOT) {};
        virtual void recalc();
        virtual Element*clone();
};

class LibraryElement : public Element
{
    QString name;
    protected:
        LibraryElement(QString name);
    public:
        LibraryElement(QString name, Document*d);
        void init(Document *d);
        virtual void recalc();
        virtual Element*clone();

        static LibraryElement *fromXml(QDomElement d_el, Document *d);
        QDomElement toXml(QDomDocument);
};

#endif // SIMPLEELEMENTS_H
