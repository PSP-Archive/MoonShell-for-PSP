
/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * $Id: asf.c 18814 2008-10-15 06:38:51Z zagor $
 *
 * Copyright (C) 2007 Dave Chapman
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
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
//#include <ctype.h>

#include <pspuser.h>

#include "config.h"

#include "codecs.h"
#include "codecs/lib/codeclib.h"
#include "ffmpeg_bswap.h"

#include "memtools.h"
#include "Unicode.h"

//#include "metadata.h"
//#include "replaygain.h"
//#include "debug.h"
//#include "rbunicode.h"
//#include "metadata_common.h"
//#include "metadata_parsers.h"
//#include "system.h"

#include "asf.h"
#include "codecs.h"

#undef DEBUGF
#define DEBUGF(...)

// ----------------------------------

#include "bswap.h"

static int read_uint32be(int fd, unsigned int* buf)
{
  size_t n;

  n = csi->read_filebuf(buf,4);
  *buf = bswap_32(*buf);
  return n;
}

#define read_uint16le(buf) csi->read_filebuf(buf, 2);
#define read_uint32le(buf) csi->read_filebuf(buf, 4);
#define read_uint64le(buf) csi->read_filebuf(buf, 8);

#define read_buf(buf,size) csi->read_filebuf((buf), (size))

//#define GetFileSize(fd) csi->GetFileSize()

#define lseek_cur(pos) csi->advance_buffer(pos)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static const unsigned char utf8comp[6] =
{
    0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC
};

#define UNI_MASK   0xC0 /* 11000000 */
#define UNI_COMP   0x80 /* 10x      */

// --------------------------------

/* TODO: Just read the GUIDs into a 16-byte array, and use memcmp to compare */
struct guid_s {
    uint32_t v1;
    uint16_t v2;
    uint16_t v3;
    uint8_t  v4[8];
};
typedef struct guid_s guid_t;

struct asf_object_s {
    guid_t       guid;
    uint64_t     size;
    uint64_t     datalen;
};
typedef struct asf_object_s asf_object_t;

