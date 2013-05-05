/******************************************************
*
*     ©keithhedger Sun  5 May 19:03:12 BST 2013
*     kdhedger68713@gmail.com
*
*     disc.cpp
* 
******************************************************/

#include <cddb/cddb.h>
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
#include <glib.h>
#include <glib/gstdio.h>

#include "globals.h"

cddb_disc_t* readDisc(void)
{
	int fd;
	int status;
	int i;
	char trackname[9];
	struct cdrom_tochdr th;
	struct cdrom_tocentry te;

	cddb_disc_t* disc=NULL;
	cddb_track_t* track=NULL;

    // open the device
	fd=open(cdrom,O_RDONLY|O_NONBLOCK);
	if(fd<0)
		{
			fprintf(stderr,"Error: Couldn't open %s\n",cdrom);
			return NULL;
		}

	// read disc status info
	status=ioctl(fd,CDROM_DISC_STATUS,CDSL_CURRENT);
	if ((status==CDS_AUDIO) || (status==CDS_MIXED))
		{
			// see if we can read the disc's table of contents (TOC).
			if (ioctl(fd, CDROMREADTOCHDR, &th)==0)
				{
					startTrack=th.cdth_trk0;
					numTracks=th.cdth_trk1;

					disc=cddb_disc_new();
					if (disc==NULL)
						printf("cddb_disc_new() failed. Out of memory?");

					te.cdte_format=CDROM_LBA;
					for (i=th.cdth_trk0;i<=th.cdth_trk1;i++)
						{
							te.cdte_track=i;
							if(ioctl(fd,CDROMREADTOCENTRY,&te)==0)
								{
									track=cddb_track_new();
									if (track==NULL)
										printf("cddb_track_new() failed. Out of memory?");

									cddb_track_set_frame_offset(track,te.cdte_addr.lba+SECONDS_TO_FRAMES(2));
									snprintf(trackname,9,"Track %d",i);
									cddb_track_set_title(track,trackname);
									cddb_track_set_artist(track,"Unknown Artist");
									cddb_disc_add_track(disc,track);
								}
						}

					te.cdte_track=CDROM_LEADOUT;
					if (ioctl(fd,CDROMREADTOCENTRY,&te)==0)
						cddb_disc_set_length(disc,(te.cdte_addr.lba+SECONDS_TO_FRAMES(2))/SECONDS_TO_FRAMES(1));
				}
		}

	close(fd);

	return(disc);
}

GList* lookupDisc(cddb_disc_t* disc)
{
	GList*			matches=NULL;
	cddb_conn_t*	connection;
	int				numMatches;

	// set up the connection to the cddb server
	connection=cddb_new();
	if (connection==NULL)
		printf("cddb_new() failed. Out of memory?");
    
	cddb_set_server_name(connection,"freedb.freedb.org");
	cddb_set_server_port(connection,8880);
	numMatches=cddb_query(connection, disc);

	// make a list of all the matches
	for (int i=0;i<numMatches;i++)
		{
			cddb_disc_t* possible_match=cddb_disc_clone(disc);
			if (!cddb_read(connection,possible_match))
				{
					cddb_error_print(cddb_errno(connection));
					printf("cddb_read() failed.");
				}
			matches=g_list_append(matches,possible_match);
        
		// move to next match
		if (i<numMatches-1)
			{
				if(!cddb_query_next(connection,disc))
					printf("Query index out of bounds.");
			}
		}

	cddb_destroy(connection);
    
    return matches;
}

void printDetails(cddb_disc_t* disc)
{
	char*			disc_artist=(char*)cddb_disc_get_artist(disc);
	char*			disc_title=(char*)cddb_disc_get_title(disc);
	char*			disc_genre=(char*)cddb_disc_get_genre(disc);
	unsigned		disc_year=cddb_disc_get_year(disc);
	cddb_track_t*	track;
	int				tracknum=1;

	if(disc_artist!=NULL)
		{
			printf("Artist - %s\n",disc_artist);
			artist=(char*)cddb_disc_get_artist(disc);
		}
	if(disc_artist!=NULL)
		{
			printf("Album - %s\n",disc_title);
			album=(char*)cddb_disc_get_title(disc);
		}

	printf("Genre - %s\n",disc_genre);
	printf("Year - %i\n",disc_year);

	for (track=cddb_disc_get_track_first(disc); track != NULL; track=cddb_disc_get_track_next(disc))
        {
			printf("Track %2.2i - %s\n",tracknum,cddb_track_get_title(track));
  			tracknum++;
        }
}
//cdda2wav dev=/dev/cdrom -t ${TNUM}+${TNUM} -alltracks -max
/*
	asprintf(&command,"curl -sk \"https://ajax.googleapis.com/ajax/services/search/images?v=1.0&q=%s+%s&as_filetype=jpg&imgsz=large&rsz=1\"",artist,album);

	fp=popen(command, "r");
	fgets((char*)&buffer[0],16384,fp);

	url=sliceBetween((char*)buffer,(char*)"unescapedUrl\":\"",(char*)"\",\"");
	if(url!=NULL)
		{
			printf("%s\n",url);
			if(download==true)
				{
					asprintf(&command,"curl -sko folder.jpg %s",url);
					system(command);
				}
		}
	pclose(fp);
	if(command!=NULL)
		g_free(command);

*/
void ripTracks(cddb_disc_t* disc)
{
	char*			disc_artist=(char*)cddb_disc_get_artist(disc);
	char*			disc_title=(char*)cddb_disc_get_title(disc);
	char*			disc_genre=(char*)cddb_disc_get_genre(disc);
	unsigned		disc_year=cddb_disc_get_year(disc);
	cddb_track_t*	track;
	int				tracknum=1;
	char*			command;
	FILE*			fp;

	if(disc_artist!=NULL)
		{
			printf("Artist - %s\n",disc_artist);
			artist=(char*)cddb_disc_get_artist(disc);
		}
	if(disc_artist!=NULL)
		{
			printf("Album - %s\n",disc_title);
			album=(char*)cddb_disc_get_title(disc);
		}

	printf("Genre - %s\n",disc_genre);
	printf("Year - %i\n",disc_year);

	for (track=cddb_disc_get_track_first(disc); track != NULL; track=cddb_disc_get_track_next(disc))
		{
			printf("Track %2.2i - %s\n",tracknum,cddb_track_get_title(track));
			g_chdir(tmpDir);
			asprintf(&command,"cdda2wav dev=/dev/cdrom -t %i+%i -alltracks -max",tracknum,tracknum);
			system(command);
			g_free(command);
			asprintf(&command,"mv audio.wav  \"%2.2i - %s.wav\"",tracknum,cddb_track_get_title(track));
			system(command);
			g_free(command);
			tracknum++;
		}

}
