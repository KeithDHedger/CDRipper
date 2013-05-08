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

#define FLACDIR "/tmp/Flacs/Music"
#define MP4DIR "/tmp/iPod/Music"
#define MP3DIR "/tmp/MP3s/Music"

//#define COMPILATIONSTRING "Compilation"
#define COMPILATIONSTRING "Various"
#define COMPILATIONARTIST "Compilation"
#define PREFERVERSION 34

extern char*			album;
extern char*			artist;
extern char*			genre;
extern unsigned int		year;
extern GtkWidget*		trackName[100];
extern GtkWidget*		trackArtist[100];
extern GtkWidget*		ripThis[100];
extern GtkWidget*		albumEntry;
extern GtkWidget*		artistEntry;
extern GtkWidget*		genreEntry;
extern GtkWidget*		yearEntry;
extern GtkWidget*		cdEntry;

extern bool				download;
extern const char*		cdrom;
extern cddb_disc_t*		disc;
extern int				startTrack;
extern int				numTracks;
extern bool				startSelect;

extern GList*			discMatches;

extern bool				justQuit;
extern bool				isCompilation;

extern bool				ripit;
extern char*			tmpDir;

extern GtkWindow*		window;

char* sliceStrLen(char* srcstring,char* startstr,int len);
char* sliceLen(char* srcstring,int tmpstartchar,int len);
char* sliceBetween(char* srcstring,char* startstr,char* endstr);
char* slice(char* srcstring,int tmpstartchar,int tmpendchar);
char* sliceDeleteChar(char* srcstring,char chr);
char* sliceDeleteRange(char* srcstring,const char* chars);

#endif
