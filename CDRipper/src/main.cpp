/*
 *
 * ©K. D. Hedger. Thu  8 Dec 12:56:53 GMT 2022 keithdhedger@gmail.com

 * This file (main.cpp) is part of CDRipper.

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <glib.h>
#include <linux/cdrom.h>
#include <cddb/cddb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "globals.h"
#include "disc.h"

#define VERSION "0.5.0"
#define UNKNOWNARG -100

struct	option long_options[] =
	{
		{"print",0,0,'p'},
		{"artist",1,0,'a'},
		{"album",1,0,'A'},
		{"rip",0,0,'r'},
		{"prefix",1,0,'x'},
		{"database",1,0,'d'},
		{"port",1,0,'P'},
		{"save",0,0,'s'},
		{"version",0,0,'v'},
		{"help",0,0,'?'},
		{0, 0, 0, 0}
	};

void printhelp(void)
{
	printf("Usage: cdripper [OPTION]\n"
	" -p, --print	Print Details\n"
	" -a, --artist	Force Artist Name\n"
	" -A, --album	Force Album Name\n"
	" -r, --rip	Rip Disc\n"
	" -x, --prefix	Prefix For Music Location ( default='/tmp' )\n"
	" -d, --database	FreeDB URL ( default='gnudb.org' )\n"
	" -P, --port	Port For FreeDB ( default='8880' )\n"
	" -s, --save	Save current config to ~/.config/cdripper.rc and exit\n"
	" -v, --version	output version information and exit\n"
	" -h, -?, --help	print this help\n\n"
	"Report bugs to keithdhedger@gmail.com\n"
	);
}

void init (void)
{
	FILE			*fd=NULL;
	char			*filename;
	char			buffer[1024];
	char			name[256];
	char			strarg[256];
	const char	*tmpprefix=prefixFolder;
	
	asprintf(&flacFolder,"%s",FLACDIR);
	asprintf(&mp4Folder,"%s",MP4DIR);
	asprintf(&mp3Folder,"%s",MP3DIR);

	asprintf(&filename,"%s/.config/cdripper.rc",getenv("HOME"));
	fd=fopen(filename,"r");
	if(fd!=NULL)
		{
			while(feof(fd)==0)
				{
					fgets(buffer,1024,fd);
					sscanf(buffer,"%s %s",(char*)&name,(char*)&strarg);

					if(strcasecmp(name,"prefixdir")==0)
						{
							free(prefixFolder);
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							asprintf(&prefixFolder,"%s",strarg);
						}

					if(strcasecmp(name,"flacdir")==0)
						{
							free(flacFolder);
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							asprintf(&flacFolder,"%s",strarg);
						}
					if(strcasecmp(name,"mp4dir")==0)
						{
							free(mp4Folder);
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							asprintf(&mp4Folder,"%s",strarg);
						}
					if(strcasecmp(name,"mp3dir")==0)
						{
							free(mp3Folder);
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							asprintf(&mp3Folder,"%s",strarg);
						}
					if(strcasecmp(name,"dburl")==0)
						{
							free(musicDb);
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							asprintf(&musicDb,"%s",strarg);
						}
					if(strcasecmp(name,"dbport")==0)
						{
							sscanf(buffer,"%*s %" VALIDFILENAMECHARS "s",(char*)&strarg);
							dbPort=atoi(strarg);
						}
				}
			fclose(fd);
		}
	g_free(filename);
}

int main(int argc, char **argv)
{
	int			c;
	char			*command;
	cddb_disc_t	*disc=NULL;
	cddb_disc_t	*tempdisc;
	bool			dosave=false;

	tmpDir=g_dir_make_tmp("CDRipXXXXXX",NULL);
	if(tmpDir==NULL)
		{
			printf("CAN'T CREATE TMP FOLDER !!!\n");
			return(1);
		}

	init();

	while (1)
		{
			int	option_index=0;
			c=getopt_long(argc,argv,"v?hrprsx:d:P:a:A:",long_options,&option_index);
			if (c==-1)
				break;

			switch(c)
				{
					case 's':
						dosave=true;
						break;

					case 'a':
						artist=optarg;
						break;

					case 'A':
						album=optarg;
						break;

					case 'd':
						free(musicDb);
						musicDb=strdup(optarg);
						break;

					case 'P':
						dbPort=atoi(optarg);
						break;

					case 'x':
						free(prefixFolder);
						prefixFolder=strdup(optarg);
						break;

					case 'p':
						print=true;
						break;

					case 'r':
						ripit=true;
						break;

					case 'v':
						printf("CDRipper %s\n",VERSION);
						return 0;
						break;

					case '?':
					case 'h':
						printhelp();
						return 0;
						break;

					default:
						fprintf(stderr,"?? Unknown argument ??\n");
						return UNKNOWNARG;
						break;
			}
		}

	if(optind<argc)
		{
			printf("non-option ARGV-elements: ");
			while (optind < argc)
				printf("%s ", argv[optind++]);
			printf("\n");
		}

	if(dosave==true)
		{
			char		*filename=NULL;
			FILE		*fd=NULL;

			asprintf(&filename,"%s/.config/cdripper.rc",getenv("HOME"));
			fd=fopen(filename,"w");
			if(fd!=NULL)
				{
					fprintf(fd,"prefixdir\t%s\n",prefixFolder);
					fprintf(fd,"flacdir\t%s\n",flacFolder);
					fprintf(fd,"mp4dir\t%s\n",mp4Folder);
					fprintf(fd,"mp3dir\t%s\n",mp3Folder);
					fprintf(fd,"dburl\t%s\n",musicDb);
					fprintf(fd,"dbport\t%i\n",dbPort);
					fclose(fd);
				}
			free(filename);	
			exit(0);
		}

	unknownTrackCnt=0;
	disc=readDisc();
	if(disc==NULL)
		{
			printf("no disc\n");
			return(1);
		}

	discMatches=lookupDisc(disc,false);
	if (discMatches==NULL)
		{
			printf("No matches found for disc :(\n");
		}
	cddb_disc_destroy(disc);

#if GLIB_MINOR_VERSION < PREFERVERSION
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gtk_init(&argc,&argv);

//add possible matches here
	if(discMatches!=NULL)
		tempdisc=(cddb_disc_t *)discMatches->data;
	else
		tempdisc=NULL;

	if(print==true)
		{
		printf("Number of items found==%i\n",g_list_length(discMatches));
		for(int j=0;j<g_list_length(discMatches);j++)
			{
				tempdisc=(cddb_disc_t *)g_list_nth_data (discMatches,j);
				printDetails(tempdisc);
			}
		}
	else
		showCDDetails(tempdisc);

	asprintf(&command,"rm -r %s",tmpDir);
	system(command);
	g_free(command);
	free(prefixFolder);
	free(flacFolder);
	free(mp4Folder);
	free(mp3Folder);
	return 0;
}

