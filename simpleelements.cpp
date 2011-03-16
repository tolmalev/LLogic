#include "simpleelements.h"
#include "controller.h"
#include "document.h"
#include "elementlibrary.h"
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

LibraryElement::LibraryElement(QString name)
{
    this->name = name;
}

LibraryElement::LibraryElement(QString name, Document *d)
{
    this->name = name;
    init(d);
}

void LibraryElement::recalc()
{
    Element *e = d->library->getElement(name);
    for(int i = 0; i < in_cnt; i++)
        e->in[i] = in[i];
    for(int i = 0; i < out_cnt; i++)
        e->out[i] = out[i];
    e->c = c;
    e->recalc();
}

void LibraryElement::init(Document *d)
{
    this->d = d;
    if(!d)
        return;
    Element *e = d->library->getElement(name);
    if(e)
    {
        this->_view = e->_view;
        this->in_cnt = e->in_cnt;
        this->out_cnt = e->out_cnt;
        in.resize(in_cnt);
        out.resize(out_cnt);
    }
}

Element* LibraryElement::clone()
{
    return new LibraryElement(name, d);
}

LibraryElement* LibraryElement::fromXml(QDomElement d_el, Document *d)
{
    if(d_el.tagName() != "element")
        return 0;
    if(d_el.attribute("type", "") != "library")
        return 0;
    if(d_el.attribute("name", "") == "")
        return 0;
    LibraryElement *el = new LibraryElement(d_el.attribute("name", ""), d);
    QDomElement ch_e = d_el.firstChildElement();
    bool view_ok  = 0;
    bool input_points_ok = 0;
    bool output_points_ok = 0;
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "view")
        {
            view_ok = el->parseView(ch_e);
        }
        else if(ch_e.tagName() == "input_points")
        {
            input_points_ok = el->parseInputPoints(ch_e);
        }
        else if(ch_e.tagName() == "output_points")
        {
            output_points_ok = el->parseOutputPoints(ch_e);
        }

        ch_e = ch_e.nextSiblingElement();
    }

    if(!view_ok || !input_points_ok || !output_points_ok)
    {
        delete el;
        return 0;
    }
    return el;
}

QDomElement LibraryElement::toXml(QDomDocument doc)
{
    QDomElement el = doc.createElement("element");
    el.setAttribute("type", "library");
    el.setAttribute("name", name);

    QDomElement v = doc.createElement("view");
    v.setAttribute("x", _view.x);
    v.setAttribute("y", _view.y);

    el.appendChild(v);

    el.appendChild(inputPointsToXml(doc));
    el.appendChild(outputPointsToXml(doc));

    return el;
}
