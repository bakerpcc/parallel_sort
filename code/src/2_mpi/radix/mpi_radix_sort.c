#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define b 32           // number of bits for integer
#define g 8            // group of bits for each scan
#define N b / g        // number of passes
#define B (1 << g)     // number of buckets, 2^g

// MPI tags constants, offset by max bucket to avoid collisions
#define COUNTS_TAG_NUM  B + 1 

// structure encapsulating buckets with arrays of elements
typedef struct list List;
struct list {
  int* array;
  size_t length;
  size_t capacity;
};


int addItem(List* list, int item) {
  if (list->length >= list->capacity) {
    size_t new_capacity = list->capacity*2;
    int* temp = realloc(list->array, new_capacity*sizeof(int));
    if (!temp) {
      printf("ERROR: Could not realloc for size %d!\n", (int) new_capacity); 
      return 0;
    }
    list->array = temp;
    list->capacity = new_capacity;
  }

  list->array[list->length++] = item;

  return 1;
}

void hintMsg(char* message) {
  fprintf(stderr, "Incorrect usage! %s\n", message);
  fprintf(stderr, "how to use: mpiexec -n [processes] p_radix_sort [f] [n] [r]\n");
  fprintf(stderr, "  [processes] - number of processes to use\n");
  fprintf(stderr, "  [f] - input file to be sorted\n");
}


// Initialize array with numbers read from a file
int read_data(char* file, const int begin, const int n, int *a) {

  // open file in read-only mode and check for errors
  FILE *file_ptr;
  file_ptr = fopen(file, "r");
  if (file_ptr == NULL) {
    return EXIT_FAILURE;
  }

  // read n numbers from a file into array a starting at begin position
  int skip;

  // first skip to the begin position
  for (int i = 0; i < begin; i++) {
    int s = fscanf(file_ptr, "%d", &skip); 
  }

  // then read numbers into array a
  for (int i = 0; i < n; i++) {
    int s = fscanf(file_ptr, "%d", &a[i]);
  }

  return EXIT_SUCCESS;
}

void result_output(int index,double result){
    FILE* f;
    f=fopen("output.txt","a");
    fprintf(f,"%d %f\n",index,result);
}

void print_data(int a[],int n)
{
    int i;
    for(i=0;i<n;i++)
        printf("%d ",a[i]);
    printf("\n");
}

// Compute j bits which appear k bits from the right in x
// Ex. to obtain rightmost bit of x call bits(x, 0, 1)
unsigned bits(unsigned x, int k, int j) {
  return (x >> k) & ~(~0 << j);
}

/*
Radix sort elements while communicating between other MPI processes
a - array of elements to be sorted
buckets - array of buckets, each bucket pointing to array of elements
P - total number of MPI processes
rank - rank of this MPI process
n - number of elements to be sorted
*/

int* radix_sort(int *a, List* buckets, const int P, const int rank, int * n) {
  int count[B][P];   // array of counts per bucket for all processes
  int l_count[B];    // array of local process counts per bucket
  int l_B = B / P;   // number of local buckets per process
  int p_sum[l_B][P]; // array of prefix sums

  // MPI request and status
  MPI_Request req;
  MPI_Status stat;

  for (int pass = 0; pass < N; pass++) {          // each pass

    // init counts arrays
    for (int j = 0; j < B; j++) {
      count[j][rank] = 0;
      l_count[j] = 0;
      buckets[j].length = 0;
    } 

    // count items per bucket
    for (int i = 0; i < *n; i++) {
      unsigned int idx = bits(a[i], pass*g, g);
      count[idx][rank]++; 
      l_count[idx]++;
      if (!addItem(&buckets[idx], a[i])) {
        return NULL;
      }
    }

    // do one-to-all transpose
    for (int p = 0; p < P; p++) {
      if (p != rank) {
        // send counts of this process to others
        MPI_Isend(
            l_count,
            B,
            MPI_INT,
            p,
            COUNTS_TAG_NUM,
            MPI_COMM_WORLD,
            &req);
      }
    }

    // receive counts from others
    for (int p = 0; p < P; p++) {
      if (p != rank) {
        MPI_Recv(
            l_count,
            B,
            MPI_INT,
            p,
            COUNTS_TAG_NUM,
            MPI_COMM_WORLD,
            &stat);

        // populate counts per bucket for other processes
        for (int i = 0; i < B; i++) {
          count[i][p] = l_count[i];
        }
      }
    }

    // calculate new size based on values received from all processes
    int new_size = 0;
    for (int j = 0; j < l_B; j++) {
      int idx = j + rank * l_B;
      for (int p = 0; p < P; p++) {
        p_sum[j][p] = new_size;
        new_size += count[idx][p];
      }
    }

    // reallocate array if newly calculated size is larger
    if (new_size > *n) {
      int* temp = realloc(a, new_size*sizeof(int));
      if (!a) {
        if (rank == 0) {
          printf("ERROR: Could not realloc for size %d!\n", new_size); 
        }
        return NULL;
      }
      // reassign pointer back to original
      a = temp;
    }

    // send keys of this process to others
    for (int j = 0; j < B; j++) {
      int p = j / l_B;   // determine which process this buckets belongs to
      int p_j = j % l_B; // transpose to that process local bucket index
      if (p != rank && buckets[j].length > 0) {
        MPI_Isend(
            buckets[j].array,
            buckets[j].length,
            MPI_INT,
            p,
            p_j,
            MPI_COMM_WORLD,
            &req);
      }
    }

    // receive keys from other processes
    for (int j = 0; j < l_B; j++) {
      // transpose from local to global index 
      int idx = j + rank * l_B; 
      for (int p = 0; p < P; p++) {

        // get bucket count
        int b_count = count[idx][p]; 
        if (b_count > 0) {

          // point to an index in array where to insert received keys
          int *dest = &a[p_sum[j][p]]; 
          if (rank != p) {
            MPI_Recv(
                dest,
                b_count,
                MPI_INT,
                p,
                j,
                MPI_COMM_WORLD,
                &stat);  

          } else {
            // is same process, copy from buckets to our array
            memcpy(dest, &buckets[idx].array[0], b_count*sizeof(int));
          }
        }
      }
    }

    // update new size
    *n = new_size;
  }

  return a;
}

