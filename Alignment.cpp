#include <stdlib.h>
#include <pthread.h>
#include "Alignment.h"
//#include "EasyPDP/EasyPDP.h"
//#include "EasyPDP/RectPattern.h"

/**
 * The sequence1 and sequence2.
 */
static Seq_t *seq1, *seq2;

/**
 * Open gap penalty.
 */
static int op;

/**
 * Extend gap penalty.
 */
static int ep;

/**
 * The matrix score values for dynamic programming
 */
static TSCORE *score_values;

/**
 * Record the sequence match direction.
 */
static Direction *directions;
int * get_directions_pointer()
{
	return directions;
}
/**
 * Size of vertical gaps
 */
static int* vertical_gaps;
int* get_vertical_gaps_pointer()
{
	return vertical_gaps;
}
/**
 * Size of horizontal gaps
 */
static int* horizontal_gaps;
int *get_horizontal_gaps_pointer()
{
	return horizontal_gaps;
}


/**
 * The cell where the traceback starts.
 */
static Cell_t trace_cell;

/** Reverses an array of characters. */
static void reverse_copy(char *dest, const char *src, int len);

/** The DAG pattern initialization function. */
//static void pattern_init();

/** The EasyPDP initialization function. */
//static void user_EasyPDP_init();

/** The traceback cell initialization function. */
static void trace_cell_init();

/** Update the trace cell. */
static void update_trace_cell(int thread_id, int node_id);

/** The Smith Waterman algorithm alignment function to process blocks of data. */
static void smith_waterman_part(int thread_id, void *block_arg);

/** The Smith Waterman traceback algorithm function. Get result. */
static void smith_waterman_traceback(Result_t *result);

/** The Smith Waterman format output. */
static void format_output(FILE *fp, Result_t *result);

/** Get the penalty score of the size of a gap */
static TSCORE gap_score(int gap);

int * get_score_values_pointer()
{
    return score_values;
}
/**/


void smith_waterman(int, SizeT, BlockT*);

/** The Smith Waterman algorithm initialization function. */
void smith_waterman_init(Alignment_t *alignment) {
	int i, j, k, m, n;

	seq1 = &alignment->seq1;
	seq2 = &alignment->seq2;
	op = alignment->op;
	ep = alignment->ep;

	m = seq1->length + 1;
	n = seq2->length + 1;
	score_values = (TSCORE *) malloc(m * n * sizeof(TSCORE));
	horizontal_gaps = (int *) malloc(m * n * sizeof(int));
	vertical_gaps = (int *) malloc(m * n * sizeof(int));
	directions = (Direction *) malloc(m * n * sizeof(Direction));

	// initialize the boundary of matrixes
	for (i = 0, k = 0; k < m; ++k, i += n) {
		directions[i] = 0; //stop.
		score_values[i] = 0;
	}
	for (j = 1; j < n; ++j) {
		directions[j] = 0;
		score_values[j] = 0;
	}

	for (i = 0; i < m; ++i) {
		for (j = 0; j < n; ++j) {
			vertical_gaps[i * n + j] = horizontal_gaps[i * n + j] = 1;
		}
	}
	trace_cell_init();


//	pattern_init();
}

/** The Smith Waterman algorithm function. */
void smith_waterman_alignment(int thread_id, SizeT dag_size, BlockT* data_block) {

	int i, j, k, idx;
	TSCORE match_score, gap_x, gap_y, gap_tmp;
	int row = seq1->length+1;
	int col = seq2->length+1;
	int row_st = data_block->block_pos.x;
	int row_ed = row_st + data_block->block_size.row;
	int col_st = data_block->block_pos.y;
        int col_ed = col_st + data_block->block_size.col;
	if (row_st <= 0)	row_st = 1;
	if (row_ed > row)	row_ed = row;
	if (col_st <= 0)	col_st = 1;
	if (col_ed > col)	col_ed = col;
	for (i = row_st; i < row_ed; ++i) {
		for (j = col_st; j < col_ed; ++j) {
			idx = i * col + j;

			// The match score value of x(i) aligns to y(j)
			match_score = get_score(seq1->acids[i - 1], seq2->acids[j - 1]);
			match_score += score_values[idx - col - 1];

			// The score value of x(i) aligns to a gap after y(j)
			gap_x = NEG_INFINITY;
			for (k = 1; k <= i; ++k) {
				gap_tmp = score_values[idx - k * col] - gap_score(k);
				if (gap_tmp > gap_x) {
					gap_x = gap_tmp;
					vertical_gaps[idx] = k;
				}
			}

			// The score value of y(j) aligns to a gap after x(i)
			gap_y = NEG_INFINITY;
			for (k = 1; k <= j; ++k) {
				gap_tmp = score_values[idx - k] - gap_score(k);
				if (gap_tmp > gap_y) {
					gap_y = gap_tmp;
					horizontal_gaps[idx] = k;
				}
			}

			// The best score value of x(*-i) aligns to y(*-j)
			score_values[idx] = 0;
			directions[idx] = 0;
			if (score_values[idx] < match_score) {
				score_values[idx] = match_score;
				directions[idx] = 1;
			}
			if (score_values[idx] < gap_x) {
				score_values[idx] = gap_x;
				directions[idx] = 2;
			}
			if (score_values[idx] < gap_y) {
				score_values[idx] = gap_y;
				directions[idx] = 3;
			}
		}
	}
}
void get_trace_cell_value()
{
	int i,j,idx;
	trace_cell.value = 0;
	int row = seq1->length + 1;
	int col = seq2->length + 1;
	for(i = 0; i < row; i++)
	{
		for(j = 0; j < col; j++)
		{
			idx = i * col + j;
			if(trace_cell.value < score_values[idx])
			{
				trace_cell.value = score_values[idx];
				trace_cell.row = i;
				trace_cell.col = j;
			}
		}
	}
}
/** The Smith Waterman algorithm output function. */
void smith_waterman_output(FILE *fp) {
	get_trace_cell_value();
	Result_t result;
	smith_waterman_traceback(&result);

	fprintf(fp, ">>sequence #1: %s(Length: %d), sequence #2: %s(Length: %d)\n",
			seq1->name, seq1->length, seq2->name, seq2->length);
	fprintf(fp, " Scoring_Matrix: %s, open gap: %d, extend gap: %d\n",
			get_scoring_matrix_id(), op, ep);

	fprintf(fp, ">Result:\n");
	fprintf(fp, " Score: %d, Aligned Length: %d\n", result.score, result.length);
	fprintf(
			fp,
			" Identity: %d/%d (%.2lf%%), Similarity: %d/%d (%.2lf%%), Gaps: %d/%d (%.2lf%%)\n",
			result.identity, result.length,
			result.identity / (result.length * 0.01), result.similarity,
			result.length, result.similarity / (result.length * 0.01),
			result.gaps, result.length, result.gaps / (result.length * 0.01));

	format_output(fp, &result);
	smith_waterman_destroy();
}

