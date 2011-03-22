#include "document.h"
#include "element.h"
#include "controller.h"
#include "workpanel.h"
#include "filestructs.h"
#include "complexelement.h"
#include "elementlibrary.h"
#include "simpleelements.h"

Document::Document(int _type, ComplexElement*el, QObject *parent) : QObject(parent), ce(el),_document_type(_type)
{
    c = new Controller();
    connect(c, SIGNAL(timeout(Controller*)), this, SLOT(timeout(Controller*)));
    connect(c, SIGNAL(calculation_finished(int)), this, SIGNAL(calculation_finished(int)));
    panel = 0;
    library = 0;
    _name = "Untiteled.lod";
    fileName = _name;

    auto_calculation = 1;
}

void Document::timeout(Controller *_c)
{
    emit timeout(this);
}

void Document::addElement(Element *e)
{
    e->setController(c);
    e->d = this;
    elements.insert(e);
    if(panel != 0)
        panel->addElement(e);
}

WorkPanel* Document::workPanel()
{
    if(!panel)
        createPanel();
    return panel;
}

void Document::createPanel()
{
    if(ce == 0)
        panel = new WorkPanel;
    else
        panel = new WorkPanel(ce);
    panel->d = this;
    foreach(Element* e, elements)
        panel->addElement(e);
    connect(panel, SIGNAL(doubleClicked(ElementWidget*)), this, SIGNAL(doubleClicked(ElementWidget*)));
    connect(panel, SIGNAL(needCalculation(Element*)), this, SLOT(needCalculation(Element*)));
}

void Document::stop_calculation()
{
    c->stop_calculation();
}

int Document::saveToFile(QString filename)
{
    QFile f(filename);
    if(!f.open(QIODevice::WriteOnly))
        return -1;
    QDomDocument doc("LDocument");
    doc.appendChild(toXml(doc));
    if(f.write(doc.toByteArray())== -1)
    {
        f.close();
        return 0;
    }

    f.close();
    return 1;
}

Document* Document::fromFile(QString filename)
{
    QFile f(filename);
    if(!f.open(QIODevice::ReadOnly))
        return 0;
    QDomDocument doc;
    if(!doc.setContent(&f))
    {
        f.close();
        return 0;
    }

    Document *d = fromXml(doc.documentElement());
    return d;
}

void Document::needCalculation(Element *e)
{
    c->queue.push_back(e);
    if(auto_calculation)
    {
        emit calculation_started();
        c->calculate();
        if(panel)
            workPanel()->update();
    }
}

Document::~Document()
{
    if(panel)
        delete panel;
    delete c;
    if(library)
        delete library;
}

int Document::addConnection(int id1, int id2)
{
    int res = c->add_connection(id1, id2);
    if(res == 0 && auto_calculation)
    {
        c->calculate();
        if(panel)
            panel->update();
    }
    return 0;
}

Document* Document::clone()
{
    Document *d = new Document;
    d->_name = name();
    if(library)
        d->library = library->clone();
    foreach(Element*e, elements)
    {
        Element *el = e->clone();
        el->_view = e->_view;
        el->in = e->in;
        el->out = e->out;

        el->c = d->c;
        el->d = d;
        foreach(int id, el->in)
        {
            d->c->new_point(id);
            d->c->connect_element(id, el);
        }

        foreach(int id, el->out)
        {
            d->c->new_point(id);
            d->c->connect_in_element(id, el);
        }

        d->elements.insert(el);
    }

    foreach(int a, c->connections.keys())
    {
        foreach(int b, *c->connections[a])
        {
            d->c->add_connection(a,b);
        }
    }

    return d;
}

bool Document::parseElements(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "element")
        {
            Element *e = Element::fromXml(ch_e, this);
            if(e == 0)
                return 0;
            e->c = c;
            e->d = this;
            foreach(int i, e->in)
            {
                if(c->new_point(i) < 0)
                    return 0;
                c->connect_element(i, e);
            }
            foreach(int i, e->out)
            {
                if(c->new_point(i) < 0)
                    return 0;
                c->connect_in_element(i, e);
            }

            elements.insert(e);
        }
        ch_e = ch_e.nextSiblingElement();
    }
    return 1;
}

bool Document::parseConnections(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "connection")
        {
            int from = ch_e.attribute("from", "-1").toInt();
            int to   = ch_e.attribute("to", "-1").toInt();

            if(to == -1 || from == -1)
                return 0;
            if(c->add_connection(from, to))
                return 0;
        }
        ch_e = ch_e.nextSiblingElement();
    }
    return 1;
}

Document * Document::fromXml(QDomElement d_el)
{
    QString type = d_el.attribute("type");
    if(type != "fullDocument")
        return 0;
    Document *d = new Document;
    d->library = 0;
    QDomElement ch_e = d_el.firstChildElement();
    bool elements_ok = 0;
    bool connections_ok = 0;

    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "elements")
        {
            elements_ok = d->parseElements(ch_e);
        }
        else if(ch_e.tagName() == "connections")
        {
            connections_ok = d->parseConnections(ch_e);
        }
        else if(ch_e.tagName() == "library")
        {
            d->library = ElementLibrary::fromXml(ch_e);
        }
        ch_e = ch_e.nextSiblingElement();
    }

    if(!elements_ok || !connections_ok)
    {
        delete d;
        return 0;
    }
    if(d->library == 0)
        d->library = new ElementLibrary;
    return d;
}

QDomElement Document::elementsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("elements");
    foreach(Element *e, elements)
    {
        result.appendChild(e->toXml(doc));
    }
    return result;
}

QDomElement Document::connectionsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("connections");
    foreach(int a, c->connections.keys())
    {
        foreach(int b, *c->connections[a])
        {
            if(a < b)
            {
                QDomElement connection = doc.createElement("connection");
                connection.setAttribute("from", a);
                connection.setAttribute("to", b);
                result.appendChild(connection);
            }
        }
    }

    return result;
}

QDomElement Document::toXml(QDomDocument doc)
{
    QDomElement document = doc.createElement("Document");
    document.setAttribute("type", "fullDocument");
    document.appendChild(library->toXml(doc));
    document.appendChild(elementsToXml(doc));
    document.appendChild(connectionsToXml(doc));
    return document;
}

bool Document::canConnect(int id1, int id2)
{
    return c->canConnect(id1, id2);
}

void Document::calcIfNeed()
{
    if(auto_calculation)
        c->calculate();
}

void Document::removePoint(int id)
{
    c->remove_point(id);
}
