/*
 * Interpolate.cpp
 *
 *  Created on: Mar 1, 2010
 *      Author: christop
 */

#include <math.h>
#include "bcuint_2nd.cpp"
#include "mex.h"

#define NRANSI
#include "nrutil.h"
#undef NRANSI

//#define DebugOut

// SPeedup would be to put filtering with [-1 0 1], and [-1 0 1]' and cross outside !

void Interpolate_2ndDerivatives ( int nlhs, mxArray *plhs[],
                                  int nrhs, const mxArray *prhs[])
{
int m, n, i, e, j;
int N(0),M(0);

int elements, nsubs;
double *outputF, *outputdFX, *outputdFY, *outputdFXX, *outputdFYY, *outputdFXY, *I, *X, *Y;

char buffer[256];

/* check: only one input and one output argument */
if (nrhs < 3)
  mexErrMsgTxt("Must have 5 input arguments:\n size of data matrix NxM N,M; Image I; X-Indices X and Y-Indices Y\n)");

for(int i = 0;i<3;i++)
if (!mxIsDouble(prhs[i]))
{
  mexErrMsgTxt("First five input argument must be a double.");
}

I = mxGetPr(prhs[0]);
X = mxGetPr(prhs[1]);
Y = mxGetPr(prhs[2]);

N = (int) mxGetN(prhs[0]);
M = (int) mxGetM(prhs[0]);

if (nlhs !=6)
  mexErrMsgTxt("Must have amount of 6 output arguments: ( F(x) and dF(x)/dX and dF(x)/dY and dF(x)/dXX and dF(x)/dYY and dF(x)/dXY)");

//if (nrhs !=5)
//  mexErrMsgTxt("Must have 5 input arguments");

/* Create the output array */
/*
for (int i=0;i<U;i++)
{
  plhs[i] = mxCreateDoubleMatrix(N, M, mxREAL);
  output  = mxGetPr(plhs[i]);
  // Populate the output
  memcpy(output, mxGetPr(prhs[i+7]), sizeof(double)*N*M);
}
*/

plhs[0]  = mxCreateDoubleMatrix(M, N, mxREAL); // the function value
outputF  = mxGetPr(plhs[0]);
plhs[1]  = mxCreateDoubleMatrix(M, N, mxREAL); // the derivative
outputdFX = mxGetPr(plhs[1]);
plhs[2]  = mxCreateDoubleMatrix(M, N, mxREAL); // the derivative
outputdFY = mxGetPr(plhs[2]);
plhs[3]  = mxCreateDoubleMatrix(M, N, mxREAL); // the 2nd derivative dxx
outputdFXX = mxGetPr(plhs[3]);
plhs[4]  = mxCreateDoubleMatrix(M, N, mxREAL); // the 2nd derivative dyy
outputdFYY = mxGetPr(plhs[4]);
plhs[5]  = mxCreateDoubleMatrix(M, N, mxREAL); // the 2nd derivative dxy
outputdFXY = mxGetPr(plhs[5]);


/* prevent you from passing a sparse matrix,
a string matrix, or a complex array. mxIsComplex
is used to determine if there is an imaginary
part of the mxArray. mxIsClass is used to determine
if the mxArray belongs to a particular class */
//if ( mxIsComplex(prhs[5])|| mxIsClass(prhs[5],"sparse") || mxIsChar(prhs[5]) )
//  mexErrMsgTxt("Input must be real, full, and nonstring");

/* Get the number of elements in the input argument */
elements=mxGetNumberOfElements(prhs[0]);

sprintf (buffer, "There is some Error %d elements./n", elements);

if (elements != N*M)
  mexErrMsgTxt(buffer);

/* Get the number of dimensions in array */
//nsubs=mxGetNumberOfDimensions(prhs[5]);
//msubs=mxGetNumberOfDimensions(prhs[6]);

// Start by computing derivatives:

//double 
//std::vector<double> Idx(N*M),Idy(N*M),Idxy(N*M);
double *Idx,*Idy,*Idxy;
Idx  = (double*) mxMalloc( N*M*sizeof(double) );
Idy  = (double*) mxMalloc( N*M*sizeof(double) );
Idxy = (double*) mxMalloc( N*M*sizeof(double) );

#pragma omp for //num_threads(omp_get_num_procs())
for(int n=0;n<N;n++)
{
  for(int m=0;m<M;m++)
  {
    int id = n*M+m; // m is the row index, that is determines y, id +1 is row below current one
//    int id_test  = m*N+n;

#ifdef DebugOut
//  mexPrintf("n=%d m=%d = %f\n", n, m, I[id]);
#endif


  if ( (m>0) && (m<M-1) )
    Idy[id]  = (I[id+1]-I[id-1])/2.0;
  if ( (n>0) && (n<N-1) )
    Idx[id]  = (I[id+M]-I[id-M])/2.0;

  if ( (m>0) && (m<M-1) && (n>0) && (n<N-1) )
    Idxy[id] = (I[id+M+1]-I[id+M-1] - I[id-M+1] + I[id-M-1]) /4.0;

  }
}

#ifdef DebugOut
//  mexPrintf("First Row\n");
#endif

// along first row ______
//                 ...
for(int n=0;n<N;n++)
{
  int m = 0;
  int id = n*M+m;

#ifdef DebugOut
//  mexPrintf("n=%d m=%d = %f - %f\n", n, m, I[id+1], I[id]);
#endif

  Idy[id]  = (I[id+1]-I[id])/1.0;

  if ( (n>0) && (n<N-1) )
    Idxy[id] = (I[id+M+1]-I[id+M] - I[id-M+1] + I[id-M]) /2.0;
  else
  {
    if ( (n>0) ) // corner upper right
      Idxy[id] = (I[id+1]-I[id] - I[id-M+1] + I[id-M]) /2.0;
    else // n== 0 // corner upper left
      Idxy[id] = (I[id+M+1]-I[id+M] - I[id+1] + I[id]) /2.0;
  }
}

#ifdef DebugOut
//  mexPrintf("Last Row\n");
#endif
//              ...
// along last row ______
for(int n=0;n<N;n++)
{
  int m = M-1;
  int id = n*M+m;

#ifdef DebugOut
//  mexPrintf("n=%d m=%d = %f - %f\n", n, M-1, I[id], I[id-1]);
#endif

  Idy[id]  = (I[id]-I[id-1])/1.0;

  if ( (n>0) && (n<N-1) )
    Idxy[id] = (I[id+M]-I[id+M-1] - I[id-M] + I[id-M-1]) /2.0;
  else
  {
    if ( (n>0) )// corner lower right
      Idxy[id] = (I[id]-I[id-1] - I[id-M] + I[id-M-1]) /2.0;
    else// corner lower left
      Idxy[id] = (I[id+M]-I[id+M-1] - I[id] + I[id-1]) /2.0;
  }
}

#ifdef DebugOut
//  mexPrintf("First Column\n");
#endif
// along first column |...
for(int m=0;m<M;m++)
{
  int n = 0;
  int id = n*M+m;

#ifdef DebugOut
//  mexPrintf("n=%d m=%d = %f - %f\n", n, m, I[id+M], I[id]);
#endif

  Idx[id]  = (I[id+M]-I[id])/1.0;

  if ( (m>0) && (m<M-1) )
    Idxy[id] = (I[id+M+1]-I[id+M-1] - I[id+1] + I[id-1]) /2.0;
  else
  {
    if ( (m>0) )// corner lower left
      Idxy[id] = (I[id+M]-I[id+M-1] - I[id] + I[id-1]) /2.0;
    else// corner upper left
      Idxy[id] = (I[id+M+1]-I[id+M] - I[id+1] + I[id]) /2.0;
  }
}

#ifdef DebugOut
 // mexPrintf("Last Column\n");
#endif
// along last column ...|
for(int m=0;m<M;m++)
{
  int n = N-1;
  int id = n*M+m;

#ifdef DebugOut
 // mexPrintf("n=%d m=%d = %f - %f\n", n, m, I[id], I[id-M]);
#endif

  Idx[id]  = (I[id]-I[id-M])/1.0;

  if ( (m>0) && (m<M-1) )
    Idxy[id] = (I[id+1]-I[id-1] - I[id-M+1] + I[id-M-1]) /2.0;
  else
  {
    if ( (m>0) ) // corner lower right
      Idxy[id] = (I[id]-I[id-1] - I[id-M] + I[id-M-1]) /2.0;
    else// corner upper right
      Idxy[id] = (I[id+1]-I[id] - I[id-M+1] + I[id-M]) /2.0;
  }
}
/////////////////////////////////////////////////////////

// works
#ifdef DebugOut
for(int n=0;n<N;n++)
{
  for(int m=0;m<M;m++)
  {
    int id = n*M+m; // m is the row index, that is determines y, id +1 is row below current one

    mexPrintf("n=%d m=%d = ( %f %f %f )\n", n, m, Idx[id], Idy[id], Idxy[id]);
  }
}
#endif

#ifdef DebugOut
int nProcs = omp_get_num_procs();
int numThreads = omp_get_num_threads();
int mThreads = omp_get_max_threads();

printf("nProcs: %d, nThreads: %d, mThreads: %d", nProcs, numThreads, mThreads);

mexPrintf("Start Interpolation\n");
#endif
#pragma omp parallel for schedule (static)
for(int n=0;n<N;n++)
{
  double **cc;
  cc=matrix(1,4,1,4);

#ifdef DebugOut
  int numPrcs = omp_get_num_procs();
  int th_id = omp_get_thread_num();
  printf("row %d threadid is : %d\n", n, th_id);
#endif

  for(int m=0;m<M;m++) // row index is y coordinate
  {
//    int id_inv = n*N+m;
    int id = n*M+m;

//    int id  = m*N+n;

    int intx, inty;
    double x,y;
    double t,u;

    // c++ style indices 
    double idx = X[id] -1.0;
    double idy = Y[id] -1.0;

    if ( (idx >= double(N-1)) )
      idx = idx - 0.000001;

    if ( (idy >= double(M-1)) )
      idy = idy - 0.000001;

    if ( (idx < double(0)) )
      idx = idx + 0.000001;

    if ( (idy < double(0)) )
      idy = idy + 0.000001;

    x = floor( idx );
    y = floor( idy );

    intx = int(x);
    inty = int(y);

    if ( (intx >= N-1) || (intx < 0) || (inty >= M-1) || (inty < 0) )
    {
      if (intx < 0)   intx = 0;
      if (intx > N-1) intx = N-1;
      if (inty < 0)   inty = 0;
      if (inty > M-1) inty = M-1;
      int myid = intx*M+inty;
      outputF[id]  = I[myid];
      outputdFX[id] = 0.0;
      outputdFY[id] = 0.0;
      continue;
    }

#ifdef DebugOut
    mexPrintf("n=%d m=%d : %d %d  %f - %f\n", n, m, intx, inty, X[id], Y[id]);
    int myid = intx*M+inty+1;
    double a  = I[myid];
    double b = Idx[myid];
    double c = Idy[myid];
    double d = Idxy[myid];
    mexPrintf("id=(%d %d) : %f %f %f %f\n", intx, inty+1, a, b, c, d);
     myid = intx*M+inty+1+M;
     a  = I[myid];
     b = Idx[myid];
     c = Idy[myid];
     d = Idxy[myid];
    mexPrintf("id=(%d %d) : %f %f %f %f\n", intx+1, inty+1, a, b, c, d);
    myid = intx*M+inty+M;
     a  = I[myid];
     b = Idx[myid];
     c = Idy[myid];
     d = Idxy[myid];
    mexPrintf("id=(%d %d) : %f %f %f %f\n", intx+1, inty, a, b, c, d);
     myid = intx*M+inty;
     a  = I[myid];
     b = Idx[myid];
     c = Idy[myid];
     d = Idxy[myid];
    mexPrintf("id=(%d %d) : %f %f %f %f\n", intx, inty, a, b, c, d);
#endif

    // first get t and u and the 
//    t = idx-x;
//    u = idy-y;

    double f[5], y1[5], y2[5], y12[5];

    int k = 2, add = 1;
    // start at lower left and go counterclockwise
    int idCC = intx*M + inty + 1;

    k = 4;
    f[k]   =    I[idCC];
    y1[k]  =  Idx[idCC];
    y2[k]  =  Idy[idCC];
    y12[k] = Idxy[idCC];

    k = 3;
    idCC   = intx*M + inty + 1 + M;
    f[k]   = I[idCC];
    y1[k]  = Idx[idCC];
    y2[k]  = Idy[idCC];
    y12[k] = Idxy[idCC];

    k = 2;
    idCC   = intx*M + inty + M;
    f[k]   = I[idCC];
    y1[k]  = Idx[idCC];
    y2[k]  = Idy[idCC];
    y12[k] = Idxy[idCC];

    k = 1;
    idCC   = intx*M + inty;
    f[k]   = I[idCC];
    y1[k]  = Idx[idCC];
    y2[k]  = Idy[idCC];
    y12[k] = Idxy[idCC];

/*
    {
    double d1,d2,**c;
    c=matrix(1,4,1,4);

    d1=idx-x;
    d2=idy-y;
    bcucof(f,y1,y2,y12,d1,d2, c);
    for (i=1;i<=4;i++)
      for (j=1;j<=4;j++)
        mexPrintf(" %f ", c[i][j]);
    mexPrintf("\n");
    }
    */

    bcuint_2nd(f, y1, y2, y12, x, x+1.0, y, y+1.0, idx, idy, &(outputF[id]), &(outputdFX[id]), &(outputdFY[id]), &(outputdFXX[id]), &(outputdFYY[id]), &(outputdFXY[id]), cc );

  }
  free_matrix(cc,1,4,1,4);
}

mxFree(Idx);
mxFree(Idy);
mxFree(Idxy);
};
