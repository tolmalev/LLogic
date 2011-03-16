#ifndef ELEMENT_H
#define ELEMENT_H

#include <QWidget>
#include "classes.h"
#include <QList>
#include <QVector>
#include <QtXml/QDomDocument>

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
    RECEIVE,
    LIBRARY,
};

QString typeString(int type);

class Element : public QObject
{
    Q_OBJECT

    friend class ElementWidget;
    friend class WorkPanel;
    friend class Document;
    friend class LibraryElement;

    protected:
        int _type;

        QVector<int> in;
        QVector<int> out;
        int in_cnt, out_cnt;
        Controller *c;
        Document *d;
        struct VisualParams{
            int x, y, width, height;
        } _view;

        void disconnectControler();
    public:
        Element();
        Element(Controller*c, int _in_cnt = -1, int _out_cnt = -1, int type = SIMPLE);

        static Element *fromXml(QDomElement, Document*d=0);
        virtual QDomElement toXml(QDomDocument);
        QDomElement viewToXml(QDomDocument);
        QDomElement inputPointsToXml(QDomDocument);
        QDomElement outputPointsToXml(QDomDocument);

        bool parseView(QDomElement);
        bool parseInputPoints(QDomElement);
        bool parseOutputPoints(QDomElement);

        virtual void recalc() = 0;
        virtual Element* clone() = 0;
        void setController(Controller * c);
        VisualParams view(){return _view;}
        int type(){return _type;}
    signals:

    public slots:
};

#endif // ELEMENT_H
