#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

#define TEST_SIZE 1000000
#define TEST_EPS 1e-2

typedef struct Simulation {
    float asset_corr; // rho
    float cut_off; // c
    float EAD;
    float LGD;
    int pf_size; // m
    int n_sims;   

    // results
    float* L_PF; 
} Simulation;

float norm_rand() {
    // transform uniform distributed random number to normal distributed
    // with Box-Muller transformation - https://en.wikipedia.org/wiki/Box–Muller_transform
    float u1 = (float)rand() / (float)RAND_MAX;
    float u2 = (float)rand() / (float)RAND_MAX;

    // actually we could compute 2 normal distributed numbers from u1 and u2
    // but we only need one here. Caching the other one probably doesn't bring a performance boost.
    return sqrtf(-2.0f*logf(u1)) * cos(2*M_PI*u2);
}

void test_random_normal_distribution() {
    float X[TEST_SIZE];
    float s = 0;
    for (int i = 0; i < TEST_SIZE; i++) {
        X[i] = norm_rand();
        s = s + X[i];
    }
    float mean = s/TEST_SIZE;
    float std = 0;
    for (int i = 0; i < TEST_SIZE; i++) {
        std = std + powf(X[i]-mean, 2);
    }
    std = sqrtf(std/TEST_SIZE);

    assert(fabsf(mean) < TEST_EPS);
    assert(fabsf(std-1) < TEST_EPS);
}

void simulate(Simulation* sim) {
    srand(time(NULL));

    for (int i = 0; i < sim->n_sims; i++) {
        sim->L_PF[i] = 0.0;
        float y = norm_rand();
        for (int j = 0; j < sim->pf_size; j++) {
            float ui = norm_rand();
            float r = sqrtf(sim->asset_corr)*y + sqrtf(1-sim->asset_corr)*ui;
            if (r <= sim->cut_off) {
                sim->L_PF[i] += sim->EAD*sim->LGD;
            }
        }
    }
}

int main() {
    Simulation sim;
    sim.asset_corr = 0.05;
    sim.cut_off = -2.0;
    sim.EAD = 1.0;
    sim.LGD = 0.6;
    sim.pf_size = 100;
    sim.n_sims = 1000;
    sim.L_PF = malloc(sizeof(float)*sim.n_sims);

    simulate(&sim);
    for (int i = 0; i < sim.n_sims; i++) {
        printf("[%d] L_PF=%.4f\n", i+1, sim.L_PF[i]);
    }

    free(sim.L_PF);
}
