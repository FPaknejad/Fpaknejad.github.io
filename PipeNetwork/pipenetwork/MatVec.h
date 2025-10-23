/*
	MatVec.hpp
	Interface for the calss Vcr and Mtx
*/
//---------------------------------------------------------------------------------
// CLASS Vcr (VECTOR)
//---------------------------------------------------------------------------------
class Vcr {       

private:
  int lenth;										// number of entries 
  double* vr;										// entries of the vector

public: 
  Vcr(int n, double*);							// constructor, all entries = t
  Vcr(const Vcr&);									// copy constructor
  Vcr& operator=(const Vcr&);						// copy assignment
  ~Vcr(){ delete[] vr; }							// destructor

  double& operator[](int i) const { return vr[i]; }	// subscript, e.g. v[3] = 1.2

  int size() const { return lenth; }				// return size of vector
  double onenorm() const;							// one norm
  double twonorm() const;							// two norm
  double maxnorm() const;							// maximum norm
  void print() const;	// print vector
  double getvalues(int);
  friend double dot(const Vcr& v1, const Vcr& v2);	// a friend
};

//---------------------------------------------------------------------------------
// CLASS Mtx (MATRIX)
//---------------------------------------------------------------------------------
class Mtx {											// square matrix

private: 
  int dimn;											// dimension of matrix
  double** mx;										// entries of the matrix

public: 
  Mtx(int n, double** a);							// constructor, all entries = t
  Mtx(const Mtx& );									// copy constructor
  Mtx& operator=(const Mtx&);						// copy assignment
  ~Mtx(){											// destructor
    for (int i = 0; i< dimn; i++) delete[]  mx[i];
    delete[] mx;
  }

  double* operator[](int i) const { return mx[i]; }	// subscript, entry at row i and
													// column j is [i][j]

  double onenorm() const;							// one norm
  double maxnorm() const;							// maximum norm
  double frobnorm() const;							// Frobenius norm
  void GaussElim(Vcr& bb) const;					// Gaussian elimination A x = bb
  void print() const;								// print matrix
};