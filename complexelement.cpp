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
        d->c->set(p.second, c->get(p.first));
    }
    d->c->calculate(1);
    foreach(p, in_connections)
    {
        c->set(p.second, d->c->get(p.first));
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

