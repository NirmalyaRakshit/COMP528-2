# Variables for compiler commands
CC = gcc
MPICC = mpicc
ICC = icc
MPIICC = mpiicc
CFLAGS = -fopenmp -lm

# Compilation instructions for serial and MPI programs using GNU and Intel compilers

gccserial: main-serial.c heat.c file-reader.c
	$(CC) -fopenmp heat.c main-serial.c file-reader.c -o heat-omp-gcc -lm 

gcccomplete: main-mpi.c heat.c file-reader.c
	$(MPICC) -fopenmp main-mpi.c heat.c file-reader.c -o heat-complete-gcc -lm

iccserial: main-serial.c heat.c file-reader.c
	$(ICC) -qopenmp main-serial.c heat.c file-reader.c -o heat-omp-icc -lm

icccomplete: main-mpi.c heat.c file-reader.c
	$(MPIICC) -qopenmp main-mpi.c heat.c file-reader.c -o heat-complete-icc -lm

clean:
	rm -f heat-omp-gcc heat-complete-gcc heat-omp-icc heat-complete-icc
