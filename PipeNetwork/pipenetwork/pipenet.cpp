//Class pipenet defined here
#include "classes.h"
#include "MatVec.h"

pipenet::pipenet(ifstream& infile)
{
	infile >> n_nodes; //inputs the first line from the .txt file
	infile >> n_tubes; //inputs the second line from the .txt file

					   //creating nodes and tubes based on the number of nodes and tubes numbers
	vec_nodes = new node*[n_nodes];//a vector of size equivalent to the number of nodes which was imported from the .txt file
	vec_tubes = new tube*[n_tubes];//a vector of size equivalent to the number of tubes

	for (int i = 0; i<n_nodes; i++) //this step puts the informations of the x,y-coords and Q of all the nodes into a 2D-array
	{
		vec_nodes[i] = NULL; //initialization of the elements
		double array[3];
		for (int j = 0; j<3; j++) // creating 3 columns in each row
		{
			infile >> array[j]; //import x, y ,Q as array[] 0, 1, 2
		}
		vec_nodes[i] = new node(i + 1, array[0], array[1], array[2]); //array 0,1,2 >>> num,x, y,Q(num refers to node number)
	}						//this has the same format as the constructor of the node class
							//this re-initializes the values of each row vectors

							// tubes

	int _1stnode, _2ndnode;

	for (int i = 0; i<n_tubes; i++) //this step also creates a 2D array that contains, node a, node b, diameter
	{
		double array[3];
		for (int j = 0; j < 3; j++)
		{
			infile >> array[j];//import a, b ,diameter as array[] 0, 1, 2
		}
		_1stnode = array[0] - 1;
		_2ndnode = array[1] - 1;
		vec_tubes[i] = new tube(i + 1, vec_nodes[_1stnode], vec_nodes[_2ndnode], array[0], array[1], array[2]);
		//this has the same format as the constructor of the tube class
		//so it's initializing the data members accordingly
		//the format is (int num,node* a,node* b,int nodeone,int nodetwo,double dia)
	}
}

void pipenet::calcflowrate()
{// Permeability matrix
	double** mtxB = new double*[n_nodes];

	for (int i = 0; i < n_nodes; i++)
	{
		mtxB[i] = new double[n_nodes];
	}

	for (int i = 0; i < n_nodes; i++)
	{
		for (int j = 0; j < n_nodes; j++)
		{
			mtxB[i][j] = 0;
		}
	}//the 2D square matrix is created


	for (int i = 0; i < n_tubes; i++)
	{
		int a = vec_tubes[i]->getn1() - 1; //subtracts 1 from node a in the txt file to match the array indeces
		int b = vec_tubes[i]->getn2() - 1;
		double Bcoef = vec_tubes[i]->getB(); //calculates B for each tube. [i] indicates an array that contains 
											 //cout<<i+1<<"\t"<<a+1<<"\t"<<b+1<<"\t"<<Bcoef<<endl;

											 //********assembly to global Bmatrix									
		mtxB[a][a] += Bcoef;
		mtxB[b][b] += Bcoef;
		mtxB[a][b] -= Bcoef;
		mtxB[b][a] -= Bcoef;
	}//Global matrix B is now created

	 ////****************** Q matrix *******************//
	double* vec_Q = new double[n_nodes];

	for (int i = 0; i < n_nodes; i++)
	{
		vec_Q[i] = (-1)*vec_nodes[i]->getQ();//getQ belongs to class node
											 //get the Q from the vec_nodes and multiplies it by -1 
											 //so create the appropriate form of Ax=B >>> Bh=-Q
	}

	//******************* Applying Boundary Conditions ************************//
	vec_Q[0] = 0.0;
	mtxB[0][0] = 1.0;
	for (int i = 1; i < n_nodes; i++)
	{
		mtxB[0][i] = 0.0; //B elements at first row=0
		mtxB[i][0] = 0.0; //B elements at first column=0

	}
	// --- Boundary conditions

	// Solves the linear system of equations
	Mtx GlobalB(n_nodes, mtxB); // Creates object GlobalB of Mtx Class  
	Vcr VecQ(n_nodes, vec_Q); // Creates object VecQ of Vcr Class 

	GlobalB.GaussElim(VecQ); // Calls Mtx Class member function GaussElim(Vcr& bb) 

	for (int i = 0; i < n_nodes; i++)
	{
		double h = VecQ.getvalues(i); // Get values of head
		vec_nodes[i]->definehead(h); // Set values of head
	}
	//**************Calculate and display flow****************//
	// Calculates and displays tube flow using Tube member function displayflow()
	for (int i = 0; i < n_tubes; i++)
	{
		vec_tubes[i]->displayflow();
	}
}
pipenet::~pipenet()
{
	for (int i = 0; i < n_nodes; i++) { delete[] vec_nodes[i]; }
	delete[] vec_nodes;
	for (int i = 0; i < n_tubes; i++) { delete[] vec_tubes[i]; }
	delete[] vec_tubes;
}