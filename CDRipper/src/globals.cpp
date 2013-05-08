/******************************************************
*
*     ©keithhedger Sun  5 May 18:58:18 BST 2013
*     kdhedger68713@gmail.com
*
*     globals.cpp
* 
******************************************************/

#include <cddb/cddb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

char*			album;
char*			artist;
char*			genre;
unsigned int	year;
GtkWidget*		trackName[100];
GtkWidget*		trackArtist[100];
GtkWidget*		ripThis[100];
GtkWidget*		albumEntry;
GtkWidget*		artistEntry;
GtkWidget*		genreEntry;
GtkWidget*		yearEntry;
GtkWidget*		cdEntry;

bool			download=true;
const char*		cdrom="/dev/cdrom";
cddb_disc_t*	disc=NULL;
int				startTrack;
int				numTracks;
bool			ripit=false;
char*			tmpDir=NULL;
bool			startSelect=false;

GList*			discMatches=NULL;

bool			justQuit=false;
bool			isCompilation=false;

//global routines
//string sliceing

char* slice(char* srcstring,int tmpstartchar,int tmpendchar)
{
	char*	dest;
	int		strsize;
	int		startchar=tmpstartchar;
	int		endchar=tmpendchar;

	if(tmpstartchar<0)
		startchar=0;

	if((tmpendchar<0) || (tmpendchar>(int)strlen(srcstring)))
		endchar=strlen(srcstring)-1;

	strsize=endchar-startchar+1;

	dest=(char*)malloc(strsize+1);
	strncpy(dest,(char*)&srcstring[startchar],strsize);
	dest[strsize]=0;

	return(dest);
}

char* sliceBetween(char* srcstring,char* startstr,char* endstr)
{
	int		startchar;
	int		endchar;
	char*	ptr;
	char*	dest=NULL;

	ptr=strstr(srcstring,startstr);
	if(ptr==NULL)
		return(NULL);
	startchar=(int)(long)ptr+strlen(startstr)-(long)srcstring;

	ptr=strstr((char*)&srcstring[startchar],endstr);
	if(ptr==NULL)
		return(NULL);
	endchar=(int)(long)ptr-(long)srcstring-1;

	dest=slice(srcstring,startchar,endchar);
	return(dest);
}

char* sliceLen(char* srcstring,int tmpstartchar,int len)
{
	char*	dest;
	int		strsize;
	int		startchar=tmpstartchar;
	int		endchar=len;

	if(tmpstartchar<0)
		startchar=0;

	if((len<0) || (len+startchar>(int)strlen(srcstring)))
		endchar=strlen(srcstring)-startchar;

	strsize=endchar;

	dest=(char*)malloc(strsize+1);
	strncpy(dest,(char*)&srcstring[startchar],endchar);
	dest[endchar]=0;

	return(dest);

}

char* sliceStrLen(char* srcstring,char* startstr,int len)
{
	char*	ptr;
	int		startchar;

	ptr=strstr(srcstring,startstr);
	if(ptr==NULL)
		return(NULL);
	startchar=(int)(long)ptr+strlen(startstr)-(long)srcstring;
	printf("%i\n",startchar);
	return(sliceLen(srcstring,startchar,len));
}

char* sliceDeleteChar(char* srcstring,char chr)
{
	char*	buffer;
	char*	destptr;
	char*	retstr=NULL;

	buffer=(char*)malloc(strlen(srcstring)+1);
	destptr=buffer;

	while(*srcstring!= 0)
		{
			if((*srcstring)==chr)
				{
					srcstring++;
				}
			else
				*destptr++=*srcstring++;
		}
	*destptr=0;
	asprintf(&retstr,"%s",buffer);
	g_free(buffer);
	return(retstr);
}

char* sliceDeleteRange(char* srcstring,const char* chars)
{
	char*	buffer;
	char*	destptr;
	char*	retstr=NULL;
//	int		i;
	bool	flag;

	buffer=(char*)malloc(strlen(srcstring)+1);
	destptr=buffer;

	while(*srcstring!= 0)
		{
			do
				{
					flag=false;
					for(int i=0;i<strlen(chars);i++)
						{
							if((*srcstring)==chars[i])
								{
									srcstring++;
									flag=true;
								}
						}
				}
			while(flag==true);
			*destptr++=*srcstring++;
		}
	*destptr=0;
	asprintf(&retstr,"%s",buffer);
	g_free(buffer);
	return(retstr);
}









