#include "document.h"
#include "element.h"
#include "controller.h"
#include "workpanel.h"
#include "filestructs.h"
#include "complexelement.h"
#include "elementlibrary.h"
#include "simpleelements.h"
#include "algorithm"

Document::Document(int _type, ComplexElement*el, QObject *parent) : QObject(parent), ce(el),_document_type(_type)
{
    c = new Controller();
    connect(c, SIGNAL(timeout(Controller*)), this, SLOT(timeout(Controller*)));
    connect(c, SIGNAL(calculation_finished(int)), this, SIGNAL(calculation_finished(int)));
    panel = 0;
    library = 0;
    _changed = 0;
    _name = "";
    fileName = _name;
    instrument = SELECT;

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
    c->queue.push_back(e);
    elements.insert(e);
    if(panel != 0)
        panel->addElement(e);
    changed();
}

void Document::addPoint(QPoint pos, int p)
{
    p = c->new_point(p);
    freePoints[p] = pos;
    if(panel)
        panel->addPoint(p, pos);
    changed();
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

    foreach(int id, freePoints.keys())
    {
        panel->addPoint(id, freePoints[id]);
    }
    connect(panel, SIGNAL(doubleClicked(ElementWidget*)), this, SIGNAL(doubleClicked(ElementWidget*)));
    connect(panel, SIGNAL(needCalculation(Element*)), this, SLOT(needCalculation(Element*)));
}

void Document::stop_calculation()
{
    c->stop_calculation();
}

int Document::saveToFile(QString filename)
{
    if(filename == "")
        filename = this->fileName;
    _name = filename.mid(filename.lastIndexOf("/")+1);
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
    _changed=0;
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
    d->fileName = filename;
    int ind = std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"));
    d->_name = filename.mid(ind+1);
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
        changed();
    }
    return res;
}

