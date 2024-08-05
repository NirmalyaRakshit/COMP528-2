#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

// Function prototypes for file reading/writing from file-reader.c
extern int read_num_of_temps(char *filename);
extern double *read_temps(char *filename, int numOfValues);
extern void *write_to_output_file(char *filename, double *output, int numOfValues);

// Function prototypes from heat.c
double get_final_temperatures(int N, int maxIter, double radTemp);
double** allocate2DArray(int size);
void free2DArray(double** array, int size);
int isRadiator(int i, int j, int N);

/*Gets the number of the radiator temperatures in the file. Returns as a single integer*/
int read_num_of_temps(char *filename) {
    FILE *file = fopen(filename,"r");
    int i;
    
    if(file == NULL) {
        printf("Unable to open file: %s", filename);
        return -1; //-1 means error
    }

    char firstline[500];
    
    if(fgets(firstline, 500, file) == NULL){
        perror("Error reading file");
        return -1; //error
    }
    
    int line_length = strlen(firstline);
    
    int numOfValues;

    const char s[2] = " ";
    char *token;
    token = strtok(firstline, s);
    i = 0;
    while(token != NULL && i == 0) {
        numOfValues = atoi(token);
        i++;
        token = strtok(NULL, s);
    }
    fclose(file);
    return numOfValues;
}

/*Gets the data from the file. Returns as an array of doubles. Ignores the first numOfValues*/
double *read_temps(char *filename, int numOfValues) {
    FILE *file = fopen(filename,"r");
    int i;
    
    if(file == NULL) {
        perror("Unable to open file: %s");
        return NULL; //error
    }

    char firstline[500];
    
    if(fgets(firstline, 500, file) == NULL){
        perror("Error reading file");
        return NULL; //error
    }

    //Ignore first line and move on since first line contains 
    //header information and we already have that. 

    double *one_d = malloc(sizeof(double) * numOfValues);
    
    for(i=0; i<numOfValues; i++) {
        if(fscanf(file, "%lf", &one_d[i]) == EOF){
            perror("Error reading file");
            return NULL; //error
        }
    }
    fclose(file);
    return one_d;
}


int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <N> <maxIter> <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int maxIter = atoi(argv[2]);
    char* input_file = argv[3];
    char* output_file = argv[4];

    // Read number of temperatures and the temperature values
    int numOfTemps = read_num_of_temps(input_file);
    double* temps = read_temps(input_file, numOfTemps);

    // Allocate memory for storing results
    double* results = (double*) malloc(numOfTemps * sizeof(double));

    // Compute final temperature for each radiator temperature
    for (int i = 0; i < numOfTemps; i++) {
        results[i] = get_final_temperatures(N, maxIter, temps[i]);
    }

    // Write results to the output file
    write_to_output_file(output_file, results, numOfTemps);

    // Free allocated memory
    free(temps);
    free(results);

    return 0;
}