#include "simpleelements.h"
#include "controller.h"
#include "document.h"
#include "elementlibrary.h"
#include <QApplication>



void NumberSendElement8::recalc()
{
    int n = num;
    for(int i = 0; i < 8; i++)
    {
	c->set(out[i], n%2);
	n /= 2;
    }
}

void NumberRecieveElement8::recalc()
{
    num=0;
    for(int i = 7; i >= 0; i--)
    {
	num=2*num+c->get(in[i]);
    }
}

void SegmentElement::recalc()
{
    state=0;
    for(int i = 7; i >= 0; i--)
    {
	state=2*state+c->get(in[i]);
    }
}

Element* NumberSendElement8::clone()
{
    NumberSendElement8 *ns = new NumberSendElement8;
    ns->num = num;
    return ns;
}

Element* NumberRecieveElement8::clone()
{
    NumberRecieveElement8 *ns = new NumberRecieveElement8;
    ns->num = 0;
    return ns;
}

Element* SegmentElement::clone()
{
    SegmentElement *ns = new SegmentElement();
    ns->state = 0;
    return ns;
}

void SendElement::recalc()
{
    c->set(out.first(), val);
}

Element* SendElement::clone()
{
    return new SendElement();
}

void ReceiveElement::recalc()
{
    val = c->get(in.first());
}

Element* ReceiveElement::clone()
{
    return new ReceiveElement();
}

void XorElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), c->get(in.first()) ^ c->get(in.last()));
    }
}

Element* XorElement::clone()
{
    return new XorElement();
}

void AndElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), c->get(in.first()) && c->get(in.last()));
    }
}

Element* AndElement::clone()
{
    return new AndElement();
}

void AndNotElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), !(c->get(in.first()) && c->get(in.last())));
    }
}

Element* AndNotElement::clone()
{
    return new AndNotElement();
}

void OrElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), c->get(in.first()) || c->get(in.last()));
    }
}

Element* OrElement::clone()
{
    return new OrElement();
}

void OrNotElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), !(c->get(in.first()) || c->get(in.last())));
    }
}

Element* OrNotElement::clone()
{
    return new OrNotElement();
}

void NotElement::recalc()
{
    if(c != 0)
    {
        c->set(out.first(), !c->get(in.first()));
    }
}

Element* NotElement::clone()
{
    return new NotElement();
}
