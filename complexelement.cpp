#include "complexelement.h"
#include "controller.h"
#include "elementlibrary.h"
#include "workpanel.h"
#include "simpleelements.h"

ComplexElement::ComplexElement(int _in_cnt, int _out_cnt )
{
    d = new Document(Document::ELEMENT, this);
    d->library = 0;
    text = "complex";
    in_cnt = _in_cnt;
    out_cnt = _out_cnt;

    if(in_cnt > 0)
    {
	in.resize(in_cnt);
	in_connections.resize(in_cnt);
    }
    if(out_cnt > 0)
    {
	out.resize(out_cnt);
	out_connections.resize(out_cnt);
    }

    _type = COMPLEX;

    _view.height = std::max(in_cnt, out_cnt) + 1;
    _view.width  = std::max(3, _view.height*2/3);
    _view.x = _view.y = 0;
}

ComplexElement::~ComplexElement()
{
    delete d;
}

void ComplexElement::recalc()
{
    QPair<int, int> p;
    for(int i = 0; i < in_cnt; i++)
    {
	d->c->set(in_connections[i], c->get(in[i]));
    }
    d->c->calculate(1);
    for(int i = 0; i <out_cnt; i++)
    {
	c->set(out[i], d->c->get(out_connections[i]));
    }
}

Element* ComplexElement::clone()
{
    ComplexElement *el = new ComplexElement;
    el->_view = _view;
    el->in_cnt = in_cnt;
    el->out_cnt = out_cnt;
    el->in.resize(in_cnt);
    el->out.resize(out_cnt);
    el->in_connections = in_connections;
    el->out_connections = out_connections;
    el->text = text;
    delete el->d;
    el->d = d->clone();
    el->d->ce = el;
    return el;
}

bool ComplexElement::parseInputConnections(QDomElement d_el)
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
	    //in_connections.push_back(QPair<int, int>(from, to));
	    in_connections[from] = to;
            d->c->new_point(to);
        }
        ch_e = ch_e.nextSiblingElement();
    }
    return 1;
}

bool ComplexElement::parseOutputConnections(QDomElement d_el)
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
	    //out_connections.push_back(QPair<int, int>(from, to));
	    out_connections[to] = from;
            d->c->new_point(from);
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
    el = new ComplexElement(in_c, out_c);
    el->text = d_el.attribute("name", "complex");

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
        else if(ch_e.tagName() == "library")
        {
            el->d->library = ElementLibrary::fromXml(ch_e);
        }

        ch_e = ch_e.nextSiblingElement();
    }

    if(!view_ok || !input_points_ok || !output_points_ok || !input_connections_ok ||
       !output_connections_ok || !elements_ok || !connections_ok)
    {
        delete el;
        return 0;
    }
    if(el->d->library == 0)
        el->d->library = new ElementLibrary;
    return el;
}

QDomElement ComplexElement::inputConnectionsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("input_connections");
    for(int i =0; i < in_cnt; i++)
    {
        QDomElement con = doc.createElement("connection");
	con.setAttribute("from", i);
	con.setAttribute("to", in_connections[i]);
        result.appendChild(con);
    }

    return result;
}

QDomElement ComplexElement::outputConnectionsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("output_connections");
    for(int i= 0; i < out_cnt; i++)
    {
        QDomElement con = doc.createElement("connection");
	con.setAttribute("from", out_connections[i]);
	con.setAttribute("to", i);
        result.appendChild(con);
    }

    return result;
}

QDomElement ComplexElement::toXml(QDomDocument doc)
{
    QDomElement result = Element::toXml(doc);
    result.setAttribute("in_cnt", in_cnt);
    result.setAttribute("out_cnt", out_cnt);
    result.setAttribute("name", text);
    result.appendChild(inputConnectionsToXml(doc));
    result.appendChild(outputConnectionsToXml(doc));
    result.appendChild(d->elementsToXml(doc));
    result.appendChild(d->connectionsToXml(doc));

    return result;
}

void ComplexElement::updateDocumentName()
{
    QString name = Element::d->name() + "\\" + text;
    d->_name = name;
}

void ComplexElement::buildTable(QString fileName)
{
    ComplexElement *e = (ComplexElement*)clone();
    if(in_cnt > 10)
	return;
    QFile f(fileName);
    if(!f.open(QFile::WriteOnly))
	return;

    for(int i = 0; i < 1<<in_cnt; i++)
    {
	QString str = "";
	for(int j = 0; j < in_cnt; j++)
	    str += QString::number((i & (1 << j)) != 0) + " ";
	for(int i = 0; i < in_cnt; i++)
	{
	    e->d->c->set(in_connections[i], (i & (1 << i)) != 0);
	}
	str += " | ";
	e->d->c->calculate(1);
	int out_res = 0;
	for(int i = 0; i < out_cnt; i++)
	{
	    out_res += (1<<i) * e->d->c->get(out_connections[i]);
	}
	for(int j = 0; j < out_cnt; j++)
	    str += QString::number((out_res & (1 << j)) != 0) + " ";

	str += "\r\n";
	f.write(str.toAscii());
    }
    f.close();
    delete e;
}

ComplexElement * ComplexElement::emptyElement(int in_cnt, int out_cnt)
{
    ComplexElement *el = new ComplexElement(in_cnt, out_cnt);
    for(int i = 0; i < in_cnt; i++)
	el->in_connections[i] = el->d->c->new_point();
    for(int i = 0; i < out_cnt; i++)
	el->out_connections[i] = el->d->c->new_point();
    return el;
}

void ComplexElement::addInPoint(int id, int before)
{
    before = std::min(before, in_cnt);
    qWarning("insert %d %d", id, before);
    int was = 0;
    int ind=-1;
    if((ind = in_connections.indexOf(id)) >= 0)
    {
	if(ind == before)
	    return;
	in_connections.remove(ind);
	was = in[ind];
	in.remove(ind);
	in_cnt--;
	if(ind <= before)
	    before--;
	qWarning("insert2 %d %d", id, before);
    }
    QVector<int>::iterator it = in_connections.begin()+before;
    QVector<int>::iterator it2 = in.begin() + before;
    in_connections.insert(it, id);
    in_cnt++;
    _view.height = std::max(in_cnt, out_cnt) + 1;
    _view.width  = std::max(3, _view.height*2/3);
    if(!was)
	in.insert(it2, Element::d->newPoint());
    else
	in.insert(it2, was);
    Element::d->updateElement(this);
}

void ComplexElement::addOutPoint(int id, int before)
{
    before = std::min(before, out_cnt);
    QVector<int>::iterator it = out_connections.begin()+before;
    QVector<int>::iterator it2 = out.begin() + before;
    bool was = 0;
    if(int ind = out_connections.indexOf(id) >= 0)
    {
	was = 1;
	out_connections.remove(ind);
	out.remove(ind);
	out_cnt--;
    }
    out_connections.insert(it, id);
    out_cnt++;
    _view.height = std::max(out_cnt, out_cnt) + 1;
    _view.width  = std::max(3, _view.height*2/3);
    if(!was)
    {
	out.insert(it2, Element::d->newPoint());
    }
    Element::d->updateElement(this);
    for(int i = 0; i < out_cnt; i++)
	qWarning("%d %d", out[i], out_connections[i]);
}

void ComplexElement::removeInPoint(int id)
{
    int ind = in_connections.indexOf(id);
    if(ind < 0)
	return;
    in_connections.remove(ind, 1);
    Element::d->removePoint(in[ind]);
    in.remove(ind, 1);
    in_cnt--;
    Element::d->updateElement(this);
}
