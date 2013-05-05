
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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "globals.h"
#include "disc.h"

#define VERSION "0.0.1"
#define UNKNOWNARG -100

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

//GList * discMatches=NULL;
//static cddb_conn_t * gbl_cddb_query_thread_conn;
//static cddb_disc_t * gbl_cddb_query_thread_disc;
//static int gbl_cddb_query_thread_running;
//static int gbl_cddb_query_thread_num_matches;
//static GThread * gbl_cddb_query_thread;

//gpointer cddb_query_thread_run(gpointer data)
//{
//    gbl_cddb_query_thread_num_matches=cddb_query(gbl_cddb_query_thread_conn, gbl_cddb_query_thread_disc);
//    if(gbl_cddb_query_thread_num_matches==-1)
//        gbl_cddb_query_thread_num_matches=0;
    
//    g_atomic_int_set(&gbl_cddb_query_thread_running, 0);
    
//    return NULL;
//}




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

	char*			url;
	char			buffer[16384];
	char*			command;
	FILE*			fp;
	cddb_disc_t*	disc=NULL;

	disc=readDisc();
	if(disc==NULL)
		{
			printf("no disc\n");
			return(1);
		}
	else
		{
			discMatches=lookupDisc(disc);
		//cddb_disc_destroy(disc);
            printf("---%i\n",g_list_length(discMatches));
      
        if (discMatches==NULL)
        	{
        	printf("XXXXXXXXXXXXXXZZZZZZZZZ\n");
            return(1);
           }
          else
          {
              printf("---%i\n",g_list_length(discMatches));          
  //          if (g_list_length(discMatches) >= 1)
  //      {
            // fill in and show the album drop-down box
         //   GtkListStore * store=gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
         //   GtkTreeIter iter;
    //        GList * curr;
            cddb_disc_t * tempdisc;
     //        for (curr=g_list_first(discMatches); curr != NULL; curr=g_list_next(curr))
      //      {
       //     	printf("WWWWWWWWWWW\n");
               // tempdisc=(cddb_disc_t *)curr->data;
                tempdisc=(cddb_disc_t *)discMatches->data;
                printDetails(tempdisc);
                
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
	cddb_disc_destroy(disc);

	return 0;
}

