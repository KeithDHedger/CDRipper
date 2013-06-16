
//Mon 31 Jul 2006 12:30:55 BST 
//
//CDRipper
//

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

#define VERSION "0.0.1"
#define UNKNOWNARG -100

struct option long_options[] =
	{
		{"print",0,0,'p'},
		{"rip",0,0,'r'},
		{"version",0,0,'v'},
		{"help",0,0,'?'},
		{0, 0, 0, 0}
	};

void printhelp(void)
{
printf("Usage: getcoverart [OPTION]\n"
	"A CLI application\n"
	" -p, --print	Print Details\n"
	" -r, --rip	Rip disc\n"
	" -v, --version	output version information and exit\n"
	" -h, -?, --help	print this help\n\n"
	"Report bugs to kdhedger@yahoo.co.uk\n"
	);
}

void init (void)
{
	FILE*	fd=NULL;
	char*	filename;
	char	buffer[1024];
	char	name[256];
	char	strarg[256];

	flacFolder=(char*)FLACDIR;
	mp4Folder=(char*)MP4DIR;
	mp3Folder=(char*)MP3DIR;

	asprintf(&filename,"%s/.config/cdripper.rc",getenv("HOME"));
	fd=fopen(filename,"r");
	if(fd!=NULL)
		{
			while(feof(fd)==0)
				{
					fgets(buffer,1024,fd);
					sscanf(buffer,"%s %s",(char*)&name,(char*)&strarg);

					if(strcasecmp(name,"flacdir")==0)
						{
							sscanf(buffer,"%*s %"VALIDFILENAMECHARS"s",(char*)&strarg);
							asprintf(&flacFolder,"%s",strarg);
						}
					if(strcasecmp(name,"mp4dir")==0)
						{
							sscanf(buffer,"%*s %"VALIDFILENAMECHARS"s",(char*)&strarg);
							asprintf(&mp4Folder,"%s",strarg);
						}
					if(strcasecmp(name,"mp3dir")==0)
						{
							sscanf(buffer,"%*s %"VALIDFILENAMECHARS"s",(char*)&strarg);
							asprintf(&mp3Folder,"%s",strarg);
						}
				}
			fclose(fd);
		}
	g_free(filename);
}

int main(int argc, char **argv)
{
	int				c;
	char*			command;
	cddb_disc_t*	disc=NULL;
	cddb_disc_t*	tempdisc;

	album=(char*)"";
	artist=(char*)"";
	tmpDir=g_dir_make_tmp("CDRipXXXXXX",NULL);
	if(tmpDir==NULL)
		{
			printf("CAN'T CREATE TMP FOLDER !!!\n");
			return(1);
		}

	init();

	while (1)
		{
			int option_index=0;
			c=getopt_long(argc,argv,"v?hrpr",long_options,&option_index);
			if (c==-1)
				break;

			switch(c)
				{

					case 'p':
						print=true;
						break;

					case 'r':
						ripit=true;
						break;

					case 'v':
						printf("getcoverart %s\n",VERSION);
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

	disc=readDisc();
	if(disc==NULL)
		{
			printf("no disc\n");
			return(1);
		}

	discMatches=lookupDisc(disc);
	if (discMatches==NULL)
		{
			printf("No matches found for disc :(\n");
			return(1);
		}
	cddb_disc_destroy(disc);

#if GLIB_MINOR_VERSION < PREFERVERSION
	g_thread_init(NULL);
#endif
	gdk_threads_init();
	gtk_init(&argc,&argv);

//add possible matches here
	tempdisc=(cddb_disc_t *)discMatches->data;
	if(print==true)
		printDetails(tempdisc);
	else
		showCDDetails(tempdisc);

	asprintf(&command,"rm -r %s",tmpDir);
	system(command);
	g_free(command);
	return 0;
}

