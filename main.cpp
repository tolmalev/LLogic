#include <QtGui/QApplication>
#include <QDir>
#include <QTextCodec>
#include <QFile>

#include "mainwindow.h"

#include "simpleelements.h"
#include "controller.h"
#include "stdio.h"
#include "document.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("unf-8"));
    MainWindow w;
    w.show();

    if(argc>1)
    {
	for(int i = 1; i < argc; i++)
	{
	    Document * d = Document::fromFile(argv[i]);
	    if(d)
		w.showDocument(d);
	    else
		w.newDocument();
	}
    }
    else
	w.newDocument();
    app.exec();



    //Count line numbers
    QDir dir("D:/programming/LLogic");
    char st[10000];
    int l = 0;
    int l1 = 0;
    int n = 0;
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
        //qDebug(s.toAscii());
        //qDebug("%d", l1);
        n++;
    }

    qDebug("%d lines in %d files", l, n);

    return 0;
}
