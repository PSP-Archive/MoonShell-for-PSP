/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id: metadata.h 29940 2011-05-31 21:26:18Z bluebrother $
 *
 * Copyright (C) 2005 Dave Chapman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

#ifndef _METADATA_H
#define _METADATA_H

#include <pspuser.h>
typedef u16 wchar;

#include <stdbool.h>
#include "config.h"
#include "file.h"

struct mp3entry {
    wchar* title;
    wchar* artist;
    wchar* albumtitle;
    wchar* genre_string;
    wchar* track_string;
    wchar* year_string;
    wchar* composer;
    wchar* comment;
    wchar* albumartist;
    unsigned int bitrate;
    unsigned long frequency;
    unsigned long first_frame_offset; /* Byte offset to first real MP3 frame. Used for skipping leading garbage to avoid gaps between tracks. */
    unsigned long length;   /* song length in ms */

    /* Musicbrainz Track ID */
    wchar* mb_track_id;
};

#endif

