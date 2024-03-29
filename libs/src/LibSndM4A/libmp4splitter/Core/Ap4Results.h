#pragma once

/*****************************************************************
|
|    AP4 - Result Codes
|
|    Copyright 2002 Gilles Boccon-Gibod
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const int AP4_SUCCESS                               =  0;
const int AP4_FAILURE                               = -1;
const int AP4_ERROR_OUT_OF_MEMORY                   = -2;
const int AP4_ERROR_INVALID_PARAMETERS              = -3;
const int AP4_ERROR_NO_SUCH_FILE                    = -4;
const int AP4_ERROR_PERMISSION_DENIED               = -5;
const int AP4_ERROR_CANNOT_OPEN_FILE                = -6;
const int AP4_ERROR_EOS                             = -7;
const int AP4_ERROR_WRITE_FAILED                    = -8;
const int AP4_ERROR_READ_FAILED                     = -9;
const int AP4_ERROR_INVALID_FORMAT                  = -10;
const int AP4_ERROR_NO_SUCH_ITEM                    = -11;
const int AP4_ERROR_OUT_OF_RANGE                    = -12;
const int AP4_ERROR_INTERNAL                        = -13;
const int AP4_ERROR_INVALID_STATE                   = -14;
const int AP4_ERROR_LIST_EMPTY                      = -15;
const int AP4_ERROR_LIST_OPERATION_ABORTED          = -16;
const int AP4_ERROR_INVALID_RTP_CONSTRUCTOR_TYPE    = -17;
const int AP4_ERROR_NOT_SUPPORTED_YET               = -18;
const int AP4_ERROR_INVALID_TRACK_TYPE              = -19;
const int AP4_ERROR_INVALID_RTP_PACKET_EXTRA_DATA   = -20;

/*----------------------------------------------------------------------
|       macros
+---------------------------------------------------------------------*/
#define AP4_FAILED(result) ((result) != AP4_SUCCESS)
#define AP4_SUCCEEDED(result) ((result) == AP4_SUCCESS)

