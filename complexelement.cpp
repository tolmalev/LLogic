#include "complexelement.h"
#include "controller.h"
#include "elementlibrary.h"
#include "workpanel.h"
#include "simpleelements.h"
#include "math.h"

using namespace std;

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
    qWarning("insert %d %d", id, before);
    int was = 0;
    int ind=-1;
    if((ind = out_connections.indexOf(id)) >= 0)
    {
	if(ind == before)
	    return;
	out_connections.remove(ind);
	was = out[ind];
	out.remove(ind);
	out_cnt--;
	if(ind <= before)
	    before--;
	qWarning("insert2 %d %d", id, before);
    }
    QVector<int>::iterator it = out_connections.begin()+before;
    QVector<int>::iterator it2 = out.begin() + before;
    out_connections.insert(it, id);
    out_cnt++;
    _view.height = std::max(in_cnt, out_cnt) + 1;
    _view.width  = std::max(3, _view.height*2/3);
    if(!was)
	out.insert(it2, Element::d->newPoint());
    else
	out.insert(it2, was);
    Element::d->updateElement(this);
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

void ComplexElement::removeOutPoint(int id)
{
    int ind = out_connections.indexOf(id);
    if(ind < 0)
	return;
    out_connections.remove(ind, 1);
    Element::d->removePoint(out[ind]);
    out.remove(ind, 1);
    out_cnt--;
    Element::d->updateElement(this);
}

int ComplexElement::check(QString s) {
    int k=0, error=0, i, n=s.length();

    bool ok;
    s.toInt(&ok,10);
    if (ok) {
        return 1;
    }

    ok=1;
    for (i=0; i<n; i++) {
        if (s[i].cell()=='(') k++;
        if (s[i].cell()==')') k--;
        if (k<0) error=1;
        if (i==0 && k!=1) ok=0;
        if (i<n-1 && k==0) ok=0;
    }
    if (k!=0) error=1;
    if (error) {
        qWarning("error with bracers");
        return 0;
    }

    if (ok) {
        s.remove(0,1);
        s.remove(n-2,1);
        return check(s);
    }

    k=0;
    for (i=0; i<n; i++) {
        if (s[i].cell()=='(') k++;
        if (s[i].cell()==')') k--;
        if (k==0 && !s[i].isNumber() && s[i].cell()!='(' && s[i].cell()!=')') {
            if (s[i].cell()!='&' && s[i].cell()!='|' && s[i].cell()!='!' &&s[i].cell()!='^') {
                qWarning("Unknown %c",s[i].cell());
                return 0;
            }
            if ((s[i].cell()=='!' && i>0) || (s[i].cell()!='!' && (i==0 || i==n-1))) {
                qWarning("Syn error");
                return 0;
            }

            if (s[i].cell()!='!') {
                QString g=s;
                s.remove(0,i+1);
                g.remove(i,n-i);
                return check(s) && check(g);
            } else {
                s.remove(0,i+1);
                return check(s);
            }
        }
    }

    qWarning("stange error...");
    return 0;
}

