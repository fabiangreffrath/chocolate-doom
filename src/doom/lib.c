//////////////////////////
//
//  LIB.C:  Miscellaneous
//
//////////////////////////

#include "marshmallow.h"
#include "pkemeter.h"

void Advance_M_Tics()
{
    if (Marshmallow_DynamicMusic && Doom_DJ.init)
        Doom_DJ.musictic++;

    marshmallow_tic++;
}


void HandleNetgameEvents()
{
    if ( Marshmallow_CheckForMultiplayerEvent() )  // this was usually in HUD_Ticker()
    {
        int i;

        for (i=0 ; i<MAXPLAYERS; i++)
        {
            chat_on = false;   // kill the input mode and cursor once it detects one of our special characters
            players[i].cmd.chatchar = 0;
        }
    }
}


void Player_Actions()
{
    PKE_Scanner();
    PushBarrel();
    ResetBarrel();
    AutoUse();
}


void Game_Actions()
{
    DroppedItemCleanup();
    CorpseCleanup();
}


void Game_Init()
{
    AddCmdLineBots();
    InitMusic();
}


static boolean CheckVileTargetDistance(mobj_t *actor)
{
	fixed_t	dist, limit;
	mobj_t *pl;
	
    if (!actor->target)
	return false;
		
    pl = actor->target;
    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);
	limit = MELEERANGE * 5;  //-20*FRACUNIT+pl->info->radius;

    if (dist <= limit)
		return true;  // too close
	else
		return false;  // far enough to attack
}


boolean CheckVileZScope(mobj_t *actor) 
{
	if (!actor->target)
		return false;

	if (actor->z > actor->target->z + VILE_Z_SCOPE_LIMIT && CheckVileTargetDistance(actor)) 
		return false;

	if (actor->z < actor->target->z - VILE_Z_SCOPE_LIMIT/* && CheckVileTargetDistance(actor)*/)  // don't check distance when vile is below player
		return false;
	
	return true;
}


void ToggleFastMonsters()
{
	int i;

	if (!fastparm)
	{
		fastparm = true;
		
		if (!menus_on)
		SHOW_MESSAGE "FAST MONSTERS ENABLED!";

		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++) 
			if (states[i].tics > 1)	{ states[i].tics >>= 1;	}

		mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
	}
	else
	{
		fastparm = false;

		if (!menus_on)
		SHOW_MESSAGE "FAST MONSTERS DISABLED!";		

		for (i=S_SARG_RUN1 ; i<=S_SARG_PAIN2 ; i++)
			states[i].tics <<= 1;

		mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
		mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
		mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
	}
}


void RestartMap ()
{
	if (Marshmallow_DynamicMusic)
		DJ_StartPlaylist(SUSPENSEFUL);

	newskill = skill_selection;  // new

	SetSkillUpgrades();

	G_DeferedInitNew(newskill, gameepisode, gamemap);  

	gaveweapons = false;

	MAIN_PLAYER.message = DEH_String(REDOMAP);

	HideAllMenus(); 

	SetKeyDelay();
}


void CollectTreasure(treasure_t color, mobj_t* toucher)
{
    switch (color)
    {
        case BLUE:
            toucher->player->message = DEH_String(TREASURE_BLUESKULL);

            treasure_bag.score += ICESKULL_REWARD;
            treasure_bag.blueskulls++;

            break;

        case GOLD:
            toucher->player->message = DEH_String(TREASURE_YELLOWSKULL);

            treasure_bag.score += GOLDSKULL_REWARD;
            treasure_bag.goldskulls++;

            break;

        case RED:
            toucher->player->message = DEH_String(TREASURE_REDSKULL);

            treasure_bag.score += BLOODSKULL_REWARD;
            treasure_bag.redskulls++;

            break;
    }

	treasure_bag.remaining_in_level--;

	if (treasure_bag.remaining_in_level == 0)
	{
		SHOW_CENTERED_MESSAGE DEH_String(TREASURECONGRATS);
		level_stats.alltreasure++;
	}
}


void ResetTreasure()
{
    treasure_bag.total_in_level = treasure_bag.remaining_in_level = 0;
}


void IncrementShotsFired(player_t* player)
{
    if (player->player_number == consoleplayer)  // Don't count bots or other players towards accuracy stats
    {
        switch (player->readyweapon) {
            case wp_shotgun:
            case wp_supershotgun:
                level_stats.shots_fired += shotgun_pellets;
                break;

            case wp_bfg:
            case wp_fist:
            case wp_chainsaw:
            case wp_missile:
                break;  // Don't count these weapons in shots_fired

            default:
                level_stats.shots_fired++;
        }
    }
}


void RegisterShotsHit(mobj_t* inflictor, mobj_t* target)
{
    if (inflictor
        && inflictor->player)
    {
        if ( !IsBot(inflictor->player)
             && IsMonster(target)
             && inflictor->player->readyweapon != wp_bfg  // Don't count these weapons in shots_hit
             && inflictor->player->readyweapon != wp_fist
             && inflictor->player->readyweapon != wp_chainsaw
             && inflictor->player->readyweapon != wp_missile
             && inflictor->player->player_number == consoleplayer )  // Don't count bots or other players towards accuracy stats
        {
            level_stats.shots_hit++;
        }
    }
}

void InfightAlert(mobj_t* actor)
{
    if (IsMonster(actor)
        && IsMonster(actor->target)
        && Marshmallow_InfightAlert )
    {
        if ( PlayerIsDead() )
            return;

        WriteToFourthConsoleLine(MONSTERFIGHT, SHORT_EXTRALINE_TIMEOUT);
    }
    else
    {
        return;
    }
}


boolean PercentChance(int chance)  
{
	int num = GetRandomIntegerInRange(1, 99);

	if (num <= chance)
		return true;
	else
		return false;
}


int GetGameType()   
{
	if (Marshmallow_WadStealing)
		return BOTH;  
	
	if (gamemode == commercial)
		return DOOM2;  
	else
		return DOOM1;  
}


void OfferSuicide()
{
    offertimeout_suicide = DEFAULT_OFFER_TIMEOUT;
    SHOW_MESSAGE DEH_String(CONFIRMSUICIDE);
}


void OfferRadsuit(player_t* player)
{
	if (!player->powers[pw_ironfeet]     
	&& player->extra_powers[ITEM_RADSUIT]
	&& !offer_medkit)
	{
        if ( PlayerIsDead() )
            return;

        offer_radsuit = true;
        offertimeout_radsuit = DEFAULT_OFFER_TIMEOUT;

        WriteToSecondConsoleLine(PROMPTRADSUIT, DEFAULT_EXTRALINE_TIMEOUT);
	}
}


void LowHealthWarning()
{
	if (MAIN_PLAYER.extra_powers[ITEM_MEDKIT] == false
		|| PlayerIsDead() )
	return;

	if (MAIN_PLAYER.health <= 33   // medkit overrides radsuit if health is low
		|| MAIN_PLAYER.mo->health <= 33)
	{
		if (offer_radsuit)   
			return;

		offer_medkit = true;
		offertimeout_medkit = DEFAULT_OFFER_TIMEOUT;

        WriteToSecondConsoleLine(PROMPTMEDKIT, DEFAULT_EXTRALINE_TIMEOUT);
	}
}


void BerserkReminder()
{
	if (!Marshmallow_BerserkReminder)
		return;

    if ( PlayerIsDead() )
        return;

	if (MAIN_PLAYER.pendingweapon == wp_fist
		&& MAIN_PLAYER.powers[pw_strength])
	{
        //SHOW_MESSAGE DEH_String(YOUBERSERK);
        WriteToSecondConsoleLine(YOUBERSERK, SHORT_EXTRALINE_TIMEOUT);
    }
}


#define MENU_KEYPRESS_DELAY 7

boolean CheckKeyDelay()  // TODO: could move the sound effect in here so it's only called in one spot 
{
	if (menu_key_delay)
		return true;		// aborting calling function

	SetKeyDelay();

	return false;
}


void SetKeyDelay()
{
	menu_key_delay = MENU_KEYPRESS_DELAY;
}


void AddIntegerToInfoReadout(char* label, int val, int line)
{
	char output[16];

	sprintf(output, "%d", val);

	HUlib_addMessageToSText(&miscreadout_output[line-1], label, output);   
}


char* ShowIntAsChar(int val, int color)
{
	char string[16];

	sprintf( string, "%d", val );
	//CrispyReplaceColor( string, color, string);

	return DEH_String( string );
}


void AddStringsToInfoReadout(char* label, char* output, int line)
{
	HUlib_addMessageToSText(&miscreadout_output[line-1], label, output);   
}


