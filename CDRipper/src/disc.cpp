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
#include <gtk/gtk.h>

#include "globals.h"

GtkWidget*	progressWindow;
char		ripName[1024];
GtkWidget*	label;
bool		labelChanged=false;

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
    
//	cddb_set_server_name(connection,"freedb.freedb.org");
	cddb_set_server_name(connection,"freedb.musicbrainz.org");
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
			printf("Track Artist - %s\n",cddb_track_get_artist(track));
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
//curl -sk "http://musicbrainz.org/search?query=various+story+songs&type=release&method=indexed" > curlout
//curl -sk "http://musicbrainz.org/release/5b3432b9-0f01-447b-8dbd-9a7f4f1bf61e/cover-art"
//<img src="http://ecx.images-amazon.com/images/I/51LlZiD3uFL.jpg" />
//	asprintf(&command,"curl -sk \"https://ajax.googleapis.com/ajax/services/search/images?v=1.0&q=%s+%s&as_filetype=jpg&imgsz=large&rsz=1\"",artist,album);

*/

void getAlbumArt()
{
	char*			command;
	FILE*			fp=NULL;
	GString*		buffer=g_string_new(NULL);;
	char			line[1024];
	char*			urlmb=NULL;
	char*			release=NULL;
	char*			artwork;
	char*			flacimage;
	char*			mp4image;
	char*			mp3image;
	const char*		artistfolder;

	asprintf(&album,"%s",gtk_entry_get_text((GtkEntry*)albumEntry));
	asprintf(&artist,"%s",gtk_entry_get_text((GtkEntry*)artistEntry));

	if(isCompilation==true)
		artistfolder=COMPILATIONARTIST;
	else
		artistfolder=artist;

	asprintf(&flacimage,"%s/%s/%s/folder.jpg",FLACDIR,artistfolder,album);
	asprintf(&mp4image,"%s/%s/%s/folder.jpg",MP4DIR,artistfolder,album);
	asprintf(&mp3image,"%s/%s/%s/folder.jpg",MP3DIR,artistfolder,album);

	album=g_strdelimit(album," ",'+');
	artist=g_strdelimit(artist," ",'+');

	asprintf(&command,"curl -sk \"http://musicbrainz.org/search?query=%s+%s&type=release&method=indexed\"",artist,album);
	fp=popen(command, "r");
	g_free(command);
	if(fp!=NULL)
		{
			while(fgets(line,1024,fp))
				g_string_append_printf(buffer,"%s",line);
			pclose(fp);
		}

	urlmb=strstr(buffer->str,"href=\"http://musicbrainz.org/release");
	if(urlmb!=NULL)
		{
			release=sliceBetween(urlmb,(char*)"href=\"",(char*)"\">");
			if(release!=NULL)
				{
					asprintf(&command,"curl -sk \"%s/cover-art\"",release);
					fp=popen(command, "r");
					g_free(command);

					g_string_erase(buffer,0,-1);
					while(fgets(line,1024,fp))
						g_string_append_printf(buffer,"%s",line);
					pclose(fp);

					artwork=sliceBetween(buffer->str,(char*)"<img src=\"",(char*)"\"");
					if(artwork!=NULL)
						{
							asprintf(&command,"wget \"%s\" -O \"%s\"",artwork,flacimage);
							system(command);
							g_free(command);
							asprintf(&command,"cp \"%s\" \"%s\"",flacimage,mp4image);
							system(command);
							g_free(command);
							asprintf(&command,"cp \"%s\" \"%s\"",flacimage,mp3image);
							system(command);
							g_free(command);
						}
				}
			}

	if(release!=NULL)
		g_free(release);
	if(artwork!=NULL)
		g_free(artwork);
	if(flacimage!=NULL)
		g_free(flacimage);
	if(mp4image!=NULL)
		g_free(mp4image);
	if(mp3image!=NULL)
		g_free(mp3image);

	g_string_free(buffer,true);
}

gboolean doneRipping(gpointer data)
{
    GtkWidget *dialog;

    g_source_remove(GPOINTER_TO_INT(g_object_get_data((GObject*)data, "source_id")));
    gtk_widget_destroy(GTK_WIDGET(data));

    return false;
}

