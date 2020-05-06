//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2015-2018 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	[crispy] Crispness menu
//

#include "doomstat.h"
#include "p_local.h" // [crispy] thinkercap
#include "s_sound.h"
#include "r_defs.h" // [crispy] laserpatch
#include "r_sky.h" // [crispy] R_InitSkyMap()

#include "m_crispy.h"

multiitem_t multiitem_bobfactor[NUM_BOBFACTORS] =
{
    {BOBFACTOR_FULL, "full"},
    {BOBFACTOR_75, "75%"},
    {BOBFACTOR_OFF, "off"},
};

multiitem_t multiitem_brightmaps[NUM_BRIGHTMAPS] =
{
    {BRIGHTMAPS_OFF, "none"},
    {BRIGHTMAPS_TEXTURES, "walls"},
    {BRIGHTMAPS_SPRITES, "items"},
    {BRIGHTMAPS_BOTH, "both"},
};

multiitem_t multiitem_centerweapon[NUM_CENTERWEAPON] =
{
    {CENTERWEAPON_OFF, "off"},
    {CENTERWEAPON_CENTER, "centered"},
    {CENTERWEAPON_BOB, "bobbing"},
};

multiitem_t multiitem_coloredhud[NUM_COLOREDHUD] =
{
    {COLOREDHUD_OFF, "off"},
    {COLOREDHUD_BAR, "status bar"},
    {COLOREDHUD_TEXT, "hud texts"},
    {COLOREDHUD_BOTH, "both"},
};

multiitem_t multiitem_crosshair[NUM_CROSSHAIRS] =
{
    {CROSSHAIR_OFF, "off"},
    {CROSSHAIR_STATIC, "static"},
    {CROSSHAIR_PROJECTED, "projected"},
};

multiitem_t multiitem_crosshairtype[] =
{
    {-1, "none"},
    {0, "cross"},
    {1, "chevron"},
    {2, "dot"},
};

multiitem_t multiitem_freeaim[NUM_FREEAIMS] =
{
    {FREEAIM_AUTO, "autoaim"},
    {FREEAIM_DIRECT, "direct"},
    {FREEAIM_BOTH, "both"},
};

multiitem_t multiitem_demotimer[NUM_DEMOTIMERS] =
{
    {DEMOTIMER_OFF, "off"},
    {DEMOTIMER_RECORD, "recording"},
    {DEMOTIMER_PLAYBACK, "playback"},
    {DEMOTIMER_BOTH, "both"},
};

multiitem_t multiitem_demotimerdir[] =
{
    {0, "none"},
    {1, "forward"},
    {2, "backward"},
};

multiitem_t multiitem_freelook[NUM_FREELOOKS] =
{
    {FREELOOK_OFF, "off"},
    {FREELOOK_SPRING, "spring"},
    {FREELOOK_LOCK, "lock"},
};

multiitem_t multiitem_jump[NUM_JUMPS] =
{
    {JUMP_OFF, "off"},
    {JUMP_LOW, "low"},
    {JUMP_HIGH, "high"},
};

multiitem_t multiitem_secretmessage[NUM_SECRETMESSAGE] =
{
    {SECRETMESSAGE_OFF, "off"},
    {SECRETMESSAGE_ON, "on"},
    {SECRETMESSAGE_COUNT, "count"},
};

multiitem_t multiitem_translucency[NUM_TRANSLUCENCY] =
{
    {TRANSLUCENCY_OFF, "off"},
    {TRANSLUCENCY_MISSILE, "projectiles"},
    {TRANSLUCENCY_ITEM, "items"},
    {TRANSLUCENCY_BOTH, "both"},
};

multiitem_t multiitem_sndchannels[4] =
{
    {8, "8"},
    {16, "16"},
    {32, "32"},
};

multiitem_t multiitem_widgets[NUM_WIDGETS] =
{
    {WIDGETS_OFF, "never"},
    {WIDGETS_AUTOMAP, "in Automap"},
    {WIDGETS_ALWAYS, "always"},
};

extern void AM_LevelInit (boolean reinit);
extern void EnableLoadingDisk (void);
extern void P_SegLengths (boolean contrast_only);
extern void R_ExecuteSetViewSize (void);
extern void R_InitLightTables (void);
extern void I_ReInitGraphics (int reinit);

