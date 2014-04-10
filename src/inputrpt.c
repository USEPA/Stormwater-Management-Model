//-----------------------------------------------------------------------------
//   inputrpt.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14 (Build 5.1.001)
//   Author:   L. Rossman
//
//   Report writing functions for input data summary.
//
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include <time.h>
#include "headers.h"
#include "lid.h"

#define WRITE(x) (report_writeLine((x)))

//=============================================================================

void inputrpt_writeInput()
//
//  Input:   none
//  Output:  none
//  Purpose: writes summary of input data to report file.
//
{
    int m;
    int i, k;
    int lidCount = 0;
    if ( ErrorCode ) return;

    WRITE("");
    WRITE("*************");
    WRITE("Element Count");
    WRITE("*************");
    fprintf(Frpt.file, "\n  Number of rain gages ...... %d", Nobjects[GAGE]);
    fprintf(Frpt.file, "\n  Number of subcatchments ... %d", Nobjects[SUBCATCH]);
    fprintf(Frpt.file, "\n  Number of nodes ........... %d", Nobjects[NODE]);
    fprintf(Frpt.file, "\n  Number of links ........... %d", Nobjects[LINK]);
    fprintf(Frpt.file, "\n  Number of pollutants ...... %d", Nobjects[POLLUT]);
    fprintf(Frpt.file, "\n  Number of land uses ....... %d", Nobjects[LANDUSE]);

    if ( Nobjects[POLLUT] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("*****************");
        WRITE("Pollutant Summary");
        WRITE("*****************");
        fprintf(Frpt.file,
    "\n                               Ppt.      GW         Kdecay");
        fprintf(Frpt.file,
    "\n  Name                 Units   Concen.   Concen.    1/days    CoPollutant");
        fprintf(Frpt.file,
    "\n  -----------------------------------------------------------------------");
        for (i = 0; i < Nobjects[POLLUT]; i++)
        {
            fprintf(Frpt.file, "\n  %-20s %5s%10.2f%10.2f%10.2f", Pollut[i].ID,
                QualUnitsWords[Pollut[i].units], Pollut[i].pptConcen,
                Pollut[i].gwConcen, Pollut[i].kDecay*SECperDAY);
            if ( Pollut[i].coPollut >= 0 )
                fprintf(Frpt.file, "    %-s  (%.2f)",
                    Pollut[Pollut[i].coPollut].ID, Pollut[i].coFraction);
        }
    }

    if ( Nobjects[LANDUSE] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("***************");
        WRITE("Landuse Summary");
        WRITE("***************");
        fprintf(Frpt.file,
    "\n                         Sweeping   Maximum      Last");
        fprintf(Frpt.file,
    "\n  Name                   Interval   Removal     Swept");
        fprintf(Frpt.file,
    "\n  ---------------------------------------------------");
        for (i=0; i<Nobjects[LANDUSE]; i++)
        {
            fprintf(Frpt.file, "\n  %-20s %10.2f%10.2f%10.2f", Landuse[i].ID,
                Landuse[i].sweepInterval, Landuse[i].sweepRemoval,
                Landuse[i].sweepDays0);
        }
    }

    if ( Nobjects[GAGE] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("****************");
        WRITE("Raingage Summary");
        WRITE("****************");
    fprintf(Frpt.file,
"\n                                                      Data       Recording");
    fprintf(Frpt.file,
"\n  Name                 Data Source                    Type       Interval ");
    fprintf(Frpt.file,
"\n  ------------------------------------------------------------------------");
        for (i = 0; i < Nobjects[GAGE]; i++)
        {
            if ( Gage[i].tSeries >= 0 )
            {
                fprintf(Frpt.file, "\n  %-20s %-30s ",
                    Gage[i].ID, Tseries[Gage[i].tSeries].ID);
                fprintf(Frpt.file, "%-10s %3d min.",
                    RainTypeWords[Gage[i].rainType],
                    (Gage[i].rainInterval)/60);
            }
            else fprintf(Frpt.file, "\n  %-20s %-30s",
                Gage[i].ID, Gage[i].fname);
        }
    }

    if ( Nobjects[SUBCATCH] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("********************");
        WRITE("Subcatchment Summary");
        WRITE("********************");
        fprintf(Frpt.file,
"\n  Name                       Area     Width   %%Imperv    %%Slope Rain Gage            Outlet              ");
        fprintf(Frpt.file,
"\n  -----------------------------------------------------------------------------------------------------------");
        for (i = 0; i < Nobjects[SUBCATCH]; i++)
        {
            fprintf(Frpt.file,"\n  %-20s %10.2f%10.2f%10.2f%10.4f %-20s ",
                Subcatch[i].ID, Subcatch[i].area*UCF(LANDAREA),
                Subcatch[i].width*UCF(LENGTH),  Subcatch[i].fracImperv*100.0,
                Subcatch[i].slope*100.0, Gage[Subcatch[i].gage].ID);
            if ( Subcatch[i].outNode >= 0 )
            {
                fprintf(Frpt.file, "%-20s", Node[Subcatch[i].outNode].ID);
            }
            else if ( Subcatch[i].outSubcatch >= 0 )
            {
                fprintf(Frpt.file, "%-20s", Subcatch[Subcatch[i].outSubcatch].ID);
            }
            if ( Subcatch[i].lidArea ) lidCount++;
        }
    }
    if ( lidCount > 0 ) lid_writeSummary();

    if ( Nobjects[NODE] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("************");
        WRITE("Node Summary");
        WRITE("************");
        fprintf(Frpt.file,
"\n                                           Invert      Max.    Ponded    External");
        fprintf(Frpt.file,
"\n  Name                 Type                 Elev.     Depth      Area    Inflow  ");
        fprintf(Frpt.file,
"\n  -------------------------------------------------------------------------------");
        for (i = 0; i < Nobjects[NODE]; i++)
        {
            fprintf(Frpt.file, "\n  %-20s %-16s%10.2f%10.2f%10.1f", Node[i].ID,
                NodeTypeWords[Node[i].type-JUNCTION],
                Node[i].invertElev*UCF(LENGTH),
                Node[i].fullDepth*UCF(LENGTH),
                Node[i].pondedArea*UCF(LENGTH)*UCF(LENGTH));
            if ( Node[i].extInflow || Node[i].dwfInflow || Node[i].rdiiInflow )
            {
                fprintf(Frpt.file, "    Yes");
            }
        }
    }

    if ( Nobjects[LINK] > 0 )
    {
        WRITE("");
        WRITE("");
        WRITE("************");
        WRITE("Link Summary");
        WRITE("************");
        fprintf(Frpt.file,
"\n  Name             From Node        To Node          Type            Length    %%Slope Roughness");
        fprintf(Frpt.file,
"\n  ---------------------------------------------------------------------------------------------");
        for (i = 0; i < Nobjects[LINK]; i++)
        {
            // --- list end nodes in their original orientation
            if ( Link[i].direction == 1 )
                fprintf(Frpt.file, "\n  %-16s %-16s %-16s ",
                    Link[i].ID, Node[Link[i].node1].ID, Node[Link[i].node2].ID);
            else
                fprintf(Frpt.file, "\n  %-16s %-16s %-16s ",
                    Link[i].ID, Node[Link[i].node2].ID, Node[Link[i].node1].ID);

            // --- list link type
            if ( Link[i].type == PUMP )
            {
                k = Link[i].subIndex;
                fprintf(Frpt.file, "%-5s PUMP  ",
                    PumpTypeWords[Pump[k].type]);
            }
            else fprintf(Frpt.file, "%-12s",
                LinkTypeWords[Link[i].type-CONDUIT]);

            // --- list length, slope and roughness for conduit links
            if (Link[i].type == CONDUIT)
            {
                k = Link[i].subIndex;
                fprintf(Frpt.file, "%10.1f%10.4f%10.4f",
                    Conduit[k].length*UCF(LENGTH),
                    Conduit[k].slope*100.0*Link[i].direction,
                    Conduit[k].roughness);
            }
        }

        WRITE("");
        WRITE("");
        WRITE("*********************");
        WRITE("Cross Section Summary");
        WRITE("*********************");
        fprintf(Frpt.file,
"\n                                        Full     Full     Hyd.     Max.   No. of     Full");
        fprintf(Frpt.file,    
"\n  Conduit          Shape               Depth     Area     Rad.    Width  Barrels     Flow");
        fprintf(Frpt.file,
"\n  ---------------------------------------------------------------------------------------");
        for (i = 0; i < Nobjects[LINK]; i++)
        {
            if (Link[i].type == CONDUIT)
            {
                k = Link[i].subIndex;
                fprintf(Frpt.file, "\n  %-16s ", Link[i].ID);
                if ( Link[i].xsect.type == CUSTOM )
                    fprintf(Frpt.file, "%-16s ", Curve[Link[i].xsect.transect].ID);
                else if ( Link[i].xsect.type == IRREGULAR )
                    fprintf(Frpt.file, "%-16s ",
                    Transect[Link[i].xsect.transect].ID);
                else fprintf(Frpt.file, "%-16s ",
                    XsectTypeWords[Link[i].xsect.type]);
                fprintf(Frpt.file, "%8.2f %8.2f %8.2f %8.2f      %3d %8.2f",
                    Link[i].xsect.yFull*UCF(LENGTH),
                    Link[i].xsect.aFull*UCF(LENGTH)*UCF(LENGTH),
                    Link[i].xsect.rFull*UCF(LENGTH),
                    Link[i].xsect.wMax*UCF(LENGTH),
                    Conduit[k].barrels,
                    Link[i].qFull*UCF(FLOW));
            }
        }
    }

    if (Nobjects[SHAPE] > 0)
    {
        WRITE("");
        WRITE("");
        WRITE("*************");
        WRITE("Shape Summary");
        WRITE("*************");
        for (i = 0; i < Nobjects[SHAPE]; i++)
        {
            k = Shape[i].curve;
            fprintf(Frpt.file, "\n\n  Shape %s", Curve[k].ID);
            fprintf(Frpt.file, "\n  Area:  ");
            for ( m = 1; m < N_SHAPE_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Shape[i].areaTbl[m]);
            }
            fprintf(Frpt.file, "\n  Hrad:  ");
            for ( m = 1; m < N_SHAPE_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Shape[i].hradTbl[m]);
            }
            fprintf(Frpt.file, "\n  Width: ");
            for ( m = 1; m < N_SHAPE_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Shape[i].widthTbl[m]);
            }
        }
    }
    WRITE("");

    if (Nobjects[TRANSECT] > 0)
    {
        WRITE("");
        WRITE("");
        WRITE("****************");
        WRITE("Transect Summary");
        WRITE("****************");
        for (i = 0; i < Nobjects[TRANSECT]; i++)
        {
            fprintf(Frpt.file, "\n\n  Transect %s", Transect[i].ID);
            fprintf(Frpt.file, "\n  Area:  ");
            for ( m = 1; m < N_TRANSECT_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Transect[i].areaTbl[m]);
            }
            fprintf(Frpt.file, "\n  Hrad:  ");
            for ( m = 1; m < N_TRANSECT_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Transect[i].hradTbl[m]);
            }
            fprintf(Frpt.file, "\n  Width: ");
            for ( m = 1; m < N_TRANSECT_TBL; m++)
            {
                 if ( m % 5 == 1 ) fprintf(Frpt.file,"\n          ");
                 fprintf(Frpt.file, "%10.4f ", Transect[i].widthTbl[m]);
            }
        }
    }
    WRITE("");
}