static const guid_t asf_guid_null =
{0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/* top level object guids */

static const guid_t asf_guid_header =
{0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_data =
{0x75B22636, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_index =
{0x33000890, 0xE5B1, 0x11CF, {0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB}};

/* header level object guids */

static const guid_t asf_guid_file_properties =
{0x8cabdca1, 0xa947, 0x11cf, {0x8E, 0xe4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_stream_properties =
{0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}};

static const guid_t asf_guid_content_description =
{0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}};

static const guid_t asf_guid_extended_content_description =
{0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}};

static const guid_t asf_guid_content_encryption =
{0x2211b3fb, 0xbd23, 0x11d2, {0xb4, 0xb7, 0x00, 0xa0, 0xc9, 0x55, 0xfc, 0x6e}};

static const guid_t asf_guid_extended_content_encryption =
{0x298ae614, 0x2622, 0x4c17, {0xb9, 0x35, 0xda, 0xe0, 0x7e, 0xe9, 0x28, 0x9c}};

/* stream type guids */

static const guid_t asf_guid_stream_type_audio =
{0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}};

static int asf_guid_match(const guid_t *guid1, const guid_t *guid2)
{
    if((guid1->v1 != guid2->v1) ||
       (guid1->v2 != guid2->v2) ||
       (guid1->v3 != guid2->v3) ||
       (memcmp(guid1->v4, guid2->v4, 8))) {
        return 0;
    }

    return 1;
}

/* Read the 16 byte GUID from a file */
static void asf_readGUID(guid_t* guid)
{
    read_uint32le(&guid->v1);
    read_uint16le(&guid->v2);
    read_uint16le(&guid->v3);
    read_buf(guid->v4, 8);
}

static void asf_read_object_header(asf_object_t *obj)
{
    asf_readGUID(&obj->guid);
    read_uint64le(&obj->size);
    obj->datalen = 0;
}

/* Parse an integer from the extended content object - we always
   convert to an int, regardless of native format.
*/
static int asf_intdecode(int type, int length)
{
    uint16_t tmp16;
    uint32_t tmp32;
    uint64_t tmp64;

    if (type==3) {
        read_uint32le(&tmp32);
        lseek_cur(length - 4);
        return (int)tmp32;
    } else if (type==4) {
        read_uint64le(&tmp64);
        lseek_cur(length - 8);
        return (int)tmp64;
    } else if (type == 5) {
        read_uint16le(&tmp16);
        lseek_cur(length - 2);
        return (int)tmp16;
    }

    return 0;
}

/* Decode a LE utf16 string from a disk buffer into a fixed-sized
   utf8 buffer.
*/

static wchar* asf_utf16LEdecode(uint16_t utf16bytes)
{
  if(utf16bytes==0) return(NULL);
  
    u8 *putf16buf=(u8*)safemalloc(utf16bytes);
    u8 *utf16=putf16buf;
    int n = read_buf(utf16, utf16bytes);
    
    wchar *pstrw_master=(wchar*)safemalloc(utf16bytes+2);
    wchar *pstrw=pstrw_master;
    
    while (n > 0) {
        unsigned long ucs;
        /* Check for a surrogate pair */
        if (utf16[1] >= 0xD8 && utf16[1] < 0xE0) {
            if (n < 4) {
                /* Truncated utf16 string, abort */
                break;
            }
            ucs = 0x10000 + ((utf16[0] << 10) | ((utf16[1] - 0xD8) << 18)
                             | utf16[2] | ((utf16[3] - 0xDC) << 8));
            utf16 += 4;
            n -= 4;
        } else {
            ucs = (utf16[0] | (utf16[1] << 8));
            utf16 += 2;
            n -= 2;
        }
        
        if(0x10000<=ucs) ucs='?';
        *pstrw++=(wchar)ucs;
    }
    
    *pstrw=0;
    
    if(putf16buf!=NULL){
      safefree(putf16buf); putf16buf=NULL;
    }
    
//    conout("W:%s\n",StrConvert_Unicode2Ank_Test(pstrw_master));
    return(pstrw_master);
}

static char* asf_utf16LEdecode2ANSI(uint16_t utf16bytes)
{
  if(utf16bytes==0){
    char *pstra=(char*)safemalloc(5);
    pstra[0]='N';
    pstra[1]='U';
    pstra[2]='L';
    pstra[3]='L';
    pstra[4]=0;
    return(pstra);
  }
  
    char *putf16buf=(char*)safemalloc(utf16bytes);
    char *utf16=putf16buf;
    int n = read_buf(putf16buf, utf16bytes);
    
    char *pstra_master=(char*)safemalloc(utf16bytes+1);
    char *pstra=pstra_master;
    
    while (n > 0) {
        unsigned long ucs;
        /* Check for a surrogate pair */
        if (utf16[1] >= 0xD8 && utf16[1] < 0xE0) {
            if (n < 4) {
                /* Truncated utf16 string, abort */
                break;
            }
            ucs = 0x10000 + ((utf16[0] << 10) | ((utf16[1] - 0xD8) << 18)
                             | utf16[2] | ((utf16[3] - 0xDC) << 8));
            utf16 += 4;
            n -= 4;
        } else {
            ucs = (utf16[0] | (utf16[1] << 8));
            utf16 += 2;
            n -= 2;
        }
        
        if(0x100<=ucs) ucs='?';
        *pstra++=(char)ucs;
    }
    
    *pstra=0;
    
    if(putf16buf!=NULL){
      safefree(putf16buf); putf16buf=NULL;
    }
    
//    conout("A:%s\n",pstra_master);
    return(pstra_master);
}

static int asf_parse_header(struct mp3entry* id3, asf_waveformatex_t* wfx)
{
    asf_object_t current;
    asf_object_t header;
    uint64_t datalen;
    int i;
    int fileprop = 0;
    uint64_t play_duration;
    uint16_t flags;
    uint32_t subobjects;

    asf_read_object_header(&header);

    DEBUGF("header.size=%d\n",(int)header.size);
    
    if (header.size < 30) {
        /* invalid size for header object */
        return ASF_ERROR_OBJECT_SIZE;
    }

    read_uint32le(&subobjects);
    
    /* Two reserved bytes - do we need to read them? */
    lseek_cur(2);

    DEBUGF("Read header - size=%d, subobjects=%d\n",(int)header.size, (int)subobjects);

    if (subobjects > 0) {
        header.datalen = header.size - 30;

        /* TODO: Check that we have datalen bytes left in the file */
        datalen = header.datalen;

        for (i=0; i<(int)subobjects; i++) {
            DEBUGF("--- Parsing header object %d - datalen=%d\n",i,(int)datalen);
            if (datalen < 24) {
                LOGF("not enough data for reading object\n");
                break;
            }

            asf_read_object_header(&current);
            DEBUGF("GUIDv1=0x%x, GUIDv2=0x%x, GUIDv3=0x%x, GUIDv4=....\n",current.guid.v1,current.guid.v2,current.guid.v3);

            if (current.size > datalen || current.size < 24) {
                LOGF("invalid object size - current.size=%d, datalen=%d\n",(int)current.size,(int)datalen);
                break;
            }

            if (asf_guid_match(&current.guid, &asf_guid_file_properties)) {
                    if (current.size < 104)
                        return ASF_ERROR_OBJECT_SIZE;

                    if (fileprop) {
                        /* multiple file properties objects not allowed */
                        return ASF_ERROR_INVALID_OBJECT;
                    }

                    fileprop = 1;
                    /* All we want is the play duration - uint64_t at offset 40 */
                    lseek_cur(40);

                    read_uint64le(&play_duration);
                    id3->length = play_duration / 10000;

                    DEBUGF("****** length = %lums\n", id3->length);

                    /* Read the packet size - uint32_t at offset 68 */
                    lseek_cur(20);
                    read_uint32le(&wfx->packet_size);

                    /* Skip bytes remaining in object */
                    lseek_cur(current.size - 24 - 72);
            } else if (asf_guid_match(&current.guid, &asf_guid_stream_properties)) {
                    guid_t guid;
                    uint32_t propdatalen;

                    if (current.size < 78)
                        return ASF_ERROR_OBJECT_SIZE;

#if 0
                    asf_byteio_getGUID(&guid, current->data);
                    datalen = asf_byteio_getDWLE(current->data + 40);
                    flags = asf_byteio_getWLE(current->data + 48);
#endif

                    asf_readGUID(&guid);

                    lseek_cur(24);
                    read_uint32le(&propdatalen);
                    lseek_cur(4);
                    read_uint16le(&flags);

                    if (!asf_guid_match(&guid, &asf_guid_stream_type_audio)) {
                        DEBUGF("Found stream properties for non audio stream, skipping\n");
                        lseek_cur(current.size - 24 - 50);
                    } else if (wfx->audiostream == -1) {
                        lseek_cur(4);
                        DEBUGF("Found stream properties for audio stream %d\n",flags&0x7f);

                        if (propdatalen < 18) {
                            return ASF_ERROR_INVALID_LENGTH;
                        }

#if 0
                        if (asf_byteio_getWLE(data + 16) > datalen - 16) {
                            return ASF_ERROR_INVALID_LENGTH;
                        }
#endif
                        read_uint16le(&wfx->codec_id);
                        read_uint16le(&wfx->channels);
                        read_uint32le(&wfx->rate);
                        read_uint32le(&wfx->bitrate);
                        wfx->bitrate *= 8;
                        read_uint16le(&wfx->blockalign);
                        read_uint16le(&wfx->bitspersample);
                        read_uint16le(&wfx->datalen);

                        /* Round bitrate to the nearest kbit */
                        id3->bitrate = (wfx->bitrate + 500) / 1000;
                        id3->frequency = wfx->rate;

                        if (wfx->codec_id == ASF_CODEC_ID_WMAV1) {
                            read_buf(wfx->data, 4);
                            lseek_cur(current.size - 24 - 72 - 4);
                            wfx->audiostream = flags&0x7f;
                        } else if (wfx->codec_id == ASF_CODEC_ID_WMAV2) {
                            read_buf(wfx->data, 6);
                            lseek_cur(current.size - 24 - 72 - 6);
                            wfx->audiostream = flags&0x7f;
                        } else {
                            LOGF("Unsupported WMA codec (Pro, Lossless, Voice, etc)\n");
                            lseek_cur(current.size - 24 - 72);
                        }

                    }
            } else if (asf_guid_match(&current.guid, &asf_guid_content_description)) {
                    /* Object contains five 16-bit string lengths, followed by the five strings:
                       title, artist, copyright, description, rating
                     */
                    uint16_t strlength[5];
                    int i;

                    DEBUGF("Found GUID_CONTENT_DESCRIPTION - size=%d\n",(int)(current.size - 24));

                    /* Read the 5 string lengths - number of bytes included trailing zero */
                    for (i=0; i<5; i++) {
                        read_uint16le(&strlength[i]);
                        DEBUGF("strlength = %u\n",strlength[i]);
                    }

                    if (strlength[0] > 0) {  /* 0 - Title */
                        id3->title=asf_utf16LEdecode(strlength[0]);
                    }

                    if (strlength[1] > 0) {  /* 1 - Artist */
                        id3->artist=asf_utf16LEdecode(strlength[1]);
                    }

                    lseek_cur(strlength[2]); /* 2 - copyright */

                    if (strlength[3] > 0) {  /* 3 - description */
                        id3->comment=asf_utf16LEdecode(strlength[3]);
                    }

                    lseek_cur(strlength[4]); /* 4 - rating */
            } else if (asf_guid_match(&current.guid, &asf_guid_extended_content_description)) {
                    uint16_t count;
                    int i;
                    int bytesleft = current.size - 24;
                    DEBUGF("Found GUID_EXTENDED_CONTENT_DESCRIPTION\n");

                    read_uint16le(&count);
                    bytesleft -= 2;
                    DEBUGF("extended metadata count = %u\n",count);

                    for (i=0; i < count; i++) {
                        char *pType;
                        {
                          u16 length;
                          read_uint16le(&length);
                          pType=asf_utf16LEdecode2ANSI(length);
                          bytesleft -= 2 + length;
                        }

                        uint16_t length, type;
                        read_uint16le(&type);
                        read_uint16le(&length);
//                        conout("type:%d, length:%d.\n",type,length);

                        if (!strcmp("WM/TrackNumber",pType)) {
                            if (type == 0) {
                                id3->track_string=asf_utf16LEdecode(length);
                            } else if ((type >=2) && (type <= 5)) {
                                lseek_cur(length);
                            } else {
                                lseek_cur(length);
                            }
                        } else if (!strcmp("WM/Genre",pType) && (type == 0)) {
                            id3->genre_string=asf_utf16LEdecode(length);
                        } else if (!strcmp("WM/AlbumTitle",pType) && (type == 0)) {
                            id3->albumtitle=asf_utf16LEdecode(length);
                        } else if (!strcmp("WM/AlbumArtist",pType) && (type == 0)) {
                            id3->albumartist=asf_utf16LEdecode(length);
                        } else if (!strcmp("WM/Composer",pType) && (type == 0)) {
                            id3->composer=asf_utf16LEdecode(length);
                        } else if (!strcmp("WM/Year",pType)) {
                            if (type == 0) {
                                id3->year_string=asf_utf16LEdecode(length);
                            } else if ((type >=2) && (type <= 5)) {
                                lseek_cur(length);
                            } else {
                                lseek_cur(length);
                            }
                        } else if (!strncmp("replaygain_", pType, 11)) {
                            lseek_cur(length);
                        } else if (!strcmp("MusicBrainz/Track Id", pType)) {
                            id3->mb_track_id=asf_utf16LEdecode(length);
                        } else {
                            lseek_cur(length);
                        }
                        bytesleft -= 4 + length;
                        if(pType!=NULL){
                          safefree(pType); pType=NULL;
                        }
                    }

                    lseek_cur(bytesleft);
            } else if (asf_guid_match(&current.guid, &asf_guid_content_encryption)
                || asf_guid_match(&current.guid, &asf_guid_extended_content_encryption)) {
                LOGF("File is encrypted\n");
                return ASF_ERROR_ENCRYPTED;
            } else {
                DEBUGF("Skipping %d bytes of object\n",(int)(current.size - 24));
                lseek_cur(current.size - 24);
            }

            DEBUGF("Parsed object - size = %d\n",(int)current.size);
            datalen -= current.size;
        }

        if (i != (int)subobjects || datalen != 0) {
            LOGF("header data doesn't match given subobject count\n");
            return ASF_ERROR_INVALID_VALUE;
        }

        DEBUGF("%d subobjects read successfully\n", i);
    }

#if 0
    tmp = asf_parse_header_validate(file, &header);
    if (tmp < 0) {
        /* header read ok but doesn't validate correctly */
        return tmp;
    }
#endif

    DEBUGF("header validated correctly\n");

    return 0;
}

void asf_metadata_init(struct mp3entry* id3, asf_waveformatex_t* wfx)
{
  id3->title=NULL;
  id3->artist=NULL;
  id3->albumtitle=NULL;
  id3->genre_string=NULL;
  id3->track_string=NULL;
  id3->year_string=NULL;
  id3->composer=NULL;
  id3->comment=NULL;
  id3->albumartist=NULL;
  id3->bitrate=0;
  id3->frequency=0;
  id3->first_frame_offset=0;
  id3->length=0;
}

bool asf_metadata_get(struct mp3entry* id3, asf_waveformatex_t* wfx)
{
    int res;
    asf_object_t obj;

    wfx->audiostream = -1;

    res = asf_parse_header(id3, wfx);

    if (res < 0) {
        LOGF("ASF: parsing error - %d\n",res);
        return false;
    }
    
    if (wfx->audiostream == -1) {
        LOGF("ASF: No WMA streams found\n");
        return false;
    }

    asf_read_object_header(&obj);

    if (!asf_guid_match(&obj.guid, &asf_guid_data)) {
        LOGF("ASF: No data object found\n");
        return false;
    }

    /* Store the current file position - no need to parse the header
       again in the codec.  The +26 skips the rest of the data object
       header.
     */
    id3->first_frame_offset = csi->curpos + 26;
    
    return true;
}

void asf_metadata_free(struct mp3entry* id3, asf_waveformatex_t* wfx)
{
#define free(ptr){ \
  if((ptr)!=NULL){ \
    safefree(ptr); (ptr)=NULL; \
  } \
}

  free(id3->title);
  free(id3->artist);
  free(id3->albumtitle);
  free(id3->genre_string);
  free(id3->track_string);
  free(id3->year_string);
  free(id3->composer);
  free(id3->comment);
  free(id3->albumartist);
  id3->bitrate=0;
  id3->frequency=0;
  id3->first_frame_offset=0;
  id3->length=0;
}

