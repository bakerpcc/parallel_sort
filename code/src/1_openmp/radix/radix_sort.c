#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <omp.h>
#include <math.h>

#define DIGITS_AMOUNT   10    /* number of all possible keys (bucket's size) */

int* a;

void read_data(int n);
void print_data(int a[],int n);
void result_output(int a,double b);
int tenPowers[10] = {
    1,      10,     100,
    1000,       10000,      100000,
    1000000,    10000000,   100000000,
    1000000000,
};

/*
 * Gives power of 10 at certain exp
 */
int p(int exp)
{
    return tenPowers[exp];
}

/*
 * Serial Radix sorting
 */
void radixsort_serial(int *input_array, int input_array_size,
                    int max_digits_size)
{
    int digit;
    int f, i, j;
    double start_time;


    /* 1.1) allocate temporary array */
    int *tmp = (int *)calloc(input_array_size, sizeof(int));
    if (!tmp) {
        fprintf(stderr, "ERROR: not enough memory!\n");
        abort();
    }
    /* 1.2) Shared buckets definition */
    int shared_buckets[max_digits_size][DIGITS_AMOUNT];
    for (j=0; j<max_digits_size; j++)
        for(f=0; f<DIGITS_AMOUNT; f++)
            shared_buckets[j][f]=0;


    for (i=0; i<input_array_size; i++) {

        /* each thread make these step to its pool of values */
        for (j=0; j<max_digits_size; j++) {
            digit = p(j);

            /* 2) each bucket is how many times that digit is read
             *    so we put in shared bucket all infos
             */
            f=input_array[i]/digit%DIGITS_AMOUNT;
            shared_buckets[j][f]++;
        }
    }


    for (j=0; j<max_digits_size; j++) {
        for (i=1; i<DIGITS_AMOUNT; i++)
            shared_buckets[j][i]+=shared_buckets[j][i-1];
    }

    for (j=0; j<max_digits_size; j++) {
        digit = p(j);

        /* 4) order tmp list as shared buckets array says */
        for(i=input_array_size-1;i>=0;i--) {
            int unit = input_array[i]/digit%DIGITS_AMOUNT;
            int pos = --shared_buckets[j][unit];
            tmp[pos]=input_array[i];
        }

        /* 5) copy into input list */
        for(i=0;i<input_array_size;i++)
            input_array[i]=tmp[i];

    }

    cfree(tmp);
}

void radixsort_parallel(int *input_array, int input_array_size,
				int max_digits_size, int chunksize)
{
	int digit;
	int f, i, j;
	double start_time;

	int *tmp = (int *)calloc(input_array_size, sizeof(int));
	if (!tmp) {
		fprintf(stderr, "ERROR: not enough memory!\n");
		abort();
	}
	
	int shared_buckets[max_digits_size][DIGITS_AMOUNT];
	for (j=0; j<max_digits_size; j++) {
		for(f=0; f<DIGITS_AMOUNT; f++)
			shared_buckets[j][f]=0;
	}


	
	#pragma omp parallel for private(digit, j, i, f) schedule(dynamic, chunksize)
	for (i=0; i<input_array_size; i++) {

		/* each thread make these step to its pool of values */
		for (j=0; j<max_digits_size; j++) {
			digit = p(j);

			/* 2) each bucket is how many times that digit is read
			 *    so we put in shared bucket all infos
			 */
			f=input_array[i]/digit%DIGITS_AMOUNT;
			#pragma omp atomic
			shared_buckets[j][f]++;
		}
	}
	

	
	for (j=0; j<max_digits_size; j++) {
		for (i=1; i<DIGITS_AMOUNT; i++)
			shared_buckets[j][i]+=shared_buckets[j][i-1];
	}

	for (j=0; j<max_digits_size; j++) {
		digit = p(j);

		/* 4) order tmp list as shared buckets array says */
		for(i=input_array_size-1;i>=0;i--) {
			int unit = input_array[i]/digit%DIGITS_AMOUNT;
			int pos = --shared_buckets[j][unit];
			tmp[pos]=input_array[i];
		}
		/* 5) copy into input list */
		for(i=0;i<input_array_size;i++)
			input_array[i]=tmp[i];
		
	}

	cfree(tmp);
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

void result_output(int a,double b){
    FILE* f;
    f=fopen("output.txt","a");
    fprintf(f,"%d %e\n",a,b);
}

int main(int argc, char **argv)
{
    int i, max_value = 0, tmp_value = 0;
    int value_lenght = 5, max_digits_size = 0;
    double num_of_values=65536;
    double start_time;
    double start,finish;
    int chunksize=1;
    int j;

    char str[10] = {0};
    char layout[80];
    

    for(j=0;j<15;j++){
    num_of_values=pow(2.0,j);
//printf("%f\n",num_of_values);
    read_data(num_of_values);

    for(i=0;i<value_lenght;i++)
        str[i] = '9';
//printf("step1\n");
    
    for(i=0;i<num_of_values; i++) {
        if (max_value < a[i])
            max_value = a[i];
    }
//printf("step2 %d\n",max_value);

    tmp_value = max_value;
    while((tmp_value/=10)>0)
            {max_digits_size++;}
        max_digits_size++;
//printf("step3 %d\n",max_digits_size);

    start = omp_get_wtime();
    radixsort_serial(a, num_of_values, max_digits_size);
    finish = omp_get_wtime();
//printf("step4\n");

    //print_data(a,num_of_values);
    
    result_output(j,finish-start);
    printf("binary logarithm of the sequence length: %d serial time cost:%e seconds\n",j,finish-start);
    max_digits_size=0;
    }

    max_value=0;
    for(j=0;j<15;j++){
    num_of_values=pow(2.0,j);
//printf("%f\n",num_of_values);
    read_data(num_of_values);

    for(i=0;i<value_lenght;i++)
        str[i] = '9';
//printf("step1\n");
    
    for(i=0;i<num_of_values; i++) {
        if (max_value < a[i])
            max_value = a[i];
    }
//printf("step2 %d\n",max_value);

    tmp_value = max_value;
    while((tmp_value/=10)>0)
            {max_digits_size++;}
        max_digits_size++;
//printf("step3 %d\n",max_digits_size);

    start = omp_get_wtime();
    radixsort_parallel(a, num_of_values, max_digits_size,chunksize);
    finish = omp_get_wtime();
//print_data(a,num_of_values);
//printf("step4\n");

    //print_data(a,num_of_values);
    
    result_output(j,finish-start);
    printf("binary logarithm of the sequence length: %d parallel time cost:%e seconds\n",j,finish-start);
    max_digits_size=0;
    }

    return 0;
}

