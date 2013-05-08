
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
		{"artist",1,0,'a'},
		{"album",1,0,'b'},
		{"no-download",0,0,'n'},
		{"rip",0,0,'r'},
		{"version",0,0,'v'},
		{"help",0,0,'?'},
		{0, 0, 0, 0}
	};

void printhelp(void)
{
printf("Usage: getcoverart [OPTION]\n"
	"A CLI application\n"
	" -l, --long1	Do somthing good\n"
	" -v, --version	output version information and exit\n"
	" -h, -?, --help	print this help\n\n"
	"Report bugs to kdhedger@yahoo.co.uk\n"
	);
}

int main(int argc, char **argv)
{
	int c;
/*
const char* data="this: ' is : / a test";
char* ret=NULL;
ret=sliceDeleteChar((char*)data,' ');

printf("---%s---\n",ret);
g_free(ret);

data="this: ( 1 & 2 ) ";
ret=sliceDeleteRange((char*)data,"( )&");

printf("+++%s+++\n",ret);
g_free(ret);
return 0;
*/
	album=(char*)"";
	artist=(char*)"";
	tmpDir=g_dir_make_tmp("CDRipXXXXXX",NULL);
	if(tmpDir==NULL)
		{
			printf("CAN'T CREATE TMP FOLDER !!!\n");
			return(1);
		}

	while (1)
		{
			int option_index=0;
			c=getopt_long(argc,argv,"v?hnr:a:b:",long_options,&option_index);
			if (c==-1)
				break;

			switch(c)
				{
					case 'a':
						artist=optarg;
						break;

					case 'b':
						album=optarg;
						break;

					case 'n':
						download=false;
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

	char*			url;
	char			buffer[16384];
	char*			command;
	FILE*			fp;
	cddb_disc_t*	disc=NULL;
	cddb_disc_t*	tempdisc;

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

	gtk_init(&argc,&argv);

//add possible matches here
	tempdisc=(cddb_disc_t *)discMatches->data;
//	printDetails(tempdisc);
	showCDDetails(tempdisc);
//	return(0);

//	if(ripit==true)
//		ripTracks(tempdisc);

	int j;


	asprintf(&album,"%s",gtk_entry_get_text((GtkEntry*)albumEntry));
	asprintf(&artist,"%s",gtk_entry_get_text((GtkEntry*)artistEntry));
	album=g_strdelimit(album," ",'+');
	artist=g_strdelimit(artist," ",'+');


//	printf("AAA%sAAA\n",artist);
//	printf("AAA%sAAA\n",album);
	

//curl -sk "http://musicbrainz.org/search?query=various+story+songs&type=release&method=indexed" > curlout
//curl -sk "http://musicbrainz.org/release/5b3432b9-0f01-447b-8dbd-9a7f4f1bf61e/cover-art"
//<img src="http://ecx.images-amazon.com/images/I/51LlZiD3uFL.jpg" />


//	asprintf(&command,"curl -sk \"https://ajax.googleapis.com/ajax/services/search/images?v=1.0&q=%s+%s&as_filetype=jpg&imgsz=large&rsz=1\"",artist,album);
	asprintf(&command,"curl -sk \"http://musicbrainz.org/search?query=%s+%s&type=release&method=indexed\"",artist,album);
printf("%s\n",command);
//return 0;
	fp=popen(command, "r");
//	fgets((char*)&buffer[0],16384,fp);
//fread
	fread((char*)buffer,1,16384,fp);
//	printf("%s\n",(char*)buffer);
	char* urlmb;
	char* release;

	urlmb=strstr((char*)buffer,"href=\"http://musicbrainz.org/release");
	printf("---%s--\n",urlmb);
	return 0;
	release=sliceBetween(urlmb,"href=\"","\">");

	printf("---%s--\n",release);

//	char*	srcptr=(char*)buffer;
//	while (srcptr!=NULL)
//		{

/*
			url=sliceBetween((char*)buffer,(char*)"unescapedUrl\":\"",(char*)"\",\"");
			if(url!=NULL)
				{
					printf("%s\n",url);
					if(download==true)
						{
							asprintf(&command,"curl -sko folder%i.jpg %s",j,url);
							system(command);
						}
				}
*/
//		}

	pclose(fp);
	if(command!=NULL)
		g_free(command);
	if(url!=NULL)
		g_free(url);
	
	asprintf(&command,"rm -r %s",tmpDir);
	system(command);
	g_free(command);
	return 0;
}

