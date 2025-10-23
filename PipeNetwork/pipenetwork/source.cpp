#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include "classes.h"

using namespace std;

int main()
{  

	cout<<"****Pipe Network for Bavaria*******"<<"\n";
	cout<<"***********Fatemeh Paknejad*********"<<"\n";

   
	ifstream infile("pipedata.txt");

	pipenet bavarian(infile);
	bavarian.calcflowrate();

	infile.close();

	system("PAUSE");
	return 0;
}