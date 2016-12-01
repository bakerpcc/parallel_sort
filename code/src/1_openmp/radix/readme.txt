1) make in cmd
[gcc -g -Wall -fopenmp -o radix_sort radix_sort.c -lm]


2) ./radix_sort <num_of_thread> 

Read data from 'input.txt', which is a 8MB txt file. 
3) We read different size of data in each iteration and compute their time cost.
Write the output result into 'output.txt'
