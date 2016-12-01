#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int *a;
void read_data(int n);
void print_data(int a[],int n);
void result_output(int a,double b);

void merge(int *, int *, int, int, int);
void mergeSort(int *, int *, int, int);

int main(int argc, char** argv) {

    double start,finish;
    int i,n,c;

    int world_rank;
    int world_size;
    
    MPI_INIT(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
        

for(i=0;i<20;i++){
    n=1<<i;
    read_data(n);
	//printf("size:%d\n",n);
	//printf("worldsize:%d\n",world_size);
    int size = n/world_size;
    
    //Send each subarray to each process
    start=MPI_Wtime();
    int *sub_array = malloc(size * sizeof(int));
    MPI_Scatter(a, size, MPI_INT, sub_array, size, MPI_INT, 0, MPI_COMM_WORLD);
/*
    printf("This is the subset array: ");
    for(c = 0; c < size; c++) {     
        printf("%d ", sub_array[c]);        
        }   
    printf("\n");
*/
    //Perform the mergesort on each process
    int *tmp_array = malloc(size * sizeof(int));
    mergeSort(sub_array, tmp_array, 0, (size - 1));

/*
    printf("This is the ordered subset array: ");
    for(c = 0; c < size; c++) {     
        printf("%d ", sub_array[c]);        
        }   
    printf("\n");
*/    
    //Gather the sorted subarrays into one array
    int *sorted = NULL;
    if(world_rank == 0) {      
        sorted = malloc(n * sizeof(int));     
        }
    
    MPI_Gather(sub_array, size, MPI_INT, sorted, size, MPI_INT, 0, MPI_COMM_WORLD);
    
    //do final mergeSort in rank 0 process
    if(world_rank == 0) {        
        int *other_array = malloc(n * sizeof(int));
        mergeSort(sorted, other_array, 0, (n - 1));
        
        //print the sorted array
/*
        printf("This is the sorted array: ");
        for(c = 0; c < n; c++) {
            
            printf("%d ", sorted[c]);
            
            }
            
        printf("\n");
*/            
        //do free

        free(sorted);
        free(other_array);
        finish=MPI_Wtime();
    	printf("No.%d :mpi total time is %lf\n",i,finish-start);  
	    result_output(i,finish-start);  
        }
    
    free(a);
    free(sub_array);
    free(tmp_array);
    MPI_Barrier(MPI_COMM_WORLD);

	}
	//Finalize MPI
    MPI_Finalize();

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
//      printf("%d ",a[i]);
    }
}

void result_output(int a,double b){
    FILE* f;
    f=fopen("output.txt","a");
    fprintf(f,"%d %e\n",a,b);
}



void print_data(int a[],int n)
{
    int i;
    for(i=0;i<n;i++)
        printf("%d ",a[i]);
    printf("\n");
}


void merge(int *a, int *b, int l, int m, int r) {   
    int h, i, j, k;
    h = l;
    i = l;
    j = m + 1;   
    while((h <= m) && (j <= r)) {        
        if(a[h] <= a[j]) {            
            b[i] = a[h];
            h++;            
            }           
        else {           
            b[i] = a[j];
            j++;            
            }           
        i++;       
        }       
    if(m < h) {        
        for(k = j; k <= r; k++) {           
            b[i] = a[k];
            i++;           
            }          
        }       
    else {        
        for(k = h; k <= m; k++) {           
            b[i] = a[k];
            i++;          
            }          
        }      
    for(k = l; k <= r; k++) {       
        a[k] = b[k];       
        }        
}

void mergeSort(int *a, int *b, int l, int r) {   
    int m;    
    if(l < r) {        
        m = (l + r)/2;        
        mergeSort(a, b, l, m);
        mergeSort(a, b, (m + 1), r);
        merge(a, b, l, m, r);
        }
}