int main(int argc, char** argv)
{
  int rank, size;
  int print_results = 0;
  double start,finish;
  int i;
  List* buckets;
  int* a;
  int* p_n;

  // initialize MPI environment and obtain basic info
  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // check for correct number of arguments
  if (argc < 2)
  {
    if (rank == 0) hintMsg("arguments is not enough.");
    MPI_Finalize();
    return EXIT_FAILURE;
  } 

  // initialize vars and allocate memory
for(i=2;i<20;i++)
{
  int n_total = 1<<i;
  int n = n_total/size;
  if (n < 1) {
    if (rank == 0) {
      hintMsg("number of elements must be >= number of processes!");
    }
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  int isDivisible = B % size;   // in case number of buckets is not divisible
  if (isDivisible > 0) {
    if (rank == 0) {
      hintMsg("number of buckets must be divisible by number of processes\n");
    } 
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  // allocate memory and initialize buckets
  // if n is not divisible by size, make the last process handle the reamainder
  if (rank == size-1) {
    int isDivisible = n_total % size;
    if (isDivisible > 0) {
      n += isDivisible;
    }
  }

  const int s = n * rank;
  a = malloc(sizeof(int) * n);

  int b_capacity = n / B;
  if (b_capacity < B) {
    b_capacity = B;
  }
  buckets = malloc(B*sizeof(List));
  for (int j = 0; j < B; j++) {
    buckets[j].array = malloc(b_capacity*sizeof(int));
    buckets[j].capacity = B;
  }

  // initialize local array
  if (read_data(argv[1], s, n, &a[0]) != EXIT_SUCCESS) {
    printf("File %s could not be opened!\n", argv[1]);
    MPI_Finalize();
    return EXIT_FAILURE;
  }

  // let all processes get here
  MPI_Barrier(MPI_COMM_WORLD);

  // take a timestamp before the sort starts
  //timestamp_type time1, time2;
  if (rank == 0) {
    //get_timestamp(&time1);
    start=MPI_Wtime();
  }

  // then run the sorting algorithm
  a = radix_sort(&a[0], buckets, size, rank, &n);

  if (a == NULL) {
    printf("ERROR: Sort failed, exiting ...\n");
    MPI_Finalize();
    return EXIT_FAILURE;
  }
 
  // wait for all processes to finish before printing results 
  MPI_Barrier(MPI_COMM_WORLD);

  // take a timestamp after the process finished sorting
  if (rank == 0) {
    //get_timestamp(&time2);
    finish = MPI_Wtime();
    // calculate fish updates per second
    //double elapsed = timestamp_diff_in_seconds(time1,time2);
    printf("%f s\n", finish-start);
    printf("%d elements sorted\n", n_total);
    printf("%f elements/s\n", n_total / (finish-start));
	result_output(i,finish-start);
	//print_data(a,n_total);

  }

}
  // release MPI resources
  MPI_Finalize();

  // release memory allocated resources
  for (int j = 0; j < B; j++) {
    free(buckets[j].array);
  }
  free(buckets);
  free(a);
  free(p_n);

  return 0;
}

