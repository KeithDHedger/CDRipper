/******************************************************
*
*     ©keithhedger Sun  5 May 18:58:18 BST 2013
*     kdhedger68713@gmail.com
*
*     globals.h
* 
******************************************************/

#ifndef _GLOBALS_
#define _GLOBALS_

extern char*			album;
extern char*			artist;
extern char*			genre;
extern unsigned int		year;
extern GtkWidget*		trackName[100];
extern GtkWidget*		trackArtist[100];

extern bool			download;
extern const char*	cdrom;
extern cddb_disc_t*	disc;
extern int			startTrack;
extern int			numTracks;
extern GList*		discMatches;
extern bool			ripit;
extern char*		tmpDir;

char* sliceStrLen(char* srcstring,char* startstr,int len);
char* sliceLen(char* srcstring,int tmpstartchar,int len);
char* sliceBetween(char* srcstring,char* startstr,char* endstr);
char* slice(char* srcstring,int tmpstartchar,int tmpendchar);


#endif
