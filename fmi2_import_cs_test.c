/*
    Copyright (C) 2012 Modelon AB

    This program is free software: you can redistribute it and/or modify
    it under the terms of the BSD style license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    FMILIB_License.txt file for more details.

    You should have received a copy of the FMILIB_License.txt file
    along with this program. If not, contact Modelon AB <http://www.modelon.com>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "config_test.h"

#include <fmilib.h>
#include <JM/jm_portability.h>


#define BUFFER 1000

void importlogger(jm_callbacks* c, jm_string module, jm_log_level_enu_t log_level, jm_string message)
{
        printf("module = %s, log level = %s: %s\n", module, jm_log_level_to_string(log_level), message);
}

/* Logger function used by the FMU internally */

void fmilogger(fmi2_component_t c, fmi2_string_t instanceName, fmi2_status_t status, fmi2_string_t category, fmi2_string_t message, ...)
{
    /* int len;
	char msg[BUFFER]; */
	va_list argp;	
	va_start(argp, message);
	/* len = jm_vsnprintf(msg, BUFFER, message, argp); */
	fmi2_log_forwarding_v(c, instanceName, status, category, message, argp);
	va_end(argp);
}

void do_exit(int code)
{
	printf("Press 'Enter' to exit\n");
	/* getchar(); */
	exit(code);
}

