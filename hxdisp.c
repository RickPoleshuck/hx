/* @(#) hxdisp.c - display functions for hx */
#include "hx.h"
static char oldname[61] = { 0};

disppage(fname)
char *fname;
{
        char nrows = MAXROWS;
        char headbuf[80];

        if ( strcmp ( oldname, fname) ){
                /* set headwindow static strings */
                strcpy ( oldname, fname);
                memset ( headbuf, '-', 79 );
                headbuf[79] = '\0';
                wmove ( headwin, 0,0 );
                waddstr ( headwin, headbuf );
                wmove ( headwin, 1, 0 );
		wdeleteln ( headwin );		/* wclear is buggy */
		wdeleteln ( headwin );
                waddstr(headwin,"File: ");
                waddstr(headwin,fname);
                wmove ( headwin, 2,0 );
                waddstr ( headwin, headbuf );
        }
        if ( readonly ){
                wmove ( headwin, 1, 53 );
                waddstr(headwin,"READONLY");
        }
retry:
        homepos = fpos;
        /* display heading position */
        hxfsect (fpos );
        wrefresh ( headwin );

        wmove ( datawin, 0, 0 );
        while ( nrows-- ){
                if ( doneflag ){
                        doneflag = FALSE;
                        break;
                }
                if (  displine () == -1 ){
                        if ( homepos == 0 ){ /* EOF and can't go back */
                                while ( nrows-- )  wdeleteln(datawin);
                                break;
                        }

                        /* EOF - repaint screen without hitting EOF */
                        fpos = homepos - ((nrows+1) * CHARS_ROW);
                        if ( fpos < 0 ) fpos = 0;
                        nrows = MAXROWS;
                        goto retry;
                }
        }
        wrefresh(datawin);
}

/* returns -1 for end of file or current pos in file */
displine ()
{
        register char nread;
        char buffer[CHARS_ROW];
        char outbuf[81];

        if (!fp || fseek ( fp, fpos, 0 ))
                return ( -1 );

        nread = fread(buffer,1,CHARS_ROW,fp);
        hxfline ( fpos, buffer, outbuf, nread );
        waddstr ( datawin, outbuf );
        waddch ( datawin, '\n' );

        fpos += CHARS_ROW;
        if ( nread == 0 )       /* EOF - no characters read */
                return ( -1 );
        return ( 0 );
}

putx ( c )      /* put a byte in hex format and append a blank */
register unsigned char c;
{
        char buf[3];

        sprintf ( buf, "%2.2x ", c );
        waddstr ( datawin, buf );
}


ascrepl ( c, buffer )
unsigned char c;
char *buffer;
{
        int x,y;

        getyx ( datawin, y, x );

        /* replace char in buffer adjusting for middle space */
        buffer[(((x>=ASC_MID)?x-1:x)-ASC_COL0) + y*CHARS_ROW] = c;

        waddch ( datawin, c );			/* replace ascii character */

        /* replace hex field  adjusting for middle space */
        wmove ( datawin, y, HEX_COL0 + ((x-ASC_COL0)*3)-(x>=ASC_MID?2:0) );
        putx ( c );
        wrefresh( datawin );

        /* reposition cursor */
	if ( ++x == (ASC_MID-1) ) ++x;		/*adjust for middle space */
        if ( x == ASC_COL0 +CHARS_ROW +1){	/* check for end of line */
                if ( y == MAXROWS-1 ){		/* or end of screen */
                        y = 0;
                        x = ASC_COL0;
                } else {
                        ++y;
                        x = ASC_COL0;
                }
        }
        wmove ( datawin, y, x );
}

hexrepl ( c, buffer )
unsigned char c;
char *buffer;
{
        char x,y;
	char buf[3];
	int  temp;

        getyx ( datawin, y, x );

        c=toupper( c );
        waddch ( datawin, c );
        wrefresh( datawin );

        /* get second character */
        while (1) {
                buf[1] = wgetch ( datawin );
                if ( ishex ( buf[1] )) break;
        }
        buf[1] = toupper (buf[1] );
        waddch ( datawin, buf[1] );
        wrefresh( datawin );

        /* replace ascii character */
	buf[0] = c;
	buf[2] ='\0';
	sscanf ( buf, "%x", &temp );
	c = temp;
        temp=(x-HEX_COL0)/3;	/* rounding will adjust for middle space */
        buffer[(temp + y*CHARS_ROW)] = c;

        wmove ( datawin, y, temp + ASC_COL0 + (x>=HEX_MID?1:0));
        if ( isprint ( c ))
                waddch ( datawin, c );
        else    waddch ( datawin, '.' );
        wrefresh( datawin );

        x += 3;
        if ( x == HEX_COL0+1 + (CHARS_ROW*3) ){
                if ( y == MAXROWS-1 ){
                        y = 0;
                        x = HEX_COL0;
                } else {
                        ++y;
                        x = HEX_COL0;
                }
        } else if ( x == HEX_MID ) ++x;
		
        wmove ( datawin, y, x );
}

/* convert hex string to binary string
   len = length of output, ie input length / 2
   returns ( -1 ) on error during conversion
*/
htos(out,in,len)
register char *in;
register char *out;
{
        char temp[3];
        register char *ptr;
        int c;

        temp[2] = '\0';

        for ( ptr = in; len-- ; ptr +=2 ){
                strncpy ( temp, ptr, 2 );
                if ( sscanf ( temp, "%x", &c ) != 1 )
                        return ( -1 );
                *out++ = c;
        }
        *out = '\0';
        return ( 0 );
}

wcommwin ( str )
char *str;
{
	wmove ( commwin, 0 , 0 );
	wdeleteln ( commwin );
	waddstr ( commwin, str );
	wrefresh ( commwin );
}
