#include <stdio.h>
#include <string.h>
#include <ap_int.h>

// directions codes
static const int CENTER = 0;
static const int NORTH = 1;
static const int NORTH_WEST = 2;
static const int WEST = 3;

// scores used for Smith Waterman similarity computation
static const short GAP_i = -1;
static const short GAP_d = -1;
static const short MATCH = 2;
static const short MISS_MATCH = -1;


#define N 256
#define M 256
#define NUM_ELEM 256
#define DATABASE_SIZE (M + 2 * (N - 1))
#define DIRECTION_MATRIX_SIZE ((N + M - 1) * N)
#define MATRIX_SIZE (N * M)

extern "C" {

void store_diagonal(int directions_index, ap_uint<512> *direction_matrix_g, ap_uint<512> compressed_diag[1]) {

	memcpy(direction_matrix_g + directions_index, compressed_diag, 64);

}



void calculate_diagonal(int num_diagonals, ap_uint<512> * string1, ap_uint<512> * string2, short northwest[N + 1], short north[N + 1], short west[N + 1], int directions_index, ap_uint<512> compressed_diag[]){
#pragma HLS INLINE region recursive
	int databaseLocalIndex = num_diagonals;
	int from, to;
	from = N * 2 - 2;
	to = N * 2 - 1;

	calculate_diagonal_for: for(int index = N - 1; index >= 0; index --){
		int val = 0;
		const short q = string1[index/NUM_ELEM].range((index%NUM_ELEM) * 2 + 1, (index%NUM_ELEM) * 2);
		short db = string2[databaseLocalIndex/NUM_ELEM].range((databaseLocalIndex%NUM_ELEM) * 2 + 1, (databaseLocalIndex%NUM_ELEM) * 2);


		if(databaseLocalIndex < N - 1 ) db = 9;

		const short match = (q == db) ? MATCH : MISS_MATCH;
		const short val1 = northwest[index] + match;
		const short val2 = north[index] + GAP_d;
		const short val3 = west[index] + GAP_i;

		if(val1 > val && val1 >= val2 && val1 >= val3){
			//val1
			northwest[index + 1] = north[index];
			north[index] = val1;
			west[index + 1] = val1;
			compressed_diag[0].range(to,from) = NORTH_WEST;
//			directionDiagonal[index] = NORTH_WEST;
		} else if (val2 > val && val2 >= val3) {
			//val2
			northwest[index + 1] = north[index];
			north[index] = val2;
			west[index + 1] = val2;
			compressed_diag[0].range(to,from) = NORTH;
//			directionDiagonal[index] = NORTH;
		}else if (val3 > val){
			//val3
			northwest[index + 1] = north[index];
			north[index] = val3;
			west[index + 1] = val3;
			compressed_diag[0].range(to,from) = WEST;
//			directionDiagonal[index] = WEST;
		}else{
			//val
			northwest[index + 1] = north[index];
			north[index] = val;
			west[index + 1] = val;
			compressed_diag[0].range(to,from) = CENTER;
//			directionDiagonal[index] = CENTER;
		}
		databaseLocalIndex ++;
		from -= 2;
		to -= 2;
	}

}



void compute_matrices( ap_uint<512> *string1_g, ap_uint<512> *string2_g, ap_uint<512> *direction_matrix_g)
{
#pragma HLS INTERFACE m_axi port=string1_g offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=string2_g offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=direction_matrix_g offset=slave bundle=gmem1

#pragma HLS INTERFACE s_axilite port=string1_g bundle=control
#pragma HLS INTERFACE s_axilite port=string2_g bundle=control
#pragma HLS INTERFACE s_axilite port=direction_matrix_g bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

	ap_uint<512> string1[N / NUM_ELEM  + 1];
//#pragma HLS ARRAY_PARTITION variable=string1 complete dim=1
	ap_uint<512> string2[(DATABASE_SIZE)/ NUM_ELEM + 1];
//#pragma HLS ARRAY_PARTITION variable=string2 complete dim=1

//	short direction_matrix[DIRECTION_MATRIX_SIZE];
//#pragma HLS ARRAY_PARTITION variable=direction_matrix complete dim=1

	memcpy(string1, string1_g, (N/NUM_ELEM + 1) * 64);
	memcpy(string2, string2_g, ((DATABASE_SIZE) / NUM_ELEM + 1) * 64);


	short north[N+1];
#pragma HLS ARRAY_PARTITION variable=north complete dim=1
	short west[N+1];
#pragma HLS ARRAY_PARTITION variable=west complete dim=1
	short northwest[N+1];
#pragma HLS ARRAY_PARTITION variable=northwest complete dim=1

/*
	short directionDiagonal[N];
#pragma HLS ARRAY_PARTITION variable=directionDiagonal complete dim=1
*/

	ap_uint<512> compressed_diag[1];

	init_dep_for:for(int i = 0; i <= N; i++){
		north[i] = 0;
		west[i] = 0;
		northwest[i] = 0;
	}

	int directions_index = 0;

	num_diag_for: for(int num_diagonals = 0; num_diagonals < N + M - 1; num_diagonals++){
#pragma HLS inline region recursive
#pragma HLS PIPELINE

		calculate_diagonal(num_diagonals, string1, string2, northwest, north, west, directions_index, compressed_diag);
		store_diagonal(directions_index,  direction_matrix_g, compressed_diag);
		directions_index ++;
	}

//	memcpy(direction_matrix_g, direction_matrix, DIRECTION_MATRIX_SIZE * sizeof(short));

	return;
}
}
