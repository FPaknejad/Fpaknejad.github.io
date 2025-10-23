#ifndef CLASSES_HPP_
#define CLASSES_HPP_
#include<iostream>
#include<fstream>
#include<string>
using namespace std;

//CLASS NODE
class node
{
private:
	int num; //node number
	double x,y,Q,head;// x-coords,y-coords,flowrate,head
public:
	node(int number,double a,double b,double flow):num(number),x(a),y(b),Q(flow){} //constructor:member initialization
	double getx();
	double gety();
	int getnum();
	double gethead();
	double getQ();
	//void display(); 
	void definehead(double);
	//~node();
};

//CLASS TUBE
class tube
{
private:
	int num,nodeone,nodetwo;
	double dia;
	double length,B,q; //iniialized using the functions below
	node* a; //a points to public members of class node
	node* b; //b points to public members of class node
public:
	tube(int,node*,node*,int,int,double);
	void calclength();
	void calcB();
	//void display();
	int getn1();
	int getn2();
	double getB();
	void displayflow();
};

//CLASS PIPENETWORK
class pipenet
{
private:
	node** vec_nodes;
	tube** vec_tubes;
	int n_nodes;
	int n_tubes;
public:	
	pipenet(ifstream&);
	//void Display();
	//void test();
	void calcflowrate();
	~pipenet();
};
#endif