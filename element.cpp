#include "element.h"
#include "simpleelements.h"
#include "complexelement.h"
#include "controller.h"
#include "cmath"
using namespace std;

Element::Element()
{
    c = 0;
    in_cnt = out_cnt = -1;
    _type = SIMPLE;
}

Element::Element(Controller* _c, int _in_cnt, int _out_cnt, int __type, QString _text)
{
    c=0;
    text = _text;
    in_cnt = _in_cnt;
    out_cnt = _out_cnt;
    in.resize(in_cnt);
    out.resize(out_cnt);
    _type = __type;
    setController(_c);
    _view.height = max(in_cnt, out_cnt) + 1;
    _view.width  = max(3, _view.height*2/3);
    _view.x = _view.y = 0;
}

void Element::setController(Controller *_c)
{
    if(c != 0)
	disconnectControler();
    c = _c;

    if(!c)return;

    int id;
    for(int i = 0; i < in_cnt; i++)
    {
	id = c->new_point();
	c->connect_element(id, this);
	in[i] = id;
    }
    for(int i = 0; i < out_cnt; i++)
    {
	id = c->new_point();
	c->connect_in_element(id, this);
	out[i] = id;
    }
}

void Element::disconnectControler()
{
    foreach(int id, in)
	c->remove_point(id);
    foreach(int id, out)
	c->remove_point(id);
    in.clear();
    out.clear();
}

bool Element::parseView(QDomElement d_el)
{
    _view.x      = d_el.attribute("x", "-1").toInt();
    _view.y      = d_el.attribute("y", "-1").toInt();
    if(d_el.hasAttribute("width"))
	_view.width  = d_el.attribute("width").toInt();
    if(d_el.hasAttribute("height"))
	_view.height = d_el.attribute("height").toInt();

    return !(_view.x < 0 || _view.y < 0 || _view.width < 0 || _view.height < 0);
}

bool Element::parseInputPoints(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    int in_c = in_cnt;
    while(!ch_e.isNull())
    {
	if(ch_e.tagName() == "point")
	{
	    int id = ch_e.attribute("id", "-1").toInt();
	    int index = ch_e.attribute("index", "-1").toInt();

	    if(id < 0 || index < 0 || index >= in_cnt)
		return 0;

	    in[index] = id;
	    in_c--;
	}
	ch_e = ch_e.nextSiblingElement();
    }
    return in_c==0;
}

bool Element::parseOutputPoints(QDomElement d_el)
{
    QDomElement ch_e = d_el.firstChildElement();
    int out_c = out_cnt;
    while(!ch_e.isNull())
    {
	if(ch_e.tagName() == "point")
	{
	    int id = ch_e.attribute("id", "-1").toInt();
	    int index = ch_e.attribute("index", "-1").toInt();

	    if(id < 0 || index < 0 || index >= out_cnt)
		return 0;

	    out[index] = id;
	    out_c--;
	}
	ch_e = ch_e.nextSiblingElement();
    }
    return !out_c;
}

Element * Element::fromXml(QDomElement d_el, Document*d)
{
    Element * el = 0;
    if(d_el.tagName() != "element")
	return 0;
    QString type = d_el.attribute("type");
    if(type == "")
	return 0;
    if(type == "and")
	el = new AndElement();
    else if(type == "or")
	el = new OrElement();
    else if(type == "not")
	el = new NotElement();
    else if(type == "andnot")
	el = new AndNotElement();
    else if(type == "ornot")
	el = new OrNotElement();
    else if(type =="send")
	el = new SendElement();
    else if(type == "receive")
	el = new ReceiveElement();
    else if(type == "xor")
	el = new XorElement();
    else if(type == "8bitsend")
	el = new NumberSendElement8();
    else if(type == "8bitrecieve")
	el = new NumberRecieveElement8();
    else if(type == "if")
	el = new IfElement();
    else if(type == "segment")
	el = new SegmentElement();
    else if(type == "complex")
	return ComplexElement::fromXml(d_el);
    else
	return 0;

    QDomElement ch_e = d_el.firstChildElement();
    el->text = d_el.attribute("text", "");
    if(el->type() == NUMSEND)
	((NumberSendElement8*)el)->update();
    if(el->type() == IF)
	((IfElement*)el)->update();
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

QString typeString(int type)
{
    switch(type)
    {
	case AND:	return "and";
	case OR:	return "or";
	case NOT:	return "not";
	case COMPLEX:	return "complex";
	case ANDNOT:	return "andnot";
	case ORNOT:	return "ornot";
	case XOR:	return "xor";
	case SEND:	return "send";
	case RECEIVE:	return "receive";
	case NUMSEND:	return "8bitsend";
	case NUMRECIEVE:return "8bitrecieve";
	case SEGMENT:	return "segment";
	case IF:	return "if";
    }
    return "";
}

QDomElement Element::viewToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("view");
    result.setAttribute("x", _view.x);
    result.setAttribute("y", _view.y);
    result.setAttribute("width", _view.width);
    result.setAttribute("height", _view.height);
    return result;
}

QDomElement Element::inputPointsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("input_points");
    for(int i = 0; i < in_cnt; i++)
    {
	QDomElement p = doc.createElement("point");
	p.setAttribute("id", in[i]);
	p.setAttribute("index", i);
	result.appendChild(p);
    }
    return result;
}

QDomElement Element::outputPointsToXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("output_points");
    for(int i = 0; i < out_cnt; i++)
    {
	QDomElement p = doc.createElement("point");
	p.setAttribute("id", out[i]);
	p.setAttribute("index", i);
	result.appendChild(p);
    }
    return result;
}

QDomElement Element::toXml(QDomDocument doc)
{
    QDomElement result = doc.createElement("element");
    result.setAttribute("type", typeString(_type));
    result.setAttribute("text", text);

    result.appendChild(viewToXml(doc));
    result.appendChild(inputPointsToXml(doc));
    result.appendChild(outputPointsToXml(doc));

    return result;
}
