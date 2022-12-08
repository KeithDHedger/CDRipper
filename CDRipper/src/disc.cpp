/*
 *
 * ©K. D. Hedger. Thu  8 Dec 12:59:37 GMT 2022 keithdhedger@gmail.com

 * This file (disc.cpp) is part of CDRipper.

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
#include "config.h"

GtkWidget	*progressWindow;
char			ripName[1024];
GtkWidget	*label;
bool			labelChanged=false;
GtkWidget	*drop;
char			text[4096]= {0,};

cddb_disc_t* readDisc(void)
{
	int		fd;
	int		status;
	int		i;
	char		trackname[9];
	struct	cdrom_tochdr th;
	struct	cdrom_tocentry te;
	char		*filename;

	cddb_disc_t	*disc=NULL;
	cddb_track_t	*track=NULL;

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
					for (i=th.cdth_trk0; i<=th.cdth_trk1; i++)
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
									unknownTrackCnt++;
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

GList* lookupDisc(cddb_disc_t *ldisc,bool disablecache)
{
	GList		*matches=NULL;
	cddb_conn_t	*connection=NULL;
	int			numMatches=-1;

	// set up the connection to the cddb server
	connection=cddb_new();
	if (connection==NULL)
		printf("cddb_new() failed. Out of memory?");
	if(disablecache==true)
		cddb_cache_disable(connection);
	cddb_set_server_port(connection,dbPort);
	cddb_set_server_name(connection,musicDb);

	numMatches=cddb_query(connection,ldisc);
	discID=cddb_disc_get_discid(ldisc);
	// make a list of all the matches
	for (int i=0; i<numMatches; i++)
		{
			cddb_disc_t* possible_match=cddb_disc_clone(ldisc);
			if (!cddb_read(connection,possible_match))
				{
					cddb_error_print(cddb_errno(connection));
					printf("cddb_read() failed.");
				}
			matches=g_list_append(matches,possible_match);

			// move to next match
			if (i<numMatches-1)
				{
					if(!cddb_query_next(connection,ldisc))
						printf("Query index out of bounds.");
				}
		}

	cddb_destroy(connection);

	return matches;
}

void printDetails(cddb_disc_t *disc)
{
	if(disc!=NULL)
		{
			char			*disc_artist=(char*)cddb_disc_get_artist(disc);
			char			*disc_title=(char*)cddb_disc_get_title(disc);
			char			*disc_genre=(char*)cddb_disc_get_genre(disc);
			unsigned		disc_year=cddb_disc_get_year(disc);
			cddb_track_t	*track;
			int				tracknum=1;

			printf("Disc ID - %x\n",discID);

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
	else
		{
			if(artist!=NULL)
				printf("Artist - %s\n",artist);
			if(album!=NULL)
				printf("Artist - %s\n",album);

			for(int i=1; i<=unknownTrackCnt; i++)
				{
					printf("Track %2.2i - %s\n",i,"Unknown Track");
				}
		}
}

void getAlbumArt()
{
	char			*command;
	FILE*		fp=NULL;
	GString*		buffer=g_string_new(NULL);;
	char			line[1024];
	char			*urlmb=NULL;
	char			*release=NULL;
	char			*artwork=NULL;
	char			*flacimage=NULL;
	char			*mp4image=NULL;
	char			*mp3image=NULL;
	const char	*artistfolder=NULL;
	char			*newart=NULL;

	album=gtk_entry_get_text((GtkEntry*)albumEntry);
	artist=gtk_entry_get_text((GtkEntry*)artistEntry);

	if(isCompilation==true)
		artistfolder=COMPILATIONARTIST;
	else
		artistfolder=artist;

//TODO//
	asprintf(&command,"glyrc cover -a \"%s\" -b \"%s\" --write stdout 2>/dev/null|convert - -density 72x72 -size 300x300 /tmp/folder.jpg",artist,album);
	//asprintf(&command,"curl $(wget --user-agent='%s' --no-netrc --random-wait --tries=4 --waitretry=1 \"https://www.discogs.com/search/?q=%%22%s%%22+%%22%s%%22&type=all\" -O - 2>&1|cat -|grep -i jpg|sed -n 's@<img data-src=\"\\(.*\\)\\.jpg.*\"@\\1.jpg@p')|convert - -density 72x72 -size 300x300 /tmp/folder.jpg",USERAGENT,artist,album);
	system(command);
	g_free(command);

	if(ripFlac==true)
		{
			asprintf(&command,"cp /tmp/folder.jpg \"%s/%s/%s/%s/folder.jpg\"",prefixFolder,flacFolder,artist,album);
			system(command);
			g_free(command);
		}

	if(ripMp4==true)
		{
			asprintf(&command,"cp /tmp/folder.jpg \"%s/%s/%s/%s/folder.jpg\"",prefixFolder,mp4Folder,artist,album);
			system(command);
			g_free(command);
		}

	if(ripMp3==true)
		{
			asprintf(&command,"cp /tmp/folder.jpg \"%s/%s/%s/%s/folder.jpg\"",prefixFolder,mp3Folder,artist,album);
			system(command);
			g_free(command);
		}
	unlink("/tmp/folder.jpg");
}

gboolean doneRipping(gpointer data)
{
	GtkWidget	*dialog;

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
	char			*command;
	FILE*		fp;
	cddb_disc_t	*disc=(cddb_disc_t*)data;
	char			*filename=NULL;
	char			*cdstr=g_strdup(gtk_entry_get_text((GtkEntry*)cdEntry));
	char			*cdnum=NULL;
	char			*tagdata;
	const char	*artistfolder;
	const char	*tagartist;

	if(isCompilation==true)
		artistfolder=COMPILATIONARTIST;
	else
		artistfolder=gtk_entry_get_text((GtkEntry*)artistEntry);

	if(strchr(cdstr,'/')!=NULL)
		asprintf(&cdnum,"%i-",atoi(cdstr));
	else
		asprintf(&cdnum,"%s","");

	for (int i=1; i<=numTracks; i++)
		{
			g_chdir(tmpDir);
			if(gtk_toggle_button_get_active((GtkToggleButton*)ripThis[i])==true)
				{
					sprintf((char*)&ripName,"Ripping Track \"%s\" ...",gtk_entry_get_text((GtkEntry*)trackName[i]));
					labelChanged=true;
					asprintf(&command,"cdda2wav dev=/dev/cdrom -t %i+%i -alltracks -max",i,i);
					system(command);
					g_free(command);

//set tags
					if(isCompilation==true)
						tagartist=gtk_entry_get_text((GtkEntry*)trackArtist[i]);
					else
						tagartist=gtk_entry_get_text((GtkEntry*)artistEntry);

					asprintf(&tagdata,"kute --artist=\"%s\" --album=\"%s\" --title=\"%s\" --track=%i --total-tracks=%i --year=\"%s\" --genre=\"%s\" --comment=\"Ripped and tagged by K.D.Hedger\" --cd=\"%s\" --discid=\"%s\""
					         ,gtk_entry_get_text((GtkEntry*)trackArtist[i])
					         ,gtk_entry_get_text((GtkEntry*)albumEntry)
					         ,gtk_entry_get_text((GtkEntry*)trackName[i])
					         ,i
					         ,numTracks
					         ,gtk_entry_get_text((GtkEntry*)yearEntry)
					         ,gtk_entry_get_text((GtkEntry*)genreEntry)
					         ,cdstr
					         ,gtk_entry_get_text((GtkEntry*)discIDEntry)
					        );

					filename=sliceDeleteRange((char*)gtk_entry_get_text((GtkEntry*)trackName[i])," :/'&^%$!{}@;?.");

					if(ripFlac==true)
						{
							asprintf(&command,"%s/%s/%s/%s",prefixFolder,flacFolder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
							g_mkdir_with_parents(command,493);
							g_free(command);

							system("flac -f --fast audio.wav");
							asprintf(&command,"%s audio.flac",tagdata);
							system(command);
							g_free(command);
							asprintf(&command,"mv audio.flac \"%s/%s/%s/%s/%s%2.2i %s.flac\"",prefixFolder,flacFolder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
							system(command);
							g_free(command);
						}
					if(ripMp4==true)
						{
							asprintf(&command,"%s/%s/%s/%s",prefixFolder,mp4Folder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
							g_mkdir_with_parents(command,493);
							g_free(command);

							system("ffmpeg -i audio.wav -vn -map_metadata -1 -qscale 0 audio.m4a");
							asprintf(&command,"%s audio.m4a",tagdata);
							system(command);
							g_free(command);
							asprintf(&command,"mv audio.m4a \"%s/%s/%s/%s/%s%2.2i %s.m4a\"",prefixFolder,mp4Folder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
							system(command);
							g_free(command);
						}
					if((ripMp3==true))
						{
							asprintf(&command,"%s/%s/%s/%s",prefixFolder,mp3Folder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry));
							g_mkdir_with_parents(command,493);
							g_free(command);

							system("ffmpeg -i audio.wav -b:a 256k -vn -map_metadata -1 audio.mp3 </dev/null");
							asprintf(&command,"%s audio.mp3",tagdata);
							system(command);
							g_free(command);
							asprintf(&command,"mv audio.mp3 \"%s/%s/%s/%s/%s%2.2i %s.mp3\"",prefixFolder,mp3Folder,artistfolder,gtk_entry_get_text((GtkEntry*)albumEntry),cdnum,i,filename);
							system(command);
							g_free(command);
						}

					g_free(tagdata);
					g_free(filename);

					gtk_toggle_button_set_active((GtkToggleButton*)ripThis[i],false);
				}
		}

	getAlbumArt();
	g_idle_add(doneRipping,progressWindow);
	g_thread_exit(NULL);
	return(NULL);
}

void ripTracks(GtkWidget *widg,gpointer data)
{
	GtkWidget	*widget;
	GtkWidget	*vbox;
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

#if GLIB_MINOR_VERSION < PREFERVERSION
	g_thread_create(doTheRip,(void*)data,false,NULL);
#else
	g_thread_new("redo",(GThreadFunc)doTheRip,data);
#endif
}

void doShutdown(GtkWidget *widget,gpointer data)
{
	justQuit=(bool)data;
	gtk_main_quit();
}

void doNothing(GtkWidget *widget,gpointer data)
{
	printf("Use A Button\n");
}


void doSensitive(GtkWidget *widget,gpointer data)
{
	bool		sens=gtk_toggle_button_get_active((GtkToggleButton*)ripThis[(long)data]);

	gtk_widget_set_sensitive(trackName[(long)data],sens);
	gtk_widget_set_sensitive(trackArtist[(long)data],sens);
}

void doCompiliation(GtkWidget *widget,gpointer data)
{
	bool		sens=gtk_toggle_button_get_active((GtkToggleButton*)widget);

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

void doSelectAll(GtkWidget *widget,gpointer data)
{
	bool		sens=gtk_toggle_button_get_active((GtkToggleButton*)widget);

	for(int i=1; i<=numTracks; i++)
		gtk_toggle_button_set_active((GtkToggleButton*)ripThis[i],sens);
}

void doDetails(cddb_disc_t *disc)
{
	char				*disc_artist=(char*)cddb_disc_get_artist(disc);
	char				*disc_title=(char*)cddb_disc_get_title(disc);
	char				*disc_genre=(char*)cddb_disc_get_genre(disc);
	unsigned			disc_year=cddb_disc_get_year(disc);
	cddb_track_t		*track;
	int				tracknum=1;
	char				*tmpstr;

	if(artist!=NULL)
		disc_artist=(char*)artist;
	else
		artist=(char*)cddb_disc_get_artist(disc);
	if(album!=NULL)
		disc_title=(char*)album;
	else
		album=(char*)cddb_disc_get_title(disc);

	GtkWidget		*hbox;
	GtkWidget		*compilation;

	if(detailsVBox!=NULL)
		gtk_widget_destroy(detailsVBox);

	detailsVBox=gtk_vbox_new(false,0);
	hbox=gtk_hbox_new(false,0);

//disc artist
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Artist:		"),false,false,0);
	artistEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),artistEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

	if(disc_artist!=NULL)
		{
			gtk_entry_set_text((GtkEntry*)artistEntry,artist);
		}

//disc title
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Album:		"),false,false,0);
	albumEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),albumEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

	if(disc_title!=NULL)
		{
			gtk_entry_set_text((GtkEntry*)albumEntry,album);
		}

//genre
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Genre:		"),false,false,0);
	genreEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),genreEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

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
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

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
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
	gtk_entry_set_text((GtkEntry*)cdEntry,"1");

//discid
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Disc ID:\t\t"),false,false,0);
	discIDEntry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),discIDEntry,true,true,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
	asprintf(&tmpstr,"%x",discID);
	gtk_entry_set_text((GtkEntry*)discIDEntry,tmpstr);
	g_free(tmpstr);

//comp
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Compilation:	"),false,false,0);
	compilation=gtk_check_button_new_with_label("");
	gtk_box_pack_start(GTK_BOX(hbox),compilation,true,true,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
	gtk_entry_set_text((GtkEntry*)cdEntry,"1");
	g_signal_connect(G_OBJECT(compilation),"toggled",G_CALLBACK(doCompiliation),NULL);
	gtk_box_pack_start(GTK_BOX(detailsVBox),gtk_hseparator_new(),false,true,4);

//rip all
	hbox=gtk_hbox_new(false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Rip All:"),false,false,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new(""),true,false,0);
	ripThis[0]=gtk_check_button_new_with_label("");
	g_signal_connect(G_OBJECT(ripThis[0]),"toggled",G_CALLBACK(doSelectAll),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),ripThis[0],false,false,0);
	gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

	gtk_box_pack_start(GTK_BOX(detailsVBox),gtk_hseparator_new(),false,true,4);

	if(disc!=NULL)
		{
			for (track=cddb_disc_get_track_first(disc); track != NULL; track=cddb_disc_get_track_next(disc))
				{
					hbox=gtk_hbox_new(false,0);
					gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Track Artist:	"),false,false,0);
					trackArtist[tracknum]=gtk_entry_new();
					gtk_box_pack_start(GTK_BOX(hbox),trackArtist[tracknum],true,true,0);
					gtk_entry_set_text((GtkEntry*)trackArtist[tracknum],cddb_track_get_artist(track));

					if(strcasecmp(cddb_track_get_artist(track),artist)!=0)
						{
							gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
						}

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

					gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
					tracknum++;
				}
		}
	else
		{
			for(int i=1; i<=unknownTrackCnt; i++)
				{
					hbox=gtk_hbox_new(false,0);
					gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new("Track Artist:	"),false,false,0);
					trackArtist[tracknum]=gtk_entry_new();
					gtk_box_pack_start(GTK_BOX(hbox),trackArtist[tracknum],true,true,0);
					if(artist!=NULL)
						gtk_entry_set_text((GtkEntry*)trackArtist[tracknum],artist);
					else
						gtk_entry_set_text((GtkEntry*)trackArtist[tracknum],"Unknown");
					gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);

					hbox=gtk_hbox_new(false,0);
					asprintf(&tmpstr,"Track %2.2i:		",tracknum);
					gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new(tmpstr),false,false,0);
					g_free(tmpstr);
					trackName[tracknum]=gtk_entry_new();
					gtk_box_pack_start(GTK_BOX(hbox),trackName[tracknum],true,true,0);
					gtk_entry_set_text((GtkEntry*)trackName[tracknum],"Untitled");

					gtk_widget_set_sensitive(trackName[tracknum],startSelect);
					gtk_widget_set_sensitive(trackArtist[tracknum],startSelect);

					ripThis[tracknum]=gtk_check_button_new_with_label("");
					gtk_box_pack_start(GTK_BOX(hbox),ripThis[tracknum],false,false,0);
					g_signal_connect(G_OBJECT(ripThis[tracknum]),"toggled",G_CALLBACK(doSensitive),(void*)(long)tracknum);

					gtk_box_pack_start(GTK_BOX(detailsVBox),hbox,false,true,0);
					tracknum++;
				}
		}

	gtk_widget_show_all(detailsVBox);
	gtk_scrolled_window_add_with_viewport((GtkScrolledWindow*)windowScrollbox,detailsVBox);
}

void redoDetails(GtkWidget *widget,gpointer data)
{
	artist=NULL;
	album=NULL;
	doDetails((cddb_disc_t *)g_list_nth_data(discMatches,gtk_combo_box_get_active((GtkComboBox*)widget)));
}

void freeData(gpointer data)
{
	cddb_disc_destroy((cddb_disc_t*)data);
}

void reScanCD(GtkWidget *widget,gpointer data)
{
	cddb_disc_t*	thedisc=NULL;
	cddb_disc_t*	tempdisc=NULL;

	for(int j=g_list_length(discMatches)-1; j>-1; j--)
		gtk_combo_box_text_remove((GtkComboBoxText *)drop,j);

	numTracks=0;
	isCompilation=false;
	g_list_free_full(discMatches,freeData);
	discMatches=NULL;
	artist=NULL;
	album=NULL;
	genre=NULL;

	thedisc=readDisc();
	if(thedisc==NULL)
		{
			printf("no disc\n");
			return;
		}

	discMatches=lookupDisc(thedisc,true);
	if(discMatches!=NULL)
		tempdisc=(cddb_disc_t *)discMatches->data;
	else
		tempdisc=NULL;

	cddb_disc_destroy(thedisc);

	if(drop!=NULL)
		{
			for(int j=g_list_length(discMatches)-1; j>-1; j--)
				gtk_combo_box_text_remove ((GtkComboBoxText *)drop,j);

			for(int j=0; j<g_list_length(discMatches); j++)
				{
					sprintf(text,"%s - %s",(char*)cddb_disc_get_artist((cddb_disc_t *)g_list_nth_data(discMatches,j)),(char*)cddb_disc_get_title((cddb_disc_t *)g_list_nth_data(discMatches,j)));
					gtk_combo_box_text_append_text((GtkComboBoxText*)drop,text);
				}
			gtk_combo_box_set_active ((GtkComboBox*)drop,0);
		}

	doDetails(tempdisc);
}

void doRipOptions(GtkWidget *widget,gpointer data)
{
	int		what=(int)(long)data;
	bool		value=gtk_toggle_button_get_active((GtkToggleButton*)widget);

	switch (what)
		{
			case RIPFLAC:
				ripFlac=value;
				break;
			case RIPMP4:
				ripMp4=value;
				break;
			case RIPMP3:
				ripMp3=value;
				break;
		}
}

void showCDDetails(cddb_disc_t *disc)
{
	GtkWidget	*button;
	GtkWidget	*hbox;

	window=(GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(window,APPNAME);
	gtk_window_set_default_size((GtkWindow*)window,600,480);
	g_signal_connect(G_OBJECT(window),"delete-event",G_CALLBACK(doNothing),NULL);

	mainWindowVBox=gtk_vbox_new(false,0);

	drop=gtk_combo_box_text_new();
	for(int j=0; j<g_list_length(discMatches); j++)
		{
			sprintf(text,"%s - %s",(char*)cddb_disc_get_artist((cddb_disc_t *)g_list_nth_data(discMatches,j)),(char*)cddb_disc_get_title((cddb_disc_t *)g_list_nth_data(discMatches,j)));
			gtk_combo_box_text_append_text((GtkComboBoxText*)drop,text);
		}
	gtk_combo_box_set_active ((GtkComboBox*)drop,0);
	g_signal_connect(G_OBJECT(drop),"changed",G_CALLBACK(redoDetails),NULL);

	gtk_box_pack_start(GTK_BOX(mainWindowVBox),drop,false,false,0);

	gtk_container_add(GTK_CONTAINER(window),mainWindowVBox);

	gtk_widget_show_all((GtkWidget*)window);

	windowScrollbox=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(windowScrollbox),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(mainWindowVBox),windowScrollbox);

	doDetails(disc);

	gtk_box_pack_start(GTK_BOX(mainWindowVBox),gtk_hseparator_new(),false,true,4);

	hbox=gtk_hbox_new(true,0);

//rip
	button=gtk_button_new_with_label("Rip CD");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(ripTracks),(void*)disc);

//rip options
//rip flac
	button=gtk_check_button_new_with_label("Rip Flac");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	gtk_toggle_button_set_active((GtkToggleButton*)button,true);
	g_signal_connect(G_OBJECT(button),"toggled",G_CALLBACK(doRipOptions),(void*)RIPFLAC);
//rip mp4
	button=gtk_check_button_new_with_label("Rip Mp4");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	g_signal_connect(G_OBJECT(button),"toggled",G_CALLBACK(doRipOptions),(void*)RIPMP4);
//rip mp3
	button=gtk_check_button_new_with_label("Rip Mp3");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	g_signal_connect(G_OBJECT(button),"toggled",G_CALLBACK(doRipOptions),(void*)RIPMP3);

//rescan
	button=gtk_button_new_with_label("Re-Scan CD");
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(reScanCD),(void*)disc);

//quit
	button=gtk_button_new_from_stock(GTK_STOCK_QUIT);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(doShutdown),NULL);
	gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);
	gtk_box_pack_start(GTK_BOX(mainWindowVBox),hbox,false,true,0);

	gtk_box_pack_start(GTK_BOX(mainWindowVBox),gtk_hseparator_new(),false,true,4);

	gtk_window_set_default_icon_name(PACKAGE);
	gtk_window_set_icon_name((GtkWindow*)window,PACKAGE);

	gtk_widget_show_all((GtkWidget*)window);
	gtk_main();
}

