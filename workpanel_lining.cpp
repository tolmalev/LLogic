#include "workpanel.h"
#include "document.h"
#include "element.h"
#include "elementwidget.h"
#include "pointwidget.h"
#include "controller.h"

#include <QPainter>

int WorkPanel::getx(int x) {
    return x/10+1;
}

int WorkPanel::getk(int x) {
    return (x-1)*10;
}

int WorkPanel::min(int x, int y) {
    if (x<y) return x;
    return y;
}

int WorkPanel::max(int x, int y) {
    if (x>y) return x;
    return y;
}

int WorkPanel::abs(int x) {
    if (x<0) return -x;
    return x;
}

int WorkPanel::bfs(tp tt) {
    kstep2=0;
    int x,y,t,i;
    for (i=0; i<kstep1; i++) {
	if (step1[i].wait) {
	    step1[i].wait--;
	    step2[kstep2]=step1[i];
	    kstep2++;
	    continue;
	}

	x=step1[i].x;
	y=step1[i].y;
	if (s[x][y].point && !firstpoint) continue;
	firstpoint=0;
	t=step1[i].t;

	if (t==2 && s[x][y-1].k==0 && (s[x][y].k2==0 || s[x][y].t2==tt)) {
	    step2[kstep2].x=x;
	    step2[kstep2].y=y-1;
	    step2[kstep2].t=2;
	    if (s[x][y].k1!=0 && s[x][y].t1!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x][y-1].k=-2;
	    s[x][y-1].re=-2;
	    kstep2++;
	}
	if (t==3 && s[x+1][y].k==0 && (s[x][y].k1==0 || s[x][y].t1==tt)) {
	    step2[kstep2].x=x+1;
	    step2[kstep2].y=y;
	    step2[kstep2].t=3;
	    if (s[x][y].k2!=0 && s[x][y].t2!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x+1][y].k=-2;
	    s[x+1][y].re=1;
	    kstep2++;
	}
	if (t==4 && s[x][y+1].k==0 && (s[x][y].k2==0 || s[x][y].t2==tt)) {
	    step2[kstep2].x=x;
	    step2[kstep2].y=y+1;
	    step2[kstep2].t=4;
	    if (s[x][y].k1!=0 && s[x][y].t1!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x][y+1].k=-2;
	    s[x][y+1].re=2;
	    kstep2++;
	}

	if (s[x-1][y].k==0 && (s[x][y].k1==0 || s[x][y].t1==tt)) {
	    step2[kstep2].x=x-1;
	    step2[kstep2].y=y;
	    step2[kstep2].t=1;
	    if (s[x][y].k2!=0 && s[x][y].t2!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x-1][y].k=-2;
	    s[x-1][y].re=-1;
	    kstep2++;
	}
	if (s[x+1][y].k==0 && (s[x][y].k1==0 || s[x][y].t1==tt)) {
	    step2[kstep2].x=x+1;
	    step2[kstep2].y=y;
	    step2[kstep2].t=3;
	    if (s[x][y].k2!=0 && s[x][y].t2!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x+1][y].k=-2;
	    s[x+1][y].re=1;
	    kstep2++;
	}
	if (s[x][y-1].k==0 && (s[x][y].k2==0 || s[x][y].t2==tt)) {
	    step2[kstep2].x=x;
	    step2[kstep2].y=y-1;
	    step2[kstep2].t=2;
	    if (s[x][y].k1!=0 && s[x][y].t1!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x][y-1].k=-2;
	    s[x][y-1].re=-2;
	    kstep2++;
	}
	if (s[x][y+1].k==0 && (s[x][y].k2==0 || s[x][y].t2==tt)) {
	    step2[kstep2].x=x;
	    step2[kstep2].y=y+1;
	    step2[kstep2].t=4;
	    if (s[x][y].k1!=0 && s[x][y].t1!=tt) step2[kstep2].wait=3; else
		step2[kstep2].wait=0;
	    s[x][y+1].k=-2;
	    s[x][y+1].re=2;
	    kstep2++;
	}
    }

    kstep1=kstep2;
    for (i=0; i<kstep1; i++) {
	step1[i]=step2[i];
    }

    if (kstep1==0) return 1;
    return 0;
}


