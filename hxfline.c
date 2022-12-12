/*
        @(#) hxfline.c - format line of binary input

        inputs:
                address- address of binary input in file
                binary - up to CHARS_ROW bytes of binary input
                output - address of 81 bytes storage
                len    - length of input

        output:
                formatted line is placed in output
*/
#include "hx.h"

hxfline ( address, binary, output, len )
long address;
char *binary;
char *output;
int len;
{
        register int i;

        ltoh ( output, address );       /* format address in hex */
        output += 8;
        *output++ = ':';
        *output++ = SPACE;

        for ( i = 0; i < 8; ++i ){      /* format first 8 bytes in hex */
                if ( i < len )          /* check for short binary */
                        btoh ( output, (unsigned int )binary[i] );
                else
                        memset ( output, SPACE, 3 );

                output += 3;
        }
        *output++ = SPACE;              /* put center divider space */
        for ( i = 8; i < 16; ++i ){     /* format second 8 bytes in hex */
                if ( i < len )          /* check for short binary */
                        btoh ( output, (unsigned int)binary[i] );
                else
                        memset ( output, SPACE, 3 );

                output += 3;
        }

        strcpy ( output, "| " );        /* output ascii/hex divider */
        output += 2;

        for ( i = 0; i < 8; ++i ){      /* format first 8 bytes in ascii */
                if ( i < len )          /* check for short binary */
                        /* output '.' if not printable character */
                        *output++ = isprint(binary[i])?binary[i]:'.';
                else
                        *output++ = SPACE;
        }
        *output++ = SPACE;              /* put center divider space */
        for ( i = 8; i < 16; ++i ){     /* format second 8 bytes in hex */
                if ( i < len )          /* check for short binary */
                        /* output '.' if not printable character */
                        *output++ = isprint(binary[i])?binary[i]:'.';
                else
                        *output++ = SPACE;
        }
        *output = '\0';                 /* null terminate string */
}

/*
        btoh - converts byte to 2 byte hex with leading 0 and trailing space
*/
static void
btoh(char* str, int number)
{
        register unsigned int c;

        str += 2;                       /* point to last character in string */
        *str-- = SPACE;                 /* add trailing space */

        c = number & 0x0f;              /* store low nibble */
        if ( c > 9 )
                *str-- = c + ('a' - 10);
        else
                *str-- = c + '0';

        c = (number >> 4) & 0x0f;       /* get high nibble */
        if ( c > 9 )
                *str = c + ('a' - 10);
        else
                *str = c + '0';
}

/*
        ltoh - converts long to 8 byte hex with leading 0's
*/
static void
ltoh( char* str, long number )
{
        register int i;
        register unsigned int c;
        str += 7;                       /* point to last character in string */

        for ( i = 0; i < 8; ++i ){
                c = number & 0x0f;      /* get low nibble */
                if ( c > 9 )            /* store nibble */
                        *str-- = c + ('a' - 10);
                else
                        *str-- = c + '0';

                number >>= 4;           /* get next nibble */
        }
}

void
hxfsect (byte)
long byte;
{
        char buf[18];

        sprintf(buf,"Sector: %6.6ld-%d",(long)(byte/512l),
                (byte%512)>255?1:0);

        wmove ( headwin, 1, 62 );
        waddstr ( headwin, buf );
}