int test_simulate_cs(fmi2_import_t* fmu)
{
	fmi2_status_t fmistatus;
	jm_status_enu_t jmstatus;

	fmi2_string_t instanceName = "Test CS model instance";
	fmi2_string_t fmuGUID;
	fmi2_string_t fmuLocation = "";
	fmi2_boolean_t visible = fmi2_false;
	fmi2_real_t relativeTol = 1e-4;
/*	fmi2_boolean_t loggingOn = fmi2_true; */
	
	/* fmi2_real_t simulation_results[] = {-0.001878, -1.722275}; */
	fmi2_real_t simulation_results[] = {0.0143633,   -1.62417};
	//fmi2_value_reference_t compare_real_variables_vr[] = {0, 1};
	size_t k;
	int i;

	fmi2_real_t tstart = 0.0;
	fmi2_real_t tcur = tstart;
	fmi2_real_t hstep = 0.001;
	fmi2_real_t tend = 2.0;
	fmi2_boolean_t StopTimeDefined = fmi2_false;

#if 0
	if (sizeof(compare_real_variables_vr)/sizeof(fmi2_value_reference_t) != sizeof(simulation_results)/sizeof(fmi2_real_t)) {
		printf("Number of simulation values and reference values are different\n");
		do_exit(CTEST_RETURN_FAIL);
	}
#endif

	printf("Version returned from FMU:   %s\n", fmi2_import_get_version(fmu));
	printf("Platform type returned:      %s\n", fmi2_import_get_types_platform(fmu));

	fmuGUID = fmi2_import_get_GUID(fmu);
    printf("GUID:      %s\n", fmuGUID);


    jmstatus = fmi2_import_instantiate(fmu, instanceName, fmi2_cosimulation, /*fmuLocation*/NULL, visible);
	if (jmstatus == jm_status_error) {
		printf("fmi2_import_instantiate failed\n");
		do_exit(CTEST_RETURN_FAIL);
	}

        fmistatus = fmi2_import_setup_experiment(fmu, fmi2_true,
            relativeTol, tstart, StopTimeDefined, tend);
    if(fmistatus != fmi2_status_ok) {
        printf("fmi2_import_setup_experiment failed\n");
        do_exit(CTEST_RETURN_FAIL);
    }

        fmistatus = fmi2_import_enter_initialization_mode(fmu);
    if(fmistatus != fmi2_status_ok) {
        printf("fmi2_import_enter_initialization_mode failed\n");
        do_exit(CTEST_RETURN_FAIL);
    }
#if 0
	fmi2_import_variable_list_t* vl = fmi2_import_get_variable_list(fmu, 1);
	size_t nv = fmi2_import_get_variable_list_size(vl);

	for( i = 0; i < nv; i++ ){
		printf("==============================\n");
		fmi2_import_variable_t* var = fmi2_import_get_variable(vl, i);
		size_t vr = fmi2_import_get_variable_vr(var);
		printf("Variable name: %s\n", fmi2_import_get_variable_name(var));
		printf("Description: %s\n", fmi2_import_get_variable_description(var));
		printf("VR: %u\n", (unsigned)vr);
		printf("Variability: %s : %d \n", fmi2_variability_to_string(fmi2_import_get_variability(var)), fmi2_import_get_variability(var));
		printf("Causality: %s : %d \n", fmi2_causality_to_string(fmi2_import_get_causality(var)), fmi2_import_get_causality(var));
		printf("Initial: %s : %d \n", fmi2_initial_to_string(fmi2_import_get_initial(var)), fmi2_import_get_initial(var));
		printf("base_type: %s : %d \n", fmi2_base_type_to_string(fmi2_import_get_variable_base_type(var)), fmi2_import_get_variable_base_type(var));
	}
	printf("==============================\n");
#endif	
	const char *szCalibVarName[] = {
		"ramp.duration",
		"ramp.startTime",
	};
	fmi2_import_variable_t *calibVarHandle[sizeof(szCalibVarName) / sizeof(szCalibVarName[0])];
	fmi2_value_reference_t calibVrTmp[sizeof(szCalibVarName) / sizeof(szCalibVarName[0])];

	for (i = 0; i < sizeof(szCalibVarName) / sizeof(szCalibVarName[0]); i++) {
		calibVarHandle[i] = fmi2_import_get_variable_by_name(fmu, szCalibVarName[i]);
		if (calibVarHandle[i] == NULL) {
			printf("fmi2_import_get_variable_by_name failed\n");
			do_exit(CTEST_RETURN_FAIL);
		}
		calibVrTmp[i] = fmi2_import_get_variable_vr(calibVarHandle[i]);
		printf("%s:valueReference is %d\n", szCalibVarName[i], calibVrTmp[i]);
	}


	fmi2_real_t iniTmp[sizeof(calibVrTmp) / sizeof(calibVrTmp[0])];
	fmistatus = fmi2_import_get_real(fmu, calibVrTmp, sizeof(calibVrTmp) / sizeof(calibVrTmp[0]), iniTmp);
	if (fmistatus != fmi2_status_ok) {
		printf("fmi2_import_set_real failed.err code=%d\n", fmistatus);
		do_exit(CTEST_RETURN_FAIL);
	}

	printf("Initial Parameters.\n");
	for (k = 0; k < sizeof(calibVrTmp) / sizeof(calibVrTmp[0]); k++) {
		printf("vr:%d,iniTmp is %lf\n", calibVrTmp[k], iniTmp[k]);
	}

	iniTmp[0] = 1.5;
	iniTmp[1] = 0.3;
	//iniTmp[2] = -9.81*1.1;

	fmistatus = fmi2_import_set_real(fmu, calibVrTmp, sizeof(calibVrTmp) / sizeof(calibVrTmp[0]), iniTmp);
	if (fmistatus != fmi2_status_ok) {
		printf("fmi2_import_set_real failed.err code=%d\n", fmistatus);
		do_exit(CTEST_RETURN_FAIL);
	}
	printf("Modified parameters.\n");
	for (k = 0; k < sizeof(calibVrTmp) / sizeof(calibVrTmp[0]); k++) {
		printf("vr:%d,iniTmp is %lf\n", calibVrTmp[k], iniTmp[k]);
	}

	// 
	const char *szVarName[] = {
		"target",
		"voltage",
		"current",
		"speed",
	};
	fmi2_import_variable_t *varHandle[sizeof(szVarName) / sizeof(szVarName[0])];
	fmi2_value_reference_t compare_real_variables_vr[sizeof(szVarName) / sizeof(szVarName[0])];


	for (i = 0; i < sizeof(szVarName) / sizeof(szVarName[0]); i++) {
		varHandle[i] = fmi2_import_get_variable_by_name(fmu, szVarName[i]);
		if (varHandle[i] == NULL) {
			printf("fmi2_import_get_variable_by_name failed\n");
			do_exit(CTEST_RETURN_FAIL);
		}
		compare_real_variables_vr[i] = fmi2_import_get_variable_vr(varHandle[i]);
		printf("%s:valueReference is %d\n", szVarName[i], compare_real_variables_vr[i]);
	}


        fmistatus = fmi2_import_exit_initialization_mode(fmu);
    if(fmistatus != fmi2_status_ok) {
        printf("fmi2_import_exit_initialization_mode failed\n");
        do_exit(CTEST_RETURN_FAIL);
    }        

	tcur = tstart;
	for (int i = 0; i < sizeof(szVarName) / sizeof(szVarName[0]); i++) {
		printf("%s,", szVarName[i]);
	}
	printf("\n");
	while (tcur < tend) {
		fmi2_boolean_t newStep = fmi2_true;
#if 0 /* Prints a real value.. */
		fmi2_real_t rvalue;
		fmi2_value_reference_t vr = 0;

		fmistatus = fmi2_import_get_real(fmu, &vr, 1, &rvalue);
		printf("rvalue = %f\n", rvalue);
#endif 
		fmistatus = fmi2_import_do_step(fmu, tcur, hstep, newStep);

		for (k = 0; k < sizeof(compare_real_variables_vr)/sizeof(fmi2_value_reference_t); k++) {
			//fmi2_value_reference_t vr = compare_real_variables_vr[k];
			//fmi2_real_t rvalue;
			//fmistatus = fmi2_import_get_real(fmu, &vr, 1, &rvalue);
		}
		{
			fmi2_real_t val[sizeof(compare_real_variables_vr) / sizeof(fmi2_value_reference_t)];
			fmi2_import_get_real(fmu, compare_real_variables_vr, sizeof(compare_real_variables_vr) / sizeof(fmi2_value_reference_t), val);
			for( int i = 0; i < sizeof(compare_real_variables_vr) / sizeof(fmi2_value_reference_t); i++){
				printf("%10g,", val[i]);
			}
			printf("\n");
		}

		tcur += hstep;
	}
#if 0
	printf("Simulation finished. Checking results\n");

	/* Validate result */
	for (k = 0; k < sizeof(compare_real_variables_vr)/sizeof(fmi2_value_reference_t); k++) {
		fmi2_value_reference_t vr = compare_real_variables_vr[k];
		fmi2_real_t rvalue;
		fmi2_real_t res;	
		fmistatus = fmi2_import_get_real(fmu, &vr, 1, &rvalue);
		res = rvalue - simulation_results[k];
		res = res > 0 ? res: -res; /* Take abs */
		if (res > 3e-3) {
			printf("Simulation results is wrong!\n");
			printf("State [%u]  %g != %g, |res| = %g\n", (unsigned)k, rvalue, simulation_results[k], res);
			printf("\n");
			do_exit(CTEST_RETURN_FAIL);
		}
	}
#endif
	fmistatus = fmi2_import_terminate(fmu);

	fmi2_import_free_instance(fmu);

	return 0;
}

