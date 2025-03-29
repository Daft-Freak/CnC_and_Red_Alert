#include <stdio.h>

#include "dipthong.h"

char *Extract_String(void const *data, int string)
{
	unsigned short int	const *ptr;
	unsigned intoffset;

	if (!data || string < 0) return(NULL);
		
	ptr = (unsigned short int const *)data;

	// assume offset of first string is end of index table
	int numstrings = ptr[0] / 2;

	// don't index past the end (might happen if expansion files missing)
	if(string >= numstrings)
		return NULL;

	return (((char*)data) + ptr[string]);
}