#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "classes.h"

#include <QWidget>
#include <QMap>
#include <QList>
#include <QQueue>
#include <QThread>
#include <QTimer>

class Controller : public QObject
{
    Q_OBJECT

    friend class CalcThread;
    friend class Document;
    friend class WorkPanel;

   // protected:
public:
        int n;
        bool calculating;
        QQueue<Element*> queue;
        QMap<int, QList<int> *> connections;
        QMap<int, Element*> e_connected;
        QMap<int, Element*> in_e_connected;
        QMap<int, int> value;

        QTimer timer;

        class CalcThread : public QThread{
            Controller *c;
            void calculate();

            virtual void run();

            friend class Controller;
        } *thread;

        bool dfs(int i, QMap<int, bool>* mp, int k);
    public:
        Controller();
        ~Controller();
        void    set(int id, int value);
        int     get(int id);
        int     new_point(int id=-1);
        void    remove_point(int id);

        int     connect_in_element(int id, Element*el);
        void    connect_element(int id, Element* el);

        bool    has_in_e_connected(int id, QMap<int, bool> *mp);
        bool    connected(int id1, int id2);
        int     add_connection(int id1, int id2);

        void    setTimeout(int time);
        void    calculate(bool wait=0);
    signals:
        void timeout(Controller*);
        void calculation_finished(int);
    public slots:
        void timer_timeout();
        void stop_calculation();
        void calc_finished();
        void calc_terminated();
};

#endif // CONTROLLER_H
