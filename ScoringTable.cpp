
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ScoringTable.h"

// The starter character of a comment line.
static const char COMMENT_FLAG = '#';

// The id (or name) of the matrix.
static char id[255];

// The order of the acids appeared,
static char acids[MAX_ACIDS];

// The total number of the acids appeared.
static int acid_num;

// The value of scoring matrix.
static TSCORE scores[MAX_ACIDS][MAX_ACIDS];

/**
 * Read scoring matrix from file.
 * 
 * @param fp		handle to the matrix file
 *
 * @return			0--success, other values means failed
 */
static int read_scoring_matrix(FILE *fp);

/**
 * Get the default index position of the acid in the matrix.
 *
 * @param acid		the acid name.
 *
 * @return			the default index position of the acid, -1 means error.
 */
static int get_acid_index(char acid);

/**
 * Loads scoring matrix from file.
 *
 * @param filename	pointer to the path of input matrix file
 *
 * @return			0--success, other values means failed
 */
int load_scoring_matrix(const char *filename) {
	int ret;

	// Open the file.
	FILE *fp = fopen(filename, "r");

	if (fp == NULL) {
		fprintf(stderr, "****WARNING****: failed to open the scoring matrix file %s.\n", filename);
		return 1;
	}

	// Store the name of the file.
	strcpy(id, filename);

	ret = read_scoring_matrix(fp);

	// Close the file.
	fclose(fp);

	if (ret != 0) {
		fprintf(stderr, "****WARNING****: failed to read the scoring matrix from file %s\n", filename);
		return 2;
	}

	return 0;
}

/**
 * Get the id (or name) of the matrix.
 *
 * @return          the id (or name) of the matrix
 */
char* get_scoring_matrix_id() {
	return id;
}

/**
 * Get the score of (acid1, acid2).
 */
TSCORE get_score(char acid1, char acid2) {
	int index_i, index_j;

	index_i = get_acid_index(acid1);
	index_j = get_acid_index(acid2);
	return scores[index_i][index_j];
}

/**
 * Output result for test.
 */
void print_scoring_table(FILE *fp) {
	int i, j;
	int index_i, index_j;
	
	fprintf(fp, " ");
	for (j = 0; j < acid_num; ++j) {
		fprintf(fp, "%5c", acids[j]);
	}
	fprintf(fp, "\n");
	
	for (i = 0; i < acid_num; ++i) {
		fprintf(fp, "%c", acids[i]);
		for (j = 0; j < acid_num; ++j) {
			index_i = get_acid_index(acids[i]);
			index_j = get_acid_index(acids[j]);
			fprintf(fp, "%5d", scores[index_i][index_j]);
		}
		fprintf(fp, "\n");
	}
}

/**
 * Get the default index position of the acid in the matrix.
 *
 * @param acid		the acid name.
 *
 * @return			the default index position of the acid, -1 means error.
 */
static int get_acid_index(char acid) {
	if (isalpha(acid)) {
		return toupper(acid) - 'A';
	} else if (acid == '*') {
		return 26;
	} else if (acid == '-') {
		return 27;
	}
	return -1;
}

/**
 * Read scoring matrix from file.
 *
 * @param fp		handle to the matrix file
 *
 * @return			0--success, other values means failed
 */
static int read_scoring_matrix(FILE *fp) {
	int i;
	int index_i, index_j;
	char chr;
	char line[1024];
	char *token;

	// Skip the comment lines.
	while (fgets(line, sizeof(line), fp) != NULL) {
		// Read the first non space character.
		sscanf(line, " %c", &chr);
		if (chr == COMMENT_FLAG) continue;
		else break;
	}

	// Read the headers line (the letters of the acids)
	acid_num = 0;
	for (i = 0; line[i] != '\0'; ++i) {
		if (get_acid_index(line[i]) >= 0) {
			acids[acid_num ++] = line[i];
		}
	}

	// Read the scores
	while (fgets(line, sizeof(line), fp) != NULL) {
		token = strtok(line, " ");
		index_i = get_acid_index(token[0]);

		if (index_i < 0) {
			break;
		}

		for (i = 0; i < acid_num; ++i) {
			token = strtok(NULL, " ");
			if (NULL == token) {
				// Error occurred.
				return 1;
			}
			index_j = get_acid_index(acids[i]);
			scores[index_i][index_j] = (TSCORE)atof(token);
		}
	}
	return 0;
}


