#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
 * int main(int argc, char* argv[])
 *
 * program to find all numbers between 2 and N that are divisible by at least one 
 * of the 3 numbers passed to the function. The MPI functions utilized in communication 
 * are MPI_Bcast, MPI_Gatherv, & MPI_Reduce.
 */
int main(int argc, char *argv[]) {
	// Initialize MPI, and get rank & number processes
	int rank, size;
	MPI_Init(NULL, NULL);
	MPI_Comm MCW = MPI_COMM_WORLD;

	MPI_Comm_rank(MCW, &rank);
	MPI_Comm_size(MCW, &size);

        unsigned int a, b, c, n;
        unsigned int i; //loop index
        FILE * fp; //for creating the output file
        char filename[100]=""; // the file name
        int * x; //the numbers in the range [2, N]

        clock_t start_p1, start_p3, end_p1,  end_p3;


        // start of part 1

        start_p1 = clock();


        // Check that the input from the user is correct.
        if(argc != 5){

                printf("usage:  ./checkdiv N a b c\n");
                printf("N: the upper bound of the range [2,N]\n");
                printf("a: first divisor\n");
                printf("b: second divisor\n");
                printf("c: third divisor\n");
                exit(1);
        }
	
	// process 0 should read in the args
	if (rank == 0) {
        	n = (unsigned int)atoi(argv[1]);
        	a = (unsigned int)atoi(argv[2]);
        	b = (unsigned int)atoi(argv[3]);
        	c = (unsigned int)atoi(argv[4]);
		
		x = (int *) calloc(n-1 ,sizeof(int));
	}

	// Process 0 must now send the a, b, c, and n to each process
	// since MIPI_Bcast is blocking, process 0 will time how long 
	// the broadcast to all processes will take.
	MPI_Bcast(&a, 1, MPI_INT, 0, MCW);
	MPI_Bcast(&b, 1, MPI_INT, 0, MCW);
	MPI_Bcast(&c, 1, MPI_INT, 0, MCW);
	MPI_Bcast(&n, 1, MPI_INT, 0, MCW);
	end_p1 = clock();
	// end part 1

	// start part 2. Note that the time for this section is the min start time and max end time.
	// Other processes must, after receiving the variables, calculate their own range.
	double start_p2 = MPI_Wtime();

	// calculate the range for each process.
	int quotient, remainder, my_first, my_count;
	div_t d = div(n-1, size);
		
	quotient =  d.quot;
	remainder = d.rem;
	
	if (rank < remainder){
		my_count = quotient + 1;
		my_first = rank * my_count;
	}
	else {
		my_count = quotient;
		my_first = (rank * my_count) + remainder;
	}
	
	// We need to calculate the start points and displacements to use MPI_Gatherv.
	// These are calculated and stored in arrays.	
	int start = 0;
	int * displace = (int *)calloc(size, sizeof(int));
	int * counts = (int *)calloc(size, sizeof(int));
	for (int i = 0; i < size; i++) {
		displace[i] = start;
		if (i < remainder){
			counts[i] = quotient + 1;
                	start += quotient + 1;
        	}
        	else {
                	counts[i] = quotient;
                	start += quotient;
        	}	
	}
	
	int * my_x = (int *) calloc(my_count, sizeof(int));
	
	// each array calculates its own list
	if (rank == 0) {
		int * my_x = (int *) calloc(my_count, sizeof(int));
		
		// compute divisible elements
		for (int i = 0; i < my_count; i++) {
			if ( (i+2) % a == 0 || (i+2) % b == 0 || (i+2) % c == 0) {
				my_x[i] = 1;
			}
		}

		MPI_Gatherv(my_x, my_count, MPI_INT, x, counts, displace, MPI_INT, 0, MCW);		
	}
	else {
		int * my_x = (int *) calloc(my_count, sizeof(int));

                // compute divisable elements.
		for (int i = 0; i < my_count; i++) {
                	if ( (my_first + i + 2) % a == 0 || (my_first + i + 2) % b == 0 || (my_first + i + 2) % c == 0) {
                        	my_x[i] = 1;
			}
		}

		// send to process 0 
		MPI_Gatherv(my_x, my_count, MPI_INT, NULL, counts, displace, MPI_INT, 0, MCW);
	}
	
	free(my_x);
	
	// end of part 2
	double end_p2 = MPI_Wtime();

	// reduce using MPI_Reduce - MPI_MAX & MPI_MIN
	double max_time;
	double my_time = end_p2 - start_p2; 
	
	
	MPI_Reduce(&my_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MCW);
	
	
        //start of part 3
	if (rank == 0) {
		// start timer
		start_p3 = clock();
        	
		strcpy(filename, argv[1]);
        	strcat(filename, ".txt");
		
		if( !(fp = fopen(filename,"w")))
        	{
                	printf("Cannot create file %s\n", filename);
                	exit(1);
        	}	
		
		for(i = 0; i <= n-2; i++)
                {
			if(x[i]){				
                        	fprintf(fp, "%d ", i + 2);
                	}
        	}
		
		if (x == NULL) {printf("HOW IS IT NULL?? \n");}
		free(x);
		
		fclose(fp);	
        	end_p3 = clock();
        	//end of part 3. Close timer.
	
		double time_1 = (end_p1 - start_p1) / CLOCKS_PER_SEC;
		double time_2 = max_time;
		double time_3 = (end_p3 - start_p3) / CLOCKS_PER_SEC; 
        	
		/* Print the times of the three parts*/
		printf("Time 1 = %f Time 2 = %f Time 3 = %f \n", time_1, time_2, time_3);		
	}
	
	// finalize and return
	MPI_Finalize();
        return 0;
}

