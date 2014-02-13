//
// The penalty matrix scoring table model.
//

#ifndef _SCORING_TABLE_H_
#define _SCORING_TABLE_H_

// The maximum size of the scoring matrix.
// We assume that the acids are labeled with letters 'A' ~ 'Z', and  '*'.
// Therefore, the default index position for letters 'A' ~ 'Z' in matrix are 0 ~ 25, and '*' is 26.
#define MAX_ACIDS 28

/**
 * The type of score.
 */
typedef int TSCORE;

/**
 * Loads scoring matrix from file system.
 *
 * @param filename	pointer to the path of input matrix file
 *
 * @return			0--success, other values means failed
 */
int load_scoring_matrix(const char *filename);

/**
 * Get the id (or name) of the matrix.
 *
 * @return          the id (or name) of the matrix
 */
char* get_scoring_matrix_id();

/**
 * Get the score of (acid1, acid2).
 */
TSCORE get_score(char acid1, char acid2);

/* For test. */
void print_scoring_table(FILE *fp);

#endif //_SCORING_TABLE_H_
