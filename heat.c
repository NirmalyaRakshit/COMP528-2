#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>


#ifndef HEAT_H
#define HEAT_H

double get_final_temperatures(int N, int maxIter, double radTemp);
double** allocate2DArray(int N);
void free2DArray(double** array, int N);
int isRadiator(int i, int j, int N);

#endif 

// Allocate memory for a 2D array
double** allocate2DArray(int size) {
    double** array = (double**)malloc(size * sizeof(double*));
    if (array == NULL) {
        perror("Memory allocation failed for 2D array");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        array[i] = (double*)malloc(size * sizeof(double));
        if (array[i] == NULL) {
            perror("Memory allocation failed for 2D array");
            exit(EXIT_FAILURE);
        }
    }
    return array;
}

// Free memory allocated for a 2D array
void free2DArray(double** array, int size) {
    for (int i = 0; i < size; i++) {
        free(array[i]);
    }
    free(array);
}

// Determine if a grid cell is part of the radiator
int isRadiator(int i, int j, int N) {
    return (i == N - 1) && (j >= floor((N - 1) * 0.3)) && (j <= ceil((N - 1) * 0.7));
}


// Compute the final temperatures after the specified number of iterations
double get_final_temperatures(int N, int maxIter, double radTemp) {
    double** curr_t = allocate2DArray(N);
    double** prev_t = allocate2DArray(N);

    // Initialize grid temperatures; set radiator cells to radTemp, others to defaultTemp
    double defaultTemp = 10.0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double temp = isRadiator(i, j, N) ? radTemp : defaultTemp;
            curr_t[i][j] = temp;
            prev_t[i][j] = temp;
        }
    }

    // Perform the temperature update iterations
    for (int iter = 0; iter < maxIter; ++iter) {
        #pragma omp parallel for collapse(2) shared(curr_t, prev_t, N) default(none)
        for (int i = 1; i < N - 1; ++i) {
            for (int j = 1; j < N - 1; ++j) {
                if (!isRadiator(i, j, N)) {
                    curr_t[i][j] = (prev_t[i][j + 1] + prev_t[i][j - 1] + prev_t[i + 1][j] + prev_t[i - 1][j]) / 4.0;
                }
            }
        }

        // Swap pointers for the next iteration
        double** temp = curr_t;
        curr_t = prev_t;
        prev_t = temp;
    }

    // Retrieve the final temperature from the center of the grid
    int centerX = (N - 1) / 2;
    int centerY = (N - 1) / 2;
    double result = prev_t[centerX][centerY];

    // Release allocated memory
    free2DArray(curr_t, N);
    free2DArray(prev_t, N);

    return result;
}