void M_CrispyToggleAutomapstats(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->automapstats--;
        if (crispy->automapstats < 0)
            crispy->automapstats = NUM_WIDGETS-1;
        break;

        case 1:
        crispy->automapstats++;
        if (crispy->automapstats > NUM_WIDGETS-1)
            crispy->automapstats = 0;
        break;
    }
}

void M_CrispyToggleBobfactor(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->bobfactor--;
        if (crispy->bobfactor < 0)
            crispy->bobfactor = NUM_BOBFACTORS-1;
        break;

        case 1:
        crispy->bobfactor++;
        if (crispy->bobfactor > NUM_BOBFACTORS-1)
            crispy->bobfactor = 0;
        break;
    }
}

void M_CrispyToggleBrightmaps(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->brightmaps--;
        if (crispy->brightmaps < 0)
            crispy->brightmaps = NUM_BRIGHTMAPS-1;
        break;

        case 1:
        crispy->brightmaps++;
        if (crispy->brightmaps > NUM_BRIGHTMAPS-1)
            crispy->brightmaps = 0;
        break;
    }
}

void M_CrispyToggleCenterweapon(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->centerweapon--;
        if (crispy->centerweapon < 0)
            crispy->centerweapon = NUM_CENTERWEAPON-1;
        break;

        case 1:
        crispy->centerweapon++;
        if (crispy->centerweapon > NUM_CENTERWEAPON-1)
            crispy->centerweapon = 0;
        break;
    }
}

void M_CrispyToggleColoredblood(int choice)
{
    thinker_t *th;

    if (gameversion == exe_chex)
    {
	return;
    }

    choice = 0;
    crispy->coloredblood = !crispy->coloredblood;

    // [crispy] switch NOBLOOD flag for Lost Souls
    for (th = thinkercap.next; th && th != &thinkercap; th = th->next)
    {
	if (th->function.acp1 == (actionf_p1)P_MobjThinker)
	{
		mobj_t *mobj = (mobj_t *)th;

		if (mobj->type == MT_SKULL)
		{
			if (crispy->coloredblood)
			{
				mobj->flags |= MF_NOBLOOD;
			}
			else
			{
				mobj->flags &= ~MF_NOBLOOD;
			}
		}
	}
    }
}

void M_CrispyToggleColoredhud(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->coloredhud--;
        if (crispy->coloredhud < 0)
            crispy->coloredhud = NUM_COLOREDHUD-1;
        break;

        case 1:
        crispy->coloredhud++;
        if (crispy->coloredhud > NUM_COLOREDHUD-1)
            crispy->coloredhud = 0;
        break;
    }
}

void M_CrispyToggleCrosshair(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->crosshair--;
        if (crispy->crosshair < 0)
            crispy->crosshair = NUM_CROSSHAIRS-1;
        break;

        case 1:
        crispy->crosshair++;
        if (crispy->crosshair > NUM_CROSSHAIRS-1)
            crispy->crosshair = 0;
        break;
    }
}

void M_CrispyToggleCrosshairHealth(int choice)
{
    choice = 0;
    crispy->crosshairhealth = !crispy->crosshairhealth;
}

void M_CrispyToggleCrosshairTarget(int choice)
{
    choice = 0;
    crispy->crosshairtarget = !crispy->crosshairtarget;
}

void M_CrispyToggleCrosshairtype(int choice)
{
    if (!crispy->crosshair)
    {
	return;
    }

    switch(choice)
    {
        case 0:
        crispy->crosshairtype--;
        if (crispy->crosshairtype < 0)
            crispy->crosshairtype = 2;
        break;

        case 1:
        crispy->crosshairtype++;
        if (crispy->crosshairtype > 2)
            crispy->crosshairtype = 0;
        break;
    }

    if (!laserpatch[crispy->crosshairtype].c)
    {
	crispy->crosshairtype = 0;
    }
}

void M_CrispyToggleDemoBar(int choice)
{
    choice = 0;
    crispy->demobar = !crispy->demobar;
}

