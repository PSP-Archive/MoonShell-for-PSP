/*
 freeverb.cpp - wrapper function for freeverb
 Made by Daisuke Nagano <breeze.nagano@nifty.com>
 Feb.26.2006

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freeverb.h"

#include "revmodel.hpp"

void* create_freeverb(void)
{
  revmodel *pmodel=new revmodel;
  if(pmodel==NULL) return NULL;
  
  pmodel->setsrate(44100);
  pmodel->setpredelay(0.1);
  pmodel->setroomsize(0.8);
  pmodel->setdamp(0.3);
  pmodel->setwidth(0.8);
  pmodel->setmode(0);
  pmodel->setdry(0.5);
  pmodel->setwet(0.3);
  pmodel->update();
  
  return (void *)pmodel;
}

void delete_freeverb(void* oself)
{
  revmodel *pmodel=(revmodel*)oself;
  
  if(pmodel!=NULL){
    delete pmodel; pmodel=NULL;
  }
}

void setlevel_freeverb(void* oself, float level)
{
  revmodel *pmodel=(revmodel*)oself;
  
  pmodel->setroomsize(level);
  pmodel->update();
}

void process_freeverb_s16(void* oself, s16 *in_data, int samplescount)
{
  revmodel *pmodel=(revmodel*)oself;
  
  pmodel->processreplace_s16(in_data, samplescount);
}

void process_freeverb_s32(void* oself, s32 *in_data, int samplescount)
{
  revmodel *pmodel=(revmodel*)oself;
  
  pmodel->processreplace_s32(in_data, samplescount);
}

void reset_freeverb(void* oself)
{
  revmodel *pmodel=(revmodel*)oself;
  
  pmodel->mute();
}

