/******************************************************
*
*     ©keithhedger Sun  5 May 19:03:12 BST 2013
*     kdhedger68713@gmail.com
*
*     disc.h
* 
******************************************************/

#ifndef _DISC_
#define _DISC_

cddb_disc_t* readDisc(void);
GList * lookupDisc(cddb_disc_t * disc);
void printDetails(cddb_disc_t* disc);
void ripTracks(cddb_disc_t* disc);
void showCDDetails(cddb_disc_t* disc);

#endif