void M_CrispyToggleDemoTimer(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->demotimer--;
        if (crispy->demotimer < 0)
            crispy->demotimer = NUM_DEMOTIMERS-1;
        break;

        case 1:
        crispy->demotimer++;
        if (crispy->demotimer > NUM_DEMOTIMERS-1)
            crispy->demotimer = 0;
        break;
    }
}

void M_CrispyToggleDemoTimerDir(int choice)
{
    if (!(crispy->demotimer & DEMOTIMER_PLAYBACK))
    {
	return;
    }

    choice = 0;
    crispy->demotimerdir = !crispy->demotimerdir;
}

void M_CrispyToggleExtAutomap(int choice)
{
    choice = 0;
    crispy->extautomap = !crispy->extautomap;
}

void M_CrispyToggleExtsaveg(int choice)
{
    choice = 0;
    crispy->extsaveg = !crispy->extsaveg;
}

void M_CrispyToggleFlipcorpses(int choice)
{
    if (gameversion == exe_chex)
    {
	return;
    }

    choice = 0;
    crispy->flipcorpses = !crispy->flipcorpses;
}

void M_CrispyToggleFreeaim(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    switch(choice)
    {
        case 0:
        crispy->freeaim--;
        if (crispy->freeaim < 0)
            crispy->freeaim = NUM_FREEAIMS-1;
        break;

        case 1:
        crispy->freeaim++;
        if (crispy->freeaim > NUM_FREEAIMS-1)
            crispy->freeaim = 0;
        break;
    }

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

static void M_CrispyToggleSkyHook (void)
{
    players[consoleplayer].lookdir = 0;
    R_InitSkyMap();
}

void M_CrispyToggleFreelook(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->freelook--;
        if (crispy->freelook < 0)
            crispy->freelook = NUM_FREELOOKS-1;
        break;

        case 1:
        crispy->freelook++;
        if (crispy->freelook > NUM_FREELOOKS-1)
            crispy->freelook = 0;
        break;
    }

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleFullsounds(int choice)
{
    int i;

    choice = 0;
    crispy->soundfull = !crispy->soundfull;

    // [crispy] weapon sound sources
    for (i = 0; i < MAXPLAYERS; i++)
    {
	if (playeringame[i])
	{
	    players[i].so = Crispy_PlayerSO(i);
	}
    }
}

static void M_CrispyToggleHiresHook (void)
{
    crispy->hires = !crispy->hires;

    // [crispy] re-initialize framebuffers, textures and renderer
    I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
    // [crispy] re-calculate framebuffer coordinates
    R_ExecuteSetViewSize();
    // [crispy] re-draw bezel
    R_FillBackScreen();
    // [crispy] re-calculate disk icon coordinates
    EnableLoadingDisk();
    // [crispy] re-calculate automap coordinates
    AM_LevelInit(true);
}

void M_CrispyToggleHires(int choice)
{
    choice = 0;

    crispy->post_rendering_hook = M_CrispyToggleHiresHook;
}

void M_CrispyToggleJumping(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    switch(choice)
    {
        case 0:
        crispy->jump--;
        if (crispy->jump < 0)
            crispy->jump = NUM_JUMPS-1;
        break;

        case 1:
        crispy->jump++;
        if (crispy->jump > NUM_JUMPS-1)
            crispy->jump = 0;
        break;
    }

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleLeveltime(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->leveltime--;
        if (crispy->leveltime < 0)
            crispy->leveltime = NUM_WIDGETS-1;
        break;

        case 1:
        crispy->leveltime++;
        if (crispy->leveltime > NUM_WIDGETS-1)
            crispy->leveltime = 0;
        break;
    }
}

void M_CrispyToggleMouseLook(int choice)
{
    choice = 0;
    crispy->mouselook = !crispy->mouselook;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyToggleNeghealth(int choice)
{
    choice = 0;
    crispy->neghealth = !crispy->neghealth;
}

void M_CrispyToggleOverunder(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    choice = 0;
    crispy->overunder = !crispy->overunder;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyTogglePitch(int choice)
{
    choice = 0;
    crispy->pitch = !crispy->pitch;

    crispy->post_rendering_hook = M_CrispyToggleSkyHook;
}

void M_CrispyTogglePlayerCoords(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->playercoords--;
        if (crispy->playercoords < 0)
            crispy->playercoords = NUM_WIDGETS-2; // [crispy] disable "always" setting
        break;

        case 1:
        crispy->playercoords++;
        if (crispy->playercoords > NUM_WIDGETS-2) // [crispy] disable "always" setting
            crispy->playercoords = 0;
        break;
    }
}

void M_CrispyToggleRecoil(int choice)
{
    if (!crispy->singleplayer)
    {
	return;
    }

    choice = 0;
    crispy->recoil = !crispy->recoil;

    // [crispy] update the "critical" struct
    CheckCrispySingleplayer(!demorecording && !demoplayback && !netgame);
}

void M_CrispyToggleSecretmessage(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->secretmessage--;
        if (crispy->secretmessage < 0)
            crispy->secretmessage = NUM_SECRETMESSAGE-1;
        break;

        case 1:
        crispy->secretmessage++;
        if (crispy->secretmessage > NUM_SECRETMESSAGE-1)
            crispy->secretmessage = 0;
        break;
    }
}

void M_CrispyToggleSmoothScaling(int choice)
{
    choice = 0;
    crispy->smoothscaling = !crispy->smoothscaling;
}

static void M_CrispyToggleSmoothLightingHook (void)
{
    crispy->smoothlight = !crispy->smoothlight;

    // [crispy] re-calculate the zlight[][] array
    R_InitLightTables();
    // [crispy] re-calculate the scalelight[][] array
    R_ExecuteSetViewSize();
    // [crispy] re-calculate fake contrast
    P_SegLengths(true);
}

void M_CrispyToggleSmoothLighting(int choice)
{
    choice = 0;

    crispy->post_rendering_hook = M_CrispyToggleSmoothLightingHook;
}

void M_CrispyToggleSndChannels(int choice)
{
    choice = 0;

    S_UpdateSndChannels();
}

void M_CrispyToggleSoundfixes(int choice)
{
    choice = 0;
    crispy->soundfix = !crispy->soundfix;
}

void M_CrispyToggleSoundMono(int choice)
{
    choice = 0;
    crispy->soundmono = !crispy->soundmono;

    S_UpdateStereoSeparation();
}

void M_CrispyToggleTranslucency(int choice)
{
    switch(choice)
    {
        case 0:
        crispy->translucency--;
        if (crispy->translucency < 0)
            crispy->translucency = NUM_TRANSLUCENCY-1;
        break;

        case 1:
        crispy->translucency++;
        if (crispy->translucency > NUM_TRANSLUCENCY-1)
            crispy->translucency = 0;
        break;
    }
}

void M_CrispyToggleUncapped(int choice)
{
    choice = 0;

    crispy->uncapped = !crispy->uncapped;
}

void M_CrispyToggleVsyncHook (void)
{
    crispy->vsync = !crispy->vsync;

    I_ReInitGraphics(REINIT_RENDERER | REINIT_TEXTURES | REINIT_ASPECTRATIO);
}

void M_CrispyToggleVsync(int choice)
{
    choice = 0;

    if (force_software_renderer)
    {
	return;
    }

    crispy->post_rendering_hook = M_CrispyToggleVsyncHook;
}

void M_CrispyToggleWeaponSquat(int choice)
{
    choice = 0;
    crispy->weaponsquat = !crispy->weaponsquat;
}

static void M_CrispyToggleWidescreenHook (void)
{
    crispy->widescreen = !crispy->widescreen;

    // [crispy] no need to re-init when switching from wide to compact
    {
	// [crispy] re-initialize framebuffers, textures and renderer
	I_ReInitGraphics(REINIT_FRAMEBUFFERS | REINIT_TEXTURES | REINIT_ASPECTRATIO);
	// [crispy] re-calculate framebuffer coordinates
	R_ExecuteSetViewSize();
	// [crispy] re-draw bezel
	R_FillBackScreen();
	// [crispy] re-calculate disk icon coordinates
	EnableLoadingDisk();
	// [crispy] re-calculate automap coordinates
	AM_LevelInit(true);
    }
}

void M_CrispyToggleWidescreen(int choice)
{
    choice = 0;

    crispy->post_rendering_hook = M_CrispyToggleWidescreenHook;
}