boolean WeHaveItem(invitem_t item)
{
	if (MAIN_PLAYER.extra_powers[item])
		return true;
	else
		return false;
}


static void SetDoom1Level()
{
	if (skip_to_level <= 9)
	{
		gameepisode = 1;
		gamemap = skip_to_level;
	}
	else if (skip_to_level <= 18)
	{
		gameepisode = 2;
		gamemap = skip_to_level - 9;
	}
	else if (skip_to_level <= 27)
	{
		gameepisode = 3;
		gamemap = skip_to_level - 18;
	}
	else if (skip_to_level <= 36)
	{
		gameepisode = 4;
		gamemap = skip_to_level - 27;
	}
	else if (skip_to_level <= 45)
	{
		gameepisode = 5;
		gamemap = skip_to_level - 36;
	}
}


static void SetNewGameSettings()
{
	if (menuactive)  // Only if coming from main menu
	switch (newgame_mode)
	{
		case SINGLEPLAYER:

			if (BotsInGame)
				Bot_RemoveBots();

			netgame = false;
			deathmatch = 0;
			nomonsters = false;
			
			if (!M_CheckParm("-dmw"))
			    Marshmallow_DeathmatchWeapons = false;
			
			CancelSandbox();

			break;

		case COOP:

			already_spawned_bots = false;

            Bot_RemoveBots();
            BotsInGame = 2;

            netgame = true;
			deathmatch = 0;
			nomonsters = false;
			
			if (!M_CheckParm("-dmw"))
			    Marshmallow_DeathmatchWeapons = false;
			
			CancelSandbox();

            Bot_SpawnBots();

			break;

		case DEATHMATCH:

			already_spawned_bots = false;

            Bot_RemoveBots();
            BotsInGame = 3;

            netgame = true;

			//if (!deathmatch)
			deathmatch = Preferred_DM_Mode;

            SetDMFlags(deathmatch);

			if (M_CheckParm("-dmm"))
				nomonsters = false;
			else
				nomonsters = true;
			
			Marshmallow_DeathmatchWeapons = true;
			
			CancelSandbox();

            Bot_SpawnBots();

			break;

		case SANDBOX:

			Bot_RemoveBots();

			deathmatch = 0;
			InitSandbox();

			break;
	}

	SetSkillUpgrades();

	if (Marshmallow_DynamicMusic)
	{
		ResetMusicTic();

		if (deathmatch)
		{
			Doom_DJ.play_mode = ALL_SONGS;
			DJ_StartPlaylist(ALL_SONGS);
		}
		else	
		{
			Doom_DJ.play_mode = DYNAMIC;
			DJ_StartPlaylist(SUSPENSEFUL);
		}
	}
}


void SkipToLevel()
{
	Bot_ResetAll();

	SetNewGameSettings();

	wipegamestate = -1;  // Why does it not wipe sometimes?

	if (skip_to_level == 0)  // 0 == RANDOM level
	{
		SkipToRandomLevel();
		return;
	}

	if (PLAYING_DOOM2)
		gamemap = skip_to_level;
	else
		SetDoom1Level();

	if (menuactive)  // Only if we're using the main menu
		G_InitNew(newskill, gameepisode, gamemap);
	else            // Change Map menu
		G_DeferedInitNew(newskill, gameepisode, gamemap);  
}


void SkipToRandomLevel()
{
	gamemap = RandomMap();
				
	if (PLAYING_DOOM1)
		gameepisode = RandomEpisode();

	wipegamestate = -1;

	if (menuactive)  // Only if we're using the main menu
		G_InitNew(newskill, gameepisode, gamemap);
	else 
		G_DeferedInitNew(newskill, gameepisode, gamemap);  
}


void ChooseLevel_Next()
{
	skip_to_level++;

	if (gamemode == commercial)
	{
		if (skip_to_level > 32) 
			skip_to_level = 0;
	}
	else
	{
		int doom_maps;

		if (Marshmallow_PlayingSigil)
			doom_maps = 45;
		else
			doom_maps = 36;

		if (skip_to_level > doom_maps)
			skip_to_level = 0;
	}
}


void ChooseLevel_Prev()
{
	skip_to_level--;

	if (gamemode == commercial)
	{
		if (skip_to_level < 0) 
			skip_to_level = 32;
	}
	else
	{
		int doom_maps;

		if (Marshmallow_PlayingSigil)
			doom_maps = 45;
		else
			doom_maps = 36;

		if (skip_to_level < 0)
			skip_to_level = doom_maps;
	}
}


void SetSkillUpgrades()
{
	switch (newskill)
	{
	case UV2_SELECTED:
		Marshmallow_InitAltUltraViolence();
		break;

	case NM2_SELECTED:
		Marshmallow_InitAltNightmare();
		break;
	
	default:
		Marshmallow_AlternateUltraViolence = false;
		Marshmallow_AlternateNightmare = false;
		Marshmallow_RespawnInNightmare = true;
		break;
	}
}


void Skill_Next()
{
	skill_selection++;
	newskill++;  
				
	if (skill_selection == MAX_SKILLMENU_ITEMS)
		skill_selection = 0;

	if (newskill == MAX_SKILLMENU_ITEMS)
		newskill = 0;

	if (newskill == 5)
		upgrade_chance = 60;
	else if (newskill == 6)
		upgrade_chance = 90;
	else
		upgrade_chance = 0;
}


void Skill_Prev()
{
	skill_selection--;
	newskill--;  
				
	if (skill_selection < 0)
		skill_selection = MAX_SKILLMENU_ITEMS-1;

	if (newskill < 0)
		newskill = MAX_SKILLMENU_ITEMS-1;

	if (newskill == 5)
		upgrade_chance = 60;
	else if (newskill == 6)
		upgrade_chance = 90;
	else
		upgrade_chance = 0;
}


void UpgradeChance_Up()
{
	upgrade_chance += 10;    

	if ( upgrade_chance > 100 )
		upgrade_chance = 0;
}


void UpgradeChance_Down()
{
	upgrade_chance -= 10;    

	if ( upgrade_chance < 0 )
		upgrade_chance = 100;
}


void HPScale_Up()
{
	MonsterHitpointsScale++;

	if ( MonsterHitpointsScale > 9 )			
		MonsterHitpointsScale = 1;
}


void HPScale_Down()
{
	MonsterHitpointsScale--;

	if ( MonsterHitpointsScale == 0 )		
		MonsterHitpointsScale = 9;
}


void NewgameMode_Up()
{
	newgame_mode++;

	if (newgame_mode > SANDBOX)
		newgame_mode = 0;
}


void NewgameMode_Down()
{
	newgame_mode--;

	if (newgame_mode < 0)
		newgame_mode = SANDBOX;
}


boolean IsWeapon(mobj_t* mo)
{
	switch (mo->sprite)
	{
	case SPR_CSAW:
		return true;
	case SPR_SHOT:
		return true;
	case SPR_SGN2:
		return true;
	case MT_CHAINGUN:
		return true;
	case SPR_LAUN:
		return true;
	case SPR_PLAS:
		return true;
	case SPR_BFUG:
		return true;
	default:
		return false;
	}
}


void CheckDuplicateLine()
{
	char* buf1;
	char* buf2;

	buf1 = MAIN_PLAYER.message;

	if (extra_textline.l)
	buf2 = extra_textline.l->l;

	if (!strcmp(buf1, buf2))
	{
		MAIN_PLAYER.message = " ";
		extra_textline_on = false;   
	}
}


void SetPlayerTarget(mobj_t* source, mobj_t* target)
{
	if (IsPlayer(source)
		&& IsMonster(target)) 
	{
		source->target = target;    
		target->target_timeout = DEFAULT_TARGET_TIMEOUT;	

		source->player->victim = target;
	}
}


void ChangeBerserkRedLength()
{
	switch (berserk_redscreen_length)
	{
	case BERSERK_REDSCREEN_VANILLA:
		berserk_redscreen_length = BERSERK_REDSCREEN_OFF;
		break;
	case BERSERK_REDSCREEN_OFF:
		berserk_redscreen_length = BERSERK_REDSCREEN_SHORT;
		break;
	case BERSERK_REDSCREEN_SHORT:
		berserk_redscreen_length = BERSERK_REDSCREEN_VANILLA;
		break;
	}
}


void SetPunchSound()
{
	if (gamemode == commercial || Marshmallow_WadStealing)
		punch_sound = sfx_skepch;
	else
		punch_sound = sfx_punch;
}


void SetWaypointIcon(boolean option_on)
{
	waypoint_icon = CANDLE; 

	if (option_on == SHOW)
	{
		main_path.show_markers = true;  
	}
	else
	{
		main_path.show_markers = false;  
	}
}


