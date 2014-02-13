#ifndef _ALIGNMENT_H_
#define _ALIGNMENT_H_

#include <stdio.h>
#include "FastaLib.h"
#include "ScoringTable.h"
#include "EasyHPS_src/DAGPattern.h"
#define NEG_INFINITY -999999999
#define SUB_SEQ_WIDTH 70

/**
 * The type of sequence match direction.
 */
typedef int Direction;

/**
 * The trace cell of alignment.
 */
typedef struct {
	int row; // Row of the cell.
	int col; // Column of the cell.
	int value; // Alignment score at this cell.
} Cell_t;

/**
 * The type of alignment result.
 */
typedef struct {
	char *markup; // Markup line for aligned sequences.
	int start1; // Alignment start location in sequence #1.
	int start2; // Alignment start location in sequence #2.
	int score; // Alignment score.
	int length; // Length of the aligned sequences.
	int identity; // Count of identical locations.
	int similarity; // Count of similar locations.
	int gaps; // Count of gap locations.
} Result_t;

typedef struct {
	Seq_t seq1; // The sequence1.
	Seq_t seq2; // The sequence2.
	int op; // The open gap penalty.
	int ep; // The extend gap penalty.
} Alignment_t;

/** The Smith Waterman algorithm initialization function. */
void smith_waterman_init(Alignment_t *alignment);

/** The Smith Waterman algorithm function. */
void smith_waterman_alignment(int, SizeT, BlockT*);

/** The Smith Waterman algorithm output function. */
void smith_waterman_output(FILE *fp);

/** The Smith Waterman algorithm destroy function. */
void smith_waterman_destroy();

int * get_score_values_pointer();

int * get_horizontal_gaps_pointer(); 

int * get_directions_pointer();

int * get_vertical_gaps_pointer();
#endif //_ALIGNMENT_H_
