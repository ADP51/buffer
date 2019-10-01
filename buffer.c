/**********************************************************************************************************
* File Name:		buffer.c
* Compiler:			MS Visual Studio 2013
* Author:			Andrew Palmer
* Course:			CST 8152 - Compilers, Lab Section: 012
* Assignment:		1
* Date:				29/9/2019
* Professor:		Sv.Ranev
* Purpose:			Contains all buffer functions.
* Function list:	b_allocate(), b_addc(), b_clear(), b_free(), b_clear(),b_isFull(),
					b_limit(), b_capacity(), b_mark(), b_mode(), b_incfactor(), b_load(),
					b_isempty(), b_getc(), b_eob(), b_print(), b_compact(), b_rflag(),
					b_retract(), b_reset(), b_getcoffset(), b_rewind(), b_location()
**********************************************************************************************************/

#include <string.h>
#include "buffer.h"

/**********************************************************************************************************
* Purpose:			Creates the buffer structure and allocates memory.
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	malloc(), b_free(), calloc()
* Parameters:		init_capacity	-short	Has to be between 0 and SHRTMAX-1
*					inc_factor		-char	Must be positive. Range
*											between 0 to 255 inclusive unsigned cast
*					o_mode			-char	must be a letter in f,a,m
* Return Value:		init			*Buffer	Points to valid buffer structure
*
* Algorithm:		Initializes the buffer structure and all of the necessary properties, based on 
					arguments passed to the function.
**********************************************************************************************************/
Buffer* b_allocate(short init_capacity, char inc_factor, char o_mode) {
	if (init_capacity < 0 || init_capacity > SHRT_MAX - 1) { /*Check init cap range*/
		return NULL;
	}

	Buffer* init = NULL;
	unsigned char cast_inc_factor = (unsigned char)inc_factor; /* Cast inc factor once */

	if ((init = (Buffer*)calloc(1, sizeof(Buffer))) == NULL) {/*Initialize Buffer struct memory */
		return NULL;
	}
	
	if (init_capacity == 0) { /*If the init_capacity parameter is 0*/
		if ((init->cb_head = (char*)malloc(sizeof(char) * DEFAULT_INIT_CAPACITY)) == NULL) {/* initialize memory for buffer */
			return NULL;
		}
		init->capacity = DEFAULT_INIT_CAPACITY; 
		if (o_mode == 'f') {
			init->inc_factor = 0;
			init->mode = O_MODE_F;
		}	
		else if (o_mode == 'a') {
			init->mode = O_MODE_A;
			init->inc_factor = (unsigned char)DEFAULT_INC_FACTOR;
		}
		else if (o_mode == 'm') {
			init->mode = O_MODE_M;
			init->inc_factor = (unsigned char)DEFAULT_INC_FACTOR;
		}
		else {
			b_free(init); /* If the o_mode is not valid free the initialized memory and return null*/
			init = NULL;
			return NULL;
		}
	}
	else {	/*If init_capacity is not 0*/
		if ((init->cb_head = (char*)malloc(sizeof(char) * init_capacity)) == NULL) {
			return NULL;
		}

		if (o_mode == 'f') {
			init->mode = O_MODE_F;
			init->inc_factor = 0;
		}
		else if (inc_factor == 0 && init_capacity != 0) {
			init->mode = 0;
			init->inc_factor = 0;
		}
		else if (o_mode == 'a' && (cast_inc_factor >= 1 && cast_inc_factor <= 255)) {
			init->mode = O_MODE_A;
			init->inc_factor = cast_inc_factor;
		}
		else if (o_mode == 'm' && cast_inc_factor >=1 && cast_inc_factor <= 100) {
			init->mode = (signed char)O_MODE_M;
			init->inc_factor = cast_inc_factor;
		}
		else {
			b_free(init);
			init = NULL;
			return NULL;
		}
		init->capacity = init_capacity;
	}

	init->flags = DEFAULT_FLAGS; /*Set the default flags*/
	
	return init;
}

