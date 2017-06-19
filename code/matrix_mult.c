// Alexey Muryshkin
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ROWS 1025
#define MAX_BUFFER 2050
#define MAX_COLS MAX_BUFFER/2

int *m1[MAX_ROWS];
int *m2[MAX_ROWS];

int *res1[MAX_ROWS];	// row sequential
int *res2[MAX_ROWS];	// row threading
int *res3[MAX_ROWS];	// col threading
int *res4[MAX_ROWS];	// col sequential

pthread_mutex_t	 mutex;

int m1rows, m1cols, m2rows, m2cols, resrows, rescols;

long int row_matrix_mult() {
	
	int i, j, k;
	struct timeval ti, tf;
	gettimeofday(&ti, NULL);

	for ( i = 0; i < m1rows; i++ )
		for ( j = 0; j < m2cols; j++ )
			for ( k = 0; k < m1cols; k++ ) {
				res1[i][j] += m1[i][k] * m2[k][j];
			}
	
	gettimeofday(&tf, NULL);

	return (long int)((tf.tv_sec - ti.tv_sec) * 1000000L + tf.tv_usec - ti.tv_usec);
} 

long int col_matrix_mult() {
	
	int i, j, k;
	struct timeval ti, tf;
	gettimeofday(&ti, NULL);
	
	for ( k = 0; k < m1cols; k++ )
		for ( i = 0; i < m1rows; i++ )
			for ( j = 0; j < m2cols; j++ ) {
				res4[i][j] += m1[i][k] * m2[k][j];
			}
	
	gettimeofday(&tf, NULL);

	return (long int)((tf.tv_sec - ti.tv_sec) * 1000000L + tf.tv_usec - ti.tv_usec);
} 

void *row_mult(void *tid) {
	
	int j, k, i = (int)tid;

	for ( j = 0; j < m2cols; j++ )
		for ( k = 0; k < m1cols; k++ )
			res2[i][j] += m1[i][k] * m2[k][j];

	pthread_exit(NULL);
}

void *col_mult(void *tid) {
	
	pthread_mutex_lock( &mutex );

	int i, j, k = (int)tid;

	for ( i = 0; i < m1rows; i++ )
		for ( j = 0; j < m2cols; j++ )
			res3[i][j] += m1[i][k] * m2[k][j];
	
	pthread_mutex_unlock( &mutex );
	pthread_exit(NULL);
}

long int prll_row_matrix_mult() {
	
	pthread_t threads[m1rows];
	int status, i;	
	
	struct timeval ti, tf;
	gettimeofday(&ti, NULL);
	
	for ( i = 0; i < m1rows; i++) {
		status = pthread_create(&threads[i], NULL, row_mult, (void *)i);
		if (status != 0) {
			printf("Oops. pthread_create returned error code %d\n", status);
		}
	}

	for (int i = 0; i < m1rows; i++)
       pthread_join(threads[i], NULL);	

	gettimeofday(&tf, NULL);
	return (long int)((tf.tv_sec - ti.tv_sec) * 1000000L + tf.tv_usec - ti.tv_usec);
}

long int prll_col_matrix_mult() {
	
	pthread_t threads[m1cols];
	int status, k;	
	
	struct timeval ti, tf;
	gettimeofday(&ti, NULL);
	
	pthread_mutex_init( &mutex, NULL );

	for ( k = 0; k < m1cols; k++) {
		status = pthread_create(&threads[k], NULL, col_mult, (void *)k);
		if (status != 0) {
			printf("Oops. pthread_create returned error code %d\n", status);
		}
	}

	for (int k = 0; k < m1cols; k++)
       pthread_join(threads[k], NULL);	
	
	pthread_mutex_destroy( &mutex );

	gettimeofday(&tf, NULL);
	return (long int)((tf.tv_sec - ti.tv_sec) * 1000000L + tf.tv_usec - ti.tv_usec);
}

