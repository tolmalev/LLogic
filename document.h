#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QString>


#include "classes.h"


class Document : public QObject
{
    Q_OBJECT

    friend class WorkPanel;
    friend class ComplexElement;
protected:
    Controller * c;
    QList<Element*> elements;
    WorkPanel *panel;
    void createPanel();
    QString _name;
    QString fileName;
    bool auto_calculation;
    int _document_type;
    enum types{
        FULL,
        ELEMENT
    };

public:
    explicit Document(int _type=FULL, QObject *parent = 0);
    ~Document();
    static   Document* fromFile(FILE *, int _document_type=FULL);
    static   Document* fromFile(QString filename);
    int     saveToFile(QString _filename = "");
    int     saveToFile(FILE*);

    int document_type(){return _document_type;}

    virtual Document * clone();

    void addElement(Element* e);
    int addConnection(int id1, int id2);

    WorkPanel *workPanel();
    QString name(){return _name;}

    void stop_calculation();

signals:
    void timeout(Document*);
    void calculation_finished(int);
    void calculation_started();
    void doubleClicked(ElementWidget*);
public slots:
    void timeout(Controller*c);
    void needCalculation(Element*);
};

#endif // DOCUMENT_H