#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include <CL/cl_ext.h>

#define N 256
#define M 256

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;

////////////////////////////////////////////////////////////////////////////////

void set_char_main(unsigned int  * array, int index, unsigned char val){
	switch(val){
		case 'A' : array[index / 16] |= (0 << ((index % 16) * 2));
		break;
		case 'C' : array[index / 16] |= (1 << ((index % 16) * 2));
		break;
		case 'G' : array[index / 16] |= (3 << ((index % 16) * 2));
		break;
		case 'T' : array[index / 16] |= (2 << ((index % 16) * 2));
		break;
	}

}

short get(char data[], int key) {
				const int position = (key % 4) * 2;
				key /= 4;
				char mask = 0;
				mask &= 00000000;

				mask |= 3 << position;

				char fin_mask =0;
				fin_mask&= 00000000;

				fin_mask |= 3 << 0;
				return (((data[key] & mask) >> ( position)) & fin_mask);

			}


unsigned short* order_matrix_blocks(unsigned short* in_matrix){
	unsigned short * out_matrix = (unsigned short*)malloc(sizeof(unsigned short) * N * M);

	int num_diag = 0;
	int store_elem = 1;
	int store_index;
	int tmp_i = 0;
	int tmp_j = 0;
	int i = 0;
	int j;

	for(i = 0; i<(M + N - 1) * N;){
	//while(store_elem != 0){
		if(num_diag < M){
			tmp_j = num_diag;
			tmp_i = 0;
		}else{
			tmp_i = (num_diag + 1) - M;
			tmp_j = M - 1;
		}

		for(j = 0; j < store_elem; j++){
			store_index = tmp_j * N + tmp_i;
			out_matrix[store_index] = in_matrix[i];
			//printf("stored %d in index %d \n", out_matrix[store_index], store_index);
			tmp_j--;
			tmp_i++;
			i++;
		}
		num_diag++;
		if(num_diag >= M){
			store_elem--;
			i = num_diag * N + N - store_elem;
		}else{
			if(store_elem != N){
				store_elem++;
				i = num_diag * N;
			}
		}
	}


	return out_matrix;
}

void compute_matrices_sw_3(char *a, char *database,
		int *matrix, short * directionMatrixSW){

	//TestBench
    int north = 0;
      int west = 0;
      int northwest = 0;


      int maxValue = 0;
      int localMaxIndex = 0;

      int val = 0;
      short dir = 0;
      int maxIndexSw = 0;


  //calculate my own SW
  for(int i = 0; i < N * M; i++){
    val = 0;
    dir=CENTER;
    if( i == 0){
      north = 0;
      northwest = 0;
      west = 0;
    }else if (i / N == 0){ //first row
      north = 0;
      northwest = 0;
      west = matrix[i - 1];
    }else if(i % N == 0){ // first col
      west = 0;
      northwest = 0;
      north = matrix[i - N];
    }else{
      west = matrix[i - 1];
      north = matrix[i - N];
      northwest = matrix [i - N - 1];
    }


    //all set, compute
    int jj = i / N;
    int ii = i % N;

    const short match = (a[ii] == database[jj]) ?  MATCH : MISS_MATCH;
    int val1 = northwest + match;
    /*
    if(i == 1002) {
      printf("west %d, north %d, northwest %d - query %c, database %c, val1 %d , match %d\n", west, north, northwest, query[ii], database[jj], val1, match);
      printf("ii %d, jj %d \n", ii, jj);
    }
    */

    if (val1 > val) {
      val = val1;
        dir = NORTH_WEST;
    }
    val1 = north + GAP_d;
    if (val1 > val) {
        val = val1;
        dir = NORTH;
    }
    val1 = west + GAP_i;
    if (val1 > val) {
      val = val1;
        dir = WEST;
    }
    //printf("val %d \n", val);
    matrix[i] = val;
    directionMatrixSW[i] = dir;

    if(val > maxValue){
      maxValue = val;
      maxIndexSw = i;
    }

  }
}