int main(int argc, char *argv[])
{
	fmi2_callback_functions_t callBackFunctions;
	const char* FMUPath;
	const char* tmpPath;
	jm_callbacks callbacks;
	fmi_import_context_t* context;
	fmi_version_enu_t version;
	jm_status_enu_t status;
	int k;

	fmi2_import_t* fmu;	

	if(argc < 3) {
		//printf("Usage: %s <fmu_file> <temporary_dir>\n", argv[0]);
		//do_exit(CTEST_RETURN_FAIL);
		FMUPath = ".\\MotorALL.fmu";
		tmpPath = ".\\MotorALL";
	}
	else {
		for (k = 0; k < argc; k++)
			printf("argv[%d] = %s\n", k, argv[k]);

		FMUPath = argv[1];
		tmpPath = argv[2];
	}


	callbacks.malloc = malloc;
    callbacks.calloc = calloc;
    callbacks.realloc = realloc;
    callbacks.free = free;
    callbacks.logger = importlogger;
	callbacks.log_level = jm_log_level_warning;
    callbacks.context = 0;

#ifdef FMILIB_GENERATE_BUILD_STAMP
	printf("Library build stamp:\n%s\n", fmilib_get_build_stamp());
#endif

	context = fmi_import_allocate_context(&callbacks);

	version = fmi_import_get_fmi_version(context, FMUPath, tmpPath);

	if(version != fmi_version_2_0_enu) {
		printf("The code only supports version 2.0\n");
		do_exit(CTEST_RETURN_FAIL);
	}

	fmu = fmi2_import_parse_xml(context, tmpPath, 0);

	if(!fmu) {
		printf("Error parsing XML, exiting\n");
		do_exit(CTEST_RETURN_FAIL);
	}
	
	if(fmi2_import_get_fmu_kind(fmu) == fmi2_fmu_kind_me) {
		printf("Only CS 2.0 is supported by this code\n");
		do_exit(CTEST_RETURN_FAIL);
	}

	callBackFunctions.logger = fmi2_log_forwarding;
	callBackFunctions.allocateMemory = calloc;
	callBackFunctions.freeMemory = free;
	callBackFunctions.componentEnvironment = fmu;

	status = fmi2_import_create_dllfmu(fmu, fmi2_fmu_kind_cs, &callBackFunctions);
	if (status == jm_status_error) {
		printf("Could not create the DLL loading mechanism(C-API) (error: %s).\n", fmi2_import_get_last_error(fmu));
		do_exit(CTEST_RETURN_FAIL);
	}

	test_simulate_cs(fmu);

	fmi2_import_destroy_dllfmu(fmu);

	fmi2_import_free(fmu);
	fmi_import_free_context(context);
	
	printf("Everything seems to be OK since you got this far=)!\n");

	do_exit(CTEST_RETURN_SUCCESS);

	return 0;
}


