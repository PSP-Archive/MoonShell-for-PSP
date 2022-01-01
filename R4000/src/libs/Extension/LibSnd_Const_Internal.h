#pragma once

#define LibSnd_DebugOut conout

#ifndef LibSnd_DebugOut
static void LibSnd_DebugOut(const char *fmt, ...)
{
}
#endif

