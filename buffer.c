#include <string.h>
#include "buffer.h"

Buffer* b_allocate(short init_capacity, char inc_factor, char o_mode) {
	if (init_capacity < 0 || init_capacity > SHRT_MAX - 1) {
		return NULL;
	}

	Buffer* init = NULL;

	if ((init = (Buffer*)calloc(1, sizeof(Buffer))) == NULL) {
		return NULL;
	}

	if (init_capacity == 0) {
		if ((init->cb_head = (char*)malloc(sizeof(char) * DEFAULT_INIT_CAPACITY)) == NULL) {
			return NULL;
		}
		init->capacity = DEFAULT_INIT_CAPACITY;
		if (o_mode == 'f') {
			init->inc_factor = 0;
			init->mode = O_MODE_F;
		}
		else if (o_mode == 'a' || o_mode == 'm') {
			init->inc_factor = (unsigned char)DEFAULT_INC_FACTOR;
			if (o_mode == 'a') {
				init->mode = O_MODE_A;
			}
			else if (o_mode == 'm') {
				init->mode = O_MODE_M;
			}
			else {
				b_free(init);
				init = NULL;
				return NULL;
			}
		}
	}
	else {
		if ((init->cb_head = (char*)malloc(sizeof(char) * init_capacity)) == NULL) {
			return NULL;
		}

		if ((o_mode == 'f' || (unsigned char)inc_factor == 0) || ((unsigned char)inc_factor == 0 && (unsigned char)inc_factor != 0)) {
			init->mode = O_MODE_F;
			init->inc_factor = 0;
		}
		else if (o_mode == 'a' && 1 <= (unsigned char)inc_factor <= 255) {
			init->mode = O_MODE_A;
			init->inc_factor = (unsigned char)inc_factor;
		}
		else if (o_mode == 'm' && 1 <= (unsigned char)inc_factor <= 100) {
			init->mode = (signed char)O_MODE_M;
			init->inc_factor = (unsigned char)inc_factor;
		}
		else {
			b_free(init);
			init = NULL;
			return NULL;
		}
		init->capacity = init_capacity;
	}

	init->flags = 0xFFFC;
	init->getc_offset = 0;
	return init;
}

Buffer* b_addc(pBuffer const pBD, char symbol)
{
	if (pBD == NULL) {
		return NULL;
	}

	short new_cap = 0; /*local variable to assign new memory for Buffer */
	char* new_Array = NULL; /* local variable for resizing Buffer */
	short available_space; /* new available space */
	short new_inc; /* The new inc_factor */

	if ((pBD->flags & CHECK_R_FLAG) == 1) {
		pBD->flags &= RESET_R_FLAG; /* Reset r_flag if need be */
	}

	if (b_isFull(pBD) == 0) { /*If buffer is not full*/
		*(pBD->cb_head + pBD->addc_offset) = symbol; /*Add sybol to buffer and increment addc_offset*/
		pBD->addc_offset++;
		return pBD;
	}

	if (pBD->mode == 0) {
		return NULL; /*If the buffer is full and Operational mode is 0 then return null*/
	}

	if (pBD->mode == 1) {
		new_cap = pBD->capacity + (short)pBD->inc_factor; /* Creates new capacity by adding inc_factor to current cap */
		if (new_cap > 0 && new_cap > (SHRT_MAX - 1)) {
			new_cap = SHRT_MAX - 1; /* If new_cap is positive but exceeds the max allowed value reassing to the max value */
		}

		if (new_cap < 0) { /* If overflow has occured return NULL */
			return NULL;
		}
	}

	if (pBD->mode == -1) {
		available_space = (SHRT_MAX - 1) - pBD->capacity;
		new_inc = available_space * (unsigned char)pBD->inc_factor / 100;
		new_cap = pBD->capacity + new_inc;

		if (0 > new_cap || new_cap > (SHRT_MAX - 1)) {
			new_cap = SHRT_MAX - 1; /* If the new_cap is larger than the allowed max but the current cap is not full assign max cap */
		}
	}

	; /* Reallocate buffer size with new_cap*/
	if ((new_Array = (char*)realloc(pBD->cb_head, sizeof(char) * new_cap)) == NULL) { /* Check to make sure realloc worked */
		return NULL;
	}

	if (pBD->cb_head != new_Array) { /*Need to set r_flag to 1*/
		pBD->flags |= SET_R_FLAG;
	}
	pBD->cb_head = new_Array;
	pBD->cb_head[pBD->addc_offset] = symbol; /*Add sybol to buffer and increment addc_offset*/
	pBD->addc_offset++;
	pBD->capacity = new_cap;
	return pBD;
}

