/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * @file
 * This program is a command-line user interface for performing tests with the
 * provided pso and functions libraries.
 *
 * @author Carlos Fernandes
 * @author Nuno Fachada
 */

// System libraries
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#ifdef _OPENMP
	#include <omp.h>
#endif

// Local libraries
#include "iniparser.h"
#include "functions.h"
#include "cec15_interface.h"
#include "cec17_bound_constrained_interface.h"
#include "pso.h"
#include "errorhandling.h"
#include "topol/staticgrid2d.h"
#include "topol/staticring1d.h"
#include "topol/staticgraph.h"

// Constants

/// Default PRNG seed
#define DEFAULT_PRNG_SEED 1234

/// Maximum length for strings containing unsigned integers
#define MAXUISLEN 11

/// Maximum file name length
#define FILE_MAX_LEN 255
/// Default input file
#define FILE_INPUT_DEFAULT "input.ini"
/// File where to save average best so far fitness between runs
#define FILE_AVG_BEST_SO_FAR "AVE_BESTSOFAR.DAT"
/// File where to save number of evaluations required to reach criterion per run
#define FILE_EVALS "AES.DAT"
/// File where to save final fitness per run
#define FILE_FITNESS "FINAL.DAT"

// Useful macros

/**
 * Macro for calculating median. Assumes sorted vector.
 *
 * @param[in] vec Sorted numeric vector.
 * @param[in] n Number of elements in vector.
 * @param[out] res Median of numeric elements in vector.
 */
#define MEDIAN(vec, n, res) \
	do { \
		if (n % 2 == 0) res = (vec[n / 2] + vec[n / 2 - 1]) / 2; \
		else res = vec[n / 2]; \
	} while (0)

/**
 * Function for selecting benchmark function.
 *
 * @param[in] func ID of benchmark function to use.
 * @return Pointer to benchmark function to use.
 */
static pso_func_opt getSelFunc(unsigned int func) {
	switch (func) {
		case 1: return &Sphere;
		case 2: return &Quadric;
		case 3: return &Hyper;
		case 4: return &Rastrigin;
		case 5: return &Griewank;
		case 6: return &Schaffer6;
		case 7: return &Weierstrass;
		case 8: return &Ackley;
		case 9: return &ShiftedQuadricWithNoise;
		case 10: return &RotatedGriewank;
		case 11: return &cec2015_01;
		case 12: return &cec2015_02;
		case 13: return &cec2015_03;
		case 14: return &cec2015_04;
		case 15: return &cec2015_05;
		case 16: return &cec2015_06;
		case 17: return &cec2015_07;
		case 18: return &cec2015_08;
		case 19: return &cec2015_09;
		case 20: return &cec2015_10;
		case 21: return &cec2015_11;
		case 22: return &cec2015_12;
		case 23: return &cec2015_13;
		case 24: return &cec2015_14;
		case 25: return &cec2015_15;
		case 26: return &cec2017_bc_01;
		case 27: return &cec2017_bc_02;
		case 28: return &cec2017_bc_03;
		case 29: return &cec2017_bc_04;
		case 30: return &cec2017_bc_05;
		case 31: return &cec2017_bc_06;
		case 32: return &cec2017_bc_07;
		case 33: return &cec2017_bc_08;
		case 34: return &cec2017_bc_09;
		case 35: return &cec2017_bc_10;
		case 36: return &cec2017_bc_11;
		case 37: return &cec2017_bc_12;
		case 38: return &cec2017_bc_13;
		case 39: return &cec2017_bc_14;
		case 40: return &cec2017_bc_15;
		case 41: return &cec2017_bc_16;
		case 42: return &cec2017_bc_17;
		case 43: return &cec2017_bc_18;
		case 44: return &cec2017_bc_19;
		case 45: return &cec2017_bc_20;
		case 46: return &cec2017_bc_21;
		case 47: return &cec2017_bc_22;
		case 48: return &cec2017_bc_23;
		case 49: return &cec2017_bc_24;
		case 50: return &cec2017_bc_25;
		case 51: return &cec2017_bc_26;
		case 52: return &cec2017_bc_27;
		case 53: return &cec2017_bc_28;
		case 54: return &cec2017_bc_29;
		case 55: return &cec2017_bc_30;
		case 56: return &cec2017_c_01;
		case 57: return &cec2017_c_02;
		case 58: return &cec2017_c_03;
		case 59: return &cec2017_c_04;
		case 60: return &cec2017_c_05;
		case 61: return &cec2017_c_06;
		case 62: return &cec2017_c_07;
		case 63: return &cec2017_c_08;
		case 64: return &cec2017_c_09;
		case 65: return &cec2017_c_10;
		case 66: return &cec2017_c_11;
		case 67: return &cec2017_c_12;
		case 68: return &cec2017_c_13;
		case 69: return &cec2017_c_14;
		case 70: return &cec2017_c_15;
		case 71: return &cec2017_c_17;
		case 72: return &cec2017_c_18;
		case 73: return &cec2017_c_19;
		case 74: return &cec2017_c_20;
		case 75: return &cec2017_c_21;
		case 76: return &cec2017_c_22;
		case 77: return &cec2017_c_23;
		case 78: return &cec2017_c_24;
		case 79: return &cec2017_c_25;
		case 80: return &cec2017_c_26;
		case 81: return &cec2017_c_27;
		case 82: return &cec2017_c_28;
		default: return NULL;
	}
}