/** The Smith Waterman algorithm destroy function. */
void smith_waterman_destroy() {
	free(score_values);
	free(directions);
	free(horizontal_gaps);
	free(vertical_gaps);
}

/** Reverses an array of characters. */
static void reverse_copy(char *dest, const char *src, int len) {
	int i, j;
	for (i = 0, j = len - 1; i < len; ++i, --j) {
		dest[i] = src[j];
	}
	dest[len] = '\0';
}

static void trace_cell_init() {
	trace_cell.row = trace_cell.col = 0;
	trace_cell.value = 0;
}


/** The Smith Waterman traceback algorithm function. Get result. */
static void smith_waterman_traceback(Result_t *result) {
	int i, j, idx, col, len, maxlen, gap_len;
	char acid1, acid2;
	char *reversed_markup; // revered markup

	col = seq2->length + 1;
	maxlen = seq1->length + seq2->length + 2; //the maximum length.
	reversed_markup = (char *) malloc(maxlen * sizeof(char));

	len = 0;
	result->score = trace_cell.value;
	result->identity = result->similarity = result->gaps = 0;

	i = trace_cell.row;
	j = trace_cell.col;
	while (idx = i * col + j, directions[idx] != 0) { 
		if (directions[idx] == 3) {
			for (gap_len = horizontal_gaps[idx]; gap_len > 0; --gap_len) {
				--j;
				reversed_markup[len++] = '2'; // Sequence #2.
				++result->gaps;
			}
		} else if (directions[idx] == 2) {
			for (gap_len = vertical_gaps[idx]; gap_len > 0; --gap_len) {
				--i;
				reversed_markup[len++] = '1'; // Sequence #1.
				++result->gaps;
			}
		} else if (directions[idx] == 1) {
			acid1 = seq1->acids[--i];
			acid2 = seq2->acids[--j];
			if (acid1 == acid2) {
				reversed_markup[len++] = '|';
				++result->identity;
				++result->similarity;
			} else if (get_score(acid1, acid2) > 0) {
				reversed_markup[len++] = ':';
				++result->similarity;
			} else {
				reversed_markup[len++] = '.';
			}
		}
	}

	result->start1 = i + 1;
	result->start2 = j + 1;
	result->length = len;
	result->markup = (char *) malloc(len * sizeof(char));
	reverse_copy(result->markup, reversed_markup, len);
}

/** The Smith Waterman format output. */
static void format_output(FILE *fp, Result_t *result) {
	int i, j, len;
	int pos1, pos2, old_pos1, old_pos2;
	char mark_ch;
	char sub_seq1[SUB_SEQ_WIDTH + 1], sub_seq2[SUB_SEQ_WIDTH + 1];
	char sub_markup[SUB_SEQ_WIDTH + 1];

	old_pos1 = pos1 = result->start1;
	old_pos2 = pos2 = result->start2;
	len = result->length;

	for (j = 0; j < len;) {
		for (i = 0; i < SUB_SEQ_WIDTH && j < len; ++i, ++j) {
			mark_ch = result->markup[j];
			if (mark_ch == '1') {
				sub_seq1[i] = seq1->acids[pos1 - 1];
				sub_seq2[i] = '-';
				sub_markup[i] = ' ';
				pos1++;
			} else if (mark_ch == '2') {
				sub_seq1[i] = '-';
				sub_seq2[i] = seq2->acids[pos2 - 1];
				sub_markup[i] = ' ';
				pos2++;
			} else {
				sub_seq1[i] = seq1->acids[pos1 - 1];
				sub_seq2[i] = seq2->acids[pos2 - 1];
				sub_markup[i] = mark_ch;
				pos1++;
				pos2++;
			}
		}
		sub_seq1[i] = sub_seq2[i] = sub_markup[i] = '\0';

		fprintf(fp, "#1 %5d %s %d\n", old_pos1, sub_seq1, pos1 - 1);
		fprintf(fp, "   %5s %s\n", " ", sub_markup);
		fprintf(fp, "#2 %5d %s %d\n", old_pos2, sub_seq2, pos2 - 1);
		fprintf(fp, "\n");

		old_pos1 = pos1;
		old_pos2 = pos2;
	}
}

/** Get the penalty score of the size of a gap */
static TSCORE gap_score(int gap) {
	return op + (gap - 1) * ep;
}
