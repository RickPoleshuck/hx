/***********************************************************************

    @(#) hx.c - interactive hex dump and patch utility

        cc -O -o hx hx.c -lcurses

        Author: Rick Poleshuck

        Warnings:       Curses is a CPU hog.
                        Terminfo definitions are often inaccurate.
                        This utility is intended to be used only on
                        regular files although it will work readonly on
                        directories also.

***********************************************************************/
#include "hx.h"
#ifndef DOS
#include <sys/ioctl.h>
#endif

char whatstring[] = "\
@(#) hx 1.2 - Author: Rick Poleshuck, Cranford, NJ, 04/20/89";

WINDOW *menuwin,
       *headwin,
       *datawin,
       *commwin;    /* windows */
FILE *fdopen(), *fp = 0;
long fpos;                      /* global position if file */
long homepos;                   /* position of home character on screen */
char noseek;                    /* This not a regular file - no seeks allowd */
char rmcomm = FALSE;            /* remove command window flag */
char readonly;                  /* open readonly flag */
char doneflag = FALSE;          /* signal caught flag */
char oktoexit = TRUE;           /* should we exit when interrupt received */
char *progname;
char oktorepeat = FALSE;        /* ok to repeat search flag */
char menu1[]=
"(f)orward, (b)ackword, (g)oto sector, (l)ast sector, (s)earch, (p)atch, (q)uit";
char menu2[]=
"(r)epeat search, (d)raw screen, (n)ew file, (h)ard copy, (c)ut & paste";
char menu3[]=
"(!)shell, <cursor up> <cursor down> ";
char patch1[]=
"<TAB> toggles hex or ascii, <ESC> to end patch mode.";
char patch2[]=
"<cursor up> <cursor down> <cursor right> <cursor left > scrolls.";

#ifndef DOS
struct termio termorig;
struct termio termhx;
#endif


void
main(argc,argv)
int argc;
char **argv;
{
        progname = argv[0];     /* save name of program in global */
        if ( !isatty ( 0 ) || !isatty ( 1 ) ){
                fprintf(stderr,"ERROR: %s terminal required.\n",progname);
                exit ( 1 );
        }

        if ( argc > 2 ){
                fprintf ( stderr, "Usage: %s {filename}\n", progname );
                exit(2);
        }
#ifndef DOS
        ioctl(0,TCGETA,&termorig);      /* save terminal config for '!' */
#endif
        signal( SIGINT, done );
        initscr();
        cbreak();
        nonl();
        noecho();

#ifndef DOS
        ioctl(0,TCGETA,&termhx);                /* save terminal for '!' */
#endif

        /* create windows */
        menuwin = newwin ( 3, 79, MENU, 0 );
        headwin = newwin ( 3, 79, HEAD, 0 );
        datawin = newwin ( MAXROWS, 79, DATA, 0 );
        commwin = newwin ( 1, 79, MAXROWS+DATA+1,0 );  /* command window */

        /* init menu window */
        wmove ( menuwin, 0, 0);
        waddstr ( menuwin, menu1 );
        wmove ( menuwin, 1, 0);
        waddstr ( menuwin, menu2 );
        wmove ( menuwin, 2, 0);
        waddstr ( menuwin, menu3 );
        wrefresh( menuwin );

        idlok(datawin,TRUE);

        hex(*++argv);

        oktoexit = TRUE;
        done();
	/* NOTREACHED */
}

