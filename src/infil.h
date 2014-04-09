//-----------------------------------------------------------------------------
//   infil.h
//
//   Project: EPA SWMM5
//   Version: 5.0
//   Date:    07/30/10   (Build 5.0.019)
//            04/20/11   (Build 5.0.022)
//   Author:  L. Rossman (US EPA)
//
//   Public interface for infiltration functions.
//-----------------------------------------------------------------------------

#ifndef INFIL_H
#define INFIL_H

//---------------------
// Enumerated Constants
//---------------------
enum InfilType {
     HORTON,                      // Horton infiltration
     GREEN_AMPT,                  // Green-Ampt infiltration
     CURVE_NUMBER};               // SCS Curve Number infiltration

//---------------------
// Horton Infiltration
//---------------------
typedef struct
{
   double        fmin;            // minimum infil. rate (ft/sec)
   double        Fmax;            // maximum total infiltration (ft);
   double        decay;           // decay coeff. of infil. rate (1/sec)
   double        regen;           // regeneration coeff. of infil. rate (1/sec)
   //-----------------------------
   double        tp;              // present time on infiltration curve (sec)
   double        f0;              // initial infil. rate (ft/sec)
}  THorton;


//-------------------------
// Green-Ampt Infiltration
//-------------------------
typedef struct
{
   double        S;               // avg. capillary suction (ft)
   double        Ks;              // saturated conductivity (ft/sec)
   double        IMDmax;          // max. soil moisture deficit (ft/ft)
   //-----------------------------
   double        IMD;             // current soil moisture deficit
   double        F;               // current cumulative infiltration (ft)
   double        T;               // time needed to drain upper zone (sec)
   double        L;               // depth of upper soil zone (ft)
   double        FU;              // current moisture content of upper zone (ft)
   double        FUmax;           // saturated moisture content of upper zone (ft)
   char          Sat;             // saturation flag
}  TGrnAmpt;


//--------------------------
// Curve Number Infiltration
//--------------------------
typedef struct
{
   double        Smax;            // max. infiltration capacity (ft)
   double        regen;           // infil. capacity regeneration constant (1/sec)
   double        Tmax;            // maximum inter-event time (sec)
   //-----------------------------
   double        S;               // current infiltration capacity (ft)
   double        F;               // current cumulative infiltration (ft)
   double        P;               // current cumulative precipitation (ft)
   double        T;               // current inter-event time (sec)
   double        Se;              // current event infiltration capacity (ft)
   double        f;               // previous infiltration rate (ft/sec)

}  TCurveNum;

//-----------------------------------------------------------------------------
//   Exported Variables
//-----------------------------------------------------------------------------
extern THorton*   HortInfil;
extern TGrnAmpt*  GAInfil;
extern TCurveNum* CNInfil;

//-----------------------------------------------------------------------------
//   Infiltration Methods
//-----------------------------------------------------------------------------
void    infil_create(int subcatchCount, int model);
void    infil_delete(void);
int     infil_readParams(int model, char* tok[], int ntoks);
void    infil_initState(int area, int model);
double  infil_getInfil(int area, int model, double tstep, double rainfall,
        double runon, double depth);                                           //(5.0.022 - LR)

int     grnampt_setParams(TGrnAmpt *infil, double p[]);
void    grnampt_initState(TGrnAmpt *infil);
double  grnampt_getInfil(TGrnAmpt *infil, double tstep, double irate,
        double depth);

#endif
