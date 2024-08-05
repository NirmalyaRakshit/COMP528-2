#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include<string.h>

// Function prototypes from heat.c
double get_final_temperatures(int N, int maxIter, double radTemp);
double** allocate2DArray(int size);
void free2DArray(double** array, int size);
int isRadiator(int i, int j, int N);

// Function prototypes for file reading/writing
int read_num_of_temps(char *filename);
double *read_temps(char *filename, int numOfValues);
void *write_to_output_file(char *filename, double *output, int numOfValues);

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
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <N> <maxIter> <input_file> <output_file>\n", argv[0]);
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]);
    int maxIter = atoi(argv[2]);
    char* input_file = argv[3];
    char* output_file = argv[4];

    int numOfTemps, local_numOfTemps;
    double *temps = NULL, *local_temps = NULL, *local_results = NULL;

    if (rank == 0) {
        numOfTemps = read_num_of_temps(input_file);
        temps = read_temps(input_file, numOfTemps);
    }

    MPI_Bcast(&numOfTemps, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_numOfTemps = numOfTemps / size;
    int remainder = numOfTemps % size;
    if (rank < remainder) {
        local_numOfTemps++;
    }

    local_temps = (double*) malloc(local_numOfTemps * sizeof(double));
    local_results = (double*) malloc(local_numOfTemps * sizeof(double));

    int* sendcounts = (int*) malloc(size * sizeof(int));
    int* displs = (int*) malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        sendcounts[i] = numOfTemps / size + (i < remainder ? 1 : 0);
        displs[i] = offset;
        offset += sendcounts[i];
    }

    MPI_Scatterv(temps, sendcounts, displs, MPI_DOUBLE, local_temps, local_numOfTemps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    #pragma omp parallel for
    for (int i = 0; i < local_numOfTemps; i++) {
        local_results[i] = get_final_temperatures(N, maxIter, local_temps[i]);
    }

    double* results = NULL;
    if (rank == 0) {
        results = (double*) malloc(numOfTemps * sizeof(double));
    }
    MPI_Gatherv(local_results, local_numOfTemps, MPI_DOUBLE, results, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        write_to_output_file(output_file, results, numOfTemps);
        free(results);
        free(temps);
    }

    free(local_temps);
    free(local_results);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
