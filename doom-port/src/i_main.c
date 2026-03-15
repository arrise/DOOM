// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log:$
//
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";



#include "doomdef.h"

#include "m_argv.h"
#include "d_main.h"

#include <stdio.h>

extern void Window_InstallCrashHandler(void);

#define SDL_MAIN_HANDLED
#include <SDL.h>

int
main
( int		argc,
  char**	argv ) 
{ 
    SDL_SetMainReady();
#ifdef _WIN32
    Window_InstallCrashHandler();
#endif

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    myargc = argc; 
    myargv = argv; 
 
    D_DoomMain (); 

    return 0;
} 