void compute_matrices_sw_2(char *query, char *database,
		int *max_index, int *similarity_matrix, short *direction_matrix){
	int north = 0;
	      int west = 0;
	      int northwest = 0;


	      int maxValue = 0;
	      int localMaxIndex = 0;

	      int val = 0;
	      short dir = 0;
	      int maxIndexSw = 0;


	  //calculate my own SW
	  for(int i = 0; i < N * M; i++){
	    val = 0;
	    dir=CENTER;
	    if( i == 0){
	      north = 0;
	      northwest = 0;
	      west = 0;
	    }else if (i / N == 0){ //first row
	      north = 0;
	      northwest = 0;
	      west = similarity_matrix[i - 1];
	    }else if(i % N == 0){ // first col
	      west = 0;
	      northwest = 0;
	      north = similarity_matrix[i - N];
	    }else{
	      west = similarity_matrix[i - 1];
	      north = similarity_matrix[i - N];
	      northwest = similarity_matrix [i - N - 1];
	    }


	    //all set, compute
	    int jj = i / N;
	    int ii = i % N;

	    const short match = (query[ii] == database[jj]) ?  MATCH : MISS_MATCH;
	    int val1 = northwest + match;


	    if (val1 > val) {
	      val = val1;
	        dir = NORTH_WEST;
	    }
	    val1 = north + GAP_d;
	    if (val1 > val) {
	        val = val1;
	        dir = NORTH;
	    }
	    val1 = west + GAP_i;
	    if (val1 > val) {
	      val = val1;
	        dir = WEST;
	    }
	    //printf("val %d \n", val);
	    similarity_matrix[i] = val;
	    direction_matrix[i] = dir;

	    if(val > maxValue){
	      maxValue = val;
	      maxIndexSw = i;
	    }

	  }
}


void compute_matrices_sw(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix)
{
	//here the real computation starts...
	int index = 0;
	int i = 0;
	int j = 0;
	short dir = CENTER;
	short match = 0;
	int val = 0;
	int north = 0;
	int west = 0;
	int northwest = 0;
	int max_value = 0;
	int test_val = 0;

	max_index[0] = 0;

	for(index = N; index < N*M; index++) {
		dir = CENTER;
		val = 0;

		i = index % N; // column index
		j = index / N; // row index

		if(i == 0) {
			// first column
			west = 0;
			northwest = 0;
		} else {

			// all columns but first
			north = similarity_matrix[index - N];
			match = ( string1[i] == string2[j] ) ? MATCH : MISS_MATCH;

			test_val = northwest + match;
			if(test_val > val){
				val = test_val;
				dir = NORTH_WEST;
			}

			test_val = north + GAP_d;
			if(test_val > val){
				val = test_val;
				dir = NORTH;
			}

			test_val = west + GAP_i;
			if(test_val > val){
				val = test_val;
				dir = WEST;
			}

			similarity_matrix[index] = val;
			direction_matrix[index] = dir;
			west = val;
			northwest = north;
			if(val > max_value) {
				max_index[0] = index;
				max_value = val;
			}
		}
	}
}

/*
 Given an event, this function returns the kernel execution time in ms
 */
float getTimeDifference(cl_event event) {
	cl_ulong time_start = 0;
	cl_ulong time_end = 0;
	float total_time = 0.0f;

	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start,
	NULL);
	clGetEventProfilingInfo(event,
	CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end,
	NULL);
	total_time = time_end - time_start;
	return total_time / 1000000.0; // To convert nanoseconds to milliseconds
}


/*
 return a random number between 0 and limit inclusive.
 */
int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

int load_file_to_memory(const char *filename, char **result) {
	int size = 0;
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		*result = NULL;
		return -1; // -1 means file opening fail
	}
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *) malloc(size + 1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -2; // -2 means file reading fail
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}