// *************** Global Variables *****************

/// Seed for pseudo-random number generator.
static unsigned int prng_seed;
/// File containing PSO parameters.
static char input_file[FILE_MAX_LEN];
/// Average best so for between runs
static double * averageBestSoFar;
/// Average best so far counter
static unsigned int avsf_counter;
/// Number of threads.
static unsigned int num_threads;
/// Problem to solve
static unsigned int problem;
// Number of runs
static unsigned int n_runs;

// Helper function for comparing two doubles
static int cmpdbl(const void *a, const void *b) {
	if (*(double*) a > *(double*) b) return 1;
	else if (*(double*) a < *(double*) b) return -1;
	else return 0;
}

// Helper function for comparing two unsigned integers
static int cmpuint(const void *a, const void *b) {
	if (*(unsigned int*) a > *(unsigned int*) b) return 1;
	else if (*(unsigned int*) a < *(unsigned int*) b) return -1;
	else return 0;
}

// Helper function for converting unsigned integer to string
// Cannot be called by threaded code
static char *uint2str(unsigned int evals) {
	static char str[MAXUISLEN]; // Not thread-safe
	snprintf(str, MAXUISLEN * sizeof(char), "%u", evals);
	return str;
}

// End-of-iteration hook for saving fitness averages between runs during the
// course of the PSO algorithm
static void avg_best_so_far(PSO *pso) {

	// Is it time to update the average (between runs) best so far?
	if (pso->evaluations % 100 == 0) {
		averageBestSoFar[avsf_counter] +=
			pso->best_so_far / (double) n_runs;
		avsf_counter++;
	}
}

/**
 * Parse command-line options and read PSO parameters from file.
 *
 * @param[in] argc Number of program arguments.
 * @param[in] argv Program arguments. Argument at index 1 is the file containing
 * the PSO parameters. Argument at index 2 is the PRNG seed.
 * @param[out] params Pointer to PSO parameters.
 * @return `EXIT_SUCCESS` if program executes successfully, `EXIT_FAILURE`
 * otherwise.
 */
