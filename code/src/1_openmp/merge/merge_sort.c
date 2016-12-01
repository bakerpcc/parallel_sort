#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>

int* a;

void read_data(int n);
void print_data(int a[],int n);
void result_output(double a,double b);

void merge_sort(int a[],int b[],int n);
void merge_sort_p(int a[],int b[],int n);
void merge_sort_split(int a[],int begin,int end,int b[]);
void merge_sort_split_p(int a[],int begin,int end,int b[]);
void merge(int a[],int begin,int middle,int end,int b[]);
void copy(int b[],int begin,int end,int a[]);


int thread_count=2;

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
		//printf("%d\n",n);
		read_data(n);
		start=omp_get_wtime();
		temp=(int*)malloc(n*sizeof(int));
		merge_sort(a,temp,n);		
		finish=omp_get_wtime();
		result_output(i,finish-start);		
		printf("binary logarithm of the sequence length: %d serial time cost:%e seconds\n",i,finish-start);
	}

	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		temp=(int*)malloc(n*sizeof(int));
		merge_sort_p(a,temp,n);		
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
//		printf("%d ",a[i]);
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

//merge_sort
void merge_sort(int a[],int b[],int n){
	merge_sort_split(a,0,n,b);
}

void merge_sort_split(int a[],int begin,int end,int b[]){
	int middle=(begin+end)/2;
	if(end-begin<2) return;
	merge_sort_split(a,begin,middle,b);
	merge_sort_split(a,middle,end,b);
	merge(a,begin,middle,end,b);
	copy(b,begin,end,a);
}

void merge_sort_p(int a[],int b[],int n){
	merge_sort_split_p(a,0,n,b);
}

void merge_sort_split_p(int a[],int begin,int end,int b[]){
	int middle=(end+begin)/2;
	if(end-begin<2) return;
#pragma omp parallel num_threads(thread_count)
	{
#pragma omp sections
		{
#pragma omp section
			merge_sort_split(a,begin,middle,b);
#pragma omp section
			merge_sort_split(a,middle,end,b);
		}
	}
	merge(a,begin,middle,end,b);
	copy(b,begin,end,a);
}

void merge(int a[],int begin,int middle,int end,int b[])
{
	int i=begin,j=middle;
	int k;
	for(k=begin;k<end;k++)
	{
		if(i<middle&&(j>=end||a[i]<=a[j])){
			b[k]=a[i];
			i++;
		}
		else{
			b[k]=a[j];
			j++;
		}
	}
}

void copy(int b[],int begin,int end,int a[]){
	int k;
	for(k=begin;k<end;k++)
		a[k]=b[k];
}
