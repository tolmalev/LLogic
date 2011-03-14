#include "element.h"
#include "controller.h"
#include "cmath"
using namespace std;

Element::Element()
{   
    c = 0;
    in_cnt = out_cnt = -1;
    _type = SIMPLE;
}

Element::Element(Controller* _c, int _in_cnt, int _out_cnt, int __type)
{
    c=0;
    in_cnt = _in_cnt;
    out_cnt = _out_cnt;
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
        in.push_back(id);
    }
    for(int i = 0; i < out_cnt; i++)
    {
        id = c->new_point();
        c->connect_in_element(id, this);
        out.push_back(id);
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