void HandleSprint()
{
	if (!too_tired_to_sprint  
		&& !sprint_timeout)  
	{
		too_tired_to_sprint = true;
		sprint_recharge = 0;

		SHOW_MESSAGE DEH_String(EXHAUSTED);
	}
	//else if (too_tired_to_sprint)
	if ( !gamekeydown[key_q] 
	     || too_tired_to_sprint)  
	{
		if (sprint_recharge < DEFAULT_SPRINT_RECHARGE)
			sprint_recharge++;

		if (sprint_timeout < DEFAULT_SPRINT_TIMEOUT)
			sprint_timeout++;
	}
	
	if (too_tired_to_sprint 
		&& sprint_recharge == DEFAULT_SPRINT_RECHARGE)
	{
		too_tired_to_sprint = false;

		sprint_timeout = DEFAULT_SPRINT_TIMEOUT;
	}
}


void ResetSprint()
{
	sprint_timeout = DEFAULT_SPRINT_TIMEOUT;
	sprint_recharge = DEFAULT_SPRINT_RECHARGE;
	too_tired_to_sprint = false;
}


void NotifyMissileLock(mobj_t* target)
{
	if ( Marshmallow_MissileAlert && 
			target == MAIN_PLAYER.mo) 
	{
		S_StartSound(NULL, sfx_tink);  

		if (!menus_on)
		{
			missilelock_on = true;
			missilelock_delay = MISSILE_LOCK_TIMEOUT;
		}
	}
}


static void DoConsoleTimeouts()
{
    if (second_consoleline_timeout)
    {
        second_consoleline_on = true;
        second_consoleline_timeout--;
    }
    else
    {
        second_consoleline_on = false;
        second_consoleline_timeout = 0;
    }

    if (third_consoleline_timeout)
    {
        third_consoleline_on = true;
        third_consoleline_timeout--;
    }
    else
    {
        third_consoleline_on = false;
        third_consoleline_timeout = 0;
    }

    if (fourth_consoleline_timeout)
    {
        fourth_consoleline_on = true;
        fourth_consoleline_timeout--;
    }
    else
    {
        fourth_consoleline_on = false;
        fourth_consoleline_timeout = 0;
    }
}


void DoTimeouts()
{
	int p;

	if (MAIN_PLAYER.victim 
		&& MAIN_PLAYER.victim->target_timeout)
		   MAIN_PLAYER.victim->target_timeout--;

    DoConsoleTimeouts();

    if (offertimeout_suicide)
		offertimeout_suicide--;
	else
		offer_suicide = false;

	if (offertimeout_radsuit)
		offertimeout_radsuit--;
	else
		offer_radsuit = false;

	if (!realnetgame
	    && !MAIN_PLAYER.mo->subsector->sector->special)
    {
        offer_radsuit = false;
        offertimeout_radsuit = 0;
        //second_consoleline_timeout = 0;
    }

	if (offertimeout_medkit)
		offertimeout_medkit--;
	else
		offer_medkit = false;

	if (mightyfist_delay)
		mightyfist_delay--;

	if (menu_key_delay)  
		menu_key_delay--;

	for (p=0; p<MAXPLAYERS; p++)
	{
		if ( !playeringame[p] )
			continue;

		if (players[p].DropObjectDelay)	
			players[p].DropObjectDelay--;
	}

	if (missilelock_delay)
		missilelock_delay--;
	
	if (!missilelock_delay
	|| MAIN_PLAYER.playerstate != PST_LIVE)
		missilelock_on = false;

	if (!usedelay)
	{
		usedelay = BOT_USE_DELAY;   
		Bots[BOT_1].waiting_for_door = false;
		Bots[BOT_2].waiting_for_door = false;
		Bots[BOT_3].waiting_for_door = false;
	}
	else
	{
		usedelay--;	
	}

	if (MAIN_PLAYER.cmd.buttons != BT_ATTACK)  // for plasma sputter
		firingdelay = 0;

	if (BotsInGame && !deathmatch)
	{
		if (gamekeydown[key_use])
		{
			usetimer++;
		}
		else
		{
			usetimer = 0;
		}
	
		if (usetimer == DEFAULT_USE_TIMER)  // for launching bot commands menu
		{
			if (help_on)
				help_on = false;

			if (botcommandmenu_on)
				botcommandmenu_on = false;
			else
				botcommandmenu_on = true;

			usetimer = 0;

			S_StartSound(NULL, sfx_tink);
		}
	}
	
	spawntics++;  // Spawntics is the same as leveltime

	HandleSprint();
}


void BFG_MegaBlast(mobj_t *actor)
{
  int i, j, n = actor->info->damage;

  fixed_t misc1 = actor->state->misc1 ? actor->state->misc1 : FRACUNIT*4;
  fixed_t misc2 = actor->state->misc2 ? actor->state->misc2 : FRACUNIT/2;

  if (!Marshmallow_BFGBlastWave || realnetgame)  // This creates too much lag in netgames
    return;

  for (i = -n; i <= n; i += 8)   
  {
    for (j = -n; j <= n; j += 8) 
    {
		mobj_t target = *actor, *mo;
		target.x += i << FRACBITS;  
		target.y += j << FRACBITS;
		target.z += P_AproxDistance(i,j) * misc1;        
		mo = P_SpawnMissile(actor, &target, MT_ARACHPLAZ);
		mo->info->seesound = NULL;  // so we don't get the same sound playing 1000 times at once
        mo->info->deathsound = NULL;  // so we don't get the same sound playing 1000 times at once
		mo->momx = FixedMul(mo->momx, misc2);
		mo->momy = FixedMul(mo->momy, misc2);           
		mo->momz = FixedMul(mo->momz, misc2);
		mo->flags &= ~MF_NOGRAVITY;
    }
  }
}


void Marshmallow_InitScaledMonster(mobj_t* monster)
{
	monster->health *= MonsterHitpointsScale;
}


mobjtype_t RandomTreasureItem(int i, int probability)  // never used these inputs
{
	int rand = GetRandomIntegerInRange(1, 100);
	
	if (rand > 98)
		return RED_SKULL;
	else if (rand > 75)
		return YELLOW_SKULL;
	else
		return BLUE_SKULL;
}


boolean Marshmallow_GiveNewInvisPowerup(mobj_t* toucher, mobj_t* special)  
{
	if (toucher->player->health >= 200)
		return false;  // leave the item
	
	if (toucher->player->health + special->remaining >= 200 
		|| toucher->player->mo->health + special->remaining >= 200)  // as-is, if we have 198 we can't pick up the demonsphere because it would take us over 200
	{
		toucher->player->health = 200;
		return true;  // remove the item
	}

	toucher->player->message = DEH_String(DEMONSPHEREHEALS);  
	S_StartSound (NULL, sfx_getpow);
	toucher->player->bonuscount += BONUSADD; 
	toucher->player->health += special->remaining;
	toucher->health += special->remaining;

	return true;
}


void Marshmallow_SetupLevel()
{	
	SetMusicThresholds();

	if (Marshmallow_KeepKeys)
		ClearKeyRing(&MAIN_PLAYER); 

	treasure_bag.score = NULL;

	if (MAIN_PLAYER.playerstate == PST_LIVE)
	{
        players[consoleplayer].attacker = NULL;
        players[consoleplayer].mo->target = NULL;
    }

	if ( MAIN_PLAYER.cheats & CF_GODMODE 
		|| MAIN_PLAYER.cheats & CF_NOCLIP )
	{
		DisableStats();  // cheats are still on; disable stats
	}
	else
	{
		ResetStats();   // then we aren't cheating; allow stats
	}

	memset(&level_stats, NULL, sizeof(player_stats_t));  // perhaps move this into ResetStats()

	newskill = gameskill;  

	Bot_ResetOrders();

	flashlight_on = false;

	marshmallow_tic = NULL;
}


boolean PlayerIsDead()
{
	player_t player = players[consoleplayer];

	if (player.playerstate != PST_LIVE
		|| !player.health
		|| !player.mo->health)
		return true;
	else
		return false;
}


void ToggleFriendlyFire()
{
	if (Marshmallow_FriendlyFire)
	{
		Marshmallow_FriendlyFire = false;
		SHOW_MESSAGE DEH_String(FFOFF);
	}
	else
	{
		Marshmallow_FriendlyFire = true;
		SHOW_MESSAGE DEH_String(FFON);
	}
}


static void SendNetgameSignal(byte signal)
{
	HU_queueChatChar(signal);   
}


