//-----------------------------------------------------------------------------
//   findroot.h
//
//   Header file for root finding method contained in findroot.c
//
//   Last modified on 11/19/13.
//-----------------------------------------------------------------------------
int findroot_Newton(double x1, double x2, double* rts, double xacc,
                    void (*func) (double x, double* f, double* df, void* p),
					void* p);
double findroot_Ridder(double x1, double x2, double xacc,
	                   double (*func)(double, void* p), void* p);