/**********************************************************************************************************
* Purpose:			Adds a character to the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	isFull(), realloc()
* Parameters:		pBD				-A pointer to an existing bufferrr
					symbol			-The given chear to add
* Return Value:		pBD				Pointer to the updated buffer
*
* Algorithm:		Will check to see if the buffer is full, if not it will add the char at the apropriate
					position. If the buffer is full, will attempt to reallocate memory for the buffer to 
					allow for more characters up to the max.
**********************************************************************************************************/
Buffer* b_addc(pBuffer const pBD, char symbol)
{
	if (pBD == NULL) {
		return NULL;
	}

	short new_cap; /*local variable to assign new memory for Buffer */
	char* new_Array = NULL; /* local variable for resizing Buffer */
	short available_space; /* new available space */
	short new_inc; /* The new inc_factor */

	if ((pBD->flags & CHECK_R_FLAG) == 1) {
		pBD->flags &= RESET_R_FLAG; /* Reset r_flag if need be */
	}

	if (b_isFull(pBD) == 0) { /*If buffer is not full*/
		*(pBD->cb_head + pBD->addc_offset) = symbol; /*Add symbol to buffer and increment addc_offset*/
		pBD->addc_offset++;
		return pBD;
	}

	if (pBD->mode == 0) {
		return NULL; /*If the buffer is full and Operational mode is 0 then return null*/
	}

	if (pBD->mode == 1) {
		new_cap = pBD->capacity + (unsigned char)pBD->inc_factor; /* Creates new capacity by adding inc_factor to current cap */
		if (new_cap < 0 || new_cap > (SHRT_MAX - 1)) {
			new_cap = SHRT_MAX - 1; /* If new_cap is positive but exceeds the max allowed value reassing to the max value */
		}

		if (new_cap < 0) { /* If overflow has occured return NULL */
			return NULL;
		}
	}

	if (pBD->mode == -1) {
		if (pBD->capacity == SHRT_MAX - 1) { /*If buffer has already reached max cap return nulll*/
			return NULL;
		}
		available_space = (SHRT_MAX - 1) - pBD->capacity;
		new_inc = (available_space * (short)(unsigned char)pBD->inc_factor) / 100;
		new_cap = pBD->capacity + new_inc;

		if (new_inc == 0) {
			if(new_cap < SHRT_MAX - 1) {
				new_cap = SHRT_MAX - 1; /* If the new_cap is larger than the allowed max but the current cap is not full assign max cap */
			}
			else {
				return NULL;
			}
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

/**********************************************************************************************************
* Purpose:			Clears the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				0
*
* Algorithm:		Reset all of the flags and set all of the offsets, as well as the mode to 0
**********************************************************************************************************/
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

/**********************************************************************************************************
* Purpose:			Frees the memory allocated to the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		
*
* Algorithm:		Frees memory 
**********************************************************************************************************/
void b_free(Buffer* const pBD) {
	if (pBD != NULL) {
		free(pBD->cb_head);
		free(pBD);
	}
}

/**********************************************************************************************************
* Purpose:			Check if buffer is at capacity
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				0
*
* Algorithm:		
**********************************************************************************************************/
int b_isFull(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	if (pBD->addc_offset == pBD->capacity) {
		return TRUE;
	}

	return FALSE;
}

/**********************************************************************************************************
* Purpose:			Gets the current size of the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		short			the value of addc_offset
*
* Algorithm:		
**********************************************************************************************************/
short b_limit(Buffer* const pBD) {
	if (pBD == NULL)
	{
		return RT_FAIL_1;
	}

	return pBD->addc_offset - 1;
}

/**********************************************************************************************************
* Purpose:			Gets the current capcity of the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		short			the capacity of the buffer
*
* Algorithm:		
**********************************************************************************************************/
short b_capacity(Buffer* pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}
	return pBD->capacity;
}

/**********************************************************************************************************
* Purpose:			Sets the mark of a buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
					short			the new value for markc_offset
* Return Value:		short			the current value of markc_offset
*
* Algorithm:		Checks mark parameter, if parameter is acceptable then set makc_offset to mark and return
					otherwise return -1.
**********************************************************************************************************/
short b_mark(pBuffer const pBD, short mark) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	if (0 <= mark || mark <= pBD->addc_offset) { /* Verify mark within range */
		pBD->markc_offset = mark; /* Set markc_offset to mark */
		return pBD->markc_offset;
	}
	else {
		return RT_FAIL_1;
	}
}

/**********************************************************************************************************
* Purpose:			Gets the bufferr mode
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				the integer value of mode
*
* Algorithm:		
**********************************************************************************************************/
int b_mode(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return pBD->mode;
}

/**********************************************************************************************************
* Purpose:			Gets the inc_factor
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		size_t			size of the inc_factor
*
* Algorithm:		
**********************************************************************************************************/
size_t b_incfactor(Buffer* const pBD) {
	if (pBD == NULL) {
		return INCFACTOR_FAIL;
	}

	return (unsigned char)pBD->inc_factor;
}

/**********************************************************************************************************
* Purpose:			Gets the content of the given file pointer and loads it into the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	fgetc(), b_addc(), ungetc(), feof()
* Parameters:		fi				-A pointer to a given file
					pBD				-A pointer to an existing buffer
* Return Value:		int				The number of characterrs loaded 
*
* Algorithm:		Will loop through the file and get one character at a time until end of file is detected.
					b_addc() will be called with each character in order to add to the buffer. Will return
					LOAD_FAIL if character is unable tttto be added.
**********************************************************************************************************/
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

/**********************************************************************************************************
* Purpose:			Checks if buffer is empty
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				True/False
*
* Algorithm:		
**********************************************************************************************************/
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

/**********************************************************************************************************
* Purpose:			Gets the char at the current getc_offset 
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		char			the char at getc_offset
*
* Algorithm:		Check if the getc_offset has reached the end of the buffer, if it has it will set the 
					EOB to 1 and return 0. Otherwise it will reset EOB to 0 and return the char, getc_offset 
					is incremented.
**********************************************************************************************************/
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

/**********************************************************************************************************
* Purpose:			Gets the EOB flag
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				the EOB flag
*
* Algorithm:		
**********************************************************************************************************/
int b_eob(Buffer* const pBD)
{
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return (pBD->flags >> 1) & 1; /* Will return the EOB bit */
}

/**********************************************************************************************************
* Purpose:			Prints the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	getc(), b_getc(), b_eob(), printf()
* Parameters:		pBD				-A pointer to an existing buffer
					nl				new line flag
* Return Value:		int				how many characters were printed
*
* Algorithm:		Will loop though the buffer and print the char returned by b_getc()
**********************************************************************************************************/
int b_print(Buffer* const pBD, char nl) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	int counter = 0;	/*counter for the number of characters printed*/
	char c;				/* the placeholder for the current char */

	do {
		c = b_getc(pBD);		/* Gets the current char from the buffer */
		if (b_eob(pBD)) break;	/* If end of buffer end printing */
		printf("%c", c);
		counter++;
	} while (!b_eob(pBD));

	if (nl != 0) {
		printf("\n");			/* Check for new line */
	}
	return counter;
}

