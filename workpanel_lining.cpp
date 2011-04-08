#include "workpanel.h"
#include "document.h"
#include "element.h"
#include "elementwidget.h"
#include "pointwidget.h"
#include "controller.h"
#include <math.h>

#include <QPainter>

int WorkPanel::getx(int x) {
    return x/grid_size+1;
}

int WorkPanel::getk(int x) {
    return (x-1)*grid_size;
}

int WorkPanel::min(int x, int y) {
    if (x<y) return x;
    return y;
}

int WorkPanel::max(int x, int y) {
    if (x>y) return x;
    return y;
}

int WorkPanel::dfs(int k, int t) {
    if (component[k]!=-1) return 0;
    component[k]=t;
    foreach(int b, *d->c->connections[k]) if (points.find(b) != points.end()) {
	dfs(b,t);
    }
    return 0;
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
    xsize=geometry().width()/grid_size+1;
    ysize=geometry().height()/grid_size+1;
    if (xsize>198) xsize=198;
    if (ysize>198) ysize=198;
    klines=0;

    int i,j,x,y,xx,yy,m,error,q,numbertry=2;
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

    maxpoint=0;
    foreach(int k, d->c->connections.keys())
	if (k>maxpoint) maxpoint=k;

    for (i=0; i<=maxpoint; i++) component[i]=-1;

    j=0;
    foreach(int k, d->c->connections.keys())
    {
	if(points.find(k) != points.end())
	{
	    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())].point=1;
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())+1].k=0;
	    s[WorkPanel::getx(p1.x())][WorkPanel::getx(p1.y())-1].k=0;
	    if (component[k]==-1) {
		dfs(k,j);
		j++;
	    }
	}
    }

    for (q=0; q<numbertry; q++) {
	foreach(int k, d->c->connections.keys()) if (points.find(k) != points.end()) {
	    QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));
	    x=WorkPanel::getx(p1.x());
	    y=WorkPanel::getx(p1.y());
	    if (x>xsize || y>ysize) {
		continue;
	    }

	    n=0;
	    foreach(int b, *d->c->connections[k]) if (points.find(b) != points.end()) {
		QPoint p2 = toGrid(points[b]->mapTo(this, QPoint(3, 3)));
		if (WorkPanel::getx(p2.x())>xsize || WorkPanel::getx(p2.y())>ysize) continue;
		list[n].x=getx(p2.x());
		list[n].y=getx(p2.y());
		list[n].t=b;
		t[n].x=k;
		t[n].y=b;
		t[n].t=component[k];
		n++;
	    }

	    for (i=0; i<n; i++) {
		for (j=0; j<klines; j++) if (lines[j].t.x==t[i].y && lines[j].t.y==t[i].x){
		    if (lines[j].x1==lines[j].x2 && abs(lines[j].y1-lines[j].y2)==1) {
			s[lines[j].x1][lines[j].y1].k2--;
			s[lines[j].x2][lines[j].y2].k2--;
		    }
		    if (lines[j].y1==lines[j].y2 && abs(lines[j].x1-lines[j].x2)==1) {
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
		    lines[klines].error=1;
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
		    lines[klines].error=0;
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
		lines[klines].error=1;
		klines++;
	    }
	}
    }


    foreach(int k, d->c->connections.keys()) if (points.find(k) != points.end())
    {
	QPoint p1 = toGrid(points[k]->mapTo(this, QPoint(3, 3)));

	if (WorkPanel::getx(p1.x())>xsize || WorkPanel::getx(p1.y())>ysize) {
	    foreach(int b, *d->c->connections[k]) if (points.find(b) != points.end())
	    {
		QPoint p2 = toGrid(points[b]->mapTo(this, QPoint(3, 3)));
		if (k<b || (WorkPanel::getx(p2.x())<=xsize && WorkPanel::getx(p2.y())<=ysize)) {
		    lines[klines].x1=WorkPanel::getx(p1.x());
		    lines[klines].y1=WorkPanel::getx(p1.y());
		    lines[klines].x2=WorkPanel::getx(p2.x());
		    lines[klines].y2=WorkPanel::getx(p2.y());
		    lines[klines].t.t=component[k];
		    lines[klines].error=1;
		    klines++;
		}
	    }
	}
    }
}

