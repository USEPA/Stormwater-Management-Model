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
#include "..\src\outputapi.h"

int testGetSubcatchResult(char* path)
{
	int i, length, error = 0;
	float* array = NULL;

	SMO_Handle p_handle = NULL;
	SMO_init(&p_handle);

	error = SMO_open(p_handle, path);
	error = SMO_getSubcatchResult(p_handle, 1, 0, &array, &length);

	if (!error)
	{
		for (i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	SMO_free((void*)array);
	error = SMO_close(&p_handle);

	return error;
}

int main(int argc, char* argv[])
{
	int length, error = 0;
	char path[MAXFILENAME] = "C:\\Users\\mtryby\\Workspace\\GitRepo\\michaeltryby\\Stormwater-Management-Model\\tests\\swmm-nrtestsuite\\benchmarks\\v517\\Example_1\\Example1.out";
    char name[MAXELENAME];

	SMO_Handle p_handle = NULL;
	SMO_init(&p_handle);

	error = SMO_open(p_handle, path);

	length = MAXELENAME;
	SMO_getElementName(p_handle, subcatch, 0, name, length);

	length = MAXELENAME;
	SMO_getElementName(p_handle, sys, 0, name, length);

	testGetSubcatchResult(path);

	return error;

}
