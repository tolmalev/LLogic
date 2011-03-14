#include "simpleelements.h"
#include "controller.h"
#include <QApplication>

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


Element* SimpleElement(int type)
{
    Element*e;
    switch(type)
    {
        case AND:       e = new AndElement;         break;
        case ANDNOT:    e = new AndNotElement;      break;
        case OR:        e = new OrElement;          break;
        case ORNOT:     e = new OrNotElement;       break;
        case NOT:       e = new NotElement;         break;
        case SEND:      e = new SendElement;        break;
        case RECEIVE:   e = new ReceiveElement;     break;
    };
    return e;
}
