//-----------------------------------------------------------------------------
//   findroot.c
//
//   Finds solution of func(x) = 0 using either the Newton-Raphson
//   method or Ridder's Method.
//   Based on code from Numerical Recipes in C (Cambridge University
//   Press, 1992).
//
//   Date:     11/19/13
//   Author:   L. Rossman
//-----------------------------------------------------------------------------

#include <math.h>
#include "findroot.h"

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define MAXIT 60


int findroot_Newton(double x1, double x2, double* rts, double xacc,
                    void (*func) (double x, double* f, double* df, void* p),
					void* p)
//
//  Using a combination of Newton-Raphson and bisection, find the root of a
//  function func bracketed between x1 and x2. The root, returned in rts,
//  will be refined until its accuracy is known within +/-xacc. func is a
//  user-supplied routine, that returns both the function value and the first
//  derivative of the function. p is a pointer to any auxilary data structure
//  that func may require. It can be NULL if not needed. The function returns
//  the number of function evaluations used or 0 if the maximum allowed
//  iterations were exceeded.
//
// NOTES:
// 1. The calling program must insure that the signs of func(x1) and func(x2)
//    are not the same, otherwise x1 and x2 do not bracket the root.
// 2. If func(x1) > func(x2) then the order of x1 and x2 should be
//    switched in the call to Newton.
//
{
    int j, n = 0;
    double df, dx, dxold, f, x;
    double temp, xhi, xlo;

    // Initialize the "stepsize before last" and the last step.
    x = *rts;
    xlo = x1;
    xhi = x2;
    dxold = fabs(x2-x1);
    dx = dxold;
    func(x, &f, &df, p);
    n++;

    // Loop over allowed iterations.
    for (j=1; j<=MAXIT; j++)
    {
        // Bisect if Newton out of range or not decreasing fast enough.
        if ( ( ( (x-xhi)*df-f)*((x-xlo)*df-f) >= 0.0
        || (fabs(2.0*f) > fabs(dxold*df) ) ) )
        {
            dxold = dx;
            dx = 0.5*(xhi-xlo);
            x = xlo + dx;
            if ( xlo == x ) break;
        }

        // Newton step acceptable. Take it.
        else
        {
            dxold = dx;
            dx = f/df;
            temp = x;
            x -= dx;
            if ( temp == x ) break;
        }

        // Convergence criterion.
        if ( fabs(dx) < xacc ) break;
 
        // Evaluate function. Maintain bracket on the root.
        func(x, &f, &df, p);
        n++;
        if ( f < 0.0 ) xlo = x;
        else           xhi = x;
    }
    *rts = x;
    if ( n <= MAXIT) return n;
    else return 0;
};


double findroot_Ridder(double x1, double x2, double xacc,
	double (*func)(double, void* p), void* p)
{
    int j;
    double ans, fhi, flo, fm, fnew, s, xhi, xlo, xm, xnew;

    flo = func(x1, p);
    fhi = func(x2, p);
    if ( flo == 0.0 ) return x1;
    if ( fhi == 0.0 ) return x2;
    ans = 0.5*(x1+x2);
    if ( (flo > 0.0 && fhi < 0.0) || (flo < 0.0 && fhi > 0.0) )
    {
        xlo = x1;
        xhi = x2;
        for (j=1; j<=MAXIT; j++) {
            xm = 0.5*(xlo + xhi);
            fm = func(xm, p);
            s = sqrt( fm*fm - flo*fhi );
            if (s == 0.0) return ans;
            xnew = xm + (xm-xlo)*( (flo >= fhi ? 1.0 : -1.0)*fm/s );
            if ( fabs(xnew - ans) <= xacc ) break;
            ans = xnew;
            fnew = func(ans, p);
            if ( SIGN(fm, fnew) != fm)
            {
                xlo = xm;
                flo = fm;
                xhi = ans;
                fhi = fnew;
            }
            else if ( SIGN(flo, fnew) != flo )
            {
                xhi = ans;
                fhi = fnew;
            }
            else if ( SIGN(fhi, fnew) != fhi)
            {
                xlo = ans;
                flo = fnew;
            }
            else return ans;
            if ( fabs(xhi - xlo) <= xacc ) return ans;
        }
        return ans;
    }
    return -1.e20;
}
