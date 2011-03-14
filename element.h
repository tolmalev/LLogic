#ifndef ELEMENT_H
#define ELEMENT_H

#include <QWidget>
#include "classes.h"
#include <QList>
#include <QVector>

enum elementTypes{
    SIMPLE,
    AND,
    ANDNOT,
    OR,
    ORNOT,
    NOT,
    XOR,
    COMPLEX,
    SEND,
    RECEIVE
};

class Element : public QObject
{
    Q_OBJECT

    friend class ElementWidget;
    friend class WorkPanel;
    friend class Document;

    protected:
        int _type;

        QVector<int> in;
        QVector<int> out;
        int in_cnt, out_cnt;
        Controller *c;
        struct VisualParams{
            int x, y, width, height;
        } _view;

        void disconnectControler();
    public:
        Element();
        Element(Controller*c, int _in_cnt = -1, int _out_cnt = -1, int type = SIMPLE);

        virtual void recalc() = 0;
        virtual Element* clone() = 0;
        void setController(Controller * c);
        VisualParams view(){return _view;}
        int type(){return _type;}
    signals:

    public slots:
};

#endif // ELEMENT_H