int b_clear(Buffer* const pBD)
{
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->addc_offset = 0;
	pBD->getc_offset = 0;
	pBD->markc_offset = 0;
	pBD->mode = 0;
	pBD->flags &= RESET_R_FLAG;
	pBD->flags &= RESET_EOB;

	return 0;
}

void b_free(Buffer* const pBD) {
	if (pBD != NULL) {
		free(pBD->cb_head);
		free(pBD);
	}
}

int b_isFull(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	if (pBD->addc_offset == pBD->capacity) {
		return 1;
	}

	return 0;
}

short b_limit(Buffer* const pBD) {
	if (pBD == NULL)
	{
		return RT_FAIL_1;
	}

	return pBD->addc_offset;
}

short b_capacity(Buffer* pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}
	return pBD->capacity;
}

short b_mark(pBuffer const pBD, short mark) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	if (0 <= mark <= pBD->addc_offset)
	{
		pBD->markc_offset = mark;
		return pBD->markc_offset;
	}
	else {
		return RT_FAIL_1;
	}
}

/*This method doesn't specify -1 to return just says to notify calling function*/
int b_mode(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return pBD->mode;
}

size_t b_incfactor(Buffer* const pBD) {
	if (pBD == NULL) {
		return 0x100;
	}

	return (unsigned char)pBD->inc_factor;
}

int b_load(FILE* const fi, Buffer* const pBD) {
	if (pBD == NULL || fi == NULL) {
		return RT_FAIL_1;
	}
	char next; /*The next char from the file*/
	int counter = 0; /*Count the characters added*/

	while (1) {
		next = (char)fgetc(fi);
		if (feof(fi)) { /*Detect end of file before calling b_addc()*/
			break;
		}
		if (b_addc(pBD, next) == NULL) {
			(char)ungetc(next, fi);
			return LOAD_FAIL;
		}
		counter++;
	}
	return counter;
}

int b_isempty(Buffer* const pBD)
{
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	if (pBD->addc_offset == 0) {
		return 1;
	}

	return 0;

}

char b_getc(Buffer* const pBD)
{
	if (pBD == NULL) {
		return RT_FAIL_2;
	}

	if (pBD->getc_offset == pBD->addc_offset) {
		pBD->flags |= SET_EOB; /*Using bitwise operation it sets the flags field eob bit to 1 and returns 0*/
		return 0;
	}

	pBD->flags &= RESET_EOB;/* using bitwise operation sets eob to 0*/
	char temp = (char)pBD->cb_head[pBD->getc_offset];
	pBD->getc_offset++;
	return temp;
}

int b_eob(Buffer* const pBD)
{
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return (pBD->flags >> 1) & 1; /* Will return the EOB bit */
}

int b_print(Buffer* const pBD, char nl) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	int counter = 0;
	char c;

	do {
		c = b_getc(pBD);
		if (b_eob(pBD)) break;
		printf("%c", c);
		counter++;
	} while (!b_eob(pBD));

	if (nl != 0) {
		printf("\n");
	}
	return counter;
}

Buffer* b_compact(Buffer* const pBD, char symbol) {

	char* temp_array; /* temp array for realloc */
	short new_cap; /* The new capacity */

	new_cap = pBD->addc_offset + 1;
	if (new_cap < 0 || new_cap == SHRT_MAX) {
		return NULL;
	}

	if ((temp_array = (char*)realloc(pBD->cb_head, sizeof(char) * new_cap)) == NULL) {
		return NULL;
	}

	pBD->capacity = new_cap;
	if (pBD->cb_head != temp_array) {
		pBD->flags |= SET_R_FLAG;
	}
	pBD->cb_head = temp_array;
	pBD->cb_head[pBD->addc_offset] = symbol;
	pBD->addc_offset++;

	return pBD;
}

char b_rflag(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return pBD->flags & 1; /* Will return the EOB bit */
}

short b_retract(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset--;
	return pBD->getc_offset;
}

short b_reset(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset = pBD->markc_offset;
	return pBD->getc_offset;
}

short b_getcoffset(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}
	return pBD->getc_offset;
}

int b_rewind(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset = 0;
	pBD->markc_offset = 0;
	return 0;
}

char* b_location(Buffer* const pBD) {
	char* temp; /* The pointer to cb_head at markc_offset */
	temp = &*(pBD->cb_head + pBD->markc_offset);
	return temp;
}