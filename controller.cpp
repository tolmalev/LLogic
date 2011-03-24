#include "controller.h"
#include "element.h"

#include <QSet>

Controller::Controller()
{
    n = 1;
    calculating=0;
    timer.setSingleShot(1);
    timer.setInterval(1000);

    connect(&timer, SIGNAL(timeout()), this, SLOT(timer_timeout()));
    //connect(&timer, SIGNAL(timeout()), this, SLOT(stop_calculation()));

    thread = new CalcThread;
    thread->c = this;
    connect(thread, SIGNAL(finished()), this, SLOT(calc_finished()));
    connect(thread, SIGNAL(terminated()), this, SLOT(calc_terminated()));
}

Controller::~Controller()
{
    delete thread;
    QList<int> * l;
    foreach(l, connections)
        delete l;
}

int Controller::get(int id)
{
    if(value.find(id) != value.end())
        return value[id];
    return -1;
}

void Controller::set(int id, int val, bool forse)
{
    if(value.find(id) != value.end())
        if(value[id] != val || forse)
        {
            if(e_connected[id] != 0)
                queue.push_back(e_connected[id]);
            value[id] = val;
            foreach(int _id, *connections[id])
                set(_id, val);
        }
}

int Controller::new_point(int id)
{
    if(id == -1)
    {
        id = n++;
    }
    else
        n = std::max(n, id+1);
    if(value.find(id) != value.end())
        return -1;
    value[id]=0;
    connections[id] = new QList<int>;
    e_connected[id] = 0;
    return id;
}

void Controller::remove_point(int id)
{
    bool h = has_in_e_connected(id);
    foreach(int i, *connections[id])
    {
        connections[i]->removeOne(id);
        if(h)
            if(!has_in_e_connected(i))
                set(i, 0);
    }

    value.remove(id);



    delete connections[id];
    connections.remove(id);
    e_connected.remove(id);
    in_e_connected.remove(id);
}

void Controller::connect_element(int id, Element *el)
{
    e_connected[id] = el;
}

void Controller::calculate(bool wait)
{
    if(wait)
    {
        while(!queue.empty())
        {
            Element*el = queue.first();
            queue.pop_front();
            el->recalc();
        }
    }
    else
    {
        calculating=1;
        timer.start();
        thread->start();
    }
}

int Controller::has_in_e_connected(int i, QMap<int, bool> *mp)
{
    bool created = 0;
    int has = 0;
    if(mp == 0)
    {
        mp = new QMap<int, bool>;
        created = 1;
    }
    if(in_e_connected[i] != 0)
        has = i;
    if(mp->find(i) == mp->end())
    {
         mp->insert(i, 1);
         foreach(int id, *connections[i])
             if(has_in_e_connected(id, mp))
             {
                has = id;
                break;
             }
    }
    if(created)
        delete mp;
    return has;
}

int Controller::connect_in_element(int id, Element *el, bool force)
{
    if(has_in_e_connected(id) && !force)
        return -1;
    in_e_connected[id] = el;
    queue.push_back(el);
    return 0;
}

bool Controller::connected(int id1, int id2)
{
    QMap<int, bool> mp;
    return dfs(id1, &mp, id2);
}

bool Controller::dfs(int i, QMap<int, bool> *mp, int k)
{
    if(i == k)
        return 1;
    if(mp->find(i) == mp->end())
    {
        mp->insert(i, 1);
        foreach(int id, *connections[i])
            if(dfs(id, mp, k))
                return 1;
    }
    return 0;
}

bool Controller::canConnect(int id1, int id2)
{
    if(value.find(id1) == value.end() || value.find(id2) == value.end())
        return 0;
    if(connected(id1, id2))
        return 0;
    bool h1 = has_in_e_connected(id1);
    bool h2 = has_in_e_connected(id2);
    if(h1 && h2)
        return 0;
    return 1;
}

int Controller::add_connection(int id1, int id2)
{
    if(!canConnect(id1, id2))
        return -1;
    bool h1 = has_in_e_connected(id1);
    bool h2 = has_in_e_connected(id2);
    connections[id1]->push_back(id2);
    connections[id2]->push_back(id1);
    if(h1)
        set(id2, value[id1]);
    if(h2)
        set(id1, value[id2]);
    return 0;
}

void Controller::setTimeout(int time)
{
    timer.setInterval(time);
}

void Controller::stop_calculation()
{
    thread->terminate();
    thread->wait();
    //qDebug("controller: calculation terminated");
    queue.clear();
    foreach(int id, value.keys())
    {
        //qDebug("%d", id);
        set(id, value[id], 1);
        Element*e =e_connected[id];
        if(e)
        {
            queue.push_back(e);
            //qDebug("add %d", e->type());
        }
    }
}

void Controller::CalcThread::run()
{
    while(!c->queue.empty())
    {
        Element*el = c->queue.first();
        c->queue.pop_front();
        el->recalc();
    }
}

void Controller::calc_finished()
{
    //qDebug("controller: calculation finished");
    timer.stop();
    calculating=1;
    emit calculation_finished(0);
}

void Controller::calc_terminated()
{
    timer.stop();
}

void Controller::timer_timeout()
{
    emit timeout(this);
}

void Controller::removeFromQueue(Element *e)
{
    while(queue.indexOf(e)>=0)
        queue.removeOne(e);
}

void Controller::remove_connection(int id1, int id2)
{
    if(value.find(id1) == value.end() || value.find(id2) == value.end())
        return;
    if(id1 == id2)
    {
        foreach(int i, *connections[id1])
        {
            connections[i]->removeOne(id1);
        }
        connections[id1]->clear();
    }
    else
    {
        connections[id1]->removeOne(id2);
        connections[id2]->removeOne(id1);
    }
}
