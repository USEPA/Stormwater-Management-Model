/*
  *  main.c - Main stub for the command line version of EPA SWMM 5.1
  *
  *  Created on: October 9, 2020
  *  Updated on:
  *
  *  Author:      See CONTRIBUTORS
  */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

// Private project includes
#include "timer.h"

// Public project includes
#include "swmm5.h"
#include "toolkit.h"


#define BAR_LEN 50l
#define MSG_LEN 84

#define FMT_USAGE \
"\nUsage:\n \
 \trunswmm <input file> <report file> <output file>\n\n"

#define FMT_HELP \
"\n\nOWA Stormwater Management Model (SWMM5) Help\n\n \
 Commands:\n \
 \t--help (-h)       Help Docs\n \
 \t--version (-v)    Build Version\n \
 \nUsage:\n \
 \t swmm5 <input file> <report file> <output file>\n\n"


static long Start;


void write_console(char *msg)
{
    fprintf(stdout, "%s", msg);
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

    // Compute percent complete
    double pct = *ratio * 100.0;

    // Compute time remaining using naive approach
    long t_r = 0;
    if (*ratio > 0.0)
        t_r = lround((1.0 - *ratio) * (current_time_millis() - Start) / *ratio);

    // Format and print progress
    char msg[MSG_LEN + 1] = {'\0'};
    char tmp[TIMER_LEN + 1] = {'\0'};

    write_console("\r");
    snprintf(msg, (size_t)(MSG_LEN + 1), "... Running [%50s] %5.1f%% [%8s]", bar,
        pct, format_time(tmp, t_r));
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
    if (argc == 4) {
        char *inputFile = argv[1];
        char *reportFile = argv[2];

        char *binaryFile = "";
        if (argc > 3)
            binaryFile = argv[3];

        Start = current_time_millis();
        swmm_run_cb(inputFile, reportFile, binaryFile, &progress_bar);

        long stop = current_time_millis();
        char time[TIMER_LEN + 1] = {'\0'};

        printf("\n\n... SWMM completed in %s", format_time(time, stop - Start));

        char errMsg[128];
        int  msgLen = 127;

        if ( swmm_getError(errMsg, msgLen) > 0 )
            printf(" with errors.\n");

        else if ( swmm_getWarnings() > 0 )
            printf(" with warnings.\n");

        else
            printf(" successfully.\n");
    }

    else if (argc == 2) {
        char *arg1 = argv[1];

        if (strcmp(arg1, "--help") == 0 || strcmp(arg1, "-h") == 0)
            printf(FMT_HELP);

        else if (strcmp(arg1, "--version") == 0 || strcmp(arg1, "-v") == 0) {
            printf("\nVersion:\n");
            printf("\tOWA-SWMM v%s (Build %.10s)\n\n", swmm_getSemVersion(), 
                swmm_getBuildId());
        }

        else {
            printf("\nError:\n");
            printf("\tUnknown Argument (See Help --help)\n\n");
        }
    }

    else {
        printf(FMT_USAGE);
    }

    return 0;
}