static void ClearNetgameSignal(int player)
{
	if (netgamesignal)
	{
		if (players[player].cmd.chatchar != players[consoleplayer].cmd.chatchar)
			return;   // only clear our own signal

		players[player].cmd.chatchar = 0; 
		netgamesignal = 0;
	}
}


byte GetNetgameSignal(int player)
{
	//ClearNetgameSignal(player);

	return players[player].cmd.chatchar;   
}


void Marshmallow_SendMultiplayerEvent(int event) 
{
//	int i;

	//for (i=0; i<MAXPLAYERS; i++)
	//{
	//    if (!playeringame[i]) 
	//	continue;

		switch (event)
		{
		case MARSHMALLOW_SOMEONE_ABORTED_MAP:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_TOQUIT);  
			break;
				 
		case MARSHMALLOW_SOMEONE_KILLED_THEMSELF:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_TOKILL);
			break;
	 
		case MARSHMALLOW_BACKPACK_WAS_DROPPED:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_TO_DROPGIFT);
			break;

		case MARSHMALLOW_OBJECT_WAS_DROPPED:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_TO_DROPMONSTER);
			break;

		case PLAYER0_NEXTOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER0_NEXTOBJECT);
			break;

		case PLAYER0_PREVOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER0_PREVOBJECT);
			break;

		case PLAYER1_NEXTOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER1_NEXTOBJECT);
			break;

		case PLAYER1_PREVOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER1_PREVOBJECT);
			break;

		case PLAYER2_NEXTOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER2_NEXTOBJECT);
			break;

		case PLAYER2_PREVOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER2_PREVOBJECT);
			break;

		case PLAYER3_NEXTOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER3_NEXTOBJECT);
			break;

		case PLAYER3_PREVOBJ:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_PLAYER3_PREVOBJECT);
			break;

		case START_BATTLE:
			SendNetgameSignal(MARSHMALLOW_CHATCHAR_TO_STARTBATTLE);
			break;

		default:
			return;  // was break
		}

		//netgamesignal = true;  // testing
	//}
}


