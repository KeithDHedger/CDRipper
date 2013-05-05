
//Mon 31 Jul 2006 12:30:55 BST 
//
//GetCoverArt
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <glib.h>
#include <linux/cdrom.h>
#include <cddb/cddb.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>


#define VERSION "0.0.0"
#define UNKNOWNARG -100

char*			album;
char*			artist;
bool			download=true;
const char*		cdrom="/dev/cdrom";
cddb_disc_t*	disc=NULL;
int				startTrack;
int				numTracks;

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

struct option long_options[] =
	{
		{"artist",1,0,'a'},
		{"album",1,0,'b'},
		{"no-download",0,0,'n'},
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

void printdetails(cddb_disc_t* disc)
{ 
	if(disc==NULL)
		printf("what\n");
   char * disc_artist=(char*)cddb_disc_get_artist(disc);
    char * disc_title=(char*)cddb_disc_get_title(disc);
    char * disc_genre=(char*)cddb_disc_get_genre(disc);
    unsigned disc_year=cddb_disc_get_year(disc);
    cddb_track_t * track;
    int tracknum=1;
   
 //   bool singleartist;

	if(disc_artist!=NULL)
		{
			printf("Artist - %s\n",disc_artist);
			//asprintf(&artist,"%s
			artist=(char*)cddb_disc_get_artist(disc);
		}
	if(disc_artist!=NULL)
		{
			printf("Album - %s\n",disc_title);
			album=(char*)cddb_disc_get_title(disc);
		}
//	printf("Album - %s\n",disc_title);
	printf("Genre - %s\n",disc_genre);
	printf("Year - %i\n",disc_year);

	for (track=cddb_disc_get_track_first(disc); track != NULL; track=cddb_disc_get_track_next(disc))
        {
        
 //           if (strcmp(disc_artist, cddb_track_get_artist(track)) != 0)
 //           {
  //              singleartist=false;
 //               break;
  //          } 
  		printf("Track %2.2i - %s\n",tracknum,cddb_track_get_title(track));
  		tracknum++;
        }

}

GList * gbl_disc_matches=NULL;
static cddb_conn_t * gbl_cddb_query_thread_conn;
static cddb_disc_t * gbl_cddb_query_thread_disc;
static int gbl_cddb_query_thread_running;
static int gbl_cddb_query_thread_num_matches;
static GThread * gbl_cddb_query_thread;

gpointer cddb_query_thread_run(gpointer data)
{
    gbl_cddb_query_thread_num_matches=cddb_query(gbl_cddb_query_thread_conn, gbl_cddb_query_thread_disc);
    if(gbl_cddb_query_thread_num_matches==-1)
        gbl_cddb_query_thread_num_matches=0;
    
//    g_atomic_int_set(&gbl_cddb_query_thread_running, 0);
    
//    return NULL;
}


GList * lookup_disc(cddb_disc_t * disc)
{
    int i;
    GList * matches=NULL;
    
    // set up the connection to the cddb server
    gbl_cddb_query_thread_conn=cddb_new();
    if (gbl_cddb_query_thread_conn==NULL)
        printf("cddb_new() failed. Out of memory?");
    
    cddb_set_server_name(gbl_cddb_query_thread_conn, "freedb.freedb.org");
    cddb_set_server_port(gbl_cddb_query_thread_conn, 8880);
   
//   cddb_http_enable(gbl_cddb_query_thread_conn);                                /* REQ */
//cddb_set_server_port(gbl_cddb_query_thread_conn, 80);                        /* REQ */
//cddb_set_server_name(gbl_cddb_query_thread_conn, "freedb.org");
//cddb_set_http_path_query(gbl_cddb_query_thread_conn, "/~cddb/cddb.cgi");
//cddb_set_http_path_submit(gbl_cddb_query_thread_conn, "/~cddb/submit.cgi");

//    if (global_prefs->use_proxy)
//    {
//        cddb_set_http_proxy_server_name(gbl_cddb_query_thread_conn, global_prefs->server_name);
//        cddb_set_http_proxy_server_port(gbl_cddb_query_thread_conn, global_prefs->port_number);
//        cddb_http_proxy_enable(gbl_cddb_query_thread_conn);
//    }
    // query cddb to find similar discs
//    g_atomic_int_set(&gbl_cddb_query_thread_running, 1);
 //   gbl_cddb_query_thread_disc=disc;
 //   gbl_cddb_query_thread=g_thread_create(cddb_query_thread_run, NULL, TRUE, NULL);
    
    // show cddb update window
// printf("AAAAAAAAAAAAAA\n");   
//int data=NULL;

//gbl_cddb_query_thread =(GThread*)cddb_query_thread_run(NULL);
//data=cddb_query(gbl_cddb_query_thread_conn,disc);
gbl_cddb_query_thread_num_matches=cddb_query(gbl_cddb_query_thread_conn, disc);

 // printf("QQQQQQQQQQQQQQQq\n");   



//    gdk_threads_enter();
//        disable_all_main_widgets();
        
       // GtkWidget* statusLbl=lookup_widget(win_main, "statusLbl");
      //  gtk_label_set_text(GTK_LABEL(statusLbl), _("<b>Getting disc info from the internet...</b>"));
      //  gtk_label_set_use_markup(GTK_LABEL(statusLbl), TRUE);
        
//        while(g_atomic_int_get(&gbl_cddb_query_thread_running) != 0)
 //       {
  //          while (gtk_events_pending())
  //              gtk_main_iteration();
  //          usleep(100000);
 //       }
        
//        gtk_label_set_text(GTK_LABEL(statusLbl), "");
        
 //       enable_all_main_widgets();
//    gdk_threads_leave();


    
    // make a list of all the matches
    for (i=0; i < gbl_cddb_query_thread_num_matches; i++)
    {
        cddb_disc_t * possible_match=cddb_disc_clone(disc);
        if (!cddb_read(gbl_cddb_query_thread_conn, possible_match))
        {
            cddb_error_print(cddb_errno(gbl_cddb_query_thread_conn));
            printf("cddb_read() failed.");
        }
        matches=g_list_append(matches, possible_match);
        
        // move to next match
        if (i < gbl_cddb_query_thread_num_matches - 1)
        {
            if (!cddb_query_next(gbl_cddb_query_thread_conn, disc))
                printf("Query index out of bounds.");
        }
    }
    
    cddb_destroy(gbl_cddb_query_thread_conn);
    
    return matches;
}


int main(int argc, char **argv)
{
	int c;
	album=(char*)"";
	artist=(char*)"";

	while (1)
		{
			int option_index=0;
			c=getopt_long(argc,argv,"v?hn:a:b:",long_options,&option_index);
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

	char*	url;
	char	buffer[16384];
	char*	command;
	FILE*	fp;
cddb_disc_t* disc=NULL;
disc=readDisc();
if(disc==NULL)
	{
		printf("no disc\n");
		return(1);
	}
else
	{
		gbl_disc_matches=lookup_disc(disc);
 printf("ZZZZZ\n");
		//cddb_disc_destroy(disc);
            printf("---%i\n",g_list_length(gbl_disc_matches));
      
        if (gbl_disc_matches==NULL)
        	{
        	printf("XXXXXXXXXXXXXXZZZZZZZZZ\n");
            return(1);
           }
          else
          {
              printf("---%i\n",g_list_length(gbl_disc_matches));          
  //          if (g_list_length(gbl_disc_matches) >= 1)
  //      {
            // fill in and show the album drop-down box
         //   GtkListStore * store=gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
         //   GtkTreeIter iter;
    //        GList * curr;
            cddb_disc_t * tempdisc;
     //        for (curr=g_list_first(gbl_disc_matches); curr != NULL; curr=g_list_next(curr))
      //      {
       //     	printf("WWWWWWWWWWW\n");
               // tempdisc=(cddb_disc_t *)curr->data;
                tempdisc=(cddb_disc_t *)gbl_disc_matches->data;
                printdetails(tempdisc);
                
          //      gtk_list_store_append(store, &iter);
           //     gtk_list_store_set(store, &iter,
            //        0, cddb_disc_get_artist(tempdisc),
            //        1, cddb_disc_get_title(tempdisc),
             //       -1);
         //   }
          //  gtk_combo_box_set_model(GTK_COMBO_BOX(pick_disc), GTK_TREE_MODEL(store));
           // gtk_combo_box_set_active(GTK_COMBO_BOX(pick_disc), 1);
           // gtk_combo_box_set_active(GTK_COMBO_BOX(pick_disc), 0);
            
          //  gtk_widget_show(lookup_widget(win_main, "disc"));
           // gtk_widget_show(lookup_widget(win_main, "pick_disc"));
        }

            
            
            }
           
		//printdetails(disc);
	//}


	album=g_strdelimit(album," ",'+');
	artist=g_strdelimit(artist," ",'+');

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
	if(url!=NULL)
		g_free(url);

	return 0;
}

