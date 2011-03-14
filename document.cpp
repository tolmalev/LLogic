#include "document.h"
#include "element.h"
#include "controller.h"
#include "workpanel.h"
#include "filestructs.h"
#include "complexelement.h"
#include "simpleelements.h"

Document::Document(int _type, QObject *parent) : QObject(parent), _document_type(_type)
{
    c = new Controller();
    connect(c, SIGNAL(timeout(Controller*)), this, SLOT(timeout(Controller*)));
    connect(c, SIGNAL(calculation_finished(int)), this, SIGNAL(calculation_finished(int)));
    panel = 0;
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
    elements.push_back(e);
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
    panel = new WorkPanel;
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

Document * Document::fromFile(FILE *f, int _document_type)
{
    if(f == 0)
        return 0;
    Document *d = new Document(_document_type);
    DocumentHeader dh;
    if(fread((void*)&dh, sizeof(dh), 1, f) != 1)
        return 0;
    switch(_document_type)
    {
        case FULL:if(dh.doc_type != 1){delete d; return 0;} break;
        case ELEMENT:if(dh.doc_type != 2){delete d; return 0;} break;
    }
    char * str = new char[dh.name_length];
    if(fread(str, 1, dh.name_length, f) != (size_t)dh.name_length)
        {delete d; return 0;}
    d->_name.fromAscii(str, dh.name_length);
    ElementHeader eh;
    for(int i = 0; i < dh.elements_count; i++)
    {
        if(fread((void*)&eh, sizeof(eh), 1, f) != 1)
            {delete d; return 0;}
        if(eh.type != COMPLEX)
        {
            Element * e = SimpleElement(eh.type);
            e->in_cnt = eh.in_cnt;
            e->out_cnt = eh.out_cnt;
            e->_view.x = eh.x;
            e->_view.y = eh.y;
            e->_view.width = eh.width;
            e->_view.height = eh.height;
            int tm;
            for(int i = 0; i < eh.in_cnt; i++)
            {
                if(fread(&tm, sizeof(i), 1, f) != 1)
                    {delete d; return 0;}
                e->in.push_back(tm);
                d->c->new_point(tm);
                d->c->connect_element(tm, e);
            }
            for(int i = 0; i < eh.out_cnt; i++)
            {
                if(fread(&tm, sizeof(i), 1, f) != 1)
                    {delete d; return 0;}
                e->out.push_back(tm);
                d->c->new_point(tm);
                d->c->connect_in_element(tm, e);
            }
            e->c = d->c;
            d->elements.push_back(e);
        }
    }
    int tm, tm2;
    int a[2];
    if(fread(&tm, sizeof(tm), 1, f) != 1)
        {delete d; return 0;}
    for(int i = 0; i < tm; i++)
    {
        if(fread(&tm2, sizeof(tm2), 1, f) != 1)
            {delete d; return 0;}
        for(int j = 0; j < tm2; j++)
        {
            if(fread(a, sizeof(int), 2, f) != 2)
                {delete d; return 0;}
            d->c->add_connection(a[0], a[1]);
            //qWarning("connection read %d %d", a[0], a[1]);
        }
    }
    fclose(f);
    return d;
}

int Document::saveToFile(QString filename)
{
    if(filename == "")
        filename = fileName;
    FILE * f = fopen(filename.toAscii(), "wb");
    return saveToFile(f);
}

int Document::saveToFile(FILE *f)
{
    if(f == 0)
        return -1;
    DocumentHeader dh;
    switch(document_type())
    {
        case FULL:dh.doc_type = 1;break;
        case ELEMENT:dh.doc_type = 2;break;
    }
    dh.name_length = _name.length();
    dh.elements_count = elements.size();
    if(fwrite((void*)&dh, sizeof(dh), 1, f) != 1)
        return -1;
    if(fwrite(_name.toAscii(), 1, _name.length(), f) != (size_t)_name.length())
        return -1;
    ElementHeader eh;
    foreach(Element*e, elements)
    {
        eh.in_cnt = e->in_cnt;
        eh.out_cnt = e->out_cnt;
        eh.x = e->view().x;
        eh.y = e->view().y;
        eh.width = e->view().width;
        eh.height = e->view().height;
        eh.type = e->_type;
        if(fwrite((void*)&eh, sizeof(eh), 1, f) != 1)
            return -1;
        foreach(int i, e->in)
            if(fwrite(&i, sizeof(i), 1, f) != 1)
                return -1;
        foreach(int i, e->out)
            if(fwrite(&i, sizeof(i), 1, f) != 1)
                return -1;
        if(e->type() == COMPLEX)
            if(((ComplexElement*)e)->saveToFile(f))
                return -1;
    }

    int tm = c->connections.count();
    if(fwrite(&tm, sizeof(tm), 1, f) != 1)
        return -1;
    foreach(int a, c->connections.keys())
    {
        tm = c->connections[a]->size();
        if(fwrite(&tm, sizeof(tm), 1, f) != 1)
            return -1;
        foreach(int b, *c->connections[a])
        {
            tm = a;
            if(fwrite(&tm, sizeof(tm), 1, f) != 1)
                return -1;
            tm = b;
            if(fwrite(&tm, sizeof(tm), 1, f) != 1)
                return -1;
        }
    }

    fclose(f);
    this->fileName = fileName;
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
}

Document::~Document()
{
    if(panel)
        delete panel;
    delete c;
}

int Document::addConnection(int id1, int id2)
{
    return c->add_connection(id1, id2);
}

Document* Document::clone()
{
    Document *d = new Document;
    d->_name = name();
    foreach(Element*e, elements)
    {
        Element *el = e->clone();
        el->_view = e->_view;
        el->in = e->in;
        el->out = e->out;

        el->c = d->c;
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

        d->elements.push_back(el);
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

Document* Document::fromFile(QString filename)
{
    Document *d = fromFile(fopen(filename.toAscii(), "rt"));
    if(d != 0)
    {
        d->fileName = filename;
        d->_name = d->fileName.right(d->fileName.size() - 1 - d->fileName.lastIndexOf("/"));
    }
    return d;
}