boolean Marshmallow_CheckForMultiplayerEvent()		
{
	playerindex_t player;
	byte		  signal;
	mobj_t*		  actor;

	for (player=0; player<MAXPLAYERS; player++)
	{		
		actor = players[player].mo;  
		signal = GetNetgameSignal(player);
		
		switch (signal)
		{
		case MARSHMALLOW_CHATCHAR_TOQUIT:   
			G_ExitLevel();
			return true;

		case MARSHMALLOW_CHATCHAR_TOKILL:
			PlayerKillsHimself( actor );  
			return true;

		case MARSHMALLOW_CHATCHAR_TO_DROPGIFT:
			DropBackpackForAFriend( actor );
			return true;

		case MARSHMALLOW_CHATCHAR_TO_DROPMONSTER:
			PlaceMonster( actor );
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER0_NEXTOBJECT:
			ChangeThingType(0, FORWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER0_PREVOBJECT:
			ChangeThingType(0, BACKWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER1_NEXTOBJECT:
			ChangeThingType(1, FORWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER1_PREVOBJECT:
			ChangeThingType(1, BACKWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER2_NEXTOBJECT:
			ChangeThingType(2, FORWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER2_PREVOBJECT:
			ChangeThingType(2, BACKWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER3_NEXTOBJECT:
			ChangeThingType(3, FORWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_PLAYER3_PREVOBJECT:
			ChangeThingType(3, BACKWARD);
			return true;

		case MARSHMALLOW_CHATCHAR_TO_STARTBATTLE:
			UnleashTheHordes();
			return true;

		default:
			continue;
		}
	}

	return false;
}


static void GameplayKeyInput()
{
    if (gamekeydown[key_r] && offertimeout_suicide)
    {
        PlayerKillsHimself(players[consoleplayer].mo);
        offertimeout_suicide = 0;
        SetKeyDelay();
        return;
    }

	if (gamekeydown[key_g] && Marshmallow_GiftDropping)  // Press 'G' to gift some ammo to a friend in realnetgame
	{
	    return; // 11-29-2020: disabled while we settle the backpack problems

		if (realnetgame)
			Marshmallow_SendMultiplayerEvent(MARSHMALLOW_BACKPACK_WAS_DROPPED);
	}

	if (gamekeydown[key_use] && viewingbot && !deathmatch)   // Press use when facing a bot to issue orders
	{	
		if (!usetimer)
			Bot_CycleOrders();
	}

	if (gamekeydown[key_f])  // Press 'F' to order squad to regroup to player's position
	{
		if (CheckKeyDelay())
			return;

		if (BotsInGame && !deathmatch)
		{
			if (botcommandmenu_on)
			{
				botcommandmenu_on = false;
				SquadRegroup();  

				return;
			}

			if (F_Key_BotFollow)
				SquadRegroup();  
		}
		/*
		if (botcommandmenu_on)
			botcommandmenu_on = false;

		if (BotsInGame && !deathmatch && F_Key_BotFollow)
			SquadRegroup();  */
	}

	if (gamekeydown[key_p])
	{
		if (botcommandmenu_on)
			botcommandmenu_on = false;
		else
			return;

		if (BotsInGame && !deathmatch)
		{
			bot_t bot;

			for (bot=BOT_1; bot<=BotsInGame; bot++)
			{
				Bot_StartPatrolling(bot);
			}

			SHOW_MESSAGE DEH_String(ORDERMSGPATROL);
            PlayRadioNoise();
		}

		SetKeyDelay();
	}

	if (gamekeydown[key_h])
	{
		if (botcommandmenu_on)
			botcommandmenu_on = false;
		else
			return;

		if (realnetgame)
		{
			//SHOW_MESSAGE "NOT AVAILABLE IN NETGAME.";  
			return;
		}

		if (BotsInGame && !deathmatch)
		{
			Bot_HoldPosition();
			SHOW_MESSAGE DEH_String(ORDERMSGHOLD);
            PlayRadioNoise();
		}
	}


	if (gamekeydown[key_t])
	{
		if (CheckKeyDelay())
			return;

		if (deathmatch)
			return;

		if (!Marshmallow_BotEnabled)
			return;

		if (viewingbot)
		{
			// NOTE: Single-bot target assignment is disabled for now
			//Bot_AssignTarget(viewingbot);
		}
		else
		{
			if ( linetarget )
			{
				Bot_AssignTargetToAll();
			}
		}
	}
	else  // so we don't trigger both

	if (gamekeydown[key_t] && Bots[lastbot].orders == FOLLOW_CURSOR)     // Press 'T' to assign a target to a bot
	{
		Bot_AssignSingleBotTarget();
	}

	if (gamekeydown[key_use] && !realnetgame)  // *** NEW - move this elsewhere later
	{
		// TODO:  use node timeout check here

		if ( specialnodes )
		Path_DropNode(&main_path, MAIN_PLAYER.mo);
	}

	if ( (gamekeydown[key_useitem] || gamekeydown[key_insert])
		&& sandbox.design_mode && !menus_on)
	{
		if (CheckKeyDelay())
			return;

		if (Marshmallow_Sandbox)
		{
			if (!realnetgame)
			{
				PlaceMonster(MAIN_PLAYER.mo);		
				
			}
			else
			{
				Marshmallow_SendMultiplayerEvent(MARSHMALLOW_OBJECT_WAS_DROPPED);  
				//SHOW_MESSAGE "SENDMPEVENT";
			}
		}
	}

	if ( (gamekeydown[key_previtem] || gamekeydown[key_pgdn])
			&& Marshmallow_Sandbox && !menus_on)  
	{
		if (CheckKeyDelay())
			return;
		
		if (!realnetgame)
			ChangeThingType(consoleplayer,BACKWARD);
		else
		{
			// TODO: switch
			if (consoleplayer == 0)  
				Marshmallow_SendMultiplayerEvent(PLAYER0_PREVOBJ);
			if (consoleplayer == 1)
				Marshmallow_SendMultiplayerEvent(PLAYER1_PREVOBJ);
			if (consoleplayer == 2)
				Marshmallow_SendMultiplayerEvent(PLAYER2_PREVOBJ);
			if (consoleplayer == 3)
				Marshmallow_SendMultiplayerEvent(PLAYER3_PREVOBJ);
		}
	}

	if ( (gamekeydown[key_nextitem] || gamekeydown[key_pgup])
		&& Marshmallow_Sandbox && !menus_on)		
	{
		if (CheckKeyDelay())
			return;

		if (!realnetgame)
			ChangeThingType(consoleplayer, FORWARD);
		else
		{
			// TODO: switch
			if (consoleplayer == 0)
				Marshmallow_SendMultiplayerEvent(PLAYER0_NEXTOBJ);
			if (consoleplayer == 1)
				Marshmallow_SendMultiplayerEvent(PLAYER1_NEXTOBJ);
			if (consoleplayer == 2)
				Marshmallow_SendMultiplayerEvent(PLAYER2_NEXTOBJ);
			if (consoleplayer == 3)
				Marshmallow_SendMultiplayerEvent(PLAYER3_NEXTOBJ);
		}
	}

	if (gamekeydown[key_b] && sandbox.design_mode)
	{
		if (CheckKeyDelay())
			return;

		if (consoleplayer != 0)
		{
			SHOW_MESSAGE DEH_String(ONLYHOST);
			return;
		}
		
		if (sandbox.count <= 0)   // so Keens and Barrels don't count in sandbox stats - TODO: maybe move this into UnleashTheHordes() ?
			SHOW_MESSAGE DEH_String(NOMONSTERS);
		else
		{
			if (!realnetgame)
				UnleashTheHordes();
			else
				Marshmallow_SendMultiplayerEvent(START_BATTLE);
		}
	}

	if (gamekeydown[key_quake2bfg])
	{
		if (realnetgame)
			return;

		if (MAIN_PLAYER.weaponowned[wp_bfg] && MAIN_PLAYER.readyweapon != wp_bfg)
			MAIN_PLAYER.pendingweapon = wp_bfg;
	}

	if (gamekeydown[key_mightyfist])
	{
		if ( realnetgame )
			return;

		MightyFistEngaged();
	}
}


void Marshmallow_Controls()
{
	GameplayKeyInput();
	HUDMenuKeyInput();
}


void LaunchHelpWidget()
{
	if (botcommandmenu_on)
		return;

	if (realnetgame) 
		return;

    PlayMenuSound(sfx_tink);
		
	if (menus_on)
		HideAllMenus();

	if (pkereadout_on)
		pkereadout_on = false;
	
	if (help_on)
		help_on = false;
	else
		help_on = true;
}


void PlayBonusSound(blipsound_t length)
{
	// If Keen sounds aren't available, just play sfx_getpow
	if ( GetGameType() == DOOM1 ) 
	{
		S_StartSound( NULL, sfx_getpow );
		return;
	}

	// Otherwise, play the Keen sounds
	if ( length == SHORTBLIP )
		S_StartSound( NULL, sfx_keenpn );
	
	if ( length == LONGBLIP )
		S_StartSound( NULL, sfx_keendt );
}


void LowAmmoWarning()
{
	weapontype_t weapon;

	weapon = MAIN_PLAYER.readyweapon;
	
	if (!Marshmallow_LowAmmoWarning)
		return;

	switch (weapon)
	{
		case wp_pistol:
		case wp_chaingun:
			if (MAIN_PLAYER.ammo[am_clip] < clipammo[am_clip])
				SHOW_MESSAGE DEH_String(LOWAMMO);
		break;
	
		case wp_shotgun:
		case wp_supershotgun:
			if (MAIN_PLAYER.ammo[am_shell] < clipammo[am_shell])
				SHOW_MESSAGE DEH_String(LOWAMMO);
		break;

		case wp_missile:
			if (MAIN_PLAYER.ammo[am_misl] < clipammo[am_misl])
				SHOW_MESSAGE DEH_String(LOWAMMO);
		break;

		case wp_plasma:
		case wp_bfg:
			if (MAIN_PLAYER.ammo[am_cell] < clipammo[am_cell])
				SHOW_MESSAGE DEH_String(LOWAMMO);
		break;
	}
}


boolean IsPlayer(mobj_t* actor)
{
	if (!actor)
		return false;

	if (actor->type == MT_PLAYER)
		return true;
	else
		return false;
}


boolean IsConsoleplayer(mobj_t* actor)
{
	if (!actor || !actor->player)
		return false;

	if (actor->player->player_number == consoleplayer)
		return true;
	else
		return false;
}


boolean IsBarrel(mobj_t* actor)  
{
	if (!actor)
		return false;

	if (actor->type == MT_BARREL)
		return true;
	else
		return false;
}


boolean IsMonster(mobj_t* actor)
{
	if (!actor)
		return false;

	switch (actor->type)
	{
	case MT_POSSESSED:
		return true;

    case MT_SHOTGUY:
		return true;

    case MT_VILE:
		return true;
  
    case MT_UNDEAD:
		return true;

    case MT_FATSO:
		return true;

    case MT_CHAINGUY:
		return true;

    case MT_TROOP:
		return true;

    case MT_SERGEANT:
		return true;

    case MT_SHADOWS:
		return true;

    case MT_HEAD:
		return true;

    case MT_BRUISER:
		return true;

    case MT_KNIGHT:
		return true;

    case MT_SKULL:
		return true;

    case MT_SPIDER:
		return true;

    case MT_BABY:
		return true;

    case MT_CYBORG:
		return true;

    case MT_PAIN:
		return true;

    case MT_WOLFSS:
		return true;

	default:
		return false;
	}
}


void AnnounceMostDangerousMonsters(mobj_t* actor)
{
	if (!Marshmallow_BossAlert)
	    return;

	switch (actor->type)
	{
        case MT_VILE:
            WriteToThirdConsoleLine(VILESPOT, SHORT_EXTRALINE_TIMEOUT);
            //actor->target->player->message = DEH_String(VILESPOT);
            break;

        case MT_CYBORG:
            WriteToThirdConsoleLine(CYBERSPOT, SHORT_EXTRALINE_TIMEOUT);
            //actor->target->player->message = DEH_String(CYBERSPOT);
            break;

        case MT_SPIDER:
            WriteToThirdConsoleLine(SPIDERSPOT, SHORT_EXTRALINE_TIMEOUT);
            //actor->target->player->message = DEH_String(SPIDERSPOT);
            break;
	}
}


void AnnounceMostDangerousMonstersDeath(mobj_t* actor)
{
	if (!Marshmallow_BossAlert) 
	    return;

	switch (actor->type)
	{
        case MT_VILE:
            //WriteToThirdConsoleLine(VILEDEAD, SHORT_EXTRALINE_TIMEOUT);
            SHOW_MESSAGE DEH_String(VILEDEAD);
            break;

        case MT_CYBORG:
            //WriteToThirdConsoleLine(CYBERDEAD, SHORT_EXTRALINE_TIMEOUT);
            SHOW_MESSAGE DEH_String(CYBERDEAD);
            break;

        case MT_SPIDER:
            //WriteToThirdConsoleLine(SPIDERDEAD, SHORT_EXTRALINE_TIMEOUT);
            SHOW_MESSAGE DEH_String(SPIDERDEAD);
            break;
	}
}


void PushBarrel()
{
	unsigned	ang;
	fixed_t		thrust =  BARREL_PUSH_THRUST;

	int p;

	if (!Marshmallow_BarrelPushing)
		return;

	for (p=0; p<MAXPLAYERS; p++)
	{
		if (!players[p].touching_barrel
			/*|| realnetgame*/)   
			continue;

		ang = players[p].mo->angle;

		ang >>= ANGLETOFINESHIFT;
		players[p].current_barrel->momx += FixedMul (thrust, finecosine[ang]);
		players[p].current_barrel->momy += FixedMul (thrust, finesine[ang]);
	}
}


void ResetBarrel()  
{
	int p;

	for (p=0; p < MAXPLAYERS; p++)
	{
		if (players[p].touching_barrel)  
		{
			if (!players[p].touching_barrel)
				return;

			if (!P_CheckMeleeRange(players[p].current_barrel))
			{
				players[p].touching_barrel = false;
				players[p].current_barrel = NULL;
			}
		}
	}
}


void MightyFistEngaged()   
{
	int		angle;
	int		damage;
	int		slope;
	
	mobj_t* actor;
	actor = MAIN_PLAYER.mo;

	if ( realnetgame || MAIN_PLAYER.playerstate != PST_LIVE)
	return;

	if (mightyfist_delay)
		return;
	else
		mightyfist_delay = 20;
		
	angle = actor->angle;
	slope = P_AimLineAttack (actor, angle, MELEERANGE);

	S_StartSound (actor, sfx_punch); // lo-fi sounding punch sound in Doom1
																				
	angle += P_SubRandom() << 20;
	damage = (P_Random ()%10+1)<<1 /** BOOSTED_FIST_DAMAGE*/;   // fixed damage for this punch

    if (MAIN_PLAYER.powers[pw_strength])	// apply berserk if we have it
	damage *= 10;

	P_LineAttack (actor, angle, MELEERANGE, slope, damage);

    // NOTE: not currently using player attack state animation for this melee attack

	if (debugmode)
	    MAIN_PLAYER.message = DEH_String(FOOTENG);

	P_NoiseAlert (actor, actor);  // To remain consistent with how regular fist works
}


void AutoUse() 
{
	if (!Marshmallow_AutoUse)
		return;

    if (gamekeydown[key_use])
        AutoUseDelay = DEFAULT_AUTO_USE_DELAY;   // Reset autouse timer so it's not re-triggering doors we already opened manually

	if ( realnetgame )
	return;

	if (!AutoUseDelay)
	{
		P_UseLines(&players[consoleplayer]);
		AutoUseDelay = DEFAULT_AUTO_USE_DELAY;
	}
	else
		AutoUseDelay--;
}


// Players drop their readyweapon at time of death
void DropWeaponOnPlayerDeath(mobj_t* target)
{
	int weapon_x, weapon_y, weapon_z;
	unsigned an;

	mobjtype_t player_weapon;
	mobj_t*	mo;

	bot_t bot;

    // Bot weapon drop
	if ( bot = IsBot(target->player) )
	{
		switch (this_Bot.weapon)
		{
		case BOT_SHOTGUN:
			player_weapon = MT_SHOTGUN;
			break;

		case BOT_SUPERSHOTGUN:
			player_weapon = MT_SUPERSHOTGUN;
			break;

		case BOT_CHAINGUN:
			player_weapon = MT_CHAINGUN;
			break;

		case BOT_MISSILE:
			player_weapon = MT_MISC27;
			break;

		case BOT_PLASMA:
			player_weapon = MT_MISC28;
			break;

		case BOT_GREENPLASMA:
			player_weapon = MT_MISC28;
			break; 

		case BOT_BFG:
			player_weapon = MT_MISC25;
			break;
		
		default:
			return;
		}
	}
	else  // Player weapon drop
	{
 		switch (target->player->readyweapon)
		{
			case wp_chainsaw:
				player_weapon = MT_MISC26;
				break;
			case wp_shotgun:
				player_weapon = MT_SHOTGUN;
				break;
			case wp_supershotgun:
				player_weapon = MT_SUPERSHOTGUN;
				break;
			case wp_chaingun:
				player_weapon = MT_CHAINGUN;
				break;
			case wp_missile:
				player_weapon = MT_MISC27;
				break;
			case wp_plasma:
				player_weapon = MT_MISC28;
				break;
			case wp_bfg:
				player_weapon = MT_MISC25;
				break;
			default:
				return;
		}
	}

	an = target->angle >> ANGLETOFINESHIFT;

	weapon_x = target->x + FixedMul (24*FRACUNIT, finecosine[an]) +  MARSHMALLOW_ITEMDROP_OFFSET;  
	weapon_y = target->y + FixedMul (24*FRACUNIT, finesine[an]) + MARSHMALLOW_ITEMDROP_OFFSET;
	weapon_z = target->z + DROP_FROM_ABOVE_FLOOR;

	mo = P_SpawnMobj (weapon_x,weapon_y,weapon_z, player_weapon);
	mo->flags |= MF_DROPPED;	

	if ( deathmatch )
		mo->drop_tic = gametic;
}


#define GOODIE_CHANCE 13  // rare enough?

void Marshmallow_NewDropItemsRoutine(mobj_t* target)
{
	int x, y, z;
	unsigned an;

	mobjtype_t item;
//	mobjtype_t player_weapon;
	mobj_t*	mo;

	item = MT_HEALTH_BONUS;

	z = ONFLOORZ;
	
	switch (target->type)
	{
		case MT_WOLFSS:
		case MT_POSSESSED:
			item = MT_CLIP;  
			break;
	
		case MT_SHOTGUY:
			item = MT_SHOTGUN;  
			break;
	
		case MT_CHAINGUY:
			if ( PercentChance( GOODIE_CHANCE ) ) 
				item = MT_BOX_OF_BULLETS;
			else
				item = MT_CHAINGUN;  
			break;

		case MT_TROOP: // Imps
		case MT_SERGEANT: // Pinkys
		case MT_SHADOWS: // Spectres
		case MT_SKULL: // Lost Souls
			if ( PercentChance( 75 ) )
				item = MT_HEALTH_BONUS; 
			else
				item = MT_ARMOR_BONUS;
			break;

		case MT_HEAD:  // Cacodemons
			if ( PercentChance( GOODIE_CHANCE ) )
				item = GenerateRandomBonusItem(); 
			else
				item = MT_HEALTH_BONUS;
			break;

		case MT_FATSO:
		case MT_BABY: 
			if ( PercentChance( GOODIE_CHANCE ) )
				item = MT_BOX_OF_CELLS; 
			else
				item = MT_CELL; 
			break;

		case MT_UNDEAD: // Revenant
			if ( PercentChance( 50 ) )
				item = MT_ROCKETAMMO; 
			else
				item = MT_ARMOR_BONUS;
			break;

		case MT_BRUISER: // Baron of Hell
			if ( PercentChance( GOODIE_CHANCE ) ) 
				item = GenerateRandomBonusItem();
			else
				item = MT_DEMONSPHERE;
			break;

		case MT_KNIGHT: 
			if ( PercentChance( GOODIE_CHANCE ) ) 
				item = GenerateRandomBonusItem();
			else
				item = MT_DEMONSPHERE;
			break;

		case MT_VILE:
			if ( PercentChance( GOODIE_CHANCE ) ) 
				item = GenerateRandomBonusItem();
			else
				item = MT_DEMONSPHERE;
			break;

		case MT_PAIN:
			if ( PercentChance( GOODIE_CHANCE ) ) 
				item = GenerateRandomBonusItem();
			else
				item = MT_DEMONSPHERE;
			break;

		case MT_CYBORG:
		case MT_SPIDER:
			if ( gamemode == commercial)
				item = MT_MEGA;		  // Drop a Megasphere in Doom II
			else
				item = MT_SOULSPHERE; // Drop a Soulsphere in Doom
			break;

		case MT_PLAYER:  
			if (Marshmallow_DropBackpack)
			{
				if ( !deathmatch )
				{
					DropInventoryInBackpack(target);  
				}
				else
				{
					CreateBackpack(target, false);  
					DropWeaponOnPlayerDeath(target);
				}
			}
			return;

	    default:
		  return;
	}
	
	an = target->angle >> ANGLETOFINESHIFT;

	x = target->x;   // If monsters are dropping the item, spawn it exactly where they died.
	y = target->y;
	z = target->z + DROP_FROM_ABOVE_FLOOR;

	mo = P_SpawnMobj (x,y,z, item);
	mo->flags |= MF_DROPPED;

	mo->flags |= MF_COUNTITEM;  // so our dropped items don't count towards intermission stats

	if (item == MT_INS 
		/*&& mo->flags &= MF_DROPPED*/)
	{
		mo->flags ^= MF_SHADOW;	 // in Goodies Mode, invisbility powerups are set with MF_SHADOW by default
		mo->remaining = target->info->spawnhealth * DEMONSPHERE_HEALTH_PERCENTAGE;
		//mo->flags ^= MF_SPECIAL;	
	}

}


void PlayerKillsHimself(mobj_t* actor)
{
	if (actor->player->mo->health > 0)
	{
		actor->player->mo->health = 0;  // so we don't pick up items while dying

		P_KillMobj(NULL, actor->player->mo);	

		//if (!crispy->singleplayer)
		    actor->player->message = DEH_String(YOUSEPPUKU);

		HideAllMenus();
	}
}


static boolean Marshmallow_CheckForFF(mobj_t* source, mobj_t* target)
{
	if (!source)
	    return false;

	if ((source->x != target->x)
		&& (source->y != target->y)
		&& (IsPlayer(source) 
		&& (IsPlayer(target))))	
			return true;
	else
		return false;
}


static void MirrorDamagePlayer(mobj_t* source, int damage)
{
	source->player->message = DEH_String(MIRRORON);
	P_DamageMobj(source, NULL, NULL, damage);  
	A_Recoil (source->player);

	if (source->player->health <= 0)
		source->player->message = DEH_String(STOPFF); 
}


static void CalcMirrorDamage(mobj_t* source)
{
	switch (source->player->readyweapon)
	{
	case wp_pistol:
	case wp_chaingun:
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_BULLETS);
		break;

	case wp_shotgun:
	case wp_supershotgun:			
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_SHOTGUN);
		break;

	case wp_missile:
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_MISSILES);
		break;

	case wp_plasma:
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_PLASMA);
		break;

	case wp_bfg:
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_BFG);
		break;

	case wp_chainsaw:
	case wp_fist:
		MirrorDamagePlayer(source, MARSHMALLOW_FF_PENALTY_MELEE);
		break;
	}
}


void Marshmallow_DoMirrorDamage(mobj_t* source, mobj_t* target)  
{
	if (!source)
		return;

	if (Marshmallow_MirrorDamage && Marshmallow_FriendlyFire) 
	{
		if ((source->x != target->x) 
			&& (source->y != target->y)
			&& (IsPlayer(source) && (IsPlayer(target))))
		{
			CalcMirrorDamage(source);
		}
	}

	if (Marshmallow_MirrorDamage && !Marshmallow_FriendlyFire)
	{
		if ((source->x != target->x) 
			&& (source->y != target->y)
			&& (IsPlayer(source) 
			&& (IsPlayer(target))))	
		{
			CalcMirrorDamage(source);
		}
	}
}


static boolean CheckForSelfDamage(mobj_t* source, mobj_t* target)
{
	if (!source) // If attacker is environment we're outta here
		return false;

	if (source->x == target->x 
		&& source->y == target->y
		&& IsPlayer(source) 
		&& IsPlayer(target))	
	{
		return true;
	}
	else 
	{
		return false;
	}
}


boolean CheckPlayerDamage(mobj_t* source, mobj_t* target) 
{
	if (!source) 
		return true;

	if ( IsBot(source->player) && IsBot(target->player) 
		&& source->player->bot_number == target->player->bot_number )   
	{
		//if (!deathmatch)
			return false;  // No bot self-damage in coop; otherwise they blow themselves up too much
		//else
		//	return true;  // Bots can hurt themselves in DM
	}

	if (deathmatch)
	{
		if (Marshmallow_CheckForFF(source, target))
			return true;	
	}

	if (Marshmallow_SelfDamage && Marshmallow_FriendlyFire)
	{
		if (CheckForSelfDamage(source, target))
			return true;

		if (Marshmallow_CheckForFF(source, target))
			return true;
	}
	
	if (!Marshmallow_SelfDamage && Marshmallow_FriendlyFire)
	{
		if (CheckForSelfDamage(source, target))
			return false;
		
		if (Marshmallow_CheckForFF(source, target))
			return true;
	}

	if (!Marshmallow_SelfDamage && !Marshmallow_FriendlyFire)
	{
		if (CheckForSelfDamage(source, target))
			return false;
		
		if (Marshmallow_CheckForFF(source, target))
			return false;
	}

	if (Marshmallow_SelfDamage && !Marshmallow_FriendlyFire)
	{
		if (CheckForSelfDamage(source, target))
			return true;

		if (Marshmallow_CheckForFF(source, target))
			return false;
	}

	// If none of the above applies then we are doing the damage
	return true; 
}


static void CheckWhatMonsterKilledYou(mobj_t* source, mobj_t* target)
{
	if (!source)
		return;

	switch (source->type)
	{
		case MT_WOLFSS:
			target->player->message = DEH_String(NAZIKILL);
			break;

		case MT_POSSESSED:
			target->player->message = DEH_String(ZOMKILL);
			break;
	
		case MT_SHOTGUY:
			target->player->message = DEH_String(SGKILL);
			break;
	
		case MT_CHAINGUY:
			target->player->message = DEH_String(CGKILL);
			break;

		case MT_TROOP: // Imps
			target->player->message = DEH_String(IMPKILL);
			break;

		case MT_SERGEANT: // Pinkys
			target->player->message = DEH_String(DEMONKILL);
			break;

		case MT_SHADOWS: // Spectres
			target->player->message = DEH_String(SPKILL);
			break;

		case MT_FATSO:
			target->player->message = DEH_String(FATKILL);
			break;

		case MT_VILE:
			target->player->message = DEH_String(VILEKILL);
			break;

		case MT_CYBORG:
			target->player->message = DEH_String(CYBERKILL);
			break;

		case MT_SPIDER:
			target->player->message = DEH_String(BOSSKILL);
			break;

		case MT_UNDEAD: // Revenant
			target->player->message = DEH_String(SKELKILL);
			break;

		case MT_BRUISER: // Baron of Hell
			target->player->message = DEH_String(BARONKILL);
			break;

		case MT_KNIGHT: 
			target->player->message = DEH_String(KNIGHTKILL);
			break;

		case MT_BABY: //Arachnotron
			target->player->message = DEH_String(BABYKILL);
			break;

	    default:
		  return;
	}
}


static void CheckWhatMonsterYouKilled(mobj_t* source, mobj_t* target)
{
	switch (target->type)
	{
		case MT_WOLFSS:
			source->player->message = DEH_String(KILLEDNAZI);
			break;

		case MT_POSSESSED:
			source->player->message = DEH_String(KILLEDPOS);
			break;
	
		case MT_SHOTGUY:
			source->player->message = DEH_String(KILLEDSARG);
			break;
	
		case MT_CHAINGUY:
			source->player->message = DEH_String(KILLEDCGUY);
			break;

		case MT_TROOP: // Imps
			source->player->message = DEH_String(KILLEDIMP);
			break;

		case MT_SERGEANT: // Pinkys
			source->player->message = DEH_String(KILLEDDEMON);
			break;

		case MT_SHADOWS: // Spectres
			source->player->message = DEH_String(KILLEDSPEC);
			break;

		case MT_FATSO:
			source->player->message = DEH_String(KILLEDFATTY);
			break;

		case MT_VILE:
			source->player->message = DEH_String(KILLEDVILE);
			break;

		case MT_CYBORG:
			source->player->message = DEH_String(KILLEDCYBER);
			break;

		case MT_SPIDER:
			source->player->message = DEH_String(KILLEDBOSS);
			break;

		case MT_UNDEAD: // Revenant
			source->player->message = DEH_String(KILLEDSKEL);
			break;

		case MT_BRUISER: // Baron of Hell
			source->player->message = DEH_String(KILLEDBARON);
			break;

		case MT_KNIGHT: 
			source->player->message = DEH_String(KILLEDKNIGHT);
			break;

		case MT_BABY: //Arachnotron
			source->player->message = DEH_String(KILLEDBABY);
			break;

	    default:
		  return;
	}
}


static void SendMessageToAllPlayers(player_t* players, char* msgToSend)  
{
	unsigned int i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (bot_in_game[i])
			continue;

		players[i].message = msgToSend;
	}
}


void AnnounceWhoKilledWhat(mobj_t* source, mobj_t* target, player_t* players)
{
	if ( !source  // Environment damage 
		&& IsMonster(target)
		&& Marshmallow_ExtendedMessages) 
	{
		SendMessageToAllPlayers(players, DEH_String(MONSTERDIED) );  
		return;
	}

	if ( IsPlayer(source) 
		&& IsMonster(target)
		&& Marshmallow_ExtendedMessages)
	{
		CheckWhatMonsterYouKilled(source, target);
		return;
	}
	
	if ( IsPlayer(source) 
		&& IsPlayer(target)  
		&& CheckForSelfDamage(source,target))
	{
		SendMessageToAllPlayers(players, DEH_String(SOMEONEBLEW));
		target->player->message = DEH_String(YOUBLEW); 
		return;
	}

	if ( IsPlayer(target)  // Monster killed you
		&& IsMonster(source)
		&& Marshmallow_DeathMessages)
	{
		CheckWhatMonsterKilledYou(source, target);
		return;
	}

	if ( !source  // Environment damage 
		&& IsPlayer(target)
		&& !IsBot(target->player)
		&& Marshmallow_DeathMessages)   
	{
	    if (offertimeout_suicide)
	        return;

		SendMessageToAllPlayers(players, DEH_String(ENVKILL));
		target->player->message = DEH_String(KILLENV);

		return;
	}

	if ( IsPlayer(source) 
		&& IsPlayer(target)  
		&& Marshmallow_CheckForFF(source,target))
	{
		target->player->message = DEH_String(KILLBYPLAYER);
		source->player->message = DEH_String(KILLPLAYER);

		return;
	}
}


void RestoreKeyRing(int player)
{
	card_t i;

	for (i=0; i<NUMCARDS; i++) 
		if (Marshmallow_KeyRing[player][i] == true)  
			players[player].cards[i] = true;
}


void SaveArsenal(int player)
{
	weapontype_t i;

	for (i=0; i<NUMWEAPONS; i++) 
		if (players[player].weaponowned[i] == true)
			Marshmallow_Arsenal[player][i] = true;
}


void SaveKeys(int player)
{
	card_t i;

	for (i=0; i<NUMCARDS; i++) 
		if (players[player].cards[i] == true)
			Marshmallow_KeyRing[player][i] = true;
}


void ClearKeyRing(player_t* plyr)  
{
	memset (Marshmallow_KeyRing, 0, sizeof (Marshmallow_KeyRing)); 
	memset (plyr->cards, 0, sizeof (plyr->cards)); 
}


void GiveAllItems()
{
	MAIN_PLAYER.extra_powers[ITEM_INVUL] = true;
	MAIN_PLAYER.extra_powers[ITEM_INVIS] = true;
	MAIN_PLAYER.extra_powers[ITEM_RADSUIT] = true;
	MAIN_PLAYER.extra_powers[ITEM_MEDKIT] = true;
	MAIN_PLAYER.extra_powers[ITEM_VISOR] = true;
	MAIN_PLAYER.extra_powers[ITEM_AUTOMAP] = true;

	MAIN_PLAYER.medkit_remaining = 100;
}


boolean PhysicsExempt(mobj_t* thing)
{
    switch (thing->type)
    {
        case MT_CYBORG:
        case MT_SPIDER:
        case MT_KNIGHT:
        case MT_BRUISER:
        case MT_BABY:
            return true;

        default:
            return false;
    }
}


boolean EnemyInRange(mobj_t* enemy)
{
    fixed_t distance;
    fixed_t range_limit = PKE_Meter.search_radius;

    mobj_t* player = players[consoleplayer].mo;

    if (!enemy)
        return false;

    distance = P_AproxDistance (player->x - enemy->x,
                            player->y - enemy->y);

    if (distance < range_limit)
    {
        //SHOW_MESSAGE "ENEMY IN RANGE!";
        return true;
    }
    else
    {
        //SHOW_MESSAGE "ENEMY OUT OF RANGE!";
        return false;
    }
}


///////////////////
//
//	Randomization  
//
///////////////////

extern int imix(int,int,float);
extern float fmix(float, float, float);

#define MAX_MAPS				1024

int shuffledMaps[MAX_MAPS];
int shuffledMapIndex = 0;
int numShuffledMaps = 0;
int isShuffledMapsInitialized = false;

extern time_t time(time_t * _Time);  // added 1-21-20 due to compiler warning
extern struct tm* localtime(const time_t * _Time);  // added 1-21-20 due to compiler warning

void SeedRandom()	
{
	time_t curtime = time(NULL);
    struct tm *curtm = localtime(&curtime);

	marshmallow_rndindex = (int)curtime % 256;  
}


void RefreshShuffledMaps()
{
	int j, k;
	int mapLowerBound, mapUpperBound;
	int isCommercial;

	isCommercial = (gamemode == commercial);
	mapLowerBound = 1;
	mapUpperBound = isCommercial ? 32 : 9;
	numShuffledMaps = mapUpperBound - mapLowerBound + 1;		// REVISIT
	
	for (k = mapLowerBound, j = 0; k <= mapUpperBound; k++, j++) {
		shuffledMaps[j] = k;
	}
	shuffledMapIndex = 0;
	RandomizeIntArray(shuffledMaps, numShuffledMaps);
	isShuffledMapsInitialized = true;
}


int GetShuffledMap()
{
	int map;

	if (!isShuffledMapsInitialized || ++shuffledMapIndex >= numShuffledMaps) {
		RefreshShuffledMaps();
	}
	map = shuffledMaps[shuffledMapIndex];
	return map;
}


int RandomMap()
{
	int map;

	map = GetShuffledMap();

	return map;
}


int RandomEpisode()
{
	int episode;
	int sigil;

	sigil = Marshmallow_PlayingSigil;

	episode = GetRandomIntegerInRange(1, 4 + sigil); 

	return episode;
}


void SetSkills()
{
    if (Marshmallow_AlternateUltraViolence)   // Check for our new skills first
        skill_selection = newskill = 5;

    else if (Marshmallow_AlternateNightmare)
        skill_selection = newskill = 6;

    else {  // Otherwise, check for the usual skills

        switch (gameskill)
        {
            case sk_baby:
                skill_selection = newskill = sk_baby;
                break;
            case sk_easy:
                skill_selection = newskill = sk_easy;
                break;
            case sk_medium:
                skill_selection = newskill = sk_medium;
                break;
            case sk_hard:
                skill_selection = newskill = sk_hard;
                break;
            case sk_nightmare:
                skill_selection = newskill = sk_nightmare;
                Marshmallow_RespawnInNightmare = false;
                break;
        }
    }
}


void ShowExtendedSkillInfo()
{
    WriteToThirdConsoleLine( WriteLineWithInteger("UPGRADE CHANCE: %d %%", upgrade_chance), SHORT_EXTRALINE_TIMEOUT );
    WriteToFourthConsoleLine( WriteLineWithInteger("HITPOINTS MULTIPLIER: %d x", MonsterHitpointsScale), SHORT_EXTRALINE_TIMEOUT );
}


char* WriteLineWithInteger(char* prefix, int value)
{
    /*static*/ char completemessage[80];

    M_snprintf(completemessage, sizeof(completemessage), prefix, value);

    return /*DEH_String(*/completemessage/*)*/;
}


char* WriteLineWithString(char* string1, char* string2)
{
    /*static*/ char completemessage[80];

    M_snprintf(completemessage, sizeof(completemessage), string1, string2);

    return /*DEH_String(*/completemessage/*)*/;
}


void KilledByPlayer(mobj_t* source, mobj_t* target)
{
    static char completemessage[80];
    playerindex_t player;
    int color;

    if ( IsBot(target) )
        return;

    if ( IsBot(target) )
        player = target->player->bot_number;
    else
        player = target->player->player_number;

    switch (player)
    {
        case 0:
            color = CR_GREEN;
            break;
        case 1:
            color = CR_GRAY;
            break;
        case 2:
            color = CR_GOLD;
            break;
        case 3:
            color = CR_DARK;
            break;
    }

    M_snprintf(completemessage, sizeof(completemessage), "You were killed by %s!", marshmallow_player_names[player]);

    CrispyReplaceColor(completemessage, color, marshmallow_player_names[player]);

    players[consoleplayer].message = DEH_String(completemessage);
}


void KilledPlayer(mobj_t* source, mobj_t* target)
{
    static char completemessage[80];
    playerindex_t player;
    int color;

    if ( IsBot(source) )
        return;

    if ( IsBot(target) )
        player = target->player->bot_number;
    else
        player = target->player->player_number;

    switch (player)
    {
        case 0:
            color = CR_GREEN;
            break;
        case 1:
            color = CR_GRAY;
            break;
        case 2:
            color = CR_GOLD;
            break;
        case 3:
            color = CR_DARK;
            break;
    }

    M_snprintf(completemessage, sizeof(completemessage), "You killed %s!", marshmallow_player_names[player]);

    CrispyReplaceColor(completemessage, color, marshmallow_player_names[player]);

    players[consoleplayer].message = DEH_String(completemessage);
}


void DisplayMedkitRemaining()
{
    static char completemessage[80];
    int medkit_remaining;

    medkit_remaining = MAIN_PLAYER.medkit_remaining;

    M_snprintf(completemessage, sizeof(completemessage), "Medkit has %d health remaining.", medkit_remaining);

    CrispyReplaceColor(completemessage, CR_GRAY, "Medkit");

    players[consoleplayer].message = DEH_String(completemessage);
}


void SetDMFlags(int dm_mode)
{
    switch (dm_mode)
    {
        case 1:
            Marshmallow_DeathmatchWeaponsStay = true;
            Marshmallow_DeathmatchItemRespawn = false;
            break;
        case 2:
            Marshmallow_DeathmatchWeaponsStay = false;
            Marshmallow_DeathmatchItemRespawn = true;
            break;
        case 3:
            Marshmallow_DeathmatchWeaponsStay = true;
            Marshmallow_DeathmatchItemRespawn = true;
            break;
    }
}


void SetMultiplayerNames()
{
    if (realnetgame)
        return;

    marshmallow_player_names[0] = "YOU";
    marshmallow_player_names[1] = "INDIGO";
    marshmallow_player_names[2] = "BROWN";
    marshmallow_player_names[3] = "RED";
}


void WriteToSecondConsoleLine(char* string, int timeout)
{
    HUlib_addMessageToSText(&second_consoleline, 0, DEH_String(string));
    second_consoleline_timeout = timeout;
}


void WriteToThirdConsoleLine(char* string, int timeout)
{
    HUlib_addMessageToSText(&third_consoleline, 0, DEH_String(string));
    third_consoleline_timeout = timeout;
}


void WriteToFourthConsoleLine(char* string, int timeout)
{
    HUlib_addMessageToSText(&fourth_consoleline, 0, DEH_String(string));
    fourth_consoleline_timeout = timeout;
}