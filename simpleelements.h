#ifndef SIMPLEELEMENTS_H
#define SIMPLEELEMENTS_H

#include "classes.h"
#include "element.h"

Element* SimpleElement(int type);

class SegmentElement : public Element
{
    friend class SegmentElementWidget;
	int state;
    public:
	SegmentElement() : Element(0, 8, 0, SEGMENT), state(0){};
	virtual void recalc();
	virtual Element*clone();
};

class NumberSendElement8 : public Element
{
    friend class NumberSendElement8Widget;
	int num;
    public:
	NumberSendElement8() : Element(0, 0, 8, NUMSEND), num(0){};
	virtual void recalc();
	virtual Element*clone();
};

class NumberRecieveElement8 : public Element
{
    friend class NumberRecieveElement8Widget;
	int num;
    public:
	NumberRecieveElement8() : Element(0, 8, 0, NUMRECIEVE), num(0){};
	virtual void recalc();
	virtual Element*clone();
};


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
        XorElement() : Element(0, 2, 1, XOR, "Xor") {};
        virtual void recalc();
        virtual Element*clone();
};

class AndElement : public Element
{
    public:
        AndElement() : Element(0, 2, 1, AND, "And") {};
        virtual void recalc();
        virtual Element*clone();
};

class AndNotElement : public Element
{
    public:
        AndNotElement() : Element(0, 2, 1, ANDNOT, "!And") {};
        virtual void recalc();
        virtual Element*clone();
};

class OrElement : public Element
{
    public:
        OrElement() : Element(0, 2, 1, OR, "Or") {};
        virtual void recalc();
        virtual Element*clone();
};

class OrNotElement : public Element
{
    public:
        OrNotElement() : Element(0, 2, 1, ORNOT, "!Or") {};
        virtual void recalc();
        virtual Element*clone();
};

class NotElement : public Element
{
    public:
        NotElement() : Element(0, 1, 1, NOT, "Not") {};
        virtual void recalc();
        virtual Element*clone();
};

#endif // SIMPLEELEMENTS_H