void WorkPanel::drawLines(QPainter &painter)
{
    int i,j;
    j=0;
    for (i=0; i<klines; i++)
	if (lines[i].t.t>j) j=lines[i].t.t;
    for (i=0; i<=j; i++) component[i]=0;
    for (i=0; i<klines; i++)
	if (lines[i].light) component[lines[i].t.t]=1;

    QPen pn(Qt::blue);
    pn.setWidth(2);
    for (i=0; i<klines; i++) {
	if (component[lines[i].t.t]) painter.setPen(pn); else
	    painter.setPen(Qt::black);

	painter.drawLine(WorkPanel::getk(lines[i].x1),WorkPanel::getk(lines[i].y1),WorkPanel::getk(lines[i].x2),WorkPanel::getk(lines[i].y2));
    }

    painter.setPen(QColor(2, 50, 4));
    painter.setBrush(QColor(2, 50, 4));
    xsize=geometry().width()/grid_size+1;
    ysize=geometry().height()/grid_size+1;
    for (j=1; j<=xsize; j++) {
	for (i=1; i<=ysize; i++) {
	    s[j][i].k=0;
	    s[j][i].re=0;
	}
    }
    for (j=0; j<klines; j++) if (!lines[j].error) {
	if (s[lines[j].x1][lines[j].y1].k!=-1) {
	    if (s[lines[j].x1][lines[j].y1].k==1 && s[lines[j].x1][lines[j].y1].t1!=lines[j].t)
		    s[lines[j].x1][lines[j].y1].k=-1; else {
		s[lines[j].x1][lines[j].y1].k=1;
		s[lines[j].x1][lines[j].y1].t1=lines[j].t;
		if (lines[j].x1==lines[j].x2) {
		    if (lines[j].y1==lines[j].y2-1) s[lines[j].x1][lines[j].y1].re|=2;
		    if (lines[j].y1==lines[j].y2+1) s[lines[j].x1][lines[j].y1].re|=8;
		}
		if (lines[j].y1==lines[j].y2) {
		    if (lines[j].x1==lines[j].x2-1) s[lines[j].x1][lines[j].y1].re|=4;
		    if (lines[j].x1==lines[j].x2+1) s[lines[j].x1][lines[j].y1].re|=1;
		}
	    }
	}

	if (s[lines[j].x2][lines[j].y2].k!=-1) {
	    if (s[lines[j].x2][lines[j].y2].k==1 && s[lines[j].x2][lines[j].y2].t1!=lines[j].t)
		    s[lines[j].x2][lines[j].y2].k=-1; else {
		s[lines[j].x2][lines[j].y2].k=1;
		s[lines[j].x2][lines[j].y2].t1=lines[j].t;
		if (lines[j].x1==lines[j].x2) {
		    if (lines[j].y1==lines[j].y2-1) s[lines[j].x2][lines[j].y2].re|=8;
		    if (lines[j].y1==lines[j].y2+1) s[lines[j].x2][lines[j].y2].re|=2;
		}
		if (lines[j].y1==lines[j].y2) {
		    if (lines[j].x1==lines[j].x2-1) s[lines[j].x2][lines[j].y2].re|=1;
		    if (lines[j].x1==lines[j].x2+1) s[lines[j].x2][lines[j].y2].re|=4;
		}
	    }
	}
    }

    for (j=1; j<=xsize; j++) {
	for (i=1; i<=ysize; i++) {
	    if (s[j][i].re==15 || s[j][i].re==7 || s[j][i].re==11 || s[j][i].re==13 || s[j][i].re==14) {
		painter.drawRect(getk(j)-1,getk(i)-1,2,2);
	    }
	}
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