Document* Document::clone()
{
    Document *d = new Document;
    d->_name = name();
    if(library)
        d->library = library->clone();
    if(ce)
    {
	QPair<int, int> p;
	foreach(p, ce->in_connections)
	    d->c->new_point(p.second);
	foreach(p, ce->out_connections)
	    d->c->new_point(p.first);
    }
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

bool Document::parseFreePoints(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    while(!ch_e.isNull())
    {
        if(ch_e.tagName() == "point")
        {
            int id = ch_e.attribute("id", "-1").toInt();
            int x   = ch_e.attribute("x", "-1").toInt();
            int y   = ch_e.attribute("y", "-1").toInt();


            if(id == -1 || x == -1 || y==-1)
                return 0;
            addPoint(QPoint(x,y), id);
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
    bool free_points_ok = 1;

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
        else if(ch_e.tagName() == "free_points")
        {
            free_points_ok = d->parseFreePoints(ch_e);
        }
        ch_e = ch_e.nextSiblingElement();
    }

    if(!elements_ok || !connections_ok || !free_points_ok)
    {
        delete d;
        return 0;
    }
    if(d->library == 0)
        d->library = new ElementLibrary;
    d->_changed = 0;
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
    if(library)
        document.appendChild(library->toXml(doc));
    document.appendChild(elementsToXml(doc));
    document.appendChild(freePointsToXml(doc));
    document.appendChild(connectionsToXml(doc));
    return document;
}

QDomElement Document::freePointsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("free_points");
    foreach(int id, freePoints.keys())
    {
        QDomElement pt = doc.createElement("point");
        pt.setAttribute("id", id);
        pt.setAttribute("x", freePoints[id].x());
        pt.setAttribute("y", freePoints[id].y());
        result.appendChild(pt);
    }
    return result;
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
    changed();
}

void Document::setInstrument(int in)
{
    if(instrument == ADDELEMENT || instrument == ADDPOINT)
        panel->stopAdding(1);
    instrument = in;
    if(in == ADDPOINT)
        workPanel()->startAddingPoint();
}

void Document::setAddingElement(Element *el)
{
    if(panel)
        panel->setAddingElement(el);
}

void Document::changed()
{
    _changed = 1;
    emit documentChanged(this);
}

void Document::moveElement(Element *e, QPoint pos)
{
    if(QPoint(e->view().x, e->view().y) != pos)
    {
        e->_view.x = pos.x();
        e->_view.y = pos.y();
        changed();
    }
}

void Document::moveFreePoint(int p, QPoint pos)
{
    if(freePoints.find(p) != freePoints.end())
    {
        freePoints[p] = pos;
	changed();
    }
}

bool operator<(QPair<QPoint, int> a, QPair<QPoint, int> b)
{
    return (a.first.y() < b.first.y()) || (a.first.y() == b.first.y() && a.first.x() < b.first.x());
}

void Document::createComplex(QSet<Element *> elements, QList<int> points)
{
    QSet<int> pts;
    QVector<QPair<QPoint, int> > input;
    QVector<QPair<QPoint, int> > output;
    int top = 10000, left = 10000;
    foreach(Element *e, elements)
    {
        left = std::min(left, e->view().x);
        top = std::min(top, e->view().y);
        foreach(int i, e->in)
            pts.insert(i);
        foreach(int i, e->out)
            pts.insert(i);
    }
    foreach(int i, points)
        pts.insert(i);    

    foreach(int i, points)
    {
        int id = c->has_in_e_connected(i);
        if(pts.find(id) == pts.end())
            input.push_back(QPair<QPoint, int> (freePoints[i], i));
        else
            output.push_back(QPair<QPoint, int> (freePoints[i], i));
        freePoints.remove(i);
    }
    qSort(input);
    qSort(output);
    ComplexElement *ce = new ComplexElement(input.count(), output.count());
    ce->d->auto_calculation = 0;

    QMap<int, int> input_id_index;
    QMap<int, int> output_id_index;
    QPair<QPoint, int> p;
    foreach(int i, pts)
        ce->d->c->new_point(i);
    for(int i = 0; i < input.count(); i++)
    {
        input_id_index[input.at(i).second] = i;
        ce->in.push_back(input.at(i).second);

        ce->in_connections.push_back(QPair<int, int>(i, input.at(i).second));
        ce->d->c->connect_in_element(input.at(i).second, ce);
        c->connect_element(input.at(i).second, ce);
    }

    for(int i = 0; i < output.count(); i++)
    {
        output_id_index[output.at(i).second] = i;
        ce->out.push_back(output.at(i).second);

        ce->out_connections.push_back(QPair<int, int>(output.at(i).second, i));
        c->connect_in_element(output.at(i).second, ce, 1);
    }


    foreach(int i, pts)
    {
        foreach(int id, *c->connections[i])
        {
            if(ce->d->addConnection(i, id) == 0)
                c->remove_connection(i, id);
        }
    }

    foreach(Element *e, elements)
    {
        e->_view.x -= left-5;
	e->_view.y -= top-1;
        foreach(int i, e->in)
        {
            ce->d->c->connect_element(i, e);
            c->remove_point(i);
        }
        foreach(int i, e->out)
        {
            ce->d->c->connect_in_element(i, e);
            c->remove_point(i);
        }
        e->c = ce->d->c;
    }
    foreach(Element *e, elements)
    {
        c->removeFromQueue(e);
        ce->d->elements.insert(e);
        this->elements.remove(e);
    }

    ce->_view.x = left;
    ce->_view.y = top;

    ((Element*)ce)->d = this;
    ((Element*)ce)->c = c;
    c->queue.push_back(ce);
    ce->d->library = new ElementLibrary();
    this->elements.insert(ce);
    if(panel != 0)
        panel->addElement(ce);
    changed();
    ce->d->auto_calculation = 1;
}

Element* Document::getLibraryElement(QString name)
{
    if(!ce)
    {
	if(!library)
	    return 0;
	return library->getElement(name);
    }
    else
	return ((Element*)ce)->d->getLibraryElement(name);
}

QList<QString> Document::libraryNames()
{
    if(!ce)
	return library->names();
    else
	return ((Element*)ce)->d->libraryNames();
}

int Document::addLibraryElement(QString name, ComplexElement *e)
{
    if(!ce)
    {
	int res = library->addElement(name, e);
	if(res)
	    return res;
	emit libraryChanged();
	changed();
	return 0;
    }
    else
	return ((Element*)ce)->d->addLibraryElement(name, e);
}

int Document::removeLibraryElement(QString name)
{
    if(!ce)
    {
	int res = library->removeElement(name);
	if(res)
	    return res;
	emit libraryChanged();
	changed();
	return 0;
    }
    else
	return ((Element*)ce)->d->removeLibraryElement(name);

}

void Document::removeConnection(int id1, int id2)
{
    c->remove_connection(id1, id2);
    changed();
}
