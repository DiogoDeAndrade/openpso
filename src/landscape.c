#include "perlin.h"
#include <stdio.h>
#include <math.h>

double landscape1(double *vars, unsigned int nvars)
{
    if (nvars!=2)
    {
        printf("\nError: Landscape functions are only defined for D=2 (D=%i)\n", nvars);
        return 0;
    }

    double x = vars[0];
    double y = vars[1];

    double ret = 0.0f;

    double amp = 20;
    double freq = 0.02;
    
    int octaves = 8;
    for (int i = 0; i < octaves; i++)
    {
       ret += amp * perlin2d(x * freq, y * freq);
       amp *= 0.5f;
       freq *= 2.0f;
    }

    return ret;
}