static void parse_params(int argc, char *argv[], PSO_PARAMS *params) {

	// INI object
	dictionary *ini;

	// Topology type
	const char *topol;

	// Did user specify a PSO parameter file?
	if (argc >= 2)
		strncpy(input_file, argv[1], FILE_MAX_LEN);
	else
		strncpy(input_file, FILE_INPUT_DEFAULT, FILE_MAX_LEN);

	// Did user specify a PRNG seed?
	if (argc >= 3)
		prng_seed = atoi(argv[2]);
	else
		prng_seed = DEFAULT_PRNG_SEED;

	// Number of threads
#ifdef _OPENMP
	num_threads = omp_get_max_threads();
#else
	num_threads = 1;
#endif

	// Try to open PSO parameters file
	ini = iniparser_load(input_file);
	if (!ini) ERROR_EXIT("Unable to parse input file '%s'", input_file);

	// Read PSO parameters file

	// Read topology related parameters

	// Determine topology
	topol = iniparser_getstring(ini, "topology:type", NULL);
	if (topol == NULL) {
		ERROR_EXIT("Topology type not defined in '%s'.", input_file);
	} else if (strcmp(topol, "staticgrid2d") == 0) {
		params->initPopSize = pso_staticgrid2d_parse_params(ini);
		params->topol.new = pso_staticgrid2d_new;
		params->topol.destroy = pso_staticgrid2d_destroy;
		params->topol.iterate = pso_staticgrid2d_iterate;
		params->topol.next = pso_staticgrid2d_next;
	} else if (strcmp(topol, "staticring1d") == 0) {
		params->initPopSize = pso_staticring1d_parse_params(ini);
		params->topol.new = pso_staticring1d_new;
		params->topol.destroy = pso_staticring1d_destroy;
		params->topol.iterate = pso_staticring1d_iterate;
		params->topol.next = pso_staticring1d_next;
	} else if (strcmp(topol, "staticgraph") == 0) {
		params->initPopSize = pso_staticgraph_parse_params(ini);
		params->topol.new = pso_staticgraph_new;
		params->topol.destroy = pso_staticgraph_destroy;
		params->topol.iterate = pso_staticgraph_iterate;
		params->topol.next = pso_staticgraph_next;
	} else {
		ERROR_EXIT("Unknown topology '%s'", topol);
	}

	if (params->initPopSize < 1) {
		ERROR_EXIT("Invalid initial population size%s", "");
	}

	// The following parameters are related to the PSO model itself, and their
	// validation is performed when creating the PSO model object, not here.
	params->max_t = (unsigned int)
		iniparser_getint(ini, "pso:max_t", 0);

	params->max_evaluations = (unsigned int)
		iniparser_getint(ini, "pso:max_evaluations", 0);

	params->algorithm = (unsigned int)
		iniparser_getint(ini, "pso:algorithm", 0);

	params->gbest = iniparser_getboolean(ini, "pso:gbest", -1);

	params->Xmax = iniparser_getdouble(ini, "pso:xmax", -DBL_MAX);

	params->Vmax = iniparser_getdouble(ini, "pso:vmax", -DBL_MAX);

	params->chi = iniparser_getdouble(ini, "pso:chi", -DBL_MAX);

	params->omega = iniparser_getdouble(ini, "pso:omega", -DBL_MAX);

	params->c = iniparser_getdouble(ini, "pso:c", -DBL_MAX);

	params->nvars = (unsigned int)
		iniparser_getint(ini, "pso:numbervariables", 0);

	params->iWeightStrategy = (unsigned int)
		iniparser_getint(ini, "pso:iweightstrategy", 2);

	params->cStrategy = (unsigned int)
		iniparser_getint(ini, "pso:cstrategy", 2);

	params->assyInitialization =
		iniparser_getboolean(ini, "pso:assyinitialization", -1);

	params->initialXmin =
		iniparser_getdouble(ini, "pso:initialxmin", -DBL_MAX);

	params->initialXmax =
		iniparser_getdouble(ini, "pso:initialxmax", -DBL_MAX);

	params->crit = iniparser_getdouble(ini, "pso:crit", -DBL_MAX);

	params->crit_keep_going =
		iniparser_getboolean(ini, "pso:crit_keep_going", -1);

	// The following parameters are not directly related with the PSO model,
	// and their validation is performed here.
	n_runs = (unsigned int) iniparser_getint(ini, "pso:n_runs", 0);
	if (n_runs < 1)
		ERROR_EXIT("Invalid input parameter: %s", "n_runs");

	problem = (unsigned int) iniparser_getint(ini, "pso:problem", 0);
	if ((problem < 1) || (problem > 82))
		ERROR_EXIT("Invalid input parameter: %s", "problem");

	// Release dictionary object
	iniparser_freedict(ini);

}

/**
 * Program starts here.
 *
 * @param[in] argc Number of program arguments.
 * @param[in] argv Program arguments. Argument at index 1 is the file containing
 * the PSO parameters. Argument at index 2 is the PRNG seed.
 * @return `EXIT_SUCCESS` if program executes successfully, `EXIT_FAILURE`
 * otherwise.
 */