gboolean updateBarTimer(gpointer data)
{
	if(GTK_IS_PROGRESS_BAR((GtkProgressBar*)data))
		{
			if(labelChanged==true)
				{
					labelChanged=false;
					gtk_label_set_text((GtkLabel*)label,ripName);
				}
			gtk_progress_bar_pulse((GtkProgressBar*)data);
			return(true);
		}
	else
		return(false);
}

gpointer doTheRip(gpointer data)
{
	int				tracknum=1;
	char*			command;
	FILE*			fp;
	cddb_disc_t*	disc=(cddb_disc_t*)data;
	char*			filename=NULL;
	char*			cdstr=g_strdup(gtk_entry_get_text((GtkEntry*)cdEntry));
	char*			cdnum=NULL;
	char*			tagdata;
	const char*		artistfolder;

	if(isCompilation==true)
		artistfolder=COMPILATIONARTIST;
	else
		artistfolder=gtk_entry_get_text((GtkEntry*)artistEntry);

	asprintf(&command,"%s/%s/%s",FLACDIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
	g_mkdir_with_parents(command,493);
	g_free(command);
	asprintf(&command,"%s/%s/%s",MP4DIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
	g_mkdir_with_parents(command,493);
	g_free(command);
	asprintf(&command,"%s/%s/%s",MP3DIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
	g_mkdir_with_parents(command,493);
	g_free(command);

	if(strchr(cdstr,'/')!=NULL)
		asprintf(&cdnum,"%i-",atoi(cdstr));
	else
		asprintf(&cdnum,"%s","");

	for (int i=1;i<=numTracks;i++)
		{
			g_chdir(tmpDir);
			if(gtk_toggle_button_get_active((GtkToggleButton*)ripThis[i])==true)
				{
					sprintf((char*)&ripName,"Ripping Track \"%s\" ...",gtk_entry_get_text((GtkEntry*)trackName[i]));
					labelChanged=true;
					asprintf(&command,"cdda2wav dev=/dev/cdrom -t %i+%i -alltracks -max",tracknum,tracknum);
					system(command);
					g_free(command);
					system("flac -f --fast audio.wav");
					system("ffmpeg -i audio.wav -q:a 0 audio.mp3");
					system("ffmpeg -i audio.wav -q:a 0 audio.m4a");

//set tags
					asprintf(&tagdata,"kute --artist=\"%s\" --album=\"%s\" --title=\"%s\" --track=%i --total-tracks=%i --year=\"%s\" --genre=\"%s\" --comment=\"Ripped and tagged by K.D.Hedger\" --cd=\"%s\""
					,gtk_entry_get_text((GtkEntry*)trackArtist[i])
					,gtk_entry_get_text((GtkEntry*)albumEntry)
					,gtk_entry_get_text((GtkEntry*)trackName[i])
					,i
					,numTracks
					,gtk_entry_get_text((GtkEntry*)yearEntry)
					,gtk_entry_get_text((GtkEntry*)genreEntry)
					,cdstr
					);

					asprintf(&command,"%s audio.flac",tagdata);
					system(command);
					g_free(command);

					asprintf(&command,"%s audio.m4a",tagdata);
					system(command);
					g_free(command);

					asprintf(&command,"%s audio.mp3",tagdata);
					system(command);
					g_free(command);
					g_free(tagdata);
					filename=sliceDeleteRange((char*)gtk_entry_get_text((GtkEntry*)trackName[i])," :/'&^%$!{}@;?.");

					asprintf(&command,"%s/%s/%s/%s%2.2i %s.flac",FLACDIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
					g_rename("audio.flac",command);
					g_free(command);
					asprintf(&command,"%s/%s/%s/%s%2.2i %s.m4a",MP4DIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
					g_rename("audio.m4a",command);
					g_free(command);
					asprintf(&command,"%s/%s/%s/%s%2.2i %s.mp3",MP3DIR,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
					g_rename("audio.mp3",command);
					g_free(command);
					g_free(filename);

					gtk_toggle_button_set_active((GtkToggleButton*)ripThis[i],false);
				}
		}

	getAlbumArt();
	g_idle_add(doneRipping,progressWindow);
	g_thread_exit(NULL);
}

void ripTracks(GtkWidget* widg,gpointer data)
{	
	GtkWidget*	widget;
	GtkWidget*	vbox;
    guint		sid;

	/* create the modal window which warns the user to wait */
	progressWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(progressWindow),true);
	gtk_window_set_title(GTK_WINDOW(progressWindow),"Ripping, Please Wait...");
	gtk_container_set_border_width(GTK_CONTAINER(progressWindow), 12);
	gtk_window_set_transient_for((GtkWindow*)progressWindow,window);
	gtk_window_set_default_size((GtkWindow*)progressWindow,400,-1);
	g_signal_connect(progressWindow,"delete_event",G_CALLBACK(gtk_true), NULL);
	vbox=gtk_vbox_new(false,12);
	gtk_widget_show(vbox);
	/* create label */
	label=gtk_label_new("Ripping, Please wait...");
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox),label);
	/* create progress bar */
	widget=gtk_progress_bar_new();
	gtk_widget_show(widget);
	gtk_container_add(GTK_CONTAINER(vbox),widget);
	/* add vbox to dialog */
	gtk_container_add(GTK_CONTAINER(progressWindow),vbox);
	gtk_widget_show(progressWindow);
	sid=g_timeout_add(100, updateBarTimer,widget);
	g_object_set_data(G_OBJECT(progressWindow), "source_id",GINT_TO_POINTER(sid));
	g_thread_new("redo",(GThreadFunc)doTheRip,data);
}

void doShutdown(GtkWidget* widget,gpointer data)
{
	justQuit=(bool)data;
	gtk_main_quit();
}

void doNothing(GtkWidget* widget,gpointer data)
{
	printf("Use A Button\n");
}


void doSensitive(GtkWidget* widget,gpointer data)
{
	bool	sens=gtk_toggle_button_get_active((GtkToggleButton*)ripThis[(long)data]);

	gtk_widget_set_sensitive(trackName[(long)data],sens);
	gtk_widget_set_sensitive(trackArtist[(long)data],sens);
}

void doCompiliation(GtkWidget* widget,gpointer data)
{
	bool	sens=gtk_toggle_button_get_active((GtkToggleButton*)widget);

	gtk_widget_set_sensitive(artistEntry,!sens);
	isCompilation=sens;

	if(sens==true)
		{
			gtk_entry_set_text((GtkEntry*)artistEntry,COMPILATIONSTRING);
		}
	else
		{
			gtk_entry_set_text((GtkEntry*)artistEntry,artist);
		}
}

void doSelectAll(GtkWidget* widget,gpointer data)
{
	bool	sens=gtk_toggle_button_get_active((GtkToggleButton*)widget);

	for(int i=1;i<=numTracks;i++)
		gtk_toggle_button_set_active((GtkToggleButton*)ripThis[i],sens);
}

void showCDDetails(cddb_disc_t* disc)
{
	char*			disc_artist=(char*)cddb_disc_get_artist(disc);
	char*			disc_title=(char*)cddb_disc_get_title(disc);
	char*			disc_genre=(char*)cddb_disc_get_genre(disc);
	unsigned		disc_year=cddb_disc_get_year(disc);
	cddb_track_t*	track;
	int				tracknum=1;
	char*			tmpstr;

	GtkWidget*		vbox;
	GtkWidget*		windowvbox;
	GtkWidget*		hbox;
	GtkWidget*		scrollbox;
	GtkWidget*		button;
	GtkWidget*		compilation;
	
	window=(GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size((GtkWindow*)window,800,600);
	g_signal_connect(G_OBJECT(window),"delete-event",G_CALLBACK(doNothing),NULL);

	vbox=gtk_vbox_new(false,0);
	hbox=gtk_hbox_new(false,0);

//disc artist
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Artist:		"),false,false,0);
	artistEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),artistEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);

	if(disc_artist!=NULL)
		{
			artist=(char*)cddb_disc_get_artist(disc);
			gtk_entry_set_text((GtkEntry*)artistEntry,artist);
		}

//disc title
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Album:		"),false,false,0);
	albumEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),albumEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);

	if(disc_title!=NULL)
		{
			album=(char*)cddb_disc_get_title(disc);
			gtk_entry_set_text((GtkEntry*)albumEntry,album);
		}

//genre
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Genre:		"),false,false,0);
	genreEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),genreEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);

	if(disc_genre!=NULL)
		{
			genre=disc_genre;
			gtk_entry_set_text((GtkEntry*)genreEntry,disc_genre);
		}
			
