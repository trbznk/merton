#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define TEST_SIZE 100000
#define TEST_EPS 1e-2

typedef struct {
    char path[1024];

    float rho;
    size_t n_sims;
    unsigned int seed;
} Config;

typedef struct {
    float ms;
    float* L_PF;
    float* LR_PF;
    float exact_EL;
    float EL;
    size_t portfolio_size;
    size_t n_sims;
} Result;

typedef struct {
    float* EAD;
    float* PD;
    float* LGD;
    size_t size;
    size_t capacity;
} Portfolio;

void free_portfolio(Portfolio* pf) {
    free(pf->EAD);
    free(pf->PD);
    free(pf->LGD);
} 

float norm_rand() {
    // transform uniform distributed random number to normal distributed
    // with Box-Muller transformation - https://en.wikipedia.org/wiki/Box–Muller_transform
    float u1 = (float)rand() / (float)RAND_MAX;
    float u2 = (float)rand() / (float)RAND_MAX;

    // actually we can compute 2 normal distributed numbers from u1 and u2
    // but we only need one here. Caching the other one probably doesn't bring a performance boost.
    return sqrtf(-2.0f*logf(u1)) * cos(2*M_PI*u2);
}

float norm_cdf(float x) {
    return 0.5 * (1 + erff(x/sqrtf(2)));
}

float erf_inv(float x) {
    // https://people.maths.ox.ac.uk/gilesm/codes/erfinv/gems.pdf
    float w, p;
    w = - logf((1.0f-x)*(1.0f+x));
    if ( w < 5.000000f ) {
        w = w - 2.500000f;
        p = 2.81022636e-08f;
        p = 3.43273939e-07f + p*w;
        p = -3.5233877e-06f + p*w;
        p = -4.39150654e-06f + p*w;
        p = 0.00021858087f + p*w;
        p = -0.00125372503f + p*w;
        p = -0.00417768164f + p*w;
        p = 0.246640727f + p*w;
        p = 1.50140941f + p*w;
    } else {
        w = sqrtf(w) - 3.000000f;
        p =  -0.000200214257f;
        p = 0.000100950558f + p*w;
        p = 0.00134934322f + p*w;
        p = -0.00367342844f + p*w;
        p = 0.00573950773f + p*w;
        p = -0.0076224613f + p*w;
        p = 0.00943887047f + p*w;
        p = 1.00167406f + p*w;
        p = 2.83297682f + p*w;
    }
    return p*x;
}

float norm_ppf(float p) {
    return sqrtf(2) * erf_inv(2*p-1);
}

Portfolio read_csv(char* path) {
    FILE* f;
    f = fopen(path, "r");

    Portfolio pf = {0};

    float c1;
    float c2;
    float c3;

    char line_buffer[1024];
    // skip first row
    fgets(line_buffer, sizeof(line_buffer), f);
    const size_t prealloc_items = 10;
    while (fscanf(f, "%*d,%f,%f,%f", &c1, &c2, &c3) != EOF) {
        if (pf.capacity == 0) {
            pf.EAD = malloc(sizeof(float)*prealloc_items);
            pf.PD = malloc(sizeof(float)*prealloc_items);
            pf.LGD = malloc(sizeof(float)*prealloc_items);
            pf.capacity += prealloc_items;
        } else if (pf.capacity == pf.size) {
            pf.EAD = realloc(pf.EAD, (pf.size+prealloc_items)*sizeof(float));
            pf.PD = realloc(pf.PD, (pf.size+prealloc_items)*sizeof(float));
            pf.LGD = realloc(pf.LGD, (pf.size+prealloc_items)*sizeof(float));
            pf.capacity += prealloc_items;
        }
        pf.EAD[pf.size] = c1;
        pf.PD[pf.size] = c2;
        pf.LGD[pf.size] = c3;
        pf.size++;
    }
    fclose(f);

    return pf;
}

Result simulate(Config config) {
    srand(config.seed);

    time_t now = clock();
    
    Portfolio pf = read_csv(config.path);

    Result result;
    result.L_PF = calloc(config.n_sims, sizeof(float));
    result.LR_PF = calloc(config.n_sims, sizeof(float));
    result.EL = 0.0f;
    result.n_sims = config.n_sims;

    result.exact_EL = 0.0f;
    for (size_t i = 0; i < pf.size; i++) {
        result.exact_EL = result.exact_EL + pf.EAD[i]*pf.LGD[i]*pf.PD[i];
    }

    for (size_t i = 0; i < config.n_sims; i++) {
        float y = norm_rand();
        float pf_EAD = 0.0;
        for (size_t j = 0; j < pf.size; j++) {
            float ui = norm_rand();
            float r = sqrtf(config.rho)*y + sqrtf(1-config.rho)*ui;
            float cut_off = norm_ppf(pf.PD[j]);
            
            // this condition is true if the asset defaults
            if (r < cut_off) {
                result.L_PF[i] += pf.EAD[j]*pf.LGD[j];
            }
            pf_EAD = pf_EAD + pf.EAD[j];
        }
        result.LR_PF[i] = result.L_PF[i] / pf_EAD;   
    }

    for (size_t i = 0; i < result.n_sims; i++) {
        result.EL = result.EL + result.L_PF[i] / result.n_sims;
    }

    result.ms = ((float)(clock()-now) / CLOCKS_PER_SEC) * 1000.0;
    result.portfolio_size = pf.size;

    free_portfolio(&pf);
    return result;
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

void test_read_csv() {
    Portfolio pf = read_csv("./example_portfolio.csv");
    assert(pf.size == 80);
    assert(pf.EAD[0] == 10000000.0f);
    assert(pf.EAD[79] == 10000000.0f);
    assert(pf.PD[0] == 0.0001f);
    assert(pf.PD[79] == 0.0094f);
    assert(pf.LGD[0] == 0.6f);
    assert(pf.LGD[79] == 0.6f);
    free_portfolio(&pf);
}

int main() {
    test_random_normal_distribution();
    test_read_csv();

    Config config = {
        .path = "./example_portfolio.csv",
        .rho = 0.05,
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
