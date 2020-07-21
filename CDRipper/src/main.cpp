
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

#define VERSION "0.0.4"
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
	printf("Usage: getcoverart [OPTION]\n"
	"A CLI application\n"
	" -p, --print	Print Details\n"
	" -a, --artist	Force Artist Name\n"
	" -A, --album	Force Album Name\n"
	" -r, --rip	Rip Disc\n"
	" -x, --prefix	Prefix For Music Location ( default='/tmp' )\n"
	" -d, --database	FreeDB URL ( default='gnudb.gnudb.org' )\n"
	" -P, --port	Port For FreeDB ( default='8880' )\n"
	" -s, --save	Save current config to ~/.config/cdripper.rc and exit\n"
	" -v, --version	output version information and exit\n"
	" -h, -?, --help	print this help\n\n"
	"Report bugs to kdhedger@yahoo.co.uk\n"
	);
}

void init (void)
{
	FILE*	fd=NULL;
	char	*filename;
	char	buffer[1024];
	char	name[256];
	char	strarg[256];
	const char	*tmpprefix=prefixFolder;

	//if(doSave==true)
	
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
							//if(cliPrefix==true)
							//	continue;
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
	int				c;
	char			*command;
	cddb_disc_t*	disc=NULL;
	cddb_disc_t*	tempdisc;
	bool dosave=false;

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
						//cliPrefix=true;
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


	if(dosave==true)
		{
			char	*filename=NULL;
			FILE	*fd=NULL;

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

			//printf("%s\n%s\n%s\n%s\n",prefixFolder,flacFolder,mp3Folder,mp4Folder);
			exit(0);
		}

	unknownTrackCnt=0;
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
		printDetails(tempdisc);
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

