#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "pso.h"

void setup_instrumentation(const char* baseFilename);
void instrument_run_start();
void instrument_run_end();
void instrument_particle(int iteration, int particleId, double x, double y, double fitness, double velocityX, double velocityY);
void instrument_problem(const char* filename, pso_func_opt problemFunction, double minX, double minY, double maxX, double maxY, unsigned int nPoints);
void instrument_topology(const char* filename, PSO* pso);

#endif