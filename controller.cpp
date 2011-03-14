#include "controller.h"
#include "element.h"

Controller::Controller()
{
    n = 0;
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

void Controller::set(int id, int val)
{
    if(value.find(id) != value.end())
        if(value[id] != val)
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
        id = n++;
    if(value.find(id) != value.end())
        return -1;
    value[id]=0;
    connections[id] = new QList<int>;
    e_connected[id] = 0;
    return id;
}

void Controller::remove_point(int id)
{
    delete connections[id];
    value.remove(id);
    connections.remove(id);
    e_connected.remove(id);
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


bool Controller::has_in_e_connected(int i, QMap<int, bool> *mp=0)
{
    bool created = 0;
    bool has = 0;
    if(mp == 0)
    {
        mp = new QMap<int, bool>;
        created = 1;
    }
    if(in_e_connected[i] != 0)
        has = 1;
    if(mp->find(i) == mp->end())
    {
         mp->insert(i, 1);
         foreach(int id, *connections[i])
             if(has_in_e_connected(id, mp))
             {
                has = 1;
                break;
             }
    }
    if(created)
        delete mp;
    return has;
}

int Controller::connect_in_element(int id, Element *el)
{
    if(has_in_e_connected(id))
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

int Controller::add_connection(int id1, int id2)
{
    if(connected(id1, id2))
        return -2;
    bool h1 = has_in_e_connected(id1);
    bool h2 = has_in_e_connected(id2);
    if(h1 && h2)
        return -1;
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
    qWarning("controller: calculation terminated");
}

void Controller::CalcThread::run()
{
    /*volatile int res = 0;
    for(int i = 0; i < 1000000000; i++)
        for(int j = 0; j < 1; j++)
            res++;*/
    while(!c->queue.empty())
    {
        Element*el = c->queue.first();
        c->queue.pop_front();
        el->recalc();
    }
}

void Controller::calc_finished()
{
    qWarning("controller: calculation finished");
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
