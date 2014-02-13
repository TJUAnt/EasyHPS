//
// The FASTA biological sequence model.
//

#ifndef _FASTA_LIB_H_
#define _FASTA_LIB_H_

/**
 * The biological(DNA/Protein) sequence information type.
 */
typedef struct {
	char name[50];
	char desc[255];
	int  length;
	char *acids;
} Seq_t;

/**
 * Load biological sequence from library file.
 *
 * @param filename	pointer to the path of library file
 * @param seq       pointer to the biological sequence which holds the result
 * @param idx       the idx-th sequence
 *
 * @return			0--success, other values means failed
 */
int load_fasta_sequence(const char *filename, Seq_t *seq, int idx);

/**
 * Destroy biological sequence.
 *
 * @param seq		pointer to the biological sequence
 */
void destroy_fasta_sequence(Seq_t *seq);

#endif //_FASTA_LIB_H_