hex (file)
char *file;
{
        int c;
        char srch[512];
        char fname[61];
        int len;                /* length of search string */
        long tpos;              /* temp for last position */
	char *shell;		/* name of current shell for '!' */

        if ( !file ){           /* no filename specified */
                disppage("NO FILE SPECIFIED!!!!");
                while ( !hxgetfn( fname ))
                        ;
                rmcomm = TRUE;          /* remove command window flag */
        } else
                strcpy ( fname, file );

        /* open data file */
        while ( ! hxopen ( fname )){
                sprintf ( srch, "Cannot open file '%s'", fname );
                disppage(srch);
                while ( !hxgetfn( fname ))
                        ;
                rmcomm = TRUE;          /* remove command window flag */
        }

        disppage(fname);
        while (1){
                c = wgetch(commwin);
                if ( rmcomm ){  /* remove command window */
			wcommwin ( "" );
                        rmcomm = FALSE;
                }
                switch ( (char)c ){
                 case 'c':                       /* cut & paste support */
			rmcomm = TRUE;
			hxcutmn();
                        break;

                 case 'h':                       /* hard copy support */
			rmcomm = TRUE;
			hxhardmn(fname);
                        break;

                 case 'n':                       /* edit new file */
                        rmcomm = TRUE;
                        if ( fp ) fclose ( fp ); /* close old file */
                        while ( !hxgetfn( fname ))
                                ;
                        while ( !hxopen ( fname )){
                                sprintf( srch, "Cannot open file '%s'", fname );
                                disppage(srch);
                                while ( !hxgetfn( fname ))
                                        ;
                        }
			disppage( fname );
                        break;

                case '!':
			clear();refresh();
#ifndef DOS
                        ioctl(0,TCSETAF,&termorig);
			if ( !(shell = getenv ( "SHELL" )) )
				shell = "/bin/sh";
                        system ( shell );
                        ioctl(0,TCSETAF,&termhx);
#else
                        system();
#endif
                        /* FALLTHRU */

                case 'd':       /* redisplay screen */
                        wclear(stdscr);
                        touchwin(menuwin);
                        touchwin(headwin);
                        touchwin(datawin);
                        touchwin(commwin);
                        wrefresh(stdscr);
                        wrefresh(menuwin);
                        wrefresh(headwin);
                        wrefresh(datawin);
                        wrefresh(commwin);
                        break;

                case 'p':       /* patch */
                        rmcomm = TRUE;
                        oktoexit = FALSE;
                        if ( readonly ){
                                wcommwin( "Cannot patch readonly file.");
                                sleep ( 2 );
                                break;
                        }
                        wcommwin( "Patch mode");
                        patch(fname);

                        oktoexit = TRUE;
                        break;

                case 'g':       /* enter sector to display */
                        rmcomm = TRUE;
                        oktoexit = FALSE;
                        wcommwin( "Enter sector: ");
                        while (1){
                                hxgetfld ( commwin, srch, isnum, 0, 8 );
                                if ( sscanf ( srch,"%ld",&fpos ) == 1 &&
                                        fpos >= 0 )
                                        break;
                                flash();
                        }
                        fpos *= 512;
                        if ( !noseek ){ /* check for eof if possible */
                                fseek ( fp, 0l, 2 );
                                tpos = ftell ( fp ) - (CHARS_ROW*MAXROWS);
                                if ( tpos < fpos ) fpos = tpos;
                        }

                        disppage(fname);
                        oktoexit = TRUE;
                        break;
                case 'r':       /* repeat search */
                        if ( !oktorepeat ){
                                wcommwin( "No previous search!");
                                rmcomm = TRUE;
                                sleep ( 2 );
                                break;
                        }
                        goto repeat;

                case 's':
                case '/':       /* enter search string */
                        rmcomm = TRUE;
                        oktorepeat = FALSE;
                        wcommwin( "Hex or ascii? (h/a): ");
                        c = wgetch(commwin);
                        c = toupper ( c );
                        if ( c == 'H' )
                                wcommwin( "Enter hex string: ");
                        else if ( c == 'A' )
                                wcommwin("Enter ascii string: ");
                        else
                                break;
                        if ( c == 'H' )
                                hxgetfld ( commwin, srch, ishex, toupper, 60 );
                        else
                                hxgetfld ( commwin, srch, isprint, 0, 60 );
                        if ( ! *srch ){ /* nothing entered for search */
                                flash();
                                break;
                        }
                        len = strlen ( srch );
                        if ((c == 'H' && len%2 != 0 ) ||
                         ( c == 'H' && ( htos ( srch, srch, len /= 2 ) ))){
                                wcommwin( "Invalid hex string!");
                                flash();
                                break;
                        }

                        oktorepeat = TRUE;
                        /* go back 1 screen before searching */
                        fpos -= 2*MAXROWS*CHARS_ROW;
                        if ( fpos < 0 ) fpos = 0;
                        fseek ( fp, fpos, 0 );

repeat:
                        oktoexit = FALSE;
                        if ( !srchasc ( srch , len, fname )){
                                wcommwin("String not found!");
                                sleep ( 2 );
                        } else
                                disppage(fname);
                        oktoexit = TRUE;
                        break;

                case 'l':       /* show end of file */
                        if ( noseek ){
                                wcommwin( "Invalid option for device!");
                                sleep ( 2 );
                                break;
                        }
                        fseek ( fp, 0l, 2 );
                        fpos = ftell ( fp ) - (CHARS_ROW*MAXROWS);

                        disppage(fname);
                        break;

                case 'f':
                case '\r':
                case '\n':
                case '+':
                        disppage(fname);
                        break;

                case 'b':
                case '-':
                        fpos -= 2*MAXROWS*CHARS_ROW;
                        if ( fpos < 0 ) fpos = 0;
                        disppage(fname);
                        break;

                case ' ':
                case KEY_DOWN:
                        wmove (datawin, 0, 0 );
                        wdeleteln(datawin);
                        wmove(datawin,MAXROWS-1,0);
                        if ( displine() == -1 ){ /* eof */
                                fpos = homepos;
                                disppage(fname);
                        } else
                                homepos += CHARS_ROW;
                        wrefresh(datawin);
                        hxfsect ( fpos );
                        wrefresh(headwin);

                        break;

                case '\b':
                case KEY_UP:
                        if ( fpos > (CHARS_ROW*MAXROWS)){
                                homepos -= CHARS_ROW;
                                wmove (datawin, 0, 0 );
                                winsertln(datawin);
                                fpos -= (CHARS_ROW*(MAXROWS+1));
                                displine();
                                fpos += (CHARS_ROW*(MAXROWS-1));
                                wrefresh(datawin);
                                hxfsect( fpos );
                                wrefresh(headwin);
                        }
                        break;

                case 'q':
                case 'Q':
                        return;
                }
        if ( doneflag ) return;
        }
}

