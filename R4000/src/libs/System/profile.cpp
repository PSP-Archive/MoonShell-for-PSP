
#include <pspuser.h>

#include <stdio.h>
#include <time.h>

#include "common.h"

#include "profile.h"

static u32 clk;

void PrfStart(void)
{
  clk=clock();
}

void PrfEnd(void)
{
  double usec=(double)(clock()-clk)/CLOCKS_PER_SEC/1000;
  printf("Profile: %4.3fms.\n",usec*1000);
}

