#include "instrument.h"
#include <stdio.h>
#include <math.h>

int     instrumentationEnabled = 0;
char    instrumentationRunFilename[512];
FILE*   instrumentationFile = NULL;

void setup_instrumentation(const char* baseFilename)
{
    snprintf((char*)&instrumentationRunFilename, 512, "%s.txt", baseFilename);

    FILE* file = fopen(instrumentationRunFilename, "wt");
    fprintf(file, "action;iteration;particle;x;y;fitness;velocity_x;velocity_y\n");
    fclose(file);

    instrumentationEnabled = 1;
}

void instrument_run_start()
{
    if (!instrumentationFile)
    {
        instrumentationFile = fopen(instrumentationRunFilename, "at");
    }
}

void instrument_run_end()
{
    if (instrumentationFile)
    {
        fclose(instrumentationFile);
        instrumentationFile = NULL;
    }
}

void instrument_particle(int iteration, int particleId, double x, double y, double fitness, double velocityX, double velocityY)
{
    if (instrumentationEnabled)
    {
        if (fabs(velocityX)+fabs(velocityY) > 0.01f)
        {
            fprintf(instrumentationFile, "0;%i;%i;%f;%f;%f;%f;%f\n", iteration, particleId,
                                                x, y, 
                                                fitness,
                                                velocityX, velocityY);
        }
    }
}

void instrument_problem(const char* filename, pso_func_opt problemFunction, double minX, double minY, double maxX, double maxY, unsigned int nPoints)
{
    char    functionFilename[512];    

    snprintf((char*)&functionFilename, 512, "%s.txt", filename);
   
    printf("Saving function sampling (%ix%i) in file %s...\n", nPoints, nPoints, functionFilename);

    FILE* file = fopen(functionFilename, "wt");
    fprintf(file, "%i\n%i\n", nPoints, nPoints);
    fprintf(file, "%f\n%f\n%f\n%f\n", minX, minY, maxX, maxY);
    fprintf(file, "%f\n", (maxX - minX) / nPoints);

    double incX = (maxX - minX) / (double)(nPoints - 1);
    double incY = (maxY - minY) / (double)(nPoints - 1);    

    double v[2] = { minX, minY };
    
    for (unsigned int y = 0; y < nPoints; y++)
    {
        for (unsigned int x = 0; x < nPoints; x++)
        {
            double value = problemFunction((double*)&v, 2);
            fprintf(file, "%f ", value);

            v[0] += incX;
        }
        fprintf(file, "\n");

        v[0] = minX;
        v[1] += incY;
    }

    fclose(file);
}
