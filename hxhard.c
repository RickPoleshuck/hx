/*
        @(#) hxhard - support for hard copy

        inputs:
                start - starting position
                length - number of bytes to print
                printer - name of printer
                spooler - flag for spooled or direct

        output:
                none
*/
#include "hx.h"

hxhardmn(fname)
char *fname;
{
	char tbuf[80];
	long start;
	long length;
	char printer[80];
	int spooler = 0;
	char *p;

	/*
	 *	Get name of printer from environment
	 */
	p = getenv ( "HXPRINTER" );
	if ( p )
		strncpy ( printer, p, sizeof ( printer ) );
	else
		*printer = '\0';
	if ( ! strncmp ( "/dev/", p, 5 ))
		spooler = 'D';
	else if ( !strncmp ( "/usr/bin/", p, 9 ) ||
		  !strncmp ( "/bin/", p, 5 ))
		spooler = 'S';

retry:
	wcommwin("\
Print from (B)yte, (S)ector, (E)ntire, or (C)urrent location? C\b" );
	/* get 1 character uppercased and verified
	   for 'b,s,c,e' */
	hxgetfld ( commwin, tbuf, isbsce, toupper, 1 );
	switch ( *tbuf ){
	 case '\0':
	 case 'C':      /* print current sector */
		start = homepos;
		length = 512;
		break;
	 case 'S':        /* enter sector number */
		wcommwin( "Enter starting sector number in decimal: " );
		hxgetfld ( commwin, tbuf, isnum, 0, 7 );
		if ( !*tbuf )
			start = homepos;
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
	 case 'E':	  /* entire file */
		start = 0;
		if ( noseek ){
			wcommwin( "Invalid option for device!");
			goto retry;
		}
		fseek ( fp, 0l, 2 );
		length = ftell ( fp );
		break;
	 case 'B':        /* enter byte address in hex */
		wcommwin( "Enter starting byte in hexadecimal: " );
		hxgetfld ( commwin, tbuf, ishex, 0, 7 );
		if ( !*tbuf )
			start = homepos;
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

	/*
	 * 	get name of printer if not already defined
	 */
	if ( ! *printer ){
		wcommwin( "Enter name of printer: " );
		hxgetfld ( commwin, printer, isprint, 0, sizeof ( printer )-1 );
		if ( !*printer )
			return;       /* no printer specified - return */

	}
	/* 
	 * 	get type of printer of not already defined 
	 */
	if ( !spooler ){
		wcommwin( "(S)pooled or (D)irect: S\b" );
		/* get either a D or an S */
		hxgetfld ( commwin, tbuf, isds, toupper, 1 );
		if ( !*tbuf ) spooler = 'S';
	}
	oktoexit = FALSE;
	wcommwin( "Printing . . ." );
	wrefresh( commwin );
	hxhard( start, length, fname, printer, spooler );

	/* 
	 * 	announce finished printing
	 */
	if ( doneflag ){
		doneflag = FALSE;
		waddstr( commwin, " Interrupted" );
	} else
		waddstr( commwin, " Done" );
	wrefresh( commwin );
	hxfsect ( homepos );
	wrefresh ( headwin );
	oktoexit = TRUE;
	return;

}

#ifdef DOS
hxhard ( start, length, fname, printer )
#else
hxhard ( pos, length, fname, printer, spooler )
#endif
long pos;
long length;
char *fname;
char *printer;
#ifndef DOS
int spooler;
#endif
{
        FILE *printfp;
        char outbuf[81];
        char buffer[CHARS_ROW];
        char lbuffer[CHARS_ROW];
        struct stat statbuf;
        char *date;
        int page = 0;
        int line = 0;
        int nread;
	int same=FALSE;

        /*
                get date of file - do this each time we
                print in case we have patched the file
                we want to print the new date
        */
        stat ( fname, &statbuf );
        date = asctime(localtime ( &statbuf.st_mtime ));
        date[strlen(date)-1] = '\0';

#ifndef DOS
        if ( spooler == 'S' ){
                if ( (printfp = hxpopen ( printer, "w" )) == NULL ){
                        beep();
                        wcommwin ( "Cannot open printer '%s'", printer );
                        return;
                }
        } else
#endif
        {
                if ( (printfp = fopen ( printer, "a" )) == NULL ){
                        beep();
                        wcommwin ( "Cannot open printer '%s'", printer );
                        return;
                }
        }

        fseek ( fp, pos, 0 );
        while ( length > 0 ){
                nread = fread( buffer, 1, CHARS_ROW, fp );
		if ( nread < CHARS_ROW ) length = 0; /* force break */
		if ( !(line=(line%55)) )         /* check for new page */
			hxhhead ( ++page, pos/512, fname, date, printfp );

		/* only print line if it is different from last line */
		/* or it is the first line on a page */
		if ( !line || memcmp ( buffer, lbuffer, CHARS_ROW )){
			hxfline ( pos, buffer, outbuf, nread );
			fprintf ( printfp, "%s\n", outbuf );
			memcpy ( lbuffer, buffer, CHARS_ROW );
			++line;
			same=FALSE;
		} else if ( same == FALSE ){
			/* only print '*' for first duplicate line */
			fputs ( "*\n", printfp );
			++line;
			same=TRUE;
		}
		if ( !(line%16) ){	/* add space every 16 lines */
			fputc ( '\n', printfp );
			line++;
		}
		length -= nread;
		pos += nread;
		if ( !(pos%512)){
			/* update headwin to show progress */
			hxfsect ( pos );
			wrefresh ( headwin );
		}
        }

#ifndef DOS
        if ( spooler == 'S' )
                hxpclose ( printfp );
        else
#endif
                fclose ( printfp );
}

/*
        Print heading ( prepend formfeed )
*/

hxhhead ( page, sector, fname, date, printfp )
int page;
long sector;
char *date;
FILE *printfp;
{

        fputc ( '\f', printfp );
        fprintf ( printfp, "\
------------------------------------------------------------------------------\
\n" );
        fprintf ( printfp, "\
HX 1.3                                                            Page: %6.6d\n",               page );
        fprintf ( printfp, "Date: %-58.58sSector: %6.6d\n", date, sector );
        fprintf ( printfp, "File: %s\n", fname );
        fprintf ( printfp, "\
------------------------------------------------------------------------------\
\n" );

}

extern int execl(), fork(), pipe(), close(), fcntl();
static int child;
#include <errno.h>

/*
 *	same as standard popen except stdout and stderr are closed
 */
FILE *
hxpopen(cmd)
char    *cmd;
{
        int     p[2];
	int 	i;

        if(pipe(p) < 0)
                return(NULL);
        if((child = fork()) == 0) {
		fclose ( fp );			/* close all files */
		for ( i = 0; i < 20; ++i )
			if ( i != p[0] )
				close ( i );
                dup(p[0]);
                close(p[0]);
		open ("/dev/null", 1 );
		dup ( stdout );
                execl("/bin/sh", "sh", "-c", cmd, (char *)0);
                _exit(1);
        }
        if(child == -1)
                return(NULL);
        close(p[0]);
        return(fdopen(p[1], "w"));
}

int
hxpclose(ptr)
FILE    *ptr;
{
        register int f, r;
        int status, (*hstat)(), (*istat)(), (*qstat)();

        f = fileno(ptr);
        (void) fclose(ptr);
        istat = signal(SIGINT, SIG_IGN);
        qstat = signal(SIGQUIT, SIG_IGN);
        hstat = signal(SIGHUP, SIG_IGN);

        /* while the child is not done and no error has occured wait in the loop*/

        while((r = wait(&status)) != child && (r != -1 || errno == EINTR))
                ;
        if(r == -1)
                status = -1;
        (void) signal(SIGINT, istat);
        (void) signal(SIGQUIT, qstat);
        (void) signal(SIGHUP, hstat);
        return(status);
}
