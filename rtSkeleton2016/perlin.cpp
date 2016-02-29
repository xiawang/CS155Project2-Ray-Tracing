/* Coherent noise function over 3 dimensions */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"
#include "perlin.h"

using namespace std;

double PerlinNoise::noise(double x, double y, double z) {
      return 0;
   }

/* compute perlin noise values given new seed */
PerlinNoise::PerlinNoise(int newSeed) {
    const int size=256;
    seed = newSeed;
    srand(seed);
    
    int vals[size];
    bool done[size];
    for (int i=0; i<size; i++)
        done[i]=false;
    for (int i=0; i<size; i++) {
        int newVal = rand() % size;
        int startVal = newVal;
        while (done[newVal]==true) {
            newVal++;
            if (newVal>=size)
                newVal=0;
            if (startVal == newVal) {
                cerr << "random errors" << endl;
                break;
            }
        }
        vals[i]=newVal;
        done[newVal]=true;
    }
    
    for (int i=0; i < size ; i++) 
        p[size+i] = p[i] = vals[i];
 
}
