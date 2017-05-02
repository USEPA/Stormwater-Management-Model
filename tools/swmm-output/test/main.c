/*
* main.c
*
*      Author: Colleen Barr
*      Modified by: Michael E. Tryby
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\src\outputAPI.h"

int testGetSubcatchResult(char* path)
{
	int error = 0;
	long length;
	float* array;

	SMOutputAPI* smoapi = SMO_init();
	error = SMO_open(smoapi, path);

	array = SMO_newOutValueArray(smoapi, getResult, subcatch, &length, &error);
	error = SMO_getSubcatchResult(smoapi, 1, 0, array);

	if (!error)
	{
		for (int i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	SMO_free(array);
	error = SMO_close(smoapi);

	return error;
}

int main(int argc, char* argv[])
{
	int length, error = 0;
	char path[MAXFILENAME] = "C:\\Users\\mtryby\\Workspace\\GitRepo\\Local\\swmm-testsuite\\Benchmarks\\v517\\Example1\\Example1.out";
    char name[MAXELENAME];
	SMOutputAPI* smoapi = SMO_init();

	error = SMO_open(smoapi, path);

	length = MAXELENAME;
	SMO_getElementName(smoapi, subcatch, 0, name, &length);

	length = MAXELENAME;
	SMO_getElementName(smoapi, sys, 0, name, &length);

	testGetSubcatchResult(path);

	return error;

}
