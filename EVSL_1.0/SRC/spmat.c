#include "def.h"
#include "blaslapack.h"
#include "struct.h"
#include "internal_proto.h"

/**-------------------------------------------------*
 * @brief convert csr to csc
 * Assume input csr is 0-based index
 * output csc 0/1 index specified by OUTINDEX      *
 * ------------------------------------------------*/
void csrcsc(int OUTINDEX, int nrow, int ncol, int job,
    double *a, int *ja, int *ia,
    double *ao, int *jao, int *iao) {
  int i,k;
  for (i=0; i<ncol+1; i++) {
    iao[i] = 0;
  }
  // compute nnz of columns of A
  for (i=0; i<nrow; i++) {
    for (k=ia[i]; k<ia[i+1]; k++) {
      iao[ja[k]+1] ++;
    }
  }
  // compute pointers from lengths
  for (i=0; i<ncol; i++) {
    iao[i+1] += iao[i];
  }
  // now do the actual copying
  for (i=0; i<nrow; i++) {
    for (k=ia[i]; k<ia[i+1]; k++) {
      int j = ja[k];
      if (job) {
        ao[iao[j]] = a[k];
      }
      jao[iao[j]++] = i + OUTINDEX;
    }
  }
  /*---- reshift iao and leave */
  for (i=ncol; i>0; i--) {
    iao[i] = iao[i-1] + OUTINDEX;
  }
  iao[0] = OUTINDEX;
}

/**-------------------------------------------*
 * @brief  Sort each row of a csr by increasing column 
 * order
 * By double transposition
 *-------------------------------------------*/
void sortrow(csrMat *A) {
  /*-------------------------------------------*/
  int nrows = A->nrows;
  int ncols = A->ncols;
  int nnz = A->ia[nrows];
  // work array
  double *b;
  int *jb, *ib;
  Malloc(b, nnz, double);
  Malloc(jb, nnz, int);
  Malloc(ib, ncols+1, int);
  // double transposition
  csrcsc(0, nrows, ncols, 1, A->a, A->ja, A->ia, b, jb, ib);
  csrcsc(0, ncols, nrows, 1, b, jb, ib, A->a, A->ja, A->ia);
  // free
  free(b);
  free(jb);
  free(ib);
}

void csr_resize(int nrow, int ncol, int nnz, csrMat *csr) {
  csr->nrows = nrow;
  csr->ncols = ncol;
  Malloc(csr->ia, nrow+1, int);
  Malloc(csr->ja, nnz, int);
  Malloc(csr->a, nnz, double);
}

void free_csr(csrMat *csr) {
  free(csr->ia);
  free(csr->ja);
  free(csr->a);
}

void free_coo(cooMat *coo) {
  free(coo->ir);
  free(coo->jc);
  free(coo->vv);
}

/*---------------------------------------------------------
 * @brief convert coo to csr
 *---------------------------------------------------------*/
int cooMat_to_csrMat(int cooidx, cooMat *coo, csrMat *csr) {
  int nnz = coo->nnz;
  //printf("@@@@ coo2csr, nnz %d\n", nnz);
  csr_resize(coo->nrows, coo->ncols, nnz, csr);

  int i;
  for (i=0; i<coo->nrows+1; i++) {
    csr->ia[i] = 0;
  }
  for (i=0; i<nnz; i++) {
    int row = coo->ir[i] - cooidx;
    csr->ia[row+1] ++;
  }
  for (i=0; i<coo->nrows; i++) {
    csr->ia[i+1] += csr->ia[i];
  }
  for (i=0; i<nnz; i++) {
    int row = coo->ir[i] - cooidx;
    int col = coo->jc[i] - cooidx;
    double val = coo->vv[i];
    int k = csr->ia[row];
    csr->a[k] = val;
    csr->ja[k] = col;
    csr->ia[row]++;
  }
  for (i=coo->nrows; i>0; i--) {
    csr->ia[i] = csr->ia[i-1];
  }
  csr->ia[0] = 0;

  sortrow(csr);
  return 0;
}


double dcsr1nrm(csrMat *A){
  // computes the 1-norm of A 
  double ta = 0.0, t = 0.0;
  int nrows =A->nrows,  one = 1, i, k, k1, len;
  int *ia = A->ia;
  double *aa = A->a;
  k = ia[0];
  for (i=0; i<nrows;i++){
    k1 = ia[i+1];
    len = k1-k;
    t = DASUM(&len,&aa[k],&one);
    ta = max(ta,t);
    k = k1;
  }
  return (ta);
}

void dcsrmv(char trans, int nrow, int ncol, double *a, 
    int *ia, int *ja, double *x, double *y) {
  int  len, jj=nrow;
  if (trans == 'N') {  
    //#pragma omp parallel for schedule(guided)
    double r ;
    /*for (i=0; i<nrow; i++) {
      r = 0.0;
      for (j=ia[i]; j<ia[i+1]; j++) {
      r += a[j] * x[ja[j]];
      }
      y[i] = r; 
      }
      */
    while (jj--) {
      len = *(ia+1) - *ia;
      ia++; 
      r = 0.0;
      while (len--)
        r += x[*ja++]*(*a++);
      *y++ = r;
    }
  } else {
    double xi; int jj, len;
    jj = nrow;
    //-------------------- this is from the matvec used in FILTLAN
    //                     column oriented - gains up to 15% in time 
    double *w = y;
    while (jj--)
      *w++ = 0.0;
    jj = nrow;
    //-------------------- column loop
    while (jj--){
      len = *(ia+1) - *ia;
      ia++;
      xi = *x++;
      while(len--)       
        y[*ja++] += xi*(*a++);
    }
  }
}

/**
* @brief y = A * x
*/
int matvec(csrMat *A, double *x, double *y) {
  /* if an external matvec routine is set, A will be ignored */
  if (evsldata.Amatvec.func) {
    (evsldata.Amatvec.func)(x, y, evsldata.Amatvec.data);
  } else {
    dcsrmv('N', A->nrows, A->ncols, A->a, A->ia, A->ja, x, y);
  }
  return 0;
}

