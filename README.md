   Version 1.0  Latest changes made on: Thu Jan 26 15:24:02 CST 2017
  -----------------------------------------------------------------------

  ███████╗           ██╗   ██╗           ███████╗           ██╗     
  ██╔════╝           ██║   ██║           ██╔════╝           ██║     
  █████╗             ██║   ██║           ███████╗           ██║     
  ██╔══╝             ╚██╗ ██╔╝           ╚════██║           ██║     
  ███████╗    ██╗     ╚████╔╝     ██╗    ███████║    ██╗    ███████╗
  ╚══════╝    ╚═╝      ╚═══╝      ╚═╝    ╚══════╝    ╚═╝    ╚══════╝

  -----------------------------------------------------------------------
    EVSL:  ChebLanTR, ChebLanNR,  ChebSI, RatLanTr, and RatLanNr  
 
  Polynomial  and   Rational  Filtered  Lanczos   and  subspace
  iteration algorithms For Symmetric Eigenvalue problems
    
  Welcome to EVSL.  EVSL is a C library for  computing the eigenvalues
  of a  symmetric matrix that are  located in a given  interval.  This
  first release  includes the routines  listed above and does  not yet
  offer full  parallel implementations  (trivial openMP  test programs
  are available among the test drivers).  EVSL also provides tools for
  spectrum  slicing,  i.e.,  the  technique  of  subdividing  a  given
  interval into  p smaller subintervals and  computing the eigenvalues
  in  each subinterval  independently.  EVSL  implements a  polynomial
  filtered  Lanczos (thick  restart, no  restart) a  rational filtered
  Lanczos  (thick  restart, no  restart),  and  a polynomial  filtered
  subspace iteration.
  
  For questions/feedback send e-mail to Yousef Saad [saad@umn.edu]
  
  -----------------------------------------------------------------------
      
  Description of contents:
  =======================
  
   *  SRC/
       * chelanTr.c    :  Polynomial Filtered thick restart Lanczos
  
       * chelanNr.c    :  Polynomial Filtered no-restart Lanczos
  
       * chebsi.c      :  Polynomial Filtered  Subspace iteration
  
       * ratlanTr.c    :  Rational Filtered thick restart Lanczos
  
       * ratlanNr.c    :  Rational Filtered no-restart Lanczos
 
       * misc_la.c       : miscellaneous linear algebra functions
                         * SymmTridEig()  wrapper to lapack's DSTEV
                         * SymmTridEigS() wrapper to lapack's DSTEMR
                         * SymEigenSolver() wrapper to lapack's DSYEV
                         * CGS_DGKS() : gram-schmidt
 
       * chebpoly.c   :
             *  chebxPltd()    : given a polynomial and vector x,
                                 compute polynomial values y = p(x)
             *  set_pol_def()  : set default values for polynomial filters
             *  dampcf()       : damping coefficient for cheb. expansions
             *  find_pol()     : find the *best* Cheb polynomial
             *  chext()        : determine polynomial for end intervals
             *  rootchb()      : balance the polynomial
             *  ChebAv()       : perform p(A) vector product: w= p(A)*v
             *  ChebAv0()      : same thing but calls matvec 
 
       * ratfilter.c   :

           * contQuad          : given the number of poles and quadrature rule
                                 compute the locations of poles on [-1 1]
           * rat2p2()          : given a rational funct. and x, 
                                 compute rational funct. value by y = r(x)
           * pfe2()            : partial fraction expansion of rat. functions
           * integg2()         : integration of 1/[(x-s1)^k1 (x-s2)^k2] 
           * weights()         : expansion coefficients of rat. filter on [-1 1] 
           * scaleweigthts()   : expansion coefficients of rat. filter on [a b]
           * set_rat_def()     : set default values for rat. filter
           * find_rat()        : find the "best" rational filter
 
       * spslice.c    :
           * kpmdos()      : kernel polynomial method for determining 
                              the dos of A
           * spslicer()    : given the dos spslicer will divide the spectrum 
                              into slices having approximately the same 
                              number of eigenvalues

       * lanbounds.c   :
           * LanBounds()    : Lanczos alg. to give a bound of spectrum
 
       * spmat.c       :  [CSR matrix routines]
           * csrcsc()          : convert CSR to CSC [used by sortrow]
           * sortrow()         : sort rows of CSR by column indices
           * csr_resize()      : memory alloc. for CSR
           * free_csr()        : memory dealloc for CSR
           * free_coo()        : memory dealloc for COO
           * cooMat_to_csrMat() : convert COO to CSR
           * matvec_gen()      : general form of matvec: y = alp*A*x + bet*y
           * matvec()          : y = A*x
           * lapgen()          : generate Laplacian on (nx)x(ny)x(nz) grid
 
       * timing.c      :
           * cheblan_timer    : timer
 
       * mactime.c     :
           * cheblan_timer for mac os    
 
       * vect.c        :  [vector operations]
           * rand_double    : produces a  random vector
           * sort_double    : sort vector by values [ascending order]
      
       * liblancheb.a  : library
  
   * TESTS_Gen/ and  TESTS_Lap/ : 
       directories containing  test drivers for
       simple model Laplacean matrices  (TESTS_Lap) or general matrices in
       sparse format read from a file (TESTS_Gen).. See below and 00README
       files therein for further information
  
  
  ---------------------------------------------------------------------------
    INSTALLATION
  ---------------------------------------------------------------------------
  
    Library:
       The users only need to modify the file makefile.in 
       [see makefile.in.example for samples of files makefile.in 
       that are given for mac-os and for Linux].
        
       1. cp makefile.in_Linux/MacOS.example makefile.in. 
       2. modify makefile.in [provide BLAS/LAPACK path]
       3. make clean; make
  
       make in the directory SRC will create the library LIB/liblancheb.a
        
    Test programs:
        In directories TESTS_Lap and TESTS_Gen you will find makefiles to 
        run sample drivers that test a few different situations.
  
        TESTS_Lap/ -- generate Laplacean matrices and compute eigenvalues in
                      given intervals. You will find 6 drivers

             * LapPLanN.c --> poly. filtering non-restarted Lanczos 
             * LapPLanR.c --> poly. filtering thick-restart Lanczos
             * LapPSI.c   --> poly. filtering subspace iteration
             * LapRLanN.c --> rational filtering non-restart Lanczos
             * LapRLanR.c --> rational filtering thick-restart Lanczos

      	     * lapl.c     --> functions to set up a laplacean matrix and also to 
                            compute the exact eigenvalues of Laplacians
  
             see details in the 00README file in the directory TESTS_Lap/
  
        TESTS_Gen/ -- read a matrix from file and compute eigenvalues in
                      given intervals. Here you will find 6 drivers
             *  GenPLanN.c --> poly. filtering non-restarted Lanczos 
             *  GenPLanR.c --> poly. filtering thick-restart Lanczos
             *  GenPSI.c   --> poly. filtering subspace iteration
             *  GenPLanR_omp.c --> poly. filtering thick-restart Lanczos with openMP
             *  GenRLanN.c --> rational filtering non-restart Lanczos
             *  GenRLanR.c --> rational filtering thick-restart Lanczos
      
             see details in the 00README file in the directory TESTS_Gen/

  -----------------------------------------------------------------------
    RATIONAL FILTERING
  -----------------------------------------------------------------------
  
    Rational filtering  requires solving  linear systems  (where the
    coefficient matrix is the original matrix shifted by a *complex*
    shift).  A linear solver routine  must be provided by the users.
    In the test programs  (Lap/GenRLan*.c), we provide examples that
    use UMFPACK a sparse direct solver package by T. Davis. (version
    4.5.3 has been tested)
  
    ======================================================================

    NOTE:  SuiteSparse is NOT distributed with EVSL, and is Copyrighted (c)
      by Timothy Davis.  Please refer to that package for its License.
            [http://faculty.cse.tamu.edu/davis/suitesparse.html]

    ======================================================================
  
    To run rational filtering Lanczos methods, users will only need to
    provide a solver function of the following prototype:
    
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

          void solvefunc(int n, double *br, double *bz, double *xr, 
  	                          double *xz, void *data);

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  
    where n  is the size  of the system,  br, bz are  the right-hand
    side (real and  imaginary parts of complex vector),  xr, xz will
    be the  solution (complex vector),  and "data" contains  all the
    data  needed  for  the  solver.    Then,  users  will  create  a
    ``solveShift'' structure that includes  the function pointer and
    the data, that  will be passed to evsl:
    
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
      
            typedef struct _solveShift {
              solveShiftFunc func;       /* function pointer to the solver routine */
              void **data;               /* = malloc(num_linsys*sizeof(void*)); */
            } solveShift;
      
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
    For example, when using UMFPACK, first define struct umfdata for umfpack 
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

          typedef struct _umfdata {
            SuiteSparse_long *Ap;
            SuiteSparse_long *Ai;
            double *Ax;
            double *Az;
            void *Numeric;
          } umfdata;

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    If the number of poles used is num, define an array umf:
    
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

          umfdata *umf = (umfdata *) malloc(num*sizeof(umfdata));

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    and pass ith shifted matrix and its LU factors information to umf[i]

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            umf[i].Ap = Ap;     umf[i].Ai = Ai;  umf[i].Ax = Ax[i];  
            umf[i].Az = Az[i];  umf[i].Numeric = Numeric[i];

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    then solshift will be 

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

             solshift.func = &solvefunc;  //* function pointer
             solshift.data = malloc(num*sizeof(void *));
             for (i=0; i<num; i++) {
               solshift.data[i] = &umf[i]; 
             }

    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    and solvefunc can be written as
    - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

             void solvefunc(int n, double *br, double *bz, double *xr, double *xz, void *data) {
               umfdata *umf = (umfdata *) data;
               SuiteSparse_long* Ap = umf->Ap;
               SuiteSparse_long* Ai = umf->Ai;
               double* Ax = umf->Ax;
               double* Az = umf->Az;  
               void* Numeric = umf->Numeric;
               double Control[UMFPACK_CONTROL]; 
               umfpack_zl_defaults(Control);
               Control[UMFPACK_IRSTEP] = 0; // no iterative refinement for umfpack 
               umfpack_zl_solve(UMFPACK_A, Ap, Ai, Ax, Az, xr, xz, br, bz, Numeric, Control, NULL); 
              }

     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  
  -----------------------------------------------------------------------
    LINKING  WITH  UMFPACK (SuiteSparse        4.5.3)
  -----------------------------------------------------------------------
    The  makefiles   in  TESTS_Lap   and  TEST_Gen,  include   paths  to
    SuiteSparse 4.5.3.   UMFPACK requires AMD, CHOLMOD,  COLAMD, CCOLAMD
    and  CAMD,  and  optionally  METIS 5.1.0.   Compile  each  of  these
    packages  to  have  the  library  file in  the  Lib  directory.   If
    SuiteSparse  is configured  with METIS,  give the  path to  METIS (v
    5.1.0)  as  well   (to  make  libmetis.a,  type   `make  config'  in
    metis-5.1.0/ and then `make') Please  refer to SuiteSparse and METIS
    for installation details.
 
  FILES:
  ------
   *  INC/
 
       * evsl.h : function  prototypes and constant definitions
    
       * blaslapack.h : C API for BLAS/LAPACK functions used in evsl
        
       * def.h            : miscellaneous macros 
        
       * struct.h         : miscellaneous structs used in evsl
                           COO, CSR   : sparse matrix format
                           polparams  : polynomial filter
                           rat        : rational filter
                           solveShift : direct solver interface
      
       * cheblan.h        : header files for ChebLan* functions
       
       * internal_proto.h : internal function prototypes for SRC/
      
     _Note_
   
     These pages are meant to be read through [Doxygen](https://www.doxygen.com)
   
