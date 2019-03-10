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

void update_database(ap_uint<512> *database, ap_uint<512> *shift_db, int num_diagonals){
	int startingIndex = N + num_diagonals;
	update_database:for(int i = 1; i < N; i++){
#pragma HLS PIPELINE
		shift_db[(i-1)/NUM_ELEM].range(((i-1)%NUM_ELEM)*2+1, ((i-1)%NUM_ELEM)*2) = shift_db[i/NUM_ELEM].range((i%NUM_ELEM)*2+1, (i%NUM_ELEM)*2);
		//set_char(shift_db, i-1, get_char(databaseLocal, i));
		//databaseLocal[i-1] = databaseLocal[i];
	}
	shift_db[(N-1)/NUM_ELEM].range(((N-1)%NUM_ELEM) * 2 + 1,((N-1)%NUM_ELEM) * 2) = database[startingIndex/NUM_ELEM].range((startingIndex%NUM_ELEM) * 2 +1, (startingIndex%NUM_ELEM) *2);
	//set_2bit(shift_db, N-1, get_2bit(database, startingIndex));
	//set_char(shift_db, N-1, get_char(database, startingIndex));
	//databaseLocal[N-1] = database[startingIndex];
}



void calculate_diagonal(int num_diagonals, ap_uint<512> * string1, ap_uint<512> * string2, int northwest[N + 1], int north[N + 1], int west[N + 1], int directions_index, ap_uint<512> compressed_diag[], ap_uint<512> shift_db[], ap_uint<512> *direction_matrix_g){
#pragma HLS INLINE region recursive
	int databaseLocalIndex = 0;
	int from, to;
	from = N * 2 - 2;
	to = N * 2 - 1;

	calculate_diagonal_for: for(int index = N - 1; index >= 0; index --){
		int val = 0;
		const short q = string1[index/NUM_ELEM].range((index%NUM_ELEM) * 2 + 1, (index%NUM_ELEM) * 2);
        unsigned int db = shift_db[databaseLocalIndex/NUM_ELEM].range((databaseLocalIndex % NUM_ELEM) * 2 + 1, (databaseLocalIndex % NUM_ELEM) * 2);


		if(databaseLocalIndex < N - 1 && databaseLocalIndex < N - 1 - num_diagonals) db = 9;

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

	memcpy((ap_int<512>*) ((direction_matrix_g + num_diagonals)), compressed_diag,  64);


}



void compute_matrices( ap_uint<512> *string1_g, ap_uint<512> *string2_g, ap_uint<512> *direction_matrix_g)
{
#pragma HLS INTERFACE m_axi port=string1_g offset=slave bundle=gmem0
#pragma HLS INTERFACE m_axi port=string2_g offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi port=direction_matrix_g offset=slave bundle=gmem2

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


	int north[N+1];
#pragma HLS ARRAY_PARTITION variable=north complete dim=1
	int west[N+1];
#pragma HLS ARRAY_PARTITION variable=west complete dim=1
	int northwest[N+1];
#pragma HLS ARRAY_PARTITION variable=northwest complete dim=1

/*
	short directionDiagonal[N];
#pragma HLS ARRAY_PARTITION variable=directionDiagonal complete dim=1
*/

	ap_uint<512> compressed_diag[1];
	ap_uint<512> shift_db[N/NUM_ELEM];

	init_dep_for:for(int i = 0; i <= N; i++){
		north[i] = 0;
		west[i] = 0;
		northwest[i] = 0;
	}

	init_db:for(int i = 0; i < N/NUM_ELEM; i++){
#pragma HLS PIPELINE
		shift_db[i] = string2[i];
	}

	int directions_index = 0;

	num_diag_for: for(int num_diagonals = 0; num_diagonals < N + M - 1; num_diagonals++){
#pragma HLS inline region recursive
#pragma HLS PIPELINE

		calculate_diagonal(num_diagonals, string1, string2, northwest, north, west, directions_index, compressed_diag, shift_db, direction_matrix_g);

		update_database(string2, shift_db, num_diagonals);
//		store_diagonal(directions_index,  direction_matrix_g, compressed_diag);
//		directions_index ++;
	}

//	memcpy(direction_matrix_g, direction_matrix, DIRECTION_MATRIX_SIZE * sizeof(short));

	return;
}
}
