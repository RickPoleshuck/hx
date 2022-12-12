/* @(#) hxpatch - patch functions for hx */
#include "hx.h"

patch(fname)
char *fname;
{
        unsigned int c;
        short nread;
        char buf[CHARS_ROW*MAXROWS];
        char mode = ASC;

        mvwdeleteln(menuwin,0,0);
        wdeleteln(menuwin);

        wmove ( menuwin, 0, 0 );
        waddstr ( menuwin, patch1 );
        wmove ( menuwin, 1, 0 );
        waddstr ( menuwin, patch2 );
        wrefresh( menuwin );

        fseek ( fp, homepos, 0 );
        nread = fread ( buf, 1, CHARS_ROW*MAXROWS, fp );
		keypad( datawin, TRUE );
        wmove ( datawin, 0, ASC_COL0 );
        while ( 1 ){
                wrefresh( datawin );
                c = wgetch (datawin);
                switch ( c ){
                 case '\t':
                        mode= mode==ASC?HEX:ASC;
                        /* fall through */
                 case ('h' - 0x40):     /* <control keys */
                 case ('j' - 0x40):
                 case ('k' - 0x40):
                 case ('l' - 0x40):
                 case KEY_LL:
                 case KEY_HOME:
                 case KEY_UP:
                 case KEY_DOWN:
                 case KEY_LEFT:
                 case BS:
                 case KEY_RIGHT:
                 case '\n':
                 case '\r':
                        pchmov ( c, mode );
                        break;
                 case ESC:
endpatch:
                        wcommwin( "Write changes (y/n)?: ");
                        echo();
                        c = wgetch(commwin);
                        c = toupper ( c );
                        noecho();

                        mvwdeleteln(menuwin,0,0);
                        wdeleteln(menuwin);
                        wmove ( menuwin, 0, 0);
                        waddstr ( menuwin, menu1 );
                        wmove ( menuwin, 1, 0);
                        waddstr ( menuwin, menu2 );
                        wmove ( menuwin, 2, 0);
                        waddstr ( menuwin, menu3 );
                        wrefresh( menuwin );
                        if ( c == 'Y' ){        /* write changes */
                                fseek ( fp, homepos, 0 );
                                fwrite ( buf, 1, nread, fp );
                                fflush ( fp );
								keypad( datawin, FALSE );
                                return;
                        }
                        /* put back original data */
                        fpos = homepos;
                        disppage(fname);

                        wcommwin ( "Changes ignored!");
						keypad( datawin, FALSE );
                        return;

                 default:
                        if ( mode == ASC ){
                                if ( isprint ( c ) )
                                        ascrepl ( (char)c, buf );
                        } else if ( mode == HEX && ishex ( c ) )
                                hexrepl ( (char)c, buf );
                        else
                                flash ();
                        break;
                }
                if ( doneflag ){
                        doneflag=FALSE;
                        goto endpatch;
                }
        }
}


pchmov ( c, mode )
int c;
char mode;
{
        char x,y;
        int col0, factor, mid;

        if ( mode == ASC ){
                factor = 1;
                col0 = ASC_COL0;
                mid  = ASC_MID;
        } else {
                factor = 3;
                col0 = HEX_COL0;
                mid  = HEX_MID;
        }

        getyx ( datawin, y, x );

        switch ( c ){
         case '\t':
                wmove ( datawin, y, col0 );
                return;

         case KEY_LL:
                wmove ( datawin, MAXROWS-1, col0 + CHARS_ROW );
                return;

         case KEY_HOME:
                wmove ( datawin, 0, col0 );
                return;

     case ('h' - 0x40):     /* <control h> */
         case KEY_LEFT:
         case BS:
                if ( x == col0 ){               /* at left margin */
                        if ( y == 0 ){          /* at home position */
                                flash();
                                return;
                        }
                        --y;
                        x = col0 + ((CHARS_ROW-1)*factor)+1; /* goto pre line */
                } else
                        /* backup, position over center*/
                        x -= (factor + (x==mid?1:0));
                wmove ( datawin, y, x );
                return;

     case ('l' - 0x40):         /* <control l> */
         case KEY_RIGHT:
                x += factor;
                if ( x == col0+(CHARS_ROW*factor) ){
                        if ( y == MAXROWS-1 ){
                                flash();
                                return;
                        }
                        ++y;
                        x = col0;
                }
                if ( x == mid ) ++x;
                wmove ( datawin, y, x );
                return;

     case ('j' - 0x40):         /* <control j> */
         case KEY_DOWN:
                if ( y == MAXROWS-1 ){
                        flash();
                        return;
                }
                wmove ( datawin, ++y, x );
                return;

     case ('k' - 0x40):         /* <control k> */
         case KEY_UP:
                if ( y == 0 ){
                        flash();
                        return;
                }
                wmove ( datawin, --y, x );
                return;
         case '\n':
         case '\r':
                if ( y == MAXROWS-1 ){
                        flash();
                        return;
                }
                wmove ( datawin, ++y, col0 );
                return;

        }
}

srchasc(str,len,fname)
register char *str;
int len;
char *fname;
{
        register int i;
        register int c;
        register long cursec;
        register char *ptr;

        cursec = fpos / 512;    /* check for when to repaint heading */
        while ( !doneflag && (c = getc(fp)) != EOF ) {
                if ( ++fpos / 512 != cursec ){
                        cursec = fpos / 512;
                        hxfsect ( fpos );
                        wrefresh(headwin);
                }
                if (c == *str){
                        for (ptr = str+1,i=len; --i; ptr++)
                                if ((c = getc(fp)) != *ptr){
                                        fseek ( fp, fpos, 0 );
                                        break;
                                }

                        if (!i){
                                --fpos;
                                return( TRUE );
                        }
                }
        }
        if ( doneflag == TRUE )
                disppage(fname);
        doneflag = FALSE;
        return( FALSE );
}
