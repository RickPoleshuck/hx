/*      @(#) hx.h - definitions and declarations for hx.c */
#include <curses.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifdef DOS
# include <stdio.h>
#endif

#define MAXROWS 16
#define CHARS_ROW 16
#define ASC_COL0  61
#define HEX_COL0  10
#define ASC_MID 70
#define HEX_MID 34
#define ESC     0x1b
#define BS      0x08
#define SPACE   0x20
#define ASC     0
#define HEX     1
#define MENU    0
#define HEAD    (MENU + 3)
#define DATA    (HEAD + 3)

#define mvwdeleteln(a,b,c) {wmove(a,b,c);wdeleteln(a);}

extern WINDOW *menuwin,
       *headwin,
       *datawin,
       *commwin;    /* windows */
FILE *fdopen();
#ifdef isprint
# undef isprint
#endif
int isprint();
int isds();
int isbsce();
int isbsc();
int isnum();
int ishex();
int toupper();
char *getenv();
char *asctime();
FILE *hxpopen();
void done();
extern FILE *fp;
extern long fpos;               /* global position if file */
extern long homepos;            /* position of home character on screen */
extern char noseek;             /* This not a regular file - no seeks allowd */
extern char readonly;           /* open readonly flag */
extern char doneflag;           /* signal caught flag */
extern char oktoexit;           /* should we exit when interrupt received */
extern char *progname;
extern char rmcomm;             /* remove command window flag */
extern char oktorepeat;         /* ok to repeat search flag */
extern char menu1[], menu2[], menu3[], patch1[], patch2[]; /* menus */
#if 0
#ifndef KEY_LL
#define KEY_LL          1
#define KEY_DOWN        2
#define KEY_UP          3
#define KEY_HOME        4
#define KEY_LEFT        5
#define KEY_RIGHT       6
#endif
#endif

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

// hx function definitions
static void ltoh(char* str, long number);
static void btoh(char* str, int number);
static void wputch (WINDOW wind, int c);
