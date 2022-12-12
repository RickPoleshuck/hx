/*
        @(#) hxcut - support for cut and paste

        inputs:
                start - starting position
                length - number of bytes to print
                output - name of output file

        output:
                none
*/
#include "hx.h"

hxcutmn()
{
	long start;
	long length;
	char tbuf[80];
	char output[80];

	wcommwin( "Cut from (B)yte, (S)ector, or (C)urrent location? C\b" );
	/* get 1 character uppercased and verified
	   for 'b,s,c' */
	hxgetfld ( commwin, tbuf, isbsc, toupper, 1 );
	switch ( *tbuf ){
	 case '\0':
	 case 'C':
		start = homepos;
		length = 512;
		break;
	 case 'S':        /* enter sector number */
		wcommwin ( "Enter starting sector number in decimal: " );
		hxgetfld ( commwin, tbuf, isnum, 0, 7 );
		if ( !*tbuf )
			start = fpos;
		else {
			sscanf ( tbuf, "%d", &start );
			start *= 512;
		}
		wcommwin( "Enter number of sectors in decimal: " );
		hxgetfld ( commwin, tbuf, isnum, 0, 7 );
		if ( !*tbuf )
			length = 512;
		else {
			sscanf ( tbuf, "%d", &length );
			length *= 512;
		}
		break;
	 case 'B':        /* enter byte address in hex */
		wcommwin( "Enter starting byte in hexadecimal: " );
		hxgetfld ( commwin, tbuf, ishex, 0, 7 );
		if ( !*tbuf )
			start = fpos;
		else
			sscanf ( tbuf, "%d", &start );

		wcommwin( "Enter number of bytes in decimal: " );
		hxgetfld ( commwin, tbuf, isnum, 0, 7 );
		if ( !*tbuf )
			length = 512;
		else {
			sscanf ( tbuf, "%d", &length );
		}
		break;
	}
	wcommwin ( "Enter name of output file to append to: " );
	hxgetfld ( commwin, output, isprint, 0, 60 );
	if ( !*output )
		return;        /* no filename specified - return */
	oktoexit = FALSE;
	hxcut ( start, length, output );
	oktoexit = TRUE;
}


hxcut( start, length, output )
long start;
long length;
char *output;
{
	FILE *outfp;
	char buffer[BUFSIZ];
	int size;
	int nread;
	int nwrit;

        if ( (outfp = fopen ( output, "a" )) == NULL ){
                beep();
                wcommwin ( "ERROR opening output file '" );
                waddstr ( commwin, output );
                waddstr ( commwin, "'" );
		wrefresh ( commwin );
                return;
        }

        fseek ( fp, start, 0 );
        while ( length > 0 ){
                if ( length > BUFSIZ )
                        size = BUFSIZ;
                else
                        size = length;

                nread = fread( buffer, 1, size, fp );
                if ( nread != size ){
                        beep();
                        wcommwin ( "Error reading file!!!");
                        return;
                }
                nwrit = fwrite ( buffer, 1, nread, outfp );
                if ( nwrit != nread ){
                        beep();
                        wcommwin ( "Error writing to output file '" );
			waddstr ( commwin, output );
			waddstr ( commwin, "'" );
			wrefresh ( commwin );
                        return;
                }
                length -= nread;
        }
        fclose ( outfp );
}
