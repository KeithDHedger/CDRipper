/*
 *
 * ©K. D. Hedger. Thu  8 Dec 12:55:11 GMT 2022 keithdhedger@gmail.com

 * This file (globals.cpp) is part of CDRipper.

 * CDRipper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * CDRipper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with CDRipper.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cddb/cddb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>

const char		*album=NULL;
const char		*artist=NULL;
const char		*genre=NULL;
unsigned int		year;
unsigned int		discID=0;

GtkWidget		*trackName[100];
GtkWidget		*trackArtist[100];
GtkWidget		*ripThis[100];
GtkWidget		*albumEntry;
GtkWidget		*artistEntry=NULL;
GtkWidget		*genreEntry;
GtkWidget		*yearEntry;
GtkWidget		*cdEntry;
GtkWidget		*discIDEntry;

GtkWidget		*mainWindowVBox;
GtkWidget		*detailsVBox=NULL;
GtkWidget		*windowScrollbox;

bool				print=false;
bool				download=true;
const char		*cdrom="/dev/cdrom";
int				startTrack;
int				numTracks;
bool				ripit=false;
char				*tmpDir=NULL;
bool				startSelect=false;

GList			*discMatches=NULL;
int				unknownTrackCnt=0;

bool				justQuit=false;
bool				isCompilation=false;

GtkWindow		*window;

char				*musicDb=strdup("gnudb.org");
int				dbPort=8880;

char				*prefixFolder=strdup("/tmp");
char				*flacFolder=NULL;
char				*mp4Folder=NULL;
char				*mp3Folder=NULL;

bool				ripFlac=true;
bool				ripMp4=false;
bool				ripMp3=false;
bool				ripLowQMp3=false;

//global routines
//string slicing

char *slice(char *srcstring,int tmpstartchar,int tmpendchar)
{
	char		*dest;
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

char *sliceBetween(char *srcstring,char *startstr,char *endstr)
{
	int		startchar;
	int		endchar;
	char		*ptr;
	char		*dest=NULL;

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

char *sliceLen(char *srcstring,int tmpstartchar,int len)
{
	char		*dest;
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

char *sliceStrLen(char *srcstring,char *startstr,int len)
{
	char		*ptr;
	int		startchar;

	ptr=strstr(srcstring,startstr);
	if(ptr==NULL)
		return(NULL);
	startchar=(int)(long)ptr+strlen(startstr)-(long)srcstring;
	printf("%i\n",startchar);
	return(sliceLen(srcstring,startchar,len));
}

char *sliceDeleteChar(char *srcstring,char chr)
{
	char		*buffer;
	char		*destptr;
	char		*retstr=NULL;

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

char *sliceDeleteRange(char *srcstring,const char *chars)
{
	char		*buffer;
	char		*destptr;
	char		*retstr=NULL;
	bool		flag;

	buffer=(char*)malloc(strlen(srcstring)+1);
	destptr=buffer;

	while(*srcstring!= 0)
		{
			do
				{
					flag=false;
					for(int i=0; i<strlen(chars); i++)
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

