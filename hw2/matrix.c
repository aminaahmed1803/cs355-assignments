#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define N 10

pthread_mutex_t lock; 
int MAX_ROW_SUM = 0;
int* A;
int* B;
int* C;

struct write_to_matrix{
   int i;
   int j;
};

/*
 * Function:  fill_matrices
 * --------------------
 * fills matrix with randomly
 * generated numbers
 */
void fill_matrices(struct write_to_matrix* data){
     int idx = (data->i*N) + data->j;
     struct timeval tv;
     gettimeofday(&tv,NULL);
     unsigned int seed = tv.tv_usec;
     A[idx] = (rand_r(&seed) % 10) + 1;
     B[idx] = (rand_r(&seed) % 10) + 1;
}

/*
 * Function:  multiply
 * --------------------
 * multiplies 2 matrices
 */
void multiply(struct write_to_matrix* data){
     int sum=0;
     for (int k=0; k<N ; k++){
          sum += A[(data->i*N) + k]*B[(k*N) + data->j];
     }
     C[(data->i*N) + data->j] = sum;
}

/*
 * Function:  find_max_row_sum
 * --------------------
 * finds the max row sum 
 * of a given matrix
 */
void find_max_row_sum(int i){
     int sum=0;
     for (int j=0; j<N ; j++){
          sum += C[(i*N)+ j];
     }
     pthread_mutex_lock(&lock);
     if (sum > MAX_ROW_SUM){
          MAX_ROW_SUM = sum;
     }
     pthread_mutex_unlock(&lock); 
}

void print_matrix(int* matrix){
     for (int i=0; i<N ; i++){
          for (int j=0 ; j<N ; j++){
               int idx = (i*N) + j;
               printf("%d ", matrix[idx]);
          }
          printf("\n");
     } printf("\n");
}

int main()
{
     // memory assigment
     A = malloc((N * N) * sizeof(int));
     B = malloc((N * N) * sizeof(int));
     C = malloc((N * N) * sizeof(int));
     pthread_t* tids = malloc(sizeof(pthread_t) * N * N);
     struct write_to_matrix* struct_data = (struct write_to_matrix *)malloc((N * N) *sizeof(struct write_to_matrix));
     
     // threaded for randomizing 

     for (int i=0; i<N ; i++){
          for (int j=0 ; j<N ; j++){
               int idx = (i*N) + j;
               struct_data[idx].i = i;
               struct_data[idx].j = j;
               pthread_create(&tids[idx], NULL, (void*) fill_matrices, (void*) &struct_data[idx]);
          }
     }

     for (int i=0; i<N ; i++){
          for (int j=0 ; j<N ; j++){
               int idx = (i*N) + j;
               pthread_join(tids[idx], NULL);
          }
     }

     //printed
     printf("Matrix A:\n"); print_matrix(A);
     printf("Matrix B:\n");print_matrix(B);

     // threaded for multiply

     for (int i=0; i<N ; i++){
          for (int j=0 ; j<N ; j++){
               int idx = (i*N) + j;
               struct_data[idx].i = i;
               struct_data[idx].j = j;
               pthread_create(&tids[idx], NULL, (void*) multiply, (void*) &struct_data[idx]);
          }
     }

     for (int i=0; i<N ; i++){
          for (int j=0 ; j<N ; j++){
               int idx = (i*N) + j;
               pthread_join(tids[idx], NULL);
          }
     }

     printf("Matrix C:\n"); print_matrix(C);

     free(tids);
     free(struct_data);
     tids = malloc(sizeof(pthread_t) * N);
     
     // compute the sum of elements in each row of C
     for (int i=0; i<N ; i++){
          pthread_create(&tids[i], NULL, (void*) find_max_row_sum, (void *) i);
     }

     for (int i=0 ; i<N ; i++){
          pthread_join(tids[i], NULL);
     }

     printf("Max Row Sum: %d\n\n", MAX_ROW_SUM);

     free(A);
     free(B);
     free(C);
     free(tids);
 
    return 0;
}