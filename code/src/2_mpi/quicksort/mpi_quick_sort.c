#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>


int *a;
void read_a(int n);
void print_a(int a[],int n);
void result_output(int a,double b);

//void Generate_list(int a[], int n);
//void print();
void quicksort_parallel(int *a,int start,int end,int m,int id,int rank);
int quicksort(int *a,int start,int end);
int partition(int *a,int start,int end);
int pow2(int num);
int log2(int num);

int main(int argc,char *argv[])
{
	int n;
	int sub_size;
	int rank, size;
	int i, j, k;
	int m, r;
	double starttime,endtime;
	MPI_Status status;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	printf("start %d/%d\n",rank,size);
for(k=0;k<20;k++){
	if(rank==0)
	{	
		sub_size=pow2(k);
		read_a(sub_size);
		//a=(int*)malloc(sub_size*sizeof(int));
		//Generate_list(a,sub_size);
	}
/*
for(i=0;i<sub_size;i++)
		{
			 printf("%d ",a[i]);
		}
printf("\n");
*/


	starttime=MPI_Wtime();
	m=log2(size);
	MPI_Bcast(&sub_size,1,MPI_INT,0,MPI_COMM_WORLD);
	quicksort_parallel(a,0,sub_size-1,m,0,rank);
	endtime=MPI_Wtime();
	if(rank==0)
	{
/*
		for(i=0;i<sub_size;i++)
		{
			 printf("%d ",a[i]);
		}
		printf("\n");
*/
printf("%d: Total time of %d is %e\n",k,rank,endtime-starttime);
result_output(k,endtime-starttime);
	}


}
	MPI_Finalize();

	return 0;
}

void read_a(int n)
{
	FILE* f;
	int i;
	f=fopen("input.txt","r");
	if(f==NULL){
		printf("a file not found.\n");
		exit(1);
	}
	a=(int*)malloc(n*sizeof(int));

	for(i=0;i<n;i++){
		fscanf(f,"%d",&a[i]);
	}
}

void result_output(int a,double b){
	FILE* f;
	f=fopen("output.txt","a");
	fprintf(f,"%d %lf\n",a,b);
}



void print_a(int a[],int n)
{
	int i;
	for(i=0;i<n;i++)
		printf("%d ",a[i]);
	printf("\n");
}

void quicksort_parallel(int *a,int start,int end,int m,int id,int rank)
{
	int i, j;
	int r;
	int MyLength;
	int *tmp;
	MPI_Status status;
	MyLength=-1;
	if(m==0)
	{
		if(rank==id)
		quicksort(a,start,end);
		return;
	}

	if(rank==id)
	{
		r=partition(a,start,end);
		MyLength=end-r;
		MPI_Send(&MyLength,1,MPI_INT,id+pow2(m-1),rank,MPI_COMM_WORLD);
		if(MyLength!=0)
		MPI_Send(a+r+1,MyLength ,MPI_INT,id+pow2(m-1),rank,MPI_COMM_WORLD);

	}

	if(rank==id+pow2(m-1))
	{
		MPI_Recv(&MyLength,1,MPI_INT,id,id,MPI_COMM_WORLD,&status);
		if(MyLength!=0)
		{
			tmp=(int *)malloc(MyLength*sizeof(int));
			if(tmp==0) perror("Malloc memory error!");
			MPI_Recv(tmp,MyLength,MPI_INT,id,id,MPI_COMM_WORLD,&status);
		}
	}
	j=r-1-start;
	MPI_Bcast(&j,1,MPI_INT,id,MPI_COMM_WORLD);
	if(j>0)
		quicksort_parallel(a,start,r-1,m-1,id,rank);
	j=MyLength;
	MPI_Bcast(&j,1,MPI_INT,id,MPI_COMM_WORLD);
	if(j>0)
		quicksort_parallel(tmp,0,MyLength-1,m-1,id+pow2(m-1),rank);
	if((rank==id+pow2(m-1)) && (MyLength!=0))
		MPI_Send(tmp,MyLength,MPI_INT,id,id+pow2(m-1),MPI_COMM_WORLD);
	if((rank==id) && (MyLength!=0))
		MPI_Recv(a+r+1,MyLength,MPI_INT,id+pow2(m-1),id+pow2(m-1),MPI_COMM_WORLD,&status);
}

int quicksort(int *a,int start,int end)
{
	int r;
	int i;
	if(start<end)
	{
		r=partition(a,start,end);
		quicksort(a,start,r-1);
		quicksort(a,r+1,end);
	}
	return 0;
}

int partition(int *a,int start,int end)
{
	int pivot;
	int i, j;
	int tmp;
	pivot=a[end];
	i=start-1;
	for(j=start;j<end;j++)
	if(a[j]<=pivot)
	{
		i++;
		tmp=a[i];
		a[i]=a[j];
		a[j]=tmp;
	}
	tmp=a[i+1];
	a[i+1]=a[end];
	a[end]=tmp;
	return i+1;

}


int pow2(int num)
{
	int result=1<<num;
	return result;
}


int log2(int num)
{
	int i, j;
	i=1;
	j=2;
	while(j<num)
	{
		j=j*2;
		i++;
	}

	if(j>num)
	i--;
	return i;
}