void WorkPanel::calculateLines()
{
    xsize=geometry().width()/10+1;
    ysize=geometry().height()/10+1;
    klines=0;

    int i,j,x,y,xx,yy,m,error,q,numbertry=3;
    tp t[1000];

    for (i=0; i<=xsize+1; i++)
	for (j=0; j<=ysize+1; j++)
	    s[i][j].k=-1;
    for (i=1; i<=xsize; i++)
	for (j=1; j<=ysize; j++) {
	    s[i][j].k=0;
	    s[i][j].re=0;
	    s[i][j].k2=0;
	    s[i][j].k1=0;
	    s[i][j].point=0;
	}

    foreach(ElementWidget *ew, elementWidgets) {
	for (i=WorkPanel::min(getx(ew->geometry().x()),getx(ew->geometry().x()+ew->geometry().width())); i<=max(getx(ew->geometry().x()),getx(ew->geometry().x()+ew->geometry().width())); i++)
	    for (j=WorkPanel::min(getx(ew->geometry().y()),getx(ew->geometry().y()+ew->geometry().height())); j<=max(getx(ew->geometry().y()),getx(ew->geometry().y()+ew->geometry().height())); j++)
		s[i][j].k=-1;
    }
    foreach(int k, d->c->connections.keys())
    {
	if(points.find(k) != points.end())
	{
	    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())].point=1;
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())+1].k=0;
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())-1].k=0;
	}
    }

    for (q=0; q<numbertry; q++) {
	foreach(int k, d->c->connections.keys()) if (points.find(k) != points.end()) {
	    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
	    x=getx(p1.x());
	    y=getx(p1.y());

	    n=0;
	    foreach(int b, *d->c->connections[k]) if (points.find(b) != points.end()) {
		QPoint p2 = toGrid(points[b]->mapTo(this, QPoint(3, 3)));
		list[n].x=getx(p2.x());
		list[n].y=getx(p2.y());
		list[n].t=b;
		t[n].x=k;
		t[n].y=b;
		n++;
	    }

	    for (i=0; i<n; i++) {
		for (j=0; j<klines; j++) if (lines[j].t.x==t[i].y && lines[j].t.y==t[i].x){
		    if (lines[j].x1==lines[j].x2 && WorkPanel::abs(lines[j].y1-lines[j].y2)==1) {
			s[lines[j].x1][lines[j].y1].k2--;
			s[lines[j].x2][lines[j].y2].k2--;
		    }
		    if (lines[j].y1==lines[j].y2 && WorkPanel::abs(lines[j].x1-lines[j].x2)==1) {
			s[lines[j].x1][lines[j].y1].k1--;
			s[lines[j].x2][lines[j].y2].k1--;
		    }
		    lines[j].t.x=-1;
		}
		m=0;
		for (j=0; j<klines; j++) if (lines[j].t.x!=-1) {
		    lines[m]=lines[j];
		    m++;
		}
		klines=m;
	    }


	    error=0;
	    for (i=0; i<n; i++) {
		if (error) {
		    lines[klines].x1=x;
		    lines[klines].y1=y;
		    lines[klines].x2=list[i-1].x;
		    lines[klines].y2=list[i-1].y;
		    lines[klines].t=t[i-1];
		    klines++;
		}

		for (int ii=1; ii<=xsize; ii++)
		    for (j=1; j<=ysize; j++)
			if (s[ii][j].k==-2) s[ii][j].k=0;

		error=0;

		kstep1=1;
		step1[0].x=x;
		step1[0].y=y;
		s[x][y].k=-2;
		step1[0].t=0;

		firstpoint=1;
		xx=list[i].x;
		yy=list[i].y;
		error=0;
		while (s[list[i].x][list[i].y].k==0) {
		    if (WorkPanel::bfs(t[i])) {
    //                    qWarning("error1 in bfs\n");
			error=1;
			break;
		    }
		}
		if (error==1 || s[list[i].x][list[i].y].k==-1) {
		    error=1;
		    continue;
		}


		xx=list[i].x;
		yy=list[i].y;
		kstep1=0;
		int kol=0;
		while (xx!=x || yy!=y) {
		    step1[kstep1].x=xx;
		    step1[kstep1].y=yy;
		    kstep1++;
		    m=s[xx][yy].re;
		    if (m==-1) {
			s[xx+1][yy].k1++;
			s[xx][yy].k1++;
			s[xx+1][yy].t1=t[i];
			s[xx][yy].t1=t[i];
			xx++;
		    }
		    if (m==-2) {
			s[xx][yy+1].k2++;
			s[xx][yy].k2++;
			s[xx][yy+1].t2=t[i];
			s[xx][yy].t2=t[i];
			yy++;
		    }
		    if (m==1) {
			s[xx][yy].k1++;
			s[xx-1][yy].k1++;
			s[xx-1][yy].t1=t[i];
			s[xx][yy].t1=t[i];
			xx--;
		    }
		    if (m==2) {
			s[xx][yy].k2++;
			s[xx][yy-1].k2++;
			s[xx][yy-1].t2=t[i];
			s[xx][yy].t2=t[i];
			yy--;
		    }

		    kol++;
		    if (kol>1000) {
    //                    qWarning("error2 in bfs\n");
			error=1;
			break;
		    }
		}
		if (error) continue;

		step1[kstep1].x=x;
		step1[kstep1].y=y;
		kstep1++;

		for (j=0; j<kstep1-1; j++) {
		    lines[klines].x1=step1[j].x;
		    lines[klines].y1=step1[j].y;
		    lines[klines].x2=step1[j+1].x;
		    lines[klines].y2=step1[j+1].y;
		    lines[klines].t=t[i];
		    klines++;
		}
	    }

	    if (error) {
		i=n;
		lines[klines].x1=x;
		lines[klines].y1=y;
		lines[klines].x2=list[i-1].x;
		lines[klines].y2=list[i-1].y;
		lines[klines].t=t[i-1];
		klines++;
	    }
	}
    }
}

void WorkPanel::drawLines(QPainter &painter)
{
//    qWarning("%d",klines);
    int i;
    for (i=0; i<klines; i++) {
	painter.drawLine(WorkPanel::getk(lines[i].x1),WorkPanel::getk(lines[i].y1),WorkPanel::getk(lines[i].x2),WorkPanel::getk(lines[i].y2));
    }

    /*if(d != 0)
    foreach(int k, d->c->connections.keys())
    {
	foreach(int b, *d->c->connections[k])
	{
	    if(k < b)
	    {
		if(points.find(k) != points.end() && points.find(b) != points.end())
		{
		    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
		    QPoint p2 = toGrid(points[b]->mapTo(this, QPoint(3, 3)));
		    painter.drawLine(p1, p2);
		}
	    }
	}
    }*/
}
