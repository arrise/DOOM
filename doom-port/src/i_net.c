#include "doomdef.h"
#include "i_system.h"
#include "d_net.h"

extern boolean netgame;
extern doomcom_t* doomcom;

void I_InitNetwork (void)
{
    doomcom = malloc (sizeof (*doomcom) );
    memset (doomcom, 0, sizeof(*doomcom) );
    doomcom->id = DOOMCOM_ID;
    doomcom->numplayers = doomcom->numnodes = 1;
    doomcom->deathmatch = false;
    doomcom->consoleplayer = 0;
    doomcom->ticdup = 1;
    doomcom->extratics = 0;
    netgame = false;
}

void I_NetCmd (void)
{
}
