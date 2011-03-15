#include "complexelement.h"
#include "controller.h"

ComplexElement::ComplexElement()
{
    d = new Document(Document::ELEMENT);

    _type = COMPLEX;
}

void ComplexElement::recalc()
{
    QPair<int, int> p;
    foreach(p, in_connections)
    {
        d->c->set(p.second, c->get(in[p.first]));
    }
    d->c->calculate(1);
    foreach(p, out_connections)
    {
        c->set(out[p.second], d->c->get(p.first));
    }
}

Element* ComplexElement::clone()
{
    ComplexElement *el = new ComplexElement;
    delete el->d;
    el->d = d->clone();
    return el;
}


int ComplexElement::saveToFile(FILE *f)
{
    int r = d->saveToFile(f);
    if(r)return r;

    return 0;
}

bool ComplexElement::parseInputConnections(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    int in_c = in_cnt;
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "connection")
        {
            int from = ch_e.attribute("from", "-1").toInt();
            int to   = ch_e.attribute("to", "-1").toInt();

            if(to == -1 || from == -1)
                return 0;
            in_connections.push_back(QPair<int, int>(from, to));
        }
        ch_e = ch_e.nextSiblingElement();
    }
    return 1;
}

bool ComplexElement::parseOutputConnections(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    int out_c = out_cnt;
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "connection")
        {
            int from = ch_e.attribute("from", "-1").toInt();
            int to   = ch_e.attribute("to", "-1").toInt();

            if(to == -1 || from == -1)
                return 0;
            out_connections.push_back(QPair<int, int>(from, to));
        }
        ch_e = ch_e.nextSiblingElement();
    }
    return 1;
}

ComplexElement * ComplexElement::fromXml(QDomElement d_el)
{
    ComplexElement * el;

    if(d_el.tagName() != "element")
        return 0;
    QString type = d_el.attribute("type");
    if(type != "complex")
        return 0;

    int in_c = d_el.attribute("in_cnt", "-1").toInt();
    int out_c = d_el.attribute("out_cnt", "-1").toInt();
    if(in_c < 0 || out_c < 0)
        return 0;
    el = new ComplexElement;
    el->in_cnt = in_c;
    el->out_cnt = out_c;
    el->in.resize(in_c);
    el->out.resize(out_c);
    el->in_connections.resize(in_c);
    el->out_connections.resize(out_c);

    QDomElement ch_e = d_el.firstChildElement();
    bool view_ok  = 0;
    bool input_points_ok = 0;
    bool output_points_ok = 0;
    bool input_connections_ok = 0;
    bool output_connections_ok = 0;
    bool elements_ok = 0;
    bool connections_ok = 0;
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
        else if(ch_e.tagName() == "input_connections")
        {
            input_connections_ok = el->parseInputConnections(ch_e);
        }
        else if(ch_e.tagName() == "output_connections")
        {
            output_connections_ok = el->parseOutputConnections(ch_e);
        }
        else if(ch_e.tagName() == "elements")
        {
            elements_ok = el->d->parseElements(ch_e);
        }
        else if(ch_e.tagName() == "connections")
        {
            connections_ok = el->d->parseConnections(ch_e);
        }

        ch_e = ch_e.nextSiblingElement();
    }

    if(!view_ok || !input_points_ok || !output_points_ok || !input_connections_ok ||
       !output_connections_ok || !elements_ok || !connections_ok)
    {
        delete el;
        return 0;
    }
    return el;
}

