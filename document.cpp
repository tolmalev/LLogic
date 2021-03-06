#include "document.h"
#include "element.h"
#include "controller.h"
#include "workpanel.h"
#include "filestructs.h"
#include "complexelement.h"
#include "elementlibrary.h"
#include "simpleelements.h"
#include "algorithm"
#include "mainwindow.h"
#include <QApplication>

Document::Document(int _type, ComplexElement*el, QObject *parent) : QObject(parent), ce(el),_document_type(_type)
{
    now_change = changes.begin();
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

void Document::addElement(Element *e, bool save)
{
    e->setController(c);
    e->d = this;
    c->queue.push_back(e);
    elements.insert(e);
    if(panel != 0)
        panel->addElement(e);
    calcIfNeed();
    changed();
    if(e->type() == COMPLEX)
	connect(((ComplexElement*)e)->d, SIGNAL(documentChanged(Document*)), this, SLOT(changed()));


    if(save)
    {
	QSet<Element*> els;
	els.insert(e);
	QMap<int, QPoint> pts;
	QList<QPair<int, int> >con;
	ElementsChange *ch = new ElementsChange(this, els, pts, con, 1);
	addChange(ch);
    }
}

int Document::addPoint(QPoint pos, int p, bool save)
{
    p = c->new_point(p);
    freePoints[p] = pos;
    if(panel)
        panel->addPoint(p, pos);
    changed();

    if(save)
    {
	QSet<Element*> els;
	QMap<int, QPoint> pts;
	pts[p] = pos;
	QList<QPair<int, int> >con;
	ElementsChange *ch = new ElementsChange(this, els, pts, con, 1);
	addChange(ch);
    }
    return p;
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
    this->fileName = filename;
    int ind = std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"));
    _name = filename.mid(ind+1);
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
    setUnchanged();

    return 1;
}

void Document::setUnchanged()
{
    _changed = 0;
    foreach(Element*e, elements)
	if(e->type() == COMPLEX)
	    ((ComplexElement*)e)->d->setUnchanged();
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
    if(d)
    {
	d->fileName = filename;
	int ind = std::max(filename.lastIndexOf("/"), filename.lastIndexOf("\\"));
	d->_name = filename.mid(ind+1);
	return d;
    }
    else
	return 0;
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
    changed();
}

Document::~Document()
{
    if(panel)
        delete panel;
    delete c;
    if(library)
        delete library;
}

int Document::addConnection(int id1, int id2, bool save)
{
    int res = c->add_connection(id1, id2);
    if(res == 0)
	calcIfNeed();
    if(save)
    {
	ConnectionsChange *ch = new ConnectionsChange(this, id1, id2, 1);
	addChange(ch);
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
	int p;
	foreach(p, ce->in_connections)
	    d->c->new_point(p);
	foreach(p, ce->out_connections)
	    d->c->new_point(p);
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
	    if(e->type() == COMPLEX)
		connect(((ComplexElement*)e)->d, SIGNAL(documentChanged(Document*)), this, SLOT(changed()));
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
    {
        c->calculate();
	if(panel)
	    panel->update();
    }
}

void Document::removePoint(int id)
{
    c->remove_point(id);
    if(panel)
	panel->removePoint(id);
    calcIfNeed();
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
	ce->in[i] = input.at(i).second;

	ce->in_connections[i] = input.at(i).second;
        ce->d->c->connect_in_element(input.at(i).second, ce);
        c->connect_element(input.at(i).second, ce);
    }

    for(int i = 0; i < output.count(); i++)
    {
        output_id_index[output.at(i).second] = i;
	ce->out[i] = output.at(i).second;

	ce->out_connections[i] = output.at(i).second;
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
    connect(ce->d, SIGNAL(documentChanged(Document*)), this, SLOT(changed()));
    connect(ce->d, SIGNAL(instrumentChanged()), MainWindow::wnd, SLOT(instrumentChanged()));
    calcIfNeed();
    changed();
    ce->d->auto_calculation = 1;

    changes.clear();
    now_change = changes.begin();
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

void Document::removeConnections(QSet<QPair<int, int> >s, bool save)
{
    QSet<QPair<int, int> > cons;
    QPair<int ,int> p, p2;
    foreach(p, s)
	 foreach(p2, c->remove_connection(p.first, p.second))
	     cons.insert(p2);
    if(save)
    {
	ConnectionsChange *ch = new ConnectionsChange(this, cons.toList(), 0);
	addChange(ch);
    }
    calcIfNeed();
    changed();
}

void Document::removeConnection(int id1, int id2, bool save)
{
    QSet<QPair<int, int> > cons = c->remove_connection(id1, id2);
    if(save)
    {
	ConnectionsChange *ch = new ConnectionsChange(this, cons.toList(), 0);
	addChange(ch);
    }
    calcIfNeed();
    changed();
}

Document* Document::rootDocument()
{
    if(!ce)
	return this;
    return ((Element*)ce)->d->rootDocument();
}

QDomElement Document::selectionToXml(QDomDocument doc, QSet<Element *> elements, QSet<int> points)
{
    QSet<int> pts = points;
    QDomElement el = doc.createElement("selection");
    int top = 10000, left = 10000;
    foreach(Element*e, elements)
    {
	top = std::min(top, e->view().y);
	left = std::min(left, e->view().x);
    }
    foreach(int i, points)
    {
	top = std::min(top, freePoints[i].y());
	left = std::min(left, freePoints[i].x());
    }

    foreach(Element*e, elements)
    {
	e->_view.x -= left;
	e->_view.y -= top;
	el.appendChild(e->toXml(doc));
	foreach(int i, e->in)
	    pts.insert(i);
	foreach(int i, e->out)
	    pts.insert(i);
	e->_view.x += left;
	e->_view.y += top;
    }
    foreach(int id, points)
    {
	QDomElement pt = doc.createElement("point");
	pt.setAttribute("id", id);
	pt.setAttribute("x", freePoints[id].x()-left);
	pt.setAttribute("y", freePoints[id].y()-top);
	el.appendChild(pt);
    }
    foreach(int id, pts)
    {
	foreach(int i, *c->connections[id])
	{
	    if(pts.find(i) != pts.end())
	    if(id > i)
	    {
		QDomElement connection = doc.createElement("connection");
		connection.setAttribute("from", id);
		connection.setAttribute("to", i);
		el.appendChild(connection);
	    }
	}
    }


    return el;
}

QMimeData* Document::toMimeData(QSet<Element *> elements, QSet<int> points)
{
    QMimeData *d = new QMimeData;
    QDomDocument doc("LDocument");
    doc.appendChild(selectionToXml(doc, elements, points));
    d->setData("LLogic/selection", doc.toByteArray());
    return d;
}

void Document::addToClipboard(QSet<Element *> elements, QSet<int> points)
{
    qApp->clipboard()->setMimeData(toMimeData(elements, points));
}

void Document::addFromClipboard()
{
    QByteArray ba = qApp->clipboard()->mimeData()->data("LLogic/selection");
    QDomDocument doc;
    if(doc.setContent(ba))
    {
	QMap<int, QPoint> _pts;
	QList<QPair<int, int> > con;

	QSet<Element*> els;
	QSet<int> points;
	QMap<int, int> pts;
	QDomElement de = doc.documentElement();
	QDomElement ch_e = de.firstChildElement();
	while(!ch_e.isNull())
	{
	    if(ch_e.tagName() == "element")
	    {
		Element *e = Element::fromXml(ch_e, this);
		if(e == 0)
		    return;
		e->c = c;
		e->d = this;
		for(int i = 0; i < e->in_cnt; i++)
		{
		    int w = e->in.at(i);
		    int p = c->new_point();
		    pts[w] = p;
		    e->in.push_back(p);
		    c->connect_element(p, e);
		}
		for(int i = 0; i < e->in_cnt; i++)
		    e->in.pop_front();
		for(int i = 0; i < e->out_cnt; i++)
		{
		    int w = e->out.at(i);
		    int p = c->new_point();
		    pts[w] = p;
		    e->out.push_back(p);
		    c->connect_in_element(p, e);
		}
		for(int i = 0; i < e->out_cnt; i++)
		    e->out.pop_front();

		elements.insert(e);
		if(e->type() == COMPLEX)
		    connect(((ComplexElement*)e)->d, SIGNAL(documentChanged(Document*)), this, SLOT(changed()));
		if(panel)
		    panel->addElement(e);
		els.insert(e);
	    }
	    else if(ch_e.tagName() == "point")
	    {
		int id = ch_e.attribute("id", "-1").toInt();
		int x   = ch_e.attribute("x", "-1").toInt();
		int y   = ch_e.attribute("y", "-1").toInt();

		if(id == -1 || x == -1 || y==-1)
		    return;
		pts[id] = addPoint(QPoint(x,y), -1, 0);
		points.insert(pts[id]);
		_pts[pts[id]] = QPoint(x, y);
	    }
	    else if(ch_e.tagName() == "connection")
	    {
		int from = ch_e.attribute("from", "-1").toInt();
		int to   = ch_e.attribute("to", "-1").toInt();

		if(to == -1 || from == -1)
		    return;
		if(c->add_connection(pts[from], pts[to]))
		    return;
		con.push_back(QPair<int, int>(pts[from], pts[to]));
	    }
	    ch_e = ch_e.nextSiblingElement();
	}
	if(panel)
	    panel->setSelection(els, points);

	ElementsChange *ch = new ElementsChange(this, els, _pts, con, 1);
	addChange(ch);
    }
    calcIfNeed();
}


bool Document::canUndo()
{
    return now_change != changes.end();
}

bool Document::canRedo()
{
    return now_change != changes.begin();
}

void Document::undo()
{
    if(!canUndo())
	return;
    (*now_change)->undo();
    now_change++;
    if(panel)
    {
	panel->update();
	panel->calculateLines();
    }
    calcIfNeed();
}

void Document::redo()
{
    if(!canRedo())
	return;
    now_change--;
    (*now_change)->redo();
    if(panel)
    {
	panel->update();
	panel->calculateLines();
    }
    calcIfNeed();
}

void Document::addChange(DocumentChange *ch)
{
    while(now_change != changes.begin())
    {
	delete changes.first();
	changes.pop_front();
    }
    changes.push_front(ch);
    now_change = changes.begin();
}

ConnectionsChange::ConnectionsChange(Document *d, int from, int to, bool add) : DocumentChange(d)
{
    QPair<int, int> p(from, to);
    con.push_back(p);
    this->add = add;
}

void ConnectionsChange::undo()
{
    if(add)
	removeConnections();
    else
	addConnections();
}

void ConnectionsChange::redo()
{
    if(add)
	addConnections();
    else
	removeConnections();
}

void ConnectionsChange::addConnections()
{
    QPair<int, int>p;
    foreach(p, con)
	d->addConnection(p.first, p.second, 0);
}

void ConnectionsChange::removeConnections()
{
    QPair<int, int>p;
    foreach(p, con)
    {
	d->removeConnection(p.first, p.second, 0);
    }
}

void MovingChange::undo()
{
    d->move(-dr, els, pts, 0);
}

void MovingChange::redo()
{
    d->move(dr, els, pts, 0);
}


ElementsChange::ElementsChange(Document *d, QSet<Element *> els, QMap<int, QPoint> pts, QList<QPair<int, int> > con, bool add) : DocumentChange(d), els(els), pts(pts), add(add)
{
    con_ch = new ConnectionsChange(d, con, add);
}

void ElementsChange::undo()
{
    if(add)
	removeAll();
    else
	addAll();
}

void ElementsChange::redo()
{
    if(!add)
	removeAll();
    else
	addAll();
}

void ElementsChange::addAll()
{
    d->add(els, pts, 0);
    con_ch->addConnections();
}

void ElementsChange::removeAll()
{
    con_ch->removeConnections();
    d->remove(els, pts.keys().toSet(), 0);
}

void Document::move(QPoint dr, QSet<Element *> els, QSet<int> pts, bool save)
{
    foreach(Element *e, els)
	moveElement(e, QPoint(e->_view.x + dr.x(), e->_view.y+dr.y()));
    foreach(int i, pts)
	moveFreePoint(i, freePoints[i] + dr);

    if(panel)
	panel->move(dr, els, pts);

    if(save && !dr.isNull())
    {
	MovingChange *ch = new MovingChange(this, dr, els, pts);
	addChange(ch);
    }
}

void Document::clone(QPoint dr, QSet<Element *> els, QSet<int> pts, bool save)
{
    QSet<Element*> _els;
    QMap<int, QPoint> _pts;
    foreach(Element *e, els)
    {
	Element *e1 = e->clone();
	e1->_view.x = e->_view.x + dr.x();
	e1->_view.y = e->_view.y + dr.y();
	addElement(e1, 0);
	_els.insert(e1);
    }
    foreach(int i, pts)
    {
	int p = addPoint(freePoints[i] + dr, -1, 0);
	_pts[p] = freePoints[i] + dr;
    }

   if(save)
   {
       QList<QPair<int, int> > con;
       ElementsChange *ch = new ElementsChange(this, _els, _pts, con, 1);
       addChange(ch);
   }
   calcIfNeed();
}

void Document::remove(QSet<Element *> els, QSet<int> pts, bool save)
{
    QMap<int, QPoint> _pts;
    QList<QPair<int, int> >con;
    if(panel)
	panel->remove(els, pts);
    foreach(Element *e, els)
    {
	foreach(int i, e->in)
	{
	    foreach(int id, *c->connections[i])
		con.push_back(QPair<int, int>(i, id));
	    removePoint(i);
	}
	foreach(int i, e->out)
	{
	    foreach(int id, *c->connections[i])
		con.push_back(QPair<int, int>(i, id));
	    removePoint(i);
	}

	elements.remove(e);
	c->removeFromQueue(e);
    }
    foreach(int p, pts)
    {
	foreach(int id, *c->connections[p])
	    con.push_back(QPair<int, int>(p, id));
	_pts[p] = freePoints[p];
	removePoint(p);
	freePoints.remove(p);
    }

    if(save)
    {
	ElementsChange *ch = new ElementsChange(this, els, _pts, con, 0);
	addChange(ch);
    }

    calcIfNeed();
}

void Document::add(QSet<Element *> els, QMap<int, QPoint> pts, bool save)
{
    foreach(Element *e, els)
    {
	foreach(int i, e->in)
	{
	    c->new_point(i);
	    c->connect_element(i, e);
	}
	foreach(int i, e->out)
	{
	    c->new_point(i);
	    c->connect_in_element(i, e);
	}
	e->c = c;
	e->d = this;
	c->queue.push_back(e);
	elements.insert(e);
	if(panel != 0)
	    panel->addElement(e);
    }
    foreach(int i, pts.keys())
	addPoint(pts[i], i, 0);

    if(save)
    {
	QList<QPair<int, int> >con;
	ElementsChange *ch = new ElementsChange(this, els, pts, con, 1);
	addChange(ch);
    }
    calcIfNeed();
}

int Document::newPoint()
{
    return c->new_point();
}

void Document::updateElement(Element *el)
{
    el->_view.height = std::max(el->in_cnt, el->out_cnt) + 1;
    el->_view.width  = std::max(3, el->_view.height*2/3);
    for(int i = 0; i < el->in_cnt; i++)
	c->connect_element(el->in[i], el);
    for(int i = 0; i < el->out_cnt; i++)
	c->connect_in_element(el->out[i], el);
    if(panel)
	panel->updateElementWidget(el);
}