//year
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Year:		"),false,false,0);
	yearEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),yearEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);

	if(disc_year!=0)
		{
			asprintf(&tmpstr,"%i",disc_year);
			gtk_entry_set_text((GtkEntry*)yearEntry,tmpstr);
			year=disc_year;
			g_free(tmpstr);
		}

//cd number
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("CD Number:	"),false,false,0);
	cdEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),cdEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);
	gtk_entry_set_text((GtkEntry*)cdEntry,"1");

//comp
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Compilation:	"),false,false,0);
	compilation=gtk_check_button_new_with_label("");
	gtk_box_pack_start(GTK_BOX(hbox),compilation,true,true,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);
	gtk_entry_set_text((GtkEntry*)cdEntry,"1");
	g_signal_connect(G_OBJECT(compilation),"toggled",G_CALLBACK(doCompiliation),NULL);

	gtk_box_pack_start(GTK_BOX(vbox),gtk_hseparator_new(),false,true,16);

//rip all
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Rip All:"),false,false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new(""),true,false,0);
	ripThis[0]=gtk_check_button_new_with_label("");
	g_signal_connect(G_OBJECT(ripThis[0]),"toggled",G_CALLBACK(doSelectAll),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),ripThis[0],false,false,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);

	gtk_box_pack_start(GTK_BOX(vbox),gtk_hseparator_new(),false,true,16);

	for (track=cddb_disc_get_track_first(disc); track != NULL; track=cddb_disc_get_track_next(disc))
		{
			hbox=gtk_hbox_new(false,0);
			gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Track Artist:	"),false,false,0);
			trackArtist[tracknum]=gtk_entry_new();
			gtk_box_pack_start(GTK_BOX(hbox),trackArtist[tracknum],true,true,0);
			gtk_entry_set_text((GtkEntry*)trackArtist[tracknum],cddb_track_get_artist(track));

			if(strcasecmp(cddb_track_get_artist(track),artist)!=0)
				{
					gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);
				}

		//	printf("Track Artist - %s\n",cddb_track_get_artist(track));
			hbox=gtk_hbox_new(false,0);
			asprintf(&tmpstr,"Track %2.2i:		",tracknum);
			gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new(tmpstr),false,false,0);
			g_free(tmpstr);
			trackName[tracknum]=gtk_entry_new();
			gtk_box_pack_start(GTK_BOX(hbox),trackName[tracknum],true,true,0);
			gtk_entry_set_text((GtkEntry*)trackName[tracknum],cddb_track_get_title(track));

			gtk_widget_set_sensitive(trackName[tracknum],startSelect);
			gtk_widget_set_sensitive(trackArtist[tracknum],startSelect);

			ripThis[tracknum]=gtk_check_button_new_with_label("");
			gtk_box_pack_start(GTK_BOX(hbox),ripThis[tracknum],false,false,0);
			g_signal_connect(G_OBJECT(ripThis[tracknum]),"toggled",G_CALLBACK(doSensitive),(void*)(long)tracknum);

			gtk_box_pack_start(GTK_BOX(vbox),hbox,false,true,0);
		//	printf("Track %2.2i - %s\n",tracknum,cddb_track_get_title(track));
  			tracknum++;
		}

	scrollbox=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollbox),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	windowvbox=gtk_vbox_new(false,0);
	gtk_container_add(GTK_CONTAINER(window),windowvbox);
	
	gtk_container_add(GTK_CONTAINER(windowvbox),scrollbox);
	gtk_scrolled_window_add_with_viewport((GtkScrolledWindow*)scrollbox,vbox);

	gtk_box_pack_start(GTK_BOX(windowvbox),gtk_hseparator_new(),false,true,16);

	hbox=gtk_hbox_new(true,0);

	button=gtk_button_new_with_label("Rip CD");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(ripTracks),(void*)disc);
	button=gtk_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(doShutdown),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	gtk_box_pack_start(GTK_BOX(windowvbox),hbox,false,true,0);

	gtk_box_pack_start(GTK_BOX(windowvbox),gtk_hseparator_new(),false,true,16);

	gtk_widget_show_all((GtkWidget*)window);
	gtk_main();
}









