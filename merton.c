#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define TEST_SIZE 1000000
#define TEST_EPS 1e-2

typedef struct {
    char path[1024];

    float asset_corr; // rho
    int n_sims;   
} Config;

typedef struct {
    float ms;
    float* L_PF;
    float* LR_PF;
    int pf_size;
} Result;

typedef struct {
    float* EAD;
    float* PD;
    float* LGD;
    int size;
} Portfolio;

float norm_rand() {
    // transform uniform distributed random number to normal distributed
    // with Box-Muller transformation - https://en.wikipedia.org/wiki/Box–Muller_transform
    float u1 = (float)rand() / (float)RAND_MAX;
    float u2 = (float)rand() / (float)RAND_MAX;

    // actually we can compute 2 normal distributed numbers from u1 and u2
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

Portfolio read_csv(char* path) {
    FILE* f;
    f = fopen(path, "r");

    Portfolio pf;
    pf.EAD = malloc(sizeof(float));
    pf.PD = malloc(sizeof(float));
    pf.LGD = malloc(sizeof(float));
    pf.size = 0;

    float c1;
    float c2;
    float c3;

    char line_buffer[1024];
    // skipt first row
    fgets(line_buffer, sizeof(line_buffer), f);
    while (fscanf(f, "%*d,%f,%f,%f", &c1, &c2, &c3) != EOF) {
        pf.size++;
        pf.EAD = realloc(pf.EAD, sizeof(float)*pf.size);
        pf.PD = realloc(pf.PD, sizeof(float)*pf.size);
        pf.LGD = realloc(pf.LGD, sizeof(float)*pf.size);
        pf.EAD[pf.size-1] = c1;
        pf.PD[pf.size-1] = c2;
        pf.LGD[pf.size-1] = c3;
    }
    fclose(f);

    return pf;
}

Result simulate(Config config) {
    srand(time(NULL));

    time_t now = clock();
    
    Portfolio pf = read_csv(config.path);
    Result result;
    result.L_PF = calloc(config.n_sims, sizeof(float));
    result.LR_PF = calloc(config.n_sims, sizeof(float));

    for (int i = 0; i < config.n_sims; i++) {
        float y = norm_rand();
        float pf_EAD = 0.0;
        for (int j = 0; j < pf.size; j++) {
            float ui = norm_rand();
            float r = sqrtf(config.asset_corr)*y + sqrtf(1-config.asset_corr)*ui;

            // this is the ppf of the normal distribution
            float cut_off = 0.5 * (1 + erff(pf.PD[j]/sqrtf(2)));

            if (r <= cut_off) {
                result.L_PF[i] += pf.EAD[j]*pf.LGD[j];
            }
            pf_EAD = pf_EAD + pf.EAD[j];
        }
        result.LR_PF[i] = result.L_PF[i] / pf_EAD;   
    }

    result.ms = ((float)(clock()-now) / CLOCKS_PER_SEC) * 1000.0;
    result.pf_size = pf.size;
    return result;
}

int main() {
    Config config = {
        .path = "./example_portfolio.csv",
        .asset_corr = 0.05,
        .n_sims = 10000
    };

    Result result = simulate(config);
    for (int i = 0; i < 10; i++) {
        printf("[%d] L_PF=%.4f, LR_PF=%.4f\n", i+1, result.L_PF[i], result.LR_PF[i]);
    }
    printf("ms: %f\n", result.ms);

    free(result.L_PF);

    return 0;
}