void read_matrix( char* filename, int **m, int *mrows, int *mcols) {
	/*
	// opening the input file
	int fd = open(filename, O_RDONLY);
	if ( fd == -1 ) {
		perror( "Cannot open the file");
		exit(-1);
	}
	
	FILE* fp = fdopen(fd, "r");
	*/
	FILE* fp = fopen(filename, "r");
	char s[MAX_BUFFER + 1];
	*mrows = 0;

	while ( fgets(s, MAX_BUFFER, fp) != NULL && *mrows < MAX_ROWS ) {
    	char *token;
  		*mcols = 0; 
		m[*mrows] = (int *) malloc( MAX_COLS * sizeof(int) );		
		
   		// get the first token 
   		token = strtok(s, " ");
		
		//printf("%d\n", *mrows);

   		// walk through other tokens
   		while( token != NULL && *mcols < MAX_COLS && (int)token[0] != 10 ) {
			m[*mrows][*mcols] = atoi(token);
			//printf("%d ", (int)token[0]);
			//printf("%d ", m[*mrows][*mcols]);
			//printf("%d ", *mcols);
			(*mcols)++;
			token = strtok(NULL, " ");
   		}
		
		//printf("\n");
		(*mrows)++;
    }

	//printf("\n\n");
	
	//close(fd);
	fclose(fp);
}

int main( int argc, char **argv ) {

	if ( argc != 3 )
	{
		printf( "Usage: mm matrix1 matrix2\n" );
		exit(-1);
	}

	read_matrix( argv[1], m1, &m1rows, &m1cols );
	read_matrix( argv[2], m2, &m2rows, &m2cols );
	
	if ( m1cols != m2rows )
	{
		printf( "Cannot multiply these matrices: %dX%d * %dX%d\n", 
				m1rows, m1cols, m2rows, m2cols );
		exit(-1);
	}
	resrows = m1rows;
	rescols = m2cols;
	
	int i, j, k;

	// Zero out the result matrix
	for ( i = 0; i < resrows; i++ )
	{
		res1[i] = (int *) malloc( rescols * sizeof(int) );
		res2[i] = (int *) malloc( rescols * sizeof(int) );	
		res3[i] = (int *) malloc( rescols * sizeof(int) );
		res4[i] = (int *) malloc( rescols * sizeof(int) );	
		for ( j = 0; j < rescols; j++ )
			res1[i][j] = res2[i][j] = res3[i][j] = res4[i][j] = 0;
	}
	
	// Matrix Multiplication Stage
	long int t1, t2, t3, t4;

	t1 = row_matrix_mult();
	t2 = prll_row_matrix_mult();
	t3 = prll_col_matrix_mult();	
	t4 = col_matrix_mult();
/*
	for ( i = 0; i < resrows; i++ ) {
		for ( j = 0; j < rescols; j++ ) {
			printf("%3d ", res1[i][j]);
		}	
		printf("\n");
	}
	
	for ( i = 0; i < 15; i++)
		printf("^");
	printf("\n");
*/
	//printf("time of row_matrix_mult is %ld\n", t1);
/*
	for ( i = 0; i < 25; i++)
		printf("#");
	printf("\n");

	for ( i = 0; i < resrows; i++ ) {
		for ( j = 0; j < rescols; j++ ) {
			printf("%3d ", res2[i][j]);
		}	
		printf("\n");
	}

	for ( i = 0; i < 15; i++)
		printf("^");
	printf("\n");
*/
	//printf("time of prll_row_matrix_mult is %ld\n", t2);
/*
	for ( i = 0; i < 25; i++)
		printf("#");
	printf("\n");

	for ( i = 0; i < resrows; i++ ) {
		for ( j = 0; j < rescols; j++ ) {
			printf("%3d ", res3[i][j]);
		}	
		printf("\n");
	}
	
	for ( i = 0; i < 15; i++)
		printf("^");
	printf("\n");
*/
	//printf("time of prll_col_matrix_mult is %ld\n", t3);	
/*
	for ( i = 0; i < 25; i++)
		printf("#");
	printf("\n");
*/
	for ( i = 0; i < resrows; i++ ) {
		for ( j = 0; j < rescols; j++ ) {
			printf("%d ", res4[i][j]);
		}	
		printf("\n");
	}
/*
	for ( i = 0; i < 15; i++)
		printf("^");
	printf("\n");
*/
	//printf("time of col_matrix_mult is %ld\n", t4);

	return 0;
}
