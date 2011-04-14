#include <QMap>

#include "elementlibrary.h"


ElementLibrary::ElementLibrary(QObject *parent) :
    QObject(parent)
{

}


bool ElementLibrary::hasElement(QString name)
{
    return elements.find(name) != elements.end();
}

ComplexElement * ElementLibrary::getElement(QString name)
{
    if(!hasElement(name))
        return 0;
    return elements[name];
}

int ElementLibrary::addElement(QString name, ComplexElement *e)
{
    if(hasElement(name))
        return -1;
    elements[name] = (ComplexElement*)e->clone();
    elements[name]->text = name;
    return 0;
}

ComplexElement *ElementLibrary::elementFromXml(QDomElement d_el)
{
    ComplexElement *el = new ComplexElement;
    int in_c = d_el.attribute("in_cnt", "-1").toInt();
    int out_c = d_el.attribute("out_cnt", "-1").toInt();
    if(in_c < 0 || out_c < 0)
        return 0;
    el = new ComplexElement(in_c, out_c);

    QDomElement ch_e = d_el.firstChildElement();
    bool view_ok  = 0;
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

    if(!view_ok || !input_connections_ok ||
       !output_connections_ok || !elements_ok || !connections_ok)
    {
        delete el;
        return 0;
    }
    return el;
}

ElementLibrary * ElementLibrary::fromXml(QDomElement d_el)
{
    if(d_el.tagName() != "library")
        return 0;
    ElementLibrary * library = new ElementLibrary;
    QDomElement ch_e = d_el.firstChildElement();
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "element")
        {
            ComplexElement *el = library->elementFromXml(ch_e);
            QString name = ch_e.attribute("name", "");
            if(el == 0 || name =="")
            {
                delete library;
                return 0;
            }
            library->addElement(name, el);
            delete el;
        }
        ch_e = ch_e.nextSiblingElement();
    }

    return library;
}

QDomElement ElementLibrary::elementToXml(QDomDocument doc, QString name)
{
    QDomElement result = doc.createElement("element");
    ComplexElement * el = elements[name];
    result.setAttribute("name", name);
    result.setAttribute("in_cnt", el->in_cnt);
    result.setAttribute("out_cnt", el->out_cnt);

    QDomElement view = doc.createElement("view");
    view.setAttribute("x", 0);
    view.setAttribute("y", 0);
    view.setAttribute("width", el->_view.width);
    view.setAttribute("height", el->_view.height);

    result.appendChild(view);

    result.appendChild(el->inputConnectionsToXml(doc));
    result.appendChild(el->outputConnectionsToXml(doc));
    result.appendChild(el->d->elementsToXml(doc));
    result.appendChild(el->d->connectionsToXml(doc));

    return result;
}

QDomElement ElementLibrary::toXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("library");
    foreach(QString name, elements.keys())
    {
        result.appendChild(elementToXml(doc, name));
    }
    return result;
}

ElementLibrary* ElementLibrary::clone()
{
    ElementLibrary *l = new ElementLibrary;
    foreach(QString name, elements.keys())
    {
        l->addElement(name, elements[name]);
    }
    return l;
}

ElementLibrary::~ElementLibrary()
{
    foreach(QString name, elements.keys())
    {
        delete elements[name];
    }
    elements.clear();
}

QList<QString> ElementLibrary::names()
{
    return elements.keys();
}

int ElementLibrary::removeElement(QString name)
{
    if(elements.find(name) == elements.end())
	return -1;
    delete elements[name];
    elements.remove(name);
    return 0;
}