int ComplexElement::connectAndGetIdOut(QString formula, int *h) {
    int i,j,l1,l2,id1,id2,n=formula.length();
    bool ok;
    i=formula.toInt(&ok,10);
    if (ok) {
        *h=0;
        return in_points[i];
    };

    int k=0;
    ok=1;
    for (i=0; i<n; i++) {
        if (formula[i].cell()=='(') k++;
        if (formula[i].cell()==')') k--;
        if (i==0 && k!=1) ok=0;
        if (i<n-1 && k==0) ok=0;
    }
    if (ok) {
        formula.remove(0,1);
        formula.remove(n-2,1);
        id1=connectAndGetIdOut(formula, &l1);
        *h=l1;
        return id1;
    }

    for (i=0; i<yetExist.size(); i++) {
        if (formula==yetExist[i].s) {
            *h=yetExist[i].h;
            return yetExist[i].out;
        }
    }

    QString s,g;
    save saving;
    saving.s=formula;

    QString op="|^&";
    OrElement *newOr = new OrElement;
    XorElement *newXor = new XorElement;
    AndElement *newAnd = new AndElement;
    for (j=0; j<3; j++) {
        k=0;
        for (i=0; i<n; i++) {
            if (formula[i].cell()=='(') k++;
            if (formula[i].cell()==')') k--;
            if (k==0 && formula[i].cell()==op[j].cell()) {
                s=formula;
                g=s;
                s.remove(0,i+1);
                g.remove(i,g.length()-i);

                id1=connectAndGetIdOut(g, &l1);
                id2=connectAndGetIdOut(s, &l2);

                *h=max(l1,l2)+1;
                saving.h=*h;

                if (op[j].cell()=='|') {
                    newOr->_view.x=10*(*h);
                    newOr->_view.y=10*y[*h]+3;
                    y[*h]++;

                    newce->d->addElement(newOr);
                    newce->d->addConnection(id1,newOr->in[0]);
                    newce->d->addConnection(id2,newOr->in[1]);
                    saving.out=newOr->out[0];
                    yetExist.push_back(saving);
                    return newOr->out[0];
                }
                if (op[j].cell()=='^') {
                    newXor->_view.x=10*(*h);
                    newXor->_view.y=10*y[*h]+3;
                    y[*h]++;

                    newce->d->addElement(newXor);
                    newce->d->addConnection(id1,newXor->in[0]);
                    newce->d->addConnection(id2,newXor->in[1]);
                    saving.out=newXor->out[0];
                    yetExist.push_back(saving);
                    return newXor->out[0];
                }
                if (op[j].cell()=='&') {
                    newAnd->_view.x=10*(*h);
                    newAnd->_view.y=10*y[*h]+3;
                    y[*h]++;

                    newce->d->addElement(newAnd);
                    newce->d->addConnection(id1,newAnd->in[0]);
                    newce->d->addConnection(id2,newAnd->in[1]);
                    saving.out=newAnd->out[0];
                    yetExist.push_back(saving);
                    return newAnd->out[0];
                }
            }
        }
    }

    //!
    s=formula;
    s.remove(0,1);

    id1=connectAndGetIdOut(s, &l1);
    NotElement *newNot = new NotElement;
    *h=l1+1;
    newNot->_view.x=10*(*h);
    newNot->_view.y=10*y[*h]+3;
    y[*h]++;
    newce->d->addElement(newNot);
    newce->d->addConnection(id1,newNot->in[0]);
    saving.out=newNot->out[0];
    saving.h=*h;
    yetExist.push_back(saving);
    return newNot->out[0];
}

Element* ComplexElement::createElementByFormula(QString formula)
{
    int i,n=0,k;
    ComplexElement creator;

    for (i=0; i<formula.length(); i++) {
        if (formula[i].cell()=='+') formula[i]='|';
        if (formula[i].cell()=='*') formula[i]='&';
        if (formula[i].cell()=='-') formula[i]='!';

        k=0;
        while (formula[i].isNumber()) {
            k=10*k+formula[i].cell()-'0';
            if (k>100) break;
            i++;
            if (i==formula.length()) break;
        }
        if (k>n) n=k;
    }
    n++;
    if (n>100) {
        qWarning("too much in-points");
        return 0;
    }

    for (i=0; i<=1000; i++) {
        creator.y.push_back(0);
    }

    creator.newce = new ComplexElement(n,1);

    if (!check(formula)) {
        return 0;
    }

    creator.in_points.clear();
    for (i=0; i<n; i++) {
        creator.in_points.push_back(creator.newce->d->c->new_point());
        creator.newce->in_connections[i]=creator.in_points[i];
    }
    int out_id = creator.newce->d->c->new_point();

    creator.newce->d->addConnection(creator.connectAndGetIdOut(formula,&i),out_id);
    creator.newce->out_connections[0]=out_id;
    return creator.newce;
}
