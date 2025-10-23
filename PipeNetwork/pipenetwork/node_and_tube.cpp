#include"classes.h"

//Functions of class node
double node::getx()
{return x;}
double node::gety()
{return y;}
int node::getnum()
{return num;}
double node::getQ()
{	return Q;}
/*void node::display()
{	cout<<num<<"\t"<<x<<","<<y<<"\t"<<Q<<endl;}*/

void node::definehead(double n)
{
	head=n;
	//cout<<"head = " <<head<<endl;
}
double node::gethead()
{
	return head;
}
/********************************************************/
/********************************************************/
//Functions of class tube

tube::tube(int number,node* A,node* B,int NoOne,int NoTwo,double Dia)
{
		num=number;
		nodeone=NoOne;
		nodetwo=NoTwo;
		a=A; //a is a pointer to node which has x and y members. 
		b=B;
		dia=Dia;
		calclength();
		calcB();
}
void tube::calclength()
{
	double xa,xb,ya,yb;
	xa=a->getx(); //a and b are pointers of type node >>> now it can access getx() function member of class  node
	xb=b->getx(); //so now a and b have access to data members of class node
	ya=a->gety();
	yb=b->gety();
	length=sqrt(((xa-xb)*(xa-xb)+(ya-yb)*(ya-yb)));
}
void tube::calcB()
{
	B=(3.14*9.81*(dia*dia*dia*dia))/(128*length*1e-6);
	//cout<<"b calculated---"<<B<<endl;   //can be used to test the B values
}
/*void tube::display()
{
	cout<<num<<"\t"<<a->getnum()<<"\t"<<b->getnum()<<"\t"<<dia<<"\n";
}*/

int tube::getn1()
{	return nodeone;}
int tube::getn2()
{return nodetwo;}
double tube::getB()
{return B;}


void tube::displayflow()
{
	 double h1,h2;
	 h1=a->gethead();
	 h2=b->gethead();
	 // cout<<"head of first node"<<h1<<"head of second node"<<h2<<endl;    //for testing purpose
	 cout<<"Tube number--  "<<num<<"\t"<<"flow--  "<<B*(h1-h2)<<"\n";
}
