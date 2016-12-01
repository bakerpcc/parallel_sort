#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int *a;

int compare(const void * a, const void * b) {
	return (*(int*) a - *(int*) b);
}

int compare2(const void * a, const void * b) {
	return (*(int*) b - *(int*) a);
}

void bitonicCompare(int *a, int len) {
	int tmp;
	for (int i = 0, j = len / 2; i < len / 2; i++, j++) {
		if (a[i] > a[j]) {
			tmp = a[i];
			a[i] = a[j];
			a[j] = tmp;
		}
	}
}

void bitonicCompare2(int *a, int len) {
	int tmp;
	for (int i = 0, j = len / 2; i < len / 2; i++, j++) {
		if (a[i] < a[j]) {
			tmp = a[i];
			a[i] = a[j];
			a[j] = tmp;
		}
	}
}

void bitonicMergeSort(int *a, int len) {
	bitonicCompare(a, len);
	if (len > 2) {
		bitonicMergeSort(a, len / 2);
		bitonicMergeSort(&a[len / 2], len / 2);
	}
}

void bitonicMergeSort2(int *a, int len) {
	bitonicCompare2(a, len);
	if (len > 2) {
		bitonicMergeSort2(a, len / 2);
		bitonicMergeSort2(&a[len / 2], len / 2);
	}
}

void bitonicSort(int *a, int len, int rank, int size) {
	int chunk_size = len / size;
	int *my_chunk = (int *) malloc(sizeof(int) * chunk_size);
	int *tmp_buffer = (int *) malloc(sizeof(int) * len);
	int direction = __builtin_popcount(rank) % 2;
	MPI_Status status;

	MPI_Scatter(a, chunk_size, MPI_INT, my_chunk, chunk_size, MPI_INT, 0,
	MPI_COMM_WORLD);


	if (direction == 0) {
			qsort(my_chunk, chunk_size, sizeof(int), compare);
	} else {
			qsort(my_chunk, chunk_size, sizeof(int), compare2);
	}

	memcpy(&tmp_buffer[chunk_size * rank], my_chunk,
			sizeof(int) * chunk_size);

	int step_chunk_size = chunk_size;
	int pair;
	for (int step = 1; step < size; step = step << 1) {
		if ((rank & step) != 0) { //send
			pair = rank & ~step;

			MPI_Send(&tmp_buffer[rank * chunk_size], step_chunk_size,
			MPI_INT, pair, 0, MPI_COMM_WORLD);

			break;
		} else { //receive
			pair = rank | step;

			MPI_Recv(&tmp_buffer[pair * chunk_size], step_chunk_size,
			MPI_INT, pair, 0, MPI_COMM_WORLD, &status);

			if (direction == 0) {
				bitonicMergeSort(&tmp_buffer[rank * chunk_size],
					step_chunk_size << 1);
			} else {
				bitonicMergeSort2(&tmp_buffer[rank * chunk_size],
					step_chunk_size << 1);
			}


		}
		step_chunk_size = step_chunk_size << 1;
	}

	if (rank == 0) {
		qsort(a, len, sizeof(int), compare);
		memcpy(a, tmp_buffer, sizeof(int) * len);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	free(my_chunk);
}

void read_data(int n)
{
	FILE* f;
	int i;
	f=fopen("input.txt","r");
	if(f==NULL){
		printf("Data file not found.\n");
		exit(1);
	}
	a=(int*)malloc(n*sizeof(int));

	for(i=0;i<n;i++){
		fscanf(f,"%d",&a[i]);
	}
}

void print_data(int a[],int n)
{
	int i;
	for(i=0;i<n;i++)
		printf("%d ",a[i]);
	printf("\n");
}

void result_output(int index,double result){
    FILE* f;
    f=fopen("aput.txt","a");
    fprintf(f,"%d %f\n",index,result);
}

int main(int argc, char *argv[]) {
	int rank; //rank of process
	int size; //number of processes
	int i,n;
	double start,finish;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Barrier(MPI_COMM_WORLD);

for(i=2;i<20;i++){
	if (rank == 0) {
		n=1<<i;
		read_data(n);
		start=MPI_Wtime();
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	bitonicSort(a, n, rank, size);

	if (rank == 0) {
		finish=MPI_Wtime();
		printf("No.%d: Time cost = %f s\n", i,finish-start);
		result_output(i,finish-start);
		//print_data(a,n);
		free(a);
	}
}
	MPI_Finalize();

	return 0;
}