void
done()
{
        signal( SIGINT, done ); /* retrap signal as soon as possible */
        if ( oktoexit ){
                clear();
                refresh();
                endwin();
                fclose ( fp );
                exit(0);
        }
        doneflag = TRUE;
        return;
}

toupper( ch )
register int ch;
{
        if ( ch >= 'a' && ch <= 'z' )
                return ( ch & ~0x20 );
        else
                return ( ch );
}

ishex( ch )
register int ch;
{
        if ( ( ch >= '0' && ch <= '9' ) ||
                        ( ch >= 'A' && ch <= 'F' ) ||
                        ( ch >= 'a' && ch <= 'f' ))
                return ( TRUE );
        else
                return ( FALSE );
}

isprint( ch )
register int ch;
{
        if (( ch >= '!' && ch <= '~' ) || ch == SPACE )
                return ( TRUE );
        else
                return ( FALSE );
}

hxgetfn(fname)
char *fname;
{

        wcommwin( "Filename? ");
        hxgetfld (commwin, fname,isprint,0,60);
        if ( *fname == '\0' )
                return ( FALSE );
        return ( TRUE );
}

isbsc ( ch )
register int ch;
{
        ch = toupper ( ch );
        if ( ch == 'B' || ch == 'S' || ch == 'C' )
                return ( TRUE );
        else
                return ( FALSE );
}

isbsce ( ch )
register int ch;
{
        ch = toupper ( ch );
        if ( ch == 'B' || ch == 'S' || ch == 'C'  || ch == 'E' )
                return ( TRUE );
        else
                return ( FALSE );
}

isnum( ch )
register int ch;
{
        if ( ch >= '0' && ch <= '9' )
                return ( TRUE );
        else
                return ( FALSE );
}

isds( ch )
register int ch;
{
        ch = toupper ( ch );
        if ( ch == 'D' || ch == 'S' )
                return ( TRUE );
        else
                return ( FALSE );
}

hxopen( fname )
char *fname;
{
        static int fd = -1;
        struct stat buf;

        if ( fd >= 0 )  /* not first time thru this function */
                close ( fd );           /* close old file */

        /* open file */
        if ( ( fd = open(fname,O_RDWR ) ) < 0 ){
                if ( ( fd = open(fname,O_RDONLY) ) < 0 )
                        return ( FALSE );
                readonly = TRUE;
        } else
                readonly = FALSE;

        fpos = 0;               /* global position of file */

        if ( readonly ){
                fp = fdopen ( fd, "r" );
        } else
                fp = fdopen ( fd, "r+" );

        fstat (fd, &buf);
        if ( buf.st_mode&(S_IFCHR|S_IFBLK) )
                noseek = TRUE;  /* block or character special file */
        else
                noseek = FALSE;

        return ( TRUE );
}

