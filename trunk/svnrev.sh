#! /bin/bash
#
rev_new_raw=$(svnversion -n . 2>/dev/null | tr '\n' ' ' | tr -d '\r')
[ -n "$rev_new_raw" ] || rev_new_raw=$(SubWCRev . 2>/dev/null | tr '\n' ' ' | tr -d '\r')

rev_new_raw=$(echo $rev_new_raw | sed 's/[^0-9]*\([0-9]*\)\(.*\)/\1 \2/')
rev_new=0
a=$(echo $rev_new_raw | sed 's/\([0-9]*\).*/\1/')
let "a+=0"
#find max rev
while [ "$a" ]; do
	[ "$a" -gt "$rev_new" ] && rev_new=$a
	rev_new_raw=$(echo -n $rev_new_raw | sed 's/[0-9]*[^0-9]*\([0-9]*\)\(.*\)/\1 \2/')
	a=$(echo $rev_new_raw | sed 's/\([0-9]*\).*/\1/') 
done

rev_old=$(cat ./source/Network/svnrev.c 2>/dev/null | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

rev_new=`expr $rev_new + 1`

if [ "$rev_new" != "$rev_old" ] || [ ! -f ./source/Network/svnrev.c ]; then
	
	cat <<EOF > ./source/Network/svnrev.c
#include <stdlib.h>

#define SVN_REV "$rev_new"

const char *GetRev()
{
	return SVN_REV;
}

int GetIntRev()
{
	return atoi(SVN_REV);
}
EOF

	if [ -n "$rev_old" ]; then
		echo "Changed Rev $rev_old to $rev_new" >&2
	else
		echo "svnrev.c created at Rev $rev_new" >&2
	fi
fi

rev_date=`date -u +%Y%m%d%H%M`
	
cat <<EOF > ./HBC/meta.xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>SaveGame Manager GX</name>
  <coder>Dj_Skual</coder>
  <version>rev$rev_new</version>
  <release_date>$rev_date</release_date>
  <!--   // remove this line to enable arguments
  <arguments>
    <arg>-ios=58</arg>
  </arguments>
  // remove this line to enable arguments  -->
  <ahb_access/>
  <short_description>SaveGames/Mii Manager</short_description> 
  <long_description>A Savegame/Mii extractor and installer for the Wii by Dj_Skual.
The GUI is done with LibWiiGui by Tantric and graphics by NeoRame/Cyrex from USB Loader GX.

Features:
 * SDHC and USB2 support
 * USB support FAT, NTFS and EXT2FS
 * Official saves support (data.bin)
 * Protected saves support
 * Mii Support
 * Multilanguage support 
 * Custom Font support
 * Themes support (direct download on www.spiffy360.com)
 * SaveGame download (direct download on www.wiisave.com)
 * GameTDB support
 * Display informations about Savegames/Data.bin in a special window
   (Name, Subname, ID, BlockSize, Icon)
 * Display informations about Miis in a special window
   (Name, Creator, Date, Gender, Favorite Color)
 * Extract, Install, Delete, Copy, Move fonctions are available
 * Custom Background Music support (AIF/MP3/OGG/BNS/WAV)
 * Online AutoUpdate
 * Homebrew Browser
 * Channel Browser
 * WiiLoad .dol receiver
 * Widesreen support
 * ScreenSaver

Credits:
  Coding: Dj_Skual, Giantpune
  Artworks: Neorame, Cyrex, Dj_Skual
  Languages: Dj Skual and Translaters

  Libwiigui: Tantric
  Libogc/Devkit: Shagkur and Wintermute
  FreeTypeGX: Armin Tamzarian.
  
Thanks to:
  Waninkoko for SaveGame Manager sources
  USB Loader GX Team for sources and artworks
  WiiXplorer Team for WiiXplorer sources
  Lustar for GameTDB (Game Title DataBase)
  Spiffy360 Team for Themes hosting
  WiiSave Team for Saves hosting
  Attila from wii-info.fr for help and support

Links:
  SaveGame Manager GX Official Site:
    http://savegamegx.net
	savegame-manager-gx/
  SaveGame Manager GX Project Page:
    http://code.google.com/p/
	savegame-manager-gx/
  WiiXplorer Support Site:
    http://code.google.com/p/
	savegame-manager-gx/updates/list
  USB Loader GX Support Site:
    http://code.google.com/p/
	usbloader-gui/updates/list
  Libwiigui Website:
    http://wiibrew.org/wiki/Libwiigui/
  GameTDB Website:
    http://gametdb.com
  Spiffy360 Website:
    http://wii.spiffy360.com/index.php
  WiiSave Website:
    http://www.wiisave.com
  FreeTypeGX Project Page:
    http://code.google.com/p/freetypegx/
  Gettext Official Page:
    http://www.gnu.org/software/gettext/
    gettext.html
  </long_description> 
</app>
EOF
