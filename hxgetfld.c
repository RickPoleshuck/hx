/***********************************************************************

        @(#) hxgetfld.c - get a field with some minor verification

   description:

        Inputs a field from a curses window

   module name:  getfield       author: Rick Poleshuck
                                date:   12/5/88
   inputs:

        str - ptr to output string, calling program ensures enough room.

        vfunct - ptr to function that verifies each character.
                verification returns  TRUE for character acceptance
                                      FALSE for invalid character

        mfunct - ptr to function to modify each character after
                 verification. Accepts and returns an integer.

        max - maximum length of input.

        wind - window used for input

   outputs:

        TRUE value is ok.
        FALSE incorrect value enterred.

   processing:


   special considerations:

        stty's should be set to echo turned off and one character input.

***********************************************************************/
#include <curses.h>
#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif
#define BS '\010'
#define SP '\040'

void wputch();

hxgetfld ( wind, str, vfunct, mfunct, max )
WINDOW *wind;
char *str;
int (*vfunct)();
int (*mfunct)();
int max;
{
        char c = 0;              /* first time through must not be BS */
        char *sptr = str;

        while ( 1 ){
                c = wgetch (wind );
                switch ( c ) {
                 case BS:
                        if ( sptr > str ){      /* backup one */
                                --sptr;
                                waddstr ( wind, "\b\040\b" );
                                wrefresh ( wind );
                        } else
                                beep(); /* ring bell */
                        break;

                 case '\r':
                 case '\n':
                        *sptr = '\0';
                        return ( TRUE );

                 default:
                        if ( (sptr - str) > max ){      /* too long */
                                beep();
                                break;
                        }

                        if ( (*vfunct)( c )){   /* if c is OK then */
                                if ( mfunct )   /* if there is a mod funct */
                                        c = (*mfunct)(c);
                                                /* modify the character */
                                /* then print the character */
                                wputch ( wind, c );
                                /* and store it in the buffer */
                                *sptr++ = c;
                        } else
                                beep ();
                        break;
                }
        }
}

static void
wputch ( wind, c )
WINDOW *wind;
int c;
{
        waddch ( wind, c );
        wrefresh ( wind );
}

