//-----------------------------------------------------------------------------
//   main.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     05/10/2018
//   Author:   L. Rossman

//   Main stub for the command line version of EPA SWMM 5.1
//   to be run with swmm5.dll.

// System includes
#include <stdio.h>
#include <string.h>
#include <math.h>

// Private project includes
#include "timer.h"
#include "shared/csio_helper.h"

// Public project includes
#include "swmm5.h"


#define BAR_LEN 50l
#define MSG_LEN 84


static long Start;


void write_console(char *msg)
{
    csio_fprintf(stdout, "%s", msg);
    fflush(stdout);
}

void progress_bar(double *ratio)
{
    char bar[BAR_LEN + 1] = {'\0'};

    // Create progress bar
    long prog_len = lround(*ratio * BAR_LEN);
    memset(bar, ' ', (size_t)BAR_LEN);
    if (prog_len < BAR_LEN)
        memset(bar, '>', (size_t)(prog_len + 1));
    memset(bar, '=', (size_t)prog_len);

    // Compute pct complete
    double pct = *ratio * 100.0;

    // Compute time remaining using naive approach
    long t_r = 0;
    if (*ratio > 0.0)
        t_r = lround((1.0 - *ratio) * (current_time_millis() - Start) / *ratio);

    // Format and print progress
    char msg[MSG_LEN + 1];
    char tmp[TIMER_LEN + 1];

    size_t n = (size_t)(MSG_LEN + 1);

    write_console("\r");
    csio_snprintf(msg, n, "... Running [%50s] %5.1f%% [%8s]", bar, pct,
        format_time(tmp, t_r));
    write_console(msg);
}


int  main(int argc, char *argv[])
//
//  Input:   argc = number of command line arguments
//           argv = array of command line arguments
//  Output:  returns error status
//  Purpose: runs the command line version of EPA SWMM 5.1.
//
//  Command line is: swmm5 f1  f2  f3
//  where f1 = name of input file, f2 = name of report file, and
//  f3 = name of binary output file if saved (or blank if not saved).
//
{
    char *inputFile;
    char *reportFile;
    char *binaryFile;
    char *arg1;
    char blank[] = "";
    char time[8];

    int  version, vMajor, vMinor, vRelease;

    char errMsg[128];
    int  msgLen = 127;

    long stop;


    // --- check for proper number of command line arguments
    if (argc == 4)
    {
        // --- extract file names from command line arguments
        inputFile = argv[1];
        reportFile = argv[2];
        if (argc > 3)
            binaryFile = argv[3];
        else
            binaryFile = blank;

        Start = current_time_millis();
        // --- run SWMM
        swmm_run(inputFile, reportFile, binaryFile, &progress_bar);

        stop = current_time_millis();

        printf("\n\n... EPA-SWMM completed in %s", format_time(time, stop - Start));

        if ( swmm_getError(errMsg, msgLen) > 0 )
            printf(" with errors.\n");

        else if ( swmm_getWarnings() > 0 )
            printf(" with warnings.\n");

        else
            printf(" successfully.\n");
    }

    else if (argc == 2)
    {
        // --- extract first argument
        arg1 = argv[1];

        if (strcmp(arg1, "--help") == 0 || strcmp(arg1, "-h") == 0)
        {
            // Help
            printf("\n\nEPA Stormwater Management Model (SWMM5) Help\n\n");
            printf("Commands:\n");
            printf("\t--help (-h)       Help Docs\n");
            printf("\t--version (-v)    Build Version\n");
            printf("\nUsage:\n");
            printf("\t swmm5 <input file> <report file> <output file>\n\n");
        }

        else if (strcmp(arg1, "--version") == 0 || strcmp(arg1, "-v") == 0)
        {
            version = swmm_getVersion();
            vMajor = version / 10000;
            vMinor = (version - 10000 * vMajor) / 1000;
            vRelease = (version - 10000 * vMajor - 1000 * vMinor);

            // Output version number
            printf("EPA-SWMM version %d.%d.%0d\n\n", vMajor, vMinor, vRelease);
        }

        else
            printf("\nUnknown Argument (See Help --help)\n\n");
    }

    else
    {
        printf("\nUsage:\n");
        printf("\t swmm5 <input file> <report file> <output file>\n\n");
    }

// --- Use the code below if you need to keep the console window visible
/*
    printf("    Press Enter to continue...");
    getchar();
*/

    return 0;
}
