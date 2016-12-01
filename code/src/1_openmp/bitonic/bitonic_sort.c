#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>

int* a;
int thread_count=2;
#define MINPOW 16
#define MINK (1<<MINPOW)
#define SIZE (1<<(MINPOW-1))

const int ASCENDING  = 1;
const int DESCENDING = 0;

void read_data(int n);
void print_data(int a[],int n);
void result_output(double a,double b);

void sort(int n);
void exchange(int i, int j);
void compare(int i, int j, int dir);
void bitonicMerge(int lo, int cnt, int dir);
void recBitonicSort(int lo, int cnt, int dir);
void impBitonicSort(int n);
void PimpBitonicSort(int n);

void sort_p(int n);
void bitonicMerge_p(int lo, int cnt, int dir);
void recBitonicSort_p(int lo, int cnt, int dir);


int main(int argc,char* argv[])

{
	double start,finish;
	double n;
	int i;
	int *temp;
	if(argv[1]!=NULL) thread_count = strtol(argv[1], NULL, 10);
	printf("thread_count:%d\n",thread_count); 	
	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		sort(n);
		finish=omp_get_wtime();
		result_output(i,finish-start);		
		printf("binary logarithm of the sequence length: %d serial time cost:%e seconds\n",i,finish-start);
	}

	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		sort_p(n);
		finish=omp_get_wtime();
		result_output(i,finish-start);
		
		printf("binary logarithm of the sequence length: %d parallel time cost:%e seconds\n",i,finish-start);
	}



	return 0;
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

void result_output(double a,double b){
	FILE* f;
	f=fopen("output.txt","a");
	fprintf(f,"%f %e\n",a,b);
}



void print_data(int a[],int n)
{
	int i;
	for(i=0;i<n;i++)
		printf("%d ",a[i]);
	printf("\n");
}

int desc( const void *a, const void *b ){
  int* arg1 = (int *)a;
  int* arg2 = (int *)b;
  if( *arg1 > *arg2 ) return -1;
  else if( *arg1 == *arg2 ) return 0;
  return 1;
}
int asc( const void *a, const void *b ){
  int* arg1 = (int *)a;
  int* arg2 = (int *)b;
  if( *arg1 < *arg2 ) return -1;
  else if( *arg1 == *arg2 ) return 0;
  return 1;
}

void compare(int i, int j, int dir) {
  if (dir==(a[i]>a[j]))
    exchange(i,j);
}

void exchange(int i, int j) {
  int t;
  t = a[i];
  a[i] = a[j];
  a[j] = t;
}


void bitonicMerge(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    int i;
    for (i=lo; i<lo+k; i++)
      compare(i, i+k, dir);
    bitonicMerge(lo, k, dir);
    bitonicMerge(lo+k, k, dir);
  }
}

void bitonicMerge_p(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    int i;
    for (i=lo; i<lo+k; i++)
      compare(i, i+k, dir);
	#pragma omp parallel 
	{
		#pragma omp sections nowait
		{
	#pragma omp section
    bitonicMerge_p(lo, k, dir);
	#pragma omp section
    bitonicMerge_p(lo+k, k, dir);
		}
	}
  }
}

void recBitonicSort(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
    recBitonicSort(lo, k, ASCENDING);
    recBitonicSort(lo+k, k, DESCENDING);
    bitonicMerge(lo, cnt, dir);
  }
}

void recBitonicSort_p(int lo, int cnt, int dir) {
  if (cnt>1) {
    int k=cnt/2;
	#pragma omp parallel 
	{
		#pragma omp sections nowait
		{
	#pragma omp section
    recBitonicSort_p(lo, k, ASCENDING);
	#pragma omp section
    recBitonicSort_p(lo+k, k, DESCENDING);
		}
	}
    bitonicMerge_p(lo, cnt, dir);
  }
}

/** function sort()
       Caller of recBitonicSort for sorting the entire array of length N
          in ASCENDING order
**/
void sort(int n) {
  recBitonicSort(0, n, ASCENDING);
}

void sort_p(int n) {
  recBitonicSort_p(0, n, ASCENDING);
}




