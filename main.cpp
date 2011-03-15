#include <QtGui/QApplication>
#include <QDir>
#include <QFile>

#include "mainwindow.h"

#include "simpleelements.h"
#include "controller.h"
#include "stdio.h"
#include "document.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();



   /*Document *d = new Document();
    d->addElement(new SendElement);
    d->addElement(new SendElement);
    d->addElement(new OrNotElement);
    d->addElement(new OrNotElement);
    d->addElement(new ReceiveElement);
    d->addElement(new ReceiveElement);

    d->addConnection(0, 2);
    d->addConnection(1, 6);

    d->addConnection(4, 5);
    d->addConnection(3, 7);

    d->addConnection(4, 8);
    d->addConnection(7, 9);*/
    //w.showDocument(d);

    //d->saveToFile("test.txt");

    Document *d2 = Document::fromFile("test.xml");
    //w.showDocument(d);
    w.showDocument(d2);
    w.showDocument(d2->clone());


    /*QDomDocument doc;
    QFile f("test.xml");
    f.open(QIODevice::ReadOnly);
    doc.setContent(&f);

    Document *d3 = Document::fromXml(doc.documentElement());
    w.showDocument(d3);
    f.close();


    f.setFileName("test2.xml");
    f.open(QIODevice::WriteOnly);
    QDomDocument doc2("test");
    doc2.appendChild(d3->toXml(doc2));
    f.write(doc2.toByteArray());
    f.close();

    f.setFileName("test2.xml");
    f.open(QIODevice::ReadOnly);
    QDomDocument doc3;
    doc3.setContent(&f);
    Document *d4 = Document::fromXml(doc3.documentElement());
    w.showDocument(d4);
    f.close();*/


    app.exec();

    d2->saveToFile("test.lod");




    //Count line numbers
    QDir dir("D:/programming/LLogic");
    char st[10000];
    int l = 0;
    int l1 = 0;
    foreach(QString s, dir.entryList(QStringList() << "*.cpp" << "*.h", QDir::Files))
    {
        if(s == "main.cpp")
            continue;
        FILE *f = fopen(dir.filePath(s).toAscii(), "rt");
        l1=0;
        while(!feof(f))
        {
            fgets(st, 10000000, f);
            l++;
            l1++;
        }
        qWarning(s.toAscii());
        qWarning("%d", l1);
    }

    qWarning("lines: %d", l);

    return 0;
}
