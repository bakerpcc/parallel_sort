#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<math.h>

int* a;

void read_data(int n);
void print_data(int a[],int n);
void result_output(double a,double b);

int partition(int s[],int l,int r);
void quick_sort_serial(int s[],int l,int r);
void quick_sort_parallel(int s[],int l,int r);


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
		quick_sort_serial(a,0,n-1);
		finish=omp_get_wtime();
		result_output(i,finish-start);		
		printf("binary logarithm of the sequence length: %d serial time cost:%e seconds\n",i,finish-start);
	}

	for(i=0;i<15;i++){
		n=pow(2.0,i);
		read_data(n);
		start=omp_get_wtime();
		quick_sort_parallel(a,0,n-1);
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

int partition(int s[],int l,int r)
{
	int i,j,x;
	if(l<r)
	{
		i=l;
		j=r;
		x=s[i];
		while(i<j){
			while(i<j&&s[j]>x)
				j--;
			if(i<j)
				s[i++]=s[j];
			while(i<j&&s[i]<x)
				i++;
			if(i<j)
				s[j--]=s[i];
		}
		s[i]=x;
	}
	return i;
}

void quick_sort_serial(int s[],int l,int r)
{
	int p;
	if(l<r){
		p=partition(s,l,r);
		quick_sort_serial(s,l,p-1);
		quick_sort_serial(s,p+1,r);
	}
}


void quick_sort_parallel(int s[],int l,int r)
{
	int p;
	if(l<r){
		p=partition(s,l,r);
		#pragma omp parallel 
		{
			#pragma omp sections nowait
			{
				#pragma omp section //section
				quick_sort_parallel(s,l,p-1);
				#pragma omp section //section
				quick_sort_parallel(s,p+1,r);
			}
		}
	}
}

