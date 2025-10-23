/*
	MatVec.cpp
	implementation of the class Vcr and Matrix
*/
#include <cstdlib>
#include <cmath>
#include <iostream> 
#include "MatVec.h"

using namespace std;

// report error and exit with return value 1.
void error(const char* t) {
  std::cout << t << ". program exited." << "\n";
  exit(1);
}
// returns the max of two doubles
// an inline function must be defined in every file in which
// it is used with exactly the same definition
inline double max(double a, double b)
{
	return (a < b) ?  b : a;
}

//---------------------------------------------------------------------------------
// members of class Vcr
//---------------------------------------------------------------------------------
// constructor
Vcr::Vcr(int n, double* a) {
  vr = new double [lenth = n]; 
  for (int i = 0; i < lenth; i++)  vr[i] = a[i];
}
// copy constructor
Vcr::Vcr(const Vcr& vec) {
  vr = new double [lenth = vec.lenth]; 
  for (int i = 0; i < lenth; i++)  vr[i] = vec.vr[i]; 
}
// copy assignment
Vcr& Vcr::operator=(const Vcr& vec) {
  if (this != &vec) {
    if (lenth != vec.lenth) error("bad vector sizes in Vcr::operator=()");
    for (int i = 0; i < lenth; i++) vr[i] = vec.vr[i];
  }
  return *this;
}
// one norm
double Vcr::onenorm() const
{
	double norm = fabs(vr[0]);
	for (int i = 1; i < lenth; i++) norm += fabs(vr[i]);
	return norm;
}
// two norm
double Vcr::twonorm() const
{
	double norm = vr[0]*vr[0];
	for (int i= 1; i < lenth; i++) norm += vr[i] * vr[i];
	return sqrt(norm);
}
// maximum norm
double Vcr::maxnorm() const
{
	double norm = fabs(vr[0]);
	for (int i = 1; i < lenth; i++) norm = max(norm, fabs(vr[i]));
	return norm;
}
// print vector
void Vcr::print() const
{
	for (int i=0;i<lenth;++i)
	{
		cout << vr[i] << endl;
	}
}

double Vcr::getvalues(int n)
{
	return vr[n];
}

// a friend
double dot(const Vcr & v1, const Vcr & v2) {
  if (v1.lenth != v2.lenth ) error("bad vector sizes in dot product");
  double tm = v1[0]*v2[0];
  for (int i = 1; i < v1.lenth; i++) tm += v1[i]*v2[i];
  return tm;
}

//---------------------------------------------------------------------------------
// members of class Mtx
//---------------------------------------------------------------------------------
// constructor
Mtx::Mtx(int n, double** a) {
  dimn = n;
  mx = new double* [dimn]; 
  for (int i =  0; i< dimn; i++) 
  {
    mx[i] = new double [dimn];
    for (int j = 0; j < dimn; j++) mx[i][j] = a[i][j];
  }
}
// copy constructor
Mtx::Mtx(const Mtx& M)                             
{
  dimn = M.dimn;
  mx = new double* [dimn]; 
  for (int i =  0; i < dimn; i++) 
  {
    mx[i] = new double [dimn];
    for (int j = 0; j < dimn; j++) mx[i][j] = M.mx[i][j];
  }

}
// copy assignment  
Mtx& Mtx::operator=(const Mtx& M)
{
  if (dimn!=M.dimn) error("Matrices have different size in =");
  for (int i =  0; i < dimn; i++) 
  {
    for (int j = 0; j < dimn; j++) mx[i][j] = M.mx[i][j];

  }
  return *this;
}
// print matrix
void Mtx::print() const
{
	for (int i=0; i<dimn; i++)
    {
		for (int j=0;j<dimn;j++) cout << mx[i][j] << "\t"; 
	    cout << endl;
	}
}
// one norm
double Mtx::onenorm() const
{
	double norm = 0.0;

	for (int j = 0; j < dimn; j++)
	{
		double temp = 0.0;	// store column abs sum
		for (int i = 0; i < dimn; i++) temp += fabs(mx[i][j]);
		norm = max(norm, temp);	
	}
	return norm;
}
// maximum norm
double Mtx::maxnorm() const
{
	double norm = 0.0;

	for (int i = 0; i < dimn; i++)
	{
		double temp = 0.0;	// store column abs sum
		for (int j = 0; j < dimn; j++) temp += fabs(mx[i][j]);
		norm = max(norm, temp);	
	}
	return norm;
}
// Frobenius norm
double Mtx::frobnorm() const
{
	double norm = 0.0;

	for (int i = 0; i < dimn; i++)
		for (int j = 0; j < dimn; j++)	norm += mx[i][j] * mx[i][j];
	
	return sqrt(norm);
}
// Gaussian elimination A x = bb
void Mtx::GaussElim(Vcr& bb) const
{
  if (dimn != bb.size() ) 
    error("matrix or vector sizes do not match");
  Mtx tmpx = *this;
  
  // LU decomposition without pivoting 
  for (int k = 0; k < dimn - 1; k++) {
    if (tmpx[k][k] == 0) 
      error("pivot is zero in Mtx::GaussElim()");
    for (int i = k + 1; i < dimn; i++) {
      if (tmpx[i][k] != 0)
	  {   // tmpx[i][k] can be complex
        double mult = tmpx[i][k]/tmpx[k][k];
        tmpx[i][k] = mult;
        for (int j = k + 1; j < dimn; j++) 
          tmpx[i][j] -= mult*tmpx[k][j];
      }
    }
  }

  // forwad substitution for L y = b. y still stored in bb
  for (int i = 1; i < dimn; i++) 
    for (int j = 0; j < i; j++) bb[i] -= tmpx[i][j]*bb[j];

  // back substitution for U x = y. x still stored in bb
  for (int i = dimn - 1; i >= 0; i--) {
    for (int j=i+1; j<dimn; j++) bb[i] -= tmpx[i][j]*bb[j];
    bb[i] /= tmpx[i][i];
  }
} // end GaussElim()