int main(int argc, char** argv) {
	printf("starting HOST code \n");
	fflush(stdout);
	int err;                            // error code returned from api calls

	char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	char *databasehw = (char*) malloc(sizeof(char) * (M + 2 *N));
//	int *similarity_matrix = (int*) malloc(sizeof(int) * N * M);
	char *direction_matrixhw = (char*) malloc(sizeof(char) * 256 * (N + M - 1)); //512 bits..
//	int *max_index = (int *) malloc(sizeof(int));

	unsigned int * query_param = (unsigned int *)malloc(sizeof(unsigned int) * (N/16 + 1));
	unsigned int * database_param = (unsigned int *)malloc(sizeof(unsigned int) * ((M + 2*(N))/16 + 1));

	printf("array defined! \n");

	fflush(stdout);

	size_t global[2];                  // global domain size for our calculation
	size_t local[2];                    // local domain size for our calculation

	cl_platform_id platform_id;         // platform id
	cl_device_id device_id;             // compute device id
	cl_context context;                 // compute context
	cl_command_queue commands;          // compute command queue
	cl_program program;                 // compute program
	cl_kernel kernel;                   // compute kernel

	char cl_platform_vendor[1001];
	char cl_platform_name[1001];

	cl_mem input_query;
	cl_mem input_database;
//	cl_mem output_similarity_matrix;
	cl_mem output_direction_matrixhw;
//	cl_mem output_max_index;

	if (argc != 2) {
		printf("%s <inputfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	for(int i = 0; i < N/16 + 1; i++) query_param[i] = 0;
	for(int i = 0; i < (M + 2*(N))/16 + 1; i++) database_param[i] = 0;

	fillRandom(query, N);
	fillRandom(database, M);
	fillRandom(databasehw, M+2*(N-1));



	for(int i = 0; i < N - 1 ; i ++)
		databasehw[i] = 'P';

	memcpy((databasehw + N - 1), database, M);

//	memset(similarity_matrix, 0, sizeof(int) * N * M);
	memset(direction_matrixhw, 0, sizeof(char) * 256 * (N + M - 1));

	printf("query \n");
	for(int i = 0; i < N; i++){
		printf("%c", query[i]);
		set_char_main(query_param, i, query[i]);
	}

	printf("\ndatabase \n");
	for(int i = 0; i < M + 2*(N); i++){
		printf("%c", databasehw[i]);
		set_char_main(database_param, i, databasehw[i]);
//		printf(" = %d, ", (database_param[i/16] >> ((i%16) * 2)) & 3);
	}
	// Connect to first platform
	//
	printf("GET platform \n");
	err = clGetPlatformIDs(1, &platform_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to find an OpenCL platform!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("GET platform vendor \n");
	err = clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 1000,
			(void *) cl_platform_vendor, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("CL_PLATFORM_VENDOR %s\n", cl_platform_vendor);
	printf("GET platform name \n");
	err = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 1000,
			(void *) cl_platform_name, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: clGetPlatformInfo(CL_PLATFORM_NAME) failed!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("CL_PLATFORM_NAME %s\n", cl_platform_name);

	// Connect to a compute device
	//
	int fpga = 1;

	printf("get device FPGA is %d  \n", fpga);
	err = clGetDeviceIDs(platform_id,
			fpga ? CL_DEVICE_TYPE_ACCELERATOR : CL_DEVICE_TYPE_CPU, 1,
			&device_id, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to create a device group!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Create a compute context
	//
	printf("create context \n");
	context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
	if (!context) {
		printf("Error: Failed to create a compute context!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Create a command commands
	//
	printf("create queue \n");
	commands = clCreateCommandQueue(context, device_id,
	CL_QUEUE_PROFILING_ENABLE, &err);
	if (!commands) {
		printf("Error: Failed to create a command commands!\n");
		printf("Error: code %i\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	int status;

	// Create Program Objects
	//

	// Load binary from disk
	unsigned char *kernelbinary;
	char *xclbin = argv[1];
	printf("loading %s\n", xclbin);
	int n_i = load_file_to_memory(xclbin, (char **) &kernelbinary);
	if (n_i < 0) {
		printf("failed to load kernel from xclbin: %s\n", xclbin);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	size_t n = n_i;
	// Create the compute program from offline
	printf("create program with binary \n");
	program = clCreateProgramWithBinary(context, 1, &device_id, &n,
			(const unsigned char **) &kernelbinary, &status, &err);
	if ((!program) || (err != CL_SUCCESS)) {
		printf("Error: Failed to create compute program from binary %d!\n",
				err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Build the program executable
	//
	printf("build program \n");
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err != CL_SUCCESS) {
		size_t len;
		char buffer[2048];

		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
				sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Create the compute kernel in the program we wish to run
	//
	printf("create kernel \n");
	kernel = clCreateKernel(program, "compute_matrices", &err);
	if (!kernel || err != CL_SUCCESS) {
		printf("Error: Failed to create compute kernel!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	input_query = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * (N/16 + 1),
	NULL, NULL);
	input_database = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * ((M + 2*(N))/16 + 1),
	NULL, NULL);
//	output_similarity_matrix = clCreateBuffer(context, CL_MEM_READ_WRITE,
//			sizeof(int) * M * N, NULL, NULL);
	output_direction_matrixhw = clCreateBuffer(context, CL_MEM_READ_WRITE,
			sizeof(char) * 256 * (N + M - 1), NULL, NULL);
//	output_max_index = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int),
//	NULL, NULL);

	if (!input_query || !input_database || !output_direction_matrixhw) {
		printf("Error: Failed to allocate device memory!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Write our data set into the input array in device memory
	err = clEnqueueWriteBuffer(commands, input_query, CL_TRUE, 0, sizeof(unsigned int) * (N/16 + 1) , query_param, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array a!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	err = clEnqueueWriteBuffer(commands, input_database, CL_TRUE, 0, sizeof(unsigned int) * ((M+2*(N))/16 + 1), database_param, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to write to source array a!\n");
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

//	err = clEnqueueWriteBuffer(commands, output_similarity_matrix, CL_TRUE, 0,
//			sizeof(int) * N * M, similarity_matrix, 0, NULL, NULL);
//	if (err != CL_SUCCESS) {
//		printf("Error: Failed to write to source array a!\n");
//		printf("Test failed\n");
//		return EXIT_FAILURE;
//	}

	err = clEnqueueWriteBuffer(commands, output_direction_matrixhw, CL_TRUE, 0,
				sizeof(char) * 256 * (N + M - 1), direction_matrixhw, 0, NULL, NULL);
		if (err != CL_SUCCESS) {
			printf("Error: Failed to write to source array a!\n");
			printf("Test failed\n");
			return EXIT_FAILURE;
		}

	// Set the arguments to our compute kernel
	//
	err = 0;
	printf("set arg 1 \n");
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_query);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 1! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("set arg 2 \n");
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_database);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 2! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	printf("set arg 3 \n");
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_direction_matrixhw);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to set kernel arguments 3! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}

	// Execute the kernel over the entire range of our 1d input data set
	// using the maximum number of work group items for this device
	//
	cl_event enqueue_kernel;
#ifdef C_KERNEL
	printf("LAUNCH task \n");
	err = clEnqueueTask(commands, kernel, 0, NULL, &enqueue_kernel);
#else
	//remember to define global and local if run with NDRange
	err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL, (size_t*) &global,
			(size_t*) &local, 0, NULL, NULL);
#endif
	if (err) {
		printf("Error: Failed to execute kernel! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
	clWaitForEvents(1, &enqueue_kernel);

	// Read back the results from the device to verify the output
	//

	cl_event readDirections;
//	err = clEnqueueReadBuffer(commands, output_similarity_matrix, CL_TRUE, 0,
//			sizeof(char) * N * M, similarity_matrix, 0, NULL, &readSimilarity);
//	if (err != CL_SUCCESS) {
//		printf("Error: Failed to read array! %d\n", err);
//		printf("Test failed\n");
//		return EXIT_FAILURE;
//	}

	err = clEnqueueReadBuffer(commands, output_direction_matrixhw, CL_TRUE, 0,
			sizeof(char) * 256 * (N + M - 1), direction_matrixhw, 0, NULL,
			&readDirections);
	if (err != CL_SUCCESS) {
		printf("Error: Failed to read array! %d\n", err);
		printf("Test failed\n");
		return EXIT_FAILURE;
	}
//	err = clEnqueueReadBuffer(commands, output_max_index, CL_TRUE, 0,
//			sizeof(int), max_index, 0, NULL, &readMax);
//	if (err != CL_SUCCESS) {
//		printf("Error: Failed to read array! %d\n", err);
//		printf("Test failed\n");
//		return EXIT_FAILURE;
//	}

//	clWaitForEvents(1, &readSimilarity);
	clWaitForEvents(1, &readDirections);
//	clWaitForEvents(1, &readMax);
	float executionTime = getTimeDifference(enqueue_kernel);

	int * matrix = ( int *) malloc(
			sizeof( int) * N * M);
	short * directionMatrixSW = ( short*) malloc(
			sizeof( short) * N * M);

	int * max_index_sw = ( int *) malloc(
				sizeof( int));

	for(int i = 0; i < N*M; i++){
		matrix[i] = 0;
		directionMatrixSW[i] = 0;
	}
//	compute_matrices_sw_2(query, database,max_index_sw, matrix, directionMatrixSW );
	compute_matrices_sw_3(query,database, matrix, directionMatrixSW);

	unsigned short * ordered_direction_matrix = (unsigned short *)malloc(sizeof(unsigned short) * N * M);

	for(int i = 0; i < N; i ++)
		printf("%c ", query[i]);
	printf("\n");

	for(int i = 0; i < M; i ++)
		printf("%c ", database[i]);
	printf("\n");
	for(int i = 0; i < M + 2 * (N -1); i ++)
			printf("%c ", databasehw[i]);
		printf("\n");

	for(int i = 0; i < N*M; i++){
		if(i % N == 0)
			printf("\n");
		printf(" %d ", directionMatrixSW[i]);
	}

	printf("hw version \n");

	for(int i = 0; i < 256*(N + M - 1); i++){
		if ( i % 256 == 0)
			printf("\n");
		printf(" %d ", direction_matrixhw[i]);
	}

	int temp_index = 0;
	int iter = 1;
	//unsigned short tempMatrixBis[N * (N+M-1)];
	unsigned short *tempMatrixBis = (unsigned short*)malloc(sizeof(unsigned short) * (N*(N+M-1)));
	for(int i = 0; i < N * (N + M - 1); i++){
		tempMatrixBis[i] = get(direction_matrixhw, temp_index);
		temp_index++;
		if(temp_index % N == 0){
			temp_index = iter * 256;
			iter++;
		}
	}

	ordered_direction_matrix = order_matrix_blocks(tempMatrixBis);

	printf("hw version ord\n");

		for(int i = 0; i < N*M; i++){
			if ( i % N == 0)
				printf("\n");
			printf(" %d ", ordered_direction_matrix[i]);
		}

	printf("both ended\n");

	printf(" execution time is %f ms \n", executionTime);
	for (int i = 0; i < N * M; i++) {
		if (directionMatrixSW[i] != ordered_direction_matrix[i]) {
			printf("Error, mismatch in the results, i + %d, SW: %d, HW %d \n",
					i, directionMatrixSW[i], ordered_direction_matrix[i]);
			return EXIT_FAILURE;
		}
	}

	printf("computation ended!- RESULTS CORRECT \n");

	// Shutdown and cleanup

	clReleaseMemObject(input_database);
	clReleaseMemObject(input_query);
	clReleaseMemObject(output_direction_matrixhw);
//	clReleaseMemObject(output_max_index);
//	clReleaseMemObject(output_similarity_matrix);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commands);
	clReleaseContext(context);

	free(matrix);
	free(directionMatrixSW);
	free(databasehw);
//	free(max_index_sw);
	free(ordered_direction_matrix);
	free(query_param);
	free(database_param);

	return EXIT_SUCCESS;
}
