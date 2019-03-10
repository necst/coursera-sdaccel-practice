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
#define M 2048
#define MATRIX_SIZE N * M

extern "C" {

void compute_matrices(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix)
{
#pragma HLS INTERFACE m_axi port=string1 offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=string2 offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=max_index offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=similarity_matrix offset=slave bundle=gmem
#pragma HLS INTERFACE m_axi port=direction_matrix offset=slave bundle=gmem

#pragma HLS INTERFACE s_axilite port=string1 bundle=control
#pragma HLS INTERFACE s_axilite port=string2 bundle=control
#pragma HLS INTERFACE s_axilite port=max_index bundle=control
#pragma HLS INTERFACE s_axilite port=similarity_matrix bundle=control
#pragma HLS INTERFACE s_axilite port=direction_matrix bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

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

	for(index = N; index < MATRIX_SIZE; index++) {
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

}