/**********************************************************************************************************
* Purpose:			Clears the buffer
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:	realloc()
* Parameters:		pBD				-A pointer to an existing buffer
					symbol			-Char to at at end
* Return Value:		pBD				Pointer to the updated buffer
*
* Algorithm:		Will try to resize the the buffer based on the current size of tthe buffer + 1 and adds symbol. 
					Sets the r_flag to 1 if memory location has changed during realloc
**********************************************************************************************************/
Buffer* b_compact(Buffer* const pBD, char symbol) {

	char* temp_array; /* temp array for realloc */
	short new_cap; /* The new capacity */

	new_cap = pBD->addc_offset + 1;
	if (new_cap < 0) { /*Compact can reach current max plus 1 (SHRT_MAX). Anything above that would cause overflow annnd be negative*/
		return NULL;
	}

	if ((temp_array = (char*)realloc(pBD->cb_head, sizeof(char) * new_cap)) == NULL) {
		return NULL;
	}

	pBD->capacity = new_cap;
	if (pBD->cb_head != temp_array) {
		pBD->flags |= SET_R_FLAG; /* Set r_flag to 1 */
	}
	pBD->cb_head = temp_array;
	temp_array = NULL;/* Remove Dangling pointer */
	pBD->cb_head[pBD->addc_offset] = symbol;
	pBD->addc_offset++;

	return pBD;
}

/**********************************************************************************************************
* Purpose:			Gets the r_flag
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		char			r_flag value
*
* Algorithm:		
**********************************************************************************************************/
char b_rflag(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	return (char)pBD->flags & 1; /* Will return the EOB bit */
}

/**********************************************************************************************************
* Purpose:			Decrements the getc_offset by one
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		short			new value of getc_offset
*
* Algorithm:		
**********************************************************************************************************/
short b_retract(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset--;
	return pBD->getc_offset;
}

/**********************************************************************************************************
* Purpose:			Reset the buffer to markc_offset
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		short			the adjusted value of getc_offset
*
* Algorithm:		
**********************************************************************************************************/
short b_reset(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset = pBD->markc_offset;
	return pBD->getc_offset;
}

/**********************************************************************************************************
* Purpose:			Gets the getc_offset
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		short			getc_offset
*
* Algorithm:		
**********************************************************************************************************/
short b_getcoffset(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}
	return pBD->getc_offset;
}

/**********************************************************************************************************
* Purpose:			Rewinds the buffer to be prinnted again
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		int				0
*
* Algorithm:		Sets the getc_offset and markset_offset to 0;
**********************************************************************************************************/
int b_rewind(Buffer* const pBD) {
	if (pBD == NULL) {
		return RT_FAIL_1;
	}

	pBD->getc_offset = 0;
	pBD->markc_offset = 0;
	return 0;
}

/**********************************************************************************************************
* Purpose:			Gets a pointer to a char at markc_offset
* Author:			Andrew Palmer
* History/Versions:	29/9/2019
* Called Function:
* Parameters:		pBD				-A pointer to an existing buffer
* Return Value:		char*			pointer to char
*
* Algorithm:		
**********************************************************************************************************/
char* b_location(Buffer* const pBD) {
	char* temp; /* The pointer to cb_head at markc_offset */
	temp = &*(pBD->cb_head + pBD->markc_offset);
	return temp;
}