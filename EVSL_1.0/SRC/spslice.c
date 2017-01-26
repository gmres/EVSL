#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "def.h"
#include "blaslapack.h"
#include "struct.h"
#include "internal_proto.h"

int kpmdos(csrMat *A, int Mdeg, int damping, int nvec, double *intv,
    double *mu, double *ecnt){
  /*----------------------------------------------------------------------
  / This function  computes the  coefficients of the  density of
  / states  in  the  chebyshev   basis.   It  also  returns  the
  / estimated number of eigenvalues in the interval given by intv.
  / -------------------------------------------------------------------------
  / INPUT: 
  / csrMat *A   == input matrix
  / int Mdeg    == degree of polynomial to be used. 
  / int damping == type of damping to be used [0=none,1=jackson,2=sigma]
  / int nvec    == number of random vectors to use for sampling
  / double *intv  == an array of length 4 
  /                 [intv[0] intv[1]] is the interval of desired eigenvalues
  /                 that must be cut (sliced) into n_int  sub-intervals
  /                 [intv[2],intv[3]] is the global interval of eigenvalues
  /                 it must contain all eigenvalues of A
  / RETURN
  / double *mu  == array of Chebyshev coefficients 
  / double *ecnt== estimated num of eigenvalues in the interval of interest
  *----------------------------------------------------------------------*/

  /*-------------------- initialize variables */
  int n = A->nrows; 
  double *vkp1 = malloc(n*sizeof(double));
  double *w = malloc(n*sizeof(double)); 
  double *vkm1 = malloc(n*sizeof(double)); 
  double *vk = malloc(n*sizeof(double));
  double *jac =  malloc((Mdeg+1)*sizeof(double));
  double *tmp,  ctr, wid; 
  double scal, t, tcnt, beta1, beta2, aa, bb;
  int k, k1, i, m, mdegp1, one=1;
  //-------------------- check if the interval is valid
  if (check_intv(intv, stdout) < 0) {
    return -1;
  }

  aa = max(intv[0], intv[2]);  bb = min(intv[1], intv[3]);
  if (intv[0] < intv[2] || intv[1] > intv[3]) {
    fprintf(stdout, " warning [%s (%d)]: interval (%e, %e) is adjusted to (%e, %e)\n",
            __FILE__, __LINE__, intv[0], intv[1], aa, bb);
  }
  /*-------------------- some needed constants */
  ctr  = (intv[3]+intv[2])/2.0;
  wid  = (intv[3]-intv[2])/2.0;
  t = max(-1.0+DBL_EPSILON, (aa-ctr)/wid);
  beta1 = acos(t);
  t = min(1.0-DBL_EPSILON, (bb-ctr)/wid);
  beta2 = acos(t);
  /*-------------------- compute damping coefs. */
  dampcf(Mdeg, damping, jac);
  //-------------------- readjust jac[0] it was divided by 2
  jac[0] = 1.0;
  memset(mu,0,(Mdeg+1)*sizeof(double));
  //-------------------- seed the random generator 
  i = time_seeder();
  srand(i);
  tcnt = 0.0;
  /*-------------------- for random loop */
  for (m=0; m<nvec; m++){
    rand_double(n, w);
    t = 1.0 / DNRM2(&n, w, &one);
    DSCAL(&n, &t, w, &one);
    memset(vkm1, 0, n*sizeof(double));
    memcpy(vk, w, n*sizeof(double));
    mu[0] += jac[0];
    //-------------------- for eigCount
    tcnt -= jac[0]*(beta2-beta1);  
    /*-------------------- Chebyshev (degree) loop */
    for (k=0; k<Mdeg; k++){
      /*-------------------- Cheb. recurrence */
      matvec(A, vk, vkp1);
      scal = k==0 ? 1.0 : 2.0;
      scal /= wid;
      for (i=0; i<n; i++)
        vkp1[i] = scal*(vkp1[i]-ctr*vk[i]) -vkm1[i];
      //-------------------- rotate pointers to exchange vectors
      tmp = vkm1;
      vkm1 = vk;
      vk = vkp1;
      vkp1 = tmp;
      /*-------------------- accumulate dot products for DOS expansion */
      k1 = k+1;
      t = 2*jac[k1]*DDOT(&n, vk, &one, w, &one);
      mu[k1] += t;
      /*-------------------- for eig. counts */
      tcnt -= t*(sin(k1*beta2)-sin(k1*beta1))/k1;  
    }
  }
  //--------------------change of interval + scaling in formula
  t = 1.0 /(((double)nvec)*PI) ;
  mdegp1 = Mdeg+1;
  DSCAL(&mdegp1, &t, mu, &one) ;
  tcnt *= t*((double) n);
  *ecnt = tcnt;
  /*-------------------- free memory  */
  free(vkp1);
  free(w);
  free(vkm1);
  free(vk);
  free(jac);
  return 0;
}

void intChx(int Mdeg, double *mu, int npts, double *xi, double *yi) { 
  //  This computes the integrals   int_xi(0)^xi(j)   p(t) dt
  //  where p(t) is the approximate DOS as given in the KPM method
  //  in the expanded form:  \sum mu_i C_i /\sqrt(1-t^2)
  //
  int ndp1, j, k;
  double val0, theta0;
  double *thetas = (double*)malloc(npts*sizeof(double));
  ndp1   = Mdeg+1; 

  //  if (xi[0]<-1.0) xi[0] = -1; 
  //if (xi[npts-1]> 1.0) xi[npts-1]  = 1; 

  for (j=0; j<npts; j++)
    thetas[j] = acos(xi[j]);
  theta0 = thetas[0];
  for (j=0; j<npts; j++) 
    yi[j] = mu[0]*(theta0 - thetas[j]);
  //-------------------- degree loop  
  for (k=1; k<ndp1; k++){
    val0 = sin(k*theta0)/k;
    //-------------------- points loop
    for (j=0; j<npts; j++)
      yi[j] += mu[k]*(val0 - sin(k*thetas[j])/k);
  }
  free (thetas);
}

int spslicer(double *sli, double *mu, int Mdeg, double *intv, int n_int, int npts){ 
  /*----------------------------------------------------------------------- 
    / given the dos function defined by mu find a partitioning
    / of sub-interval [a,b] of the spectrum so each 
    / subinterval has about the same number of eigenvalues
    / Mdeg = degree.. mu is of length Mdeg+1  [0---> Mdeg]
    / on return [ sli[i],sli[i+1] ] is a subinterval (slice).
    /
    / double *sli, = see above (output)
    / double *mu,  = coeffs of polynomial (input)
    / int Mdeg,    = degree of polynomial (input)
    / double *intv = an array of length 4 
    /                [intv[0] intv[1]] is the interval of desired eigenvalues
    /                that must be cut (sliced) into n_int  sub-intervals
    /                [intv[2],intv[3]] is the global interval of eigenvalues
    /                it must contain all eigenvalues of A
    / int n_int,   = number of slices wanted (input)
    / int npts     = number of points to use for discretizing the interval
    /                [a b]. The more points the more accurate the intervals. 
    /                it is recommended to set npts to a few times the number 
    /                of eigenvalues in the interval [a b] (input). 
    /----------------------------------------------------------------------*/
  //-------------------- transform to [-1, 1]   
  int ls, ii;
  double  ctr, wid, aL, bL, target, aa, bb;

  if (check_intv(intv, stdout) < 0) {
    return -1;
  }

  // adjust a, b: intv[0], intv[1]
  aa = max(intv[0], intv[2]);  bb = min(intv[1], intv[3]);
  if (intv[0] < intv[2] || intv[1] > intv[3]) {
    fprintf(stdout, " warning [%s (%d)]:  interval (%e, %e) is adjusted to (%e, %e)\n",
        __FILE__, __LINE__, intv[0], intv[1], aa, bb);
  }

  //-------------------- 
  memset(sli,0,(n_int+1)*sizeof(double));
  //-------------------- transform to ref interval [-1 1]
  //-------------------- take care of trivial case n_int==1
  if (n_int == 1){
    sli[0] = intv[0];
    sli[1] = intv[1];
    return 0;
  }
  //-------------------- general case 
  ctr = (intv[3] + intv[2])/2;
  wid = (intv[3] - intv[2])/2;
  aL  = (aa - ctr)/wid;   // (a - ctr)/wid 
  bL  = (bb - ctr)/wid;   // (b - ctr)/wid
  aL = max(aL,-1.0);
  bL = min(bL,1.0);
  npts = max(npts,2*n_int+1);
  double *xi = (double*)malloc(npts*sizeof(double));
  double *yi = malloc(npts*sizeof(double));
  linspace(aL, bL, npts, xi);
  //printf(" aL %15.3e bL %15.3e \n",aL,bL);
  //-------------------- get all integrals at the xi's 
  //-------------------- exact integrals  used.
  intChx(Mdeg, mu, npts, xi, yi) ; 
  //-------------------- goal: equal share of integral per slice
  target = yi[npts-1] / (double)n_int;
  ls = 0;
  ii = 0;
  // use the unadjust left boundary
  sli[ls] = intv[0];
  //-------------------- main loop 
  while (++ls < n_int) {
    while (ii < npts && yi[ii] < target) {
      ii++;
    }
    if (ii == npts) {
      break;
    }
    //-------------------- take best of 2 points in interval
    if ( (target-yi[ii-1]) < (yi[ii]-target)) {
      ii--;
    }
    sli[ls] = ctr + wid*xi[ii];
    //-------------------- update target.. Slice size adjusted
    target = yi[ii] + (yi[npts-1] - yi[ii])/(n_int-ls);
    //printf("ls %d, n_int %d, target %e\n", ls, n_int, target);
  }

  // use the unadjust left boundary
  sli[n_int] = intv[1];
  /*-------------------- free arrays */
  free(xi);
  free(yi);

  //-------------------- check errors
  if (ls != n_int) {
    return 1;
  }
  for (ii=1; ii<=n_int; ii++) {
    if (sli[ii] <= sli[ii-1]) {
      return 2;
    }
  }
  return 0;
}