int main(int argc, char *argv[]) {

	/// PSO parameters, to be read from file
	PSO_PARAMS params;

	// Parse command-line arguments and read PSO parameter file - this must be
	// here in order to read number of runs, although number of runs could be a
	// command-line argument instead
	parse_params(argc, argv, &params);

	// Aux. file pointer
	FILE * out;
	// PSO model object, from pso library
	PSO * pso;

	// Medians for fitness and evaluations
	float fitmedian, evalsmedian;

	// Number of evaluations required to reach criterion per run
	unsigned int crit_evals[n_runs];
	// Final fitness per run
	double best_so_far[n_runs];
	// Number of successes, i.e. number of times that criterion was reach with
	// number of evaluations < max_evaluations
	unsigned int successes = 0;

	// Show parameter information to user
	printf("\nPARAMS\n------\n");
	printf("Num threads        : %u\n", num_threads);
	printf("PSO parameter file : %s\n", input_file);
	printf("PRNG seed          : %d\n", prng_seed);
	printf("\n");

	// Initialize averageBestSoFar array, set contents to zero
	averageBestSoFar =
		(double *) calloc(params.max_evaluations / 100, sizeof(double));

	printf("\nRUNS\n----\n");

	// Perform PSO runs
	for (unsigned int i = 0; i < n_runs; ++i) {

		// Reset average best so far counter
		avsf_counter = 0;

		// Initialize PSO for current run
		pso = pso_new(params , getSelFunc(problem), prng_seed + i);
		if (!pso) ERROR_EXIT("%s", pso_error);

		// Add end-of-iteration hooks
		pso_hook_add(pso, avg_best_so_far);

		// PSO main cycle for current run
		pso_run(pso);

		// If the number of evaluations was not enough to get below the stop
		// criteria, set it to the maximum number of performed evaluations
		if (pso->crit_evals) {
			crit_evals[i] = pso->crit_evals;
			successes++;
		} else {
			crit_evals[i] = UINT_MAX;
		}

		// Inform user of current run performance
		printf("Run %4u | BestFit = %10.5g | AvgFit = %10.5g | "
			"Evals = %10s\n", i, (double) pso->best_so_far,
			(double) pso->average_fitness,
			pso->crit_evals ? uint2str(crit_evals[i]) : "--");

		// Keep best so far for current run
		best_so_far[i] = (double) pso->best_so_far;

		// Release PSO model for current run
		pso_destroy(pso);
	}

	// Save file containing the average (between runs) best so far fitness
	// after 100, 200, ... evaluations.
	out = fopen(FILE_AVG_BEST_SO_FAR, "w");
	for (unsigned int i = 0; i < avsf_counter; ++i)
		fprintf(out, "%.40g\n", (double) averageBestSoFar[i]);
	fclose(out);
	free(averageBestSoFar);

	// Save number of evaluations required for getting below stop criterion
	out = fopen(FILE_EVALS, "w");
	for (unsigned int i = 0; i < n_runs; ++i) {
		if (crit_evals[i] < UINT_MAX) {
			fprintf(out, "%u\n", crit_evals[i]);
		}
	}
	fclose(out);

	// Save best so far for each run
	out = fopen(FILE_FITNESS, "w");
	for (unsigned int i = 0; i < n_runs; ++i) {
		fprintf(out, "%.45g\n", best_so_far[i]);
	}
	fclose(out);

	// Show statistics over all runs
	printf("\nSTATISTICS\n----------\n");
	printf("          \t %11s \t %11s \t %11s\n", "Median", "Min", "Max");

	// Fitness statistics
	qsort(best_so_far, n_runs, sizeof(double), cmpdbl);
	MEDIAN(best_so_far, n_runs, fitmedian);

	// Evaluation statistics
	printf("Fitness = \t %10.5e \t %10.5e \t %10.5e\n",
		fitmedian, best_so_far[0], best_so_far[n_runs - 1]);
	if (successes) {
		qsort(crit_evals, n_runs, sizeof(unsigned int), cmpuint);
		MEDIAN(crit_evals, successes, evalsmedian);
		printf("Evals   = \t %11.1f \t %11u \t %11u\n",
			evalsmedian, crit_evals[0], crit_evals[successes - 1]);
	} else {
		printf("Evals   = \t %11s \t %11s \t %11s\n", "--", "--", "--");
	}

	// Number and percentage of successes
	printf("\nSuccess = %u/%u (%2.2f%%)\n\n", successes, n_runs,
		100.0 * (float) successes / (float) n_runs);

	// End program successfully
	return EXIT_SUCCESS;
}
