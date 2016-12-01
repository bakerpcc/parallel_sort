#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>

int* a;

void read_data(int n);
void print_data(int a[],int n);
void result_output(double a,double b);
//add oddeven
void odd_even_serial(int a[],int n);
void odd_even_parallel(int a[],int n);


int thread_count=2;

int main(int argc,char* argv[])

{
	double start,finish;
	double n;
	int i;

	if(argv[1]!=NULL) thread_count = strtol(argv[1], NULL, 10);
	printf("thread_count:%d\n",thread_count); 	
	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		odd_even_serial(a,n);
		finish=omp_get_wtime();
		result_output(i,finish-start);		
		printf("binary logarithm of the sequence length: %d serial time cost:%e seconds\n",i,finish-start);
	}

	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		odd_even_parallel(a,n);		
		finish=omp_get_wtime();
		result_output(i,finish-start);
		
		printf("binary logarithm of the parallel length: %d parallel time cost:%e seconds\n",i,finish-start);
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

//odd_even
void odd_even_serial(int a[], int n) {
   int phase, i, tmp;
   for (phase = 0; phase < n; phase++) {
      if (phase % 2 == 0)
         for (i = 1; i < n; i += 2) {
            if (a[i-1] > a[i]) {
               tmp = a[i-1];
               a[i-1] = a[i];
               a[i] = tmp;
            }
         }
      else
         for (i = 1; i < n-1; i += 2) {
            if (a[i] > a[i+1]) {
               tmp = a[i+1];
               a[i+1] = a[i];
               a[i] = tmp;
            }
         }
   }
} 

void odd_even_parallel(int a[], int n) {
   int phase, i, tmp;
#        pragma omp parallel num_threads(thread_count) \
            default(none) shared(a, n) private(i, tmp, phase)
   for (phase = 0; phase < n; phase++) {
      if (phase % 2 == 0)
#        pragma omp for
         for (i = 1; i < n; i += 2) {
            if (a[i-1] > a[i]) {
               tmp = a[i-1];
               a[i-1] = a[i];
               a[i] = tmp;
            }
         }
      else
#        pragma omp for
         for (i = 1; i < n-1; i += 2) {
            if (a[i] > a[i+1]) {
               tmp = a[i+1];
               a[i+1] = a[i];
               a[i] = tmp;
            }
         }
   }
}

