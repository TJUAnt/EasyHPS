
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "FastaLib.h"

// The starter character of a information line.
const char INFO_FLAG = '>';

/**
 * Load biological sequence from library file.
 *
 * @param fp		handler to the path of library file
 * @param seq       pointer to the result sequence
 *
 * @return			0--success, other values means failed
 */
static int read_fasta_sequence(FILE *fp, Seq_t *seq);

/**
 * Load biological sequence from library file.
 *
 * @param filename	pointer to the path of library file
 * @param seq       pointer to the biological sequence which holds the result
 * @param idx       the idx-th sequence
 *
 * @return			0--success, other values means failed
 */
int load_fasta_sequence(const char *filename, Seq_t *seq, int idx) {
	int ret = 0;

	// open the file.
	FILE *fp = fopen(filename, "r");

	if (fp == NULL) {
		fprintf(stderr, "****WARNING****: failed to open the fasta library file %s\n", filename);
		return 1;
	}

	while (ret == 0 && idx > 0) {
		ret = read_fasta_sequence(fp, seq);
		idx--;
	}

	// Close the file.
	fclose(fp);

	if (ret != 0) {
		fprintf(stderr, "****WARNING****: failed to read the fasta library file %s\n", filename);
		return 2;
	}

	return 0;
}

/**
 * Destroy biological sequence.
 *
 * @param seq		pointer to the biological sequence
 */
void destroy_fasta_sequence(Seq_t *seq) {
	seq->name[0] = '\0';
	seq->desc[0] = '\0';
	seq->length = 0;
	free(seq->acids);
	seq->acids = NULL;
}

/**
 * Load biological sequence from library file.
 *
 * @param fp		handler to the path of library file
 * @param seq       pointer to the result sequence
 *
 * @return			0--success, other values means failed
 */
static int read_fasta_sequence(FILE *fp, Seq_t *seq) {
	int i;
	char line[1024];
	
	//Get the header information of the sequence.
	while (fgets(line, sizeof(line), fp) != NULL) {
		// Read the first non space character.
		if (line[0] == INFO_FLAG) {
			break;
		}
	}

	if (line == NULL) {
		// Error occurred.
		return 1;
	}

	//Get the name of the sequence.
	for (i = 1; i < 50; ++i) {
		if (line[i] == ' ' || line[i] == '\t') {
			break;
		}
		seq->name[i-1] = line[i];
	}
	seq->name[i-1] = '\0';

	//Get the description of the sequence.
	strcpy(seq->desc, line + i + 1);
	*(strchr(seq->desc, '\n')) = '\0';

	//Initialize the sequence length to zero. 
	seq->length = 0;
	seq->acids = NULL;

	//Read the sequence.
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (line[0] == INFO_FLAG) {
			break;
		}
		seq->acids = (char*) realloc(seq->acids, strlen(line) + seq->length);
		for (i = 0; i < strlen(line); ++i) {
			if (line[i] == '*' || line[i] == '-') {
				seq->acids[seq->length++] = line[i];
			} else if (isalpha(line[i])) {
				seq->acids[seq->length++] = toupper(line[i]);
			}
		}
	}
	seq->acids[seq->length] = '\0';

	return 0;
}

