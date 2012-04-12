/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "g_local.h"

qboolean G_IsOnFireteam( int entityNum, fireteamData_t** teamNum );

/*
==================
G_SendScore

Sends current scoreboard information
==================
*/
void G_SendScore( gentity_t *ent ) {
	char entry[128];
	int i;
	gclient_t   *cl;
	int numSorted;
	int team, size, count;
	char buffer[1024];
	char startbuffer[32];

	// send the latest information on all clients
	numSorted = level.numConnectedClients;

	// Nico, replaced hardcoded value by a define
	// http://games.chruker.dk/enemy_territory/modding_project_bugfix.php?bug_id=068
	if ( numSorted > MAX_CLIENTS ) {
		numSorted = MAX_CLIENTS;
	}

	i = 0;
	// Gordon: team doesnt actually mean team, ignore...
	for ( team = 0; team < 2; team++ ) {
		*buffer = '\0';
		*startbuffer = '\0';
		if ( team == 0 ) {
			Q_strncpyz( startbuffer, va( "sc0 %i %i", level.teamScores[TEAM_AXIS], level.teamScores[TEAM_ALLIES] ), 32 );
		} else {
			Q_strncpyz( startbuffer, "sc1", 32 );
		}
		size = strlen( startbuffer ) + 1;
		count = 0;

		for (; i < numSorted ; i++ ) {
			/* Nico, removed respawnLeft
			int ping, playerClass, respawnsLeft;*/
			int ping, playerClass;

			cl = &level.clients[level.sortedClients[i]];

			if ( g_entities[level.sortedClients[i]].r.svFlags & SVF_POW ) {
				continue;
			}

			// NERVE - SMF - if on same team, send across player class
			// Gordon: FIXME: remove/move elsewhere?
			/* Nico, removed multiview
			if ( cl->ps.persistant[PERS_TEAM] == ent->client->ps.persistant[PERS_TEAM] || G_smvLocateEntityInMVList( ent, level.sortedClients[i], qfalse ) ) {*/
			if (cl->ps.persistant[PERS_TEAM] == ent->client->ps.persistant[PERS_TEAM]) {
				playerClass = cl->ps.stats[STAT_PLAYER_CLASS];
			} else {
				playerClass = 0;
			}

			// NERVE - SMF - number of respawns left
			/* Nico, removed respawnLeft
			respawnsLeft = cl->ps.persistant[PERS_RESPAWNS_LEFT];
			if ( g_gametype.integer == GT_WOLF_LMS ) {
				if ( g_entities[level.sortedClients[i]].health <= 0 ) {
					respawnsLeft = -2;
				}
			} else {
				if ( ( respawnsLeft == 0 && ( ( cl->ps.pm_flags & PMF_LIMBO ) || ( ( level.intermissiontime ) && g_entities[level.sortedClients[i]].health <= 0 ) ) ) ) {
					respawnsLeft = -2;
				}
			}*/

			if ( cl->pers.connected == CON_CONNECTING ) {
				ping = -1;
			} else {
				ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
			}

			/* Nico, removed LMS
			if ( g_gametype.integer == GT_WOLF_LMS ) {
				// Nico, removed respawnLeft
				// Com_sprintf( entry, sizeof( entry ), " %i %i %i %i %i %i %i", level.sortedClients[i], cl->ps.persistant[PERS_SCORE], ping,
							 ( level.time - cl->pers.enterTime ) / 60000, g_entities[level.sortedClients[i]].s.powerups, playerClass, respawnsLeft );
				Com_sprintf( entry, sizeof( entry ), " %i %i %i %i %i %i", level.sortedClients[i], cl->ps.persistant[PERS_SCORE], ping,
							 ( level.time - cl->pers.enterTime ) / 60000, g_entities[level.sortedClients[i]].s.powerups, playerClass );
			} else {*/

				/* Nico, removed skills
				int j, totalXP;

				for ( totalXP = 0, j = 0; j < SK_NUM_SKILLS; j++ ) {
					totalXP += cl->sess.skillpoints[j];
				}

				int totalXP = 0;*/

				// Nico, removed respawnLeft
				// Com_sprintf( entry, sizeof( entry ), " %i %i %i %i %i %i %i", level.sortedClients[i], totalXP, ping,
				//			 ( level.time - cl->pers.enterTime ) / 60000, g_entities[level.sortedClients[i]].s.powerups, playerClass, respawnsLeft );
				Com_sprintf( entry, sizeof( entry ), " %i %i %i %i %i", level.sortedClients[i], ping,
					( level.time - cl->pers.enterTime ) / 60000, g_entities[level.sortedClients[i]].s.powerups, playerClass );
			// }

			if ( size + strlen( entry ) > 1000 ) {
				i--; // we need to redo this client in the next buffer (if we can)
				break;
			}
			size += strlen( entry );

			Q_strcat( buffer, 1024, entry );
			if ( ++count >= 32 ) {
				i--; // we need to redo this client in the next buffer (if we can)
				break;
			}
		}

		if ( count > 0 || team == 0 ) {
			trap_SendServerCommand( ent - g_entities, va( "%s %i%s", startbuffer, count, buffer ) );
		}
	}
}

/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
	ent->client->wantsscore = qtrue;
//	G_SendScore( ent );
}

/*
==================
CheatsOk
==================
*/
qboolean    CheatsOk( gentity_t *ent ) {
	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Cheats are not enabled on this server.\n\"" ) );
		return qfalse;
	}
	if ( ent->health <= 0 ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"You must be alive to use this command.\n\"" ) );
		return qfalse;
	}
	return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char    *ConcatArgs( int start ) {
	int i, c, tlen;
	static char line[MAX_STRING_CHARS];
	int len;
	char arg[MAX_STRING_CHARS];

	len = 0;
	c = trap_Argc();
	for ( i = start ; i < c ; i++ ) {
		trap_Argv( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
			break;
		}
		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) {
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out, qboolean fToLower ) {
	while ( *in ) {
		if ( *in == 27 || *in == '^' ) {
			in++;       // skip color code
			if ( *in ) {
				in++;
			}
			continue;
		}

		if ( *in < 32 ) {
			in++;
			continue;
		}

		*out++ = ( fToLower ) ? tolower( *in++ ) : *in++;
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
	gclient_t   *cl;
	int idnum;
	char s2[MAX_STRING_CHARS];
	char n2[MAX_STRING_CHARS];
	qboolean fIsNumber = qtrue;

	// See if its a number or string
	for ( idnum = 0; idnum < strlen( s ) && s[idnum] != 0; idnum++ ) {
		if ( s[idnum] < '0' || s[idnum] > '9' ) {
			fIsNumber = qfalse;
			break;
		}
	}

	// check for a name match
	SanitizeString( s, s2, qtrue );
	for ( idnum = 0, cl = level.clients; idnum < level.maxclients; idnum++, cl++ ) {
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}

		SanitizeString( cl->pers.netname, n2, qtrue );
		if ( !strcmp( n2, s2 ) ) {
			return( idnum );
		}
	}

	// numeric values are just slot numbers
	if ( fIsNumber ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			CPx( to - g_entities, va( "print \"Bad client slot: [lof]%i\n\"", idnum ) );
			return -1;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected != CON_CONNECTED ) {
			CPx( to - g_entities, va( "print \"Client[lof] %i [lon]is not active\n\"", idnum ) );
			return -1;
		}
		return( idnum );
	}

	CPx( to - g_entities, va( "print \"User [lof]%s [lon]is not on the server\n\"", s ) );
	return( -1 );
}

/*
Nico, commented because give command was removed and this is no longer used
==================
Cmd_Give_f

Give items to a client
==================

void Cmd_Give_f( gentity_t *ent ) {
	char        *name, *amt;
	int i;
	qboolean give_all;
	int amount;
	qboolean hasAmount = qfalse;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	//----(SA)	check for an amount (like "give health 30")
	amt = ConcatArgs( 2 );
	if ( *amt ) {
		hasAmount = qtrue;
	}
	amount = atoi( amt );
	//----(SA)	end

	name = ConcatArgs( 1 );

	if ( Q_stricmp( name, "all" ) == 0 ) {
		give_all = qtrue;
	} else {
		give_all = qfalse;
	}

	if ( Q_stricmpn( name, "skill", 5 ) == 0 ) {
		if ( hasAmount ) {
			if ( amount >= 0 && amount < SK_NUM_SKILLS ) {
				G_AddSkillPoints( ent, amount, 20 );
				G_DebugAddSkillPoints( ent, amount, 20, "give skill" );
			}
		} else {
			// bumps all skills with 1 level
			for ( i = 0; i < SK_NUM_SKILLS; i++ ) {
				G_AddSkillPoints( ent, i, 20 );
				G_DebugAddSkillPoints( ent, i, 20, "give skill" );
			}
		}
		return;
	}

	if ( Q_stricmpn( name, "medal", 5 ) == 0 ) {
		for ( i = 0; i < SK_NUM_SKILLS; i++ ) {
			if ( !ent->client->sess.medals[i] ) {
				ent->client->sess.medals[i] = 1;
			}
		}
		ClientUserinfoChanged( ent - g_entities );
		return;
	}

	if ( give_all || Q_stricmpn( name, "health", 6 ) == 0 ) {
		//----(SA)	modified
		if ( amount ) {
			ent->health += amount;
		} else {
			ent->health = ent->client->ps.stats[STAT_MAX_HEALTH];
		}
		if ( !give_all ) {
			return;
		}
	}

	if ( give_all || Q_stricmp( name, "weapons" ) == 0 ) {
		for ( i = 0; i < WP_NUM_WEAPONS; i++ ) {
			if ( BG_WeaponInWolfMP( i ) ) {
				COM_BitSet( ent->client->ps.weapons, i );
			}
		}

		if ( !give_all ) {
			return;
		}
	}

	if ( give_all || Q_stricmpn( name, "ammo", 4 ) == 0 ) {
		if ( amount ) {
			if ( ent->client->ps.weapon
				 && ent->client->ps.weapon != WP_SATCHEL && ent->client->ps.weapon != WP_SATCHEL_DET
				 ) {
				Add_Ammo( ent, ent->client->ps.weapon, amount, qtrue );
			}
		} else {
			for ( i = 1 ; i < WP_NUM_WEAPONS ; i++ ) {
				if ( COM_BitCheck( ent->client->ps.weapons, i ) && i != WP_SATCHEL && i != WP_SATCHEL_DET ) {
					Add_Ammo( ent, i, 9999, qtrue );
				}
			}
		}

		if ( !give_all ) {
			return;
		}
	}

	//	"give allammo <n>" allows you to give a specific amount of ammo to /all/ weapons while
	//	allowing "give ammo <n>" to only give to the selected weap.
	if ( Q_stricmpn( name, "allammo", 7 ) == 0 && amount ) {
		for ( i = 1 ; i < WP_NUM_WEAPONS; i++ )
			Add_Ammo( ent, i, amount, qtrue );

		if ( !give_all ) {
			return;
		}
	}

	//---- (SA) Wolf keys
	if ( give_all || Q_stricmp( name, "keys" ) == 0 ) {
		ent->client->ps.stats[STAT_KEYS] = ( 1 << KEY_NUM_KEYS ) - 2;
		if ( !give_all ) {
			return;
		}
	}
	//---- (SA) end
}*/


/*
Nico, commented this because god command was removed but this function could be useful later

==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================

void Cmd_God_f( gentity_t *ent ) {
	char    *msg;
	char    *name;
	qboolean godAll = qfalse;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	name = ConcatArgs( 1 );

	// are we supposed to make all our teammates gods too?
	if ( Q_stricmp( name, "all" ) == 0 ) {
		godAll = qtrue;
	}

	// can only use this cheat in single player
	if ( godAll && g_gametype.integer == GT_SINGLE_PLAYER ) {
		int j;
		qboolean settingFlag = qtrue;
		gentity_t *other;

		// are we turning it on or off?
		if ( ent->flags & FL_GODMODE ) {
			settingFlag = qfalse;
		}

		// loop through all players
		for ( j = 0; j < level.maxclients; j++ )
		{
			other = &g_entities[j];
			// if they're on the same team
			if ( OnSameTeam( other, ent ) ) {
				// set or clear the flag
				if ( settingFlag ) {
					other->flags |= FL_GODMODE;
				} else {
					other->flags &= ~FL_GODMODE;
				}
			}
		}
		if ( settingFlag ) {
			msg = "godmode all ON\n";
		} else {
			msg = "godmode all OFF\n";
		}
	} else
	{
		if ( !Q_stricmp( name, "on" ) || atoi( name ) ) {
			ent->flags |= FL_GODMODE;
		} else if ( !Q_stricmp( name, "off" ) || !Q_stricmp( name, "0" ) ) {
			ent->flags &= ~FL_GODMODE;
		} else {
			ent->flags ^= FL_GODMODE;
		}
		if ( !( ent->flags & FL_GODMODE ) ) {
			msg = "godmode OFF\n";
		} else {
			msg = "godmode ON\n";
		}
	}

	trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}*/

/*
Nico, commented this because it's no longer used
==================
Cmd_Nofatigue_f

Sets client to nofatigue

argv(0) nofatigue
==================

void Cmd_Nofatigue_f( gentity_t *ent ) {
	char    *msg;

	char    *name = ConcatArgs( 1 );

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( !Q_stricmp( name, "on" ) || atoi( name ) ) {
		ent->flags |= FL_NOFATIGUE;
	} else if ( !Q_stricmp( name, "off" ) || !Q_stricmp( name, "0" ) ) {
		ent->flags &= ~FL_NOFATIGUE;
	} else {
		ent->flags ^= FL_NOFATIGUE;
	}

	if ( !( ent->flags & FL_NOFATIGUE ) ) {
		msg = "nofatigue OFF\n";
	} else {
		msg = "nofatigue ON\n";
	}

	trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}*/

/*
Nico, removed notarget command so this function is unused
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================

void Cmd_Notarget_f( gentity_t *ent ) {
	char    *msg;

	if ( !CheatsOk( ent ) ) {
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if ( !( ent->flags & FL_NOTARGET ) ) {
		msg = "notarget OFF\n";
	} else {
		msg = "notarget ON\n";
	}

	trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}*/


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
	char    *msg;

	char    *name = ConcatArgs( 1 );

	if ( !CheatsOk( ent ) ) {
		return;
	}

	if ( !Q_stricmp( name, "on" ) || atoi( name ) ) {
		ent->client->noclip = qtrue;
	} else if ( !Q_stricmp( name, "off" ) || !Q_stricmp( name, "0" ) ) {
		ent->client->noclip = qfalse;
	} else {
		ent->client->noclip = !ent->client->noclip;
	}

	if ( ent->client->noclip ) {
		msg = "noclip ON\n";
	} else {
		msg = "noclip OFF\n";
	}

	trap_SendServerCommand( ent - g_entities, va( "print \"%s\"", msg ) );
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
		 ( ent->client->ps.pm_flags & PMF_LIMBO ) ||
		 ent->health <= 0 || level.match_pause != PAUSE_NONE ) {
		return;
	}

	ent->flags &= ~FL_GODMODE;
	ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
	ent->client->ps.persistant[PERS_HWEAPON_USE] = 0; // TTimo - if using /kill while at MG42
	player_die( ent, ent, ent, ( g_gamestate.integer == GS_PLAYING ) ? 100000 : 135, MOD_SUICIDE );
}

void G_TeamDataForString( const char* teamstr, int clientNum, team_t* team, spectatorState_t* sState, int* specClient ) {
	*sState = SPECTATOR_NOT;
	if ( !Q_stricmp( teamstr, "follow1" ) ) {
		*team =     TEAM_SPECTATOR;
		*sState =   SPECTATOR_FOLLOW;
		if ( specClient ) {
			*specClient = -1;
		}
	} else if ( !Q_stricmp( teamstr, "follow2" ) ) {
		*team =     TEAM_SPECTATOR;
		*sState =   SPECTATOR_FOLLOW;
		if ( specClient ) {
			*specClient = -2;
		}
	} else if ( !Q_stricmp( teamstr, "spectator" ) || !Q_stricmp( teamstr, "s" ) ) {
		*team =     TEAM_SPECTATOR;
		*sState =   SPECTATOR_FREE;
	} else if ( !Q_stricmp( teamstr, "red" ) || !Q_stricmp( teamstr, "r" ) || !Q_stricmp( teamstr, "axis" ) ) {
		*team =     TEAM_AXIS;
	} else if ( !Q_stricmp( teamstr, "blue" ) || !Q_stricmp( teamstr, "b" ) || !Q_stricmp( teamstr, "allies" ) ) {
		*team =     TEAM_ALLIES;
	} else {
		*team = PickTeam( clientNum );
		if ( !G_teamJoinCheck( *team, &g_entities[clientNum] ) ) {
			*team = ( ( TEAM_AXIS | TEAM_ALLIES ) & ~*team );
		}
	}
}

/*
=================
SetTeam
=================
*/
qboolean SetTeam( gentity_t *ent, char *s, qboolean force, weapon_t w1, weapon_t w2, qboolean setweapons ) {
	team_t team, oldTeam;
	gclient_t           *client;
	int clientNum;
	spectatorState_t specState;
	int specClient;

	/* Nico, removed respawnLeft
	int respawnsLeft;*/

	//
	// see what change is requested
	//
	client = ent->client;

	clientNum = client - level.clients;
	specClient = 0;

	// ydnar: preserve respawn count
	/* Nico, removed respawnLeft
	respawnsLeft = client->ps.persistant[ PERS_RESPAWNS_LEFT ];*/

	G_TeamDataForString( s, client - level.clients, &team, &specState, &specClient );

	if ( team != TEAM_SPECTATOR ) {
		// Ensure the player can join
		if ( !G_teamJoinCheck( team, ent ) ) {
			// Leave them where they were before the command was issued
			return( qfalse );
		}

		/* Nico, removed team switch protection
		if ( g_noTeamSwitching.integer && ( team != ent->client->sess.sessionTeam && ent->client->sess.sessionTeam != TEAM_SPECTATOR ) && g_gamestate.integer == GS_PLAYING && !force ) {
			trap_SendServerCommand( clientNum, "cp \"You cannot switch during a match, please wait until the round ends.\n\"" );
			return qfalse;  // ignore the request
		}*/

		/* Nico, removed balancedteams
		if ( ( ( g_gametype.integer == GT_WOLF_LMS && g_lms_teamForceBalance.integer ) || g_teamForceBalance.integer ) && !force ) {
			int counts[TEAM_NUM_TEAMS];

			counts[TEAM_ALLIES] = TeamCount( ent - g_entities, TEAM_ALLIES );
			counts[TEAM_AXIS] = TeamCount( ent - g_entities, TEAM_AXIS );

			// We allow a spread of one
			if ( team == TEAM_AXIS && counts[TEAM_AXIS] - counts[TEAM_ALLIES] >= 1 ) {
				CP( "cp \"The Axis has too many players.\n\"" );
				return qfalse; // ignore the request
			}
			if ( team == TEAM_ALLIES && counts[TEAM_ALLIES] - counts[TEAM_AXIS] >= 1 ) {
				CP( "cp \"The Allies have too many players.\n\"" );
				return qfalse; // ignore the request
			}

			// It's ok, the team we are switching to has less or same number of players
		}*/
	}

	/* Nico, removed gameClients limits
	if ( g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer ) {
		team = TEAM_SPECTATOR;
	}*/

	//
	// decide if we will allow the change
	//
	oldTeam = client->sess.sessionTeam;
	if ( team == oldTeam && team != TEAM_SPECTATOR ) {
		return qfalse;
	}

	// NERVE - SMF - prevent players from switching to regain deployments
	/* Nico, removed respawnLeft
	if ( g_gametype.integer != GT_WOLF_LMS ) {
		if ( ( g_maxlives.integer > 0 ||
			   ( g_alliedmaxlives.integer > 0 && ent->client->sess.sessionTeam == TEAM_ALLIES ) ||
			   ( g_axismaxlives.integer > 0 && ent->client->sess.sessionTeam == TEAM_AXIS ) )

			 && ent->client->ps.persistant[PERS_RESPAWNS_LEFT] == 0 && oldTeam != TEAM_SPECTATOR ) {
			CP( "cp \"You can't switch teams because you are out of lives.\n\" 3" );
			return qfalse;  // ignore the request
		}
	}*/

	// DHM - Nerve
	// OSP
	if ( team != TEAM_SPECTATOR ) {
		client->pers.initialSpawn = qfalse;
		// no MV in-game
		/* Nico, removed multiview
		if ( client->pers.mvCount > 0 ) {
			G_smvRemoveInvalidClients( ent, TEAM_AXIS );
			G_smvRemoveInvalidClients( ent, TEAM_ALLIES );
		}*/
	}

	// he starts at 'base'
	// RF, in single player, bots always use regular spawn points
	/* Nico, removed gametypes
	if ( !( ( g_gametype.integer == GT_SINGLE_PLAYER || g_gametype.integer == GT_COOP ) && ( ent->r.svFlags & SVF_BOT ) ) ) {
		client->pers.teamState.state = TEAM_BEGIN;
	}*/

	if ( oldTeam != TEAM_SPECTATOR ) {
		if ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) {
			// Kill him (makes sure he loses flags, etc)
			ent->flags &= ~FL_GODMODE;
			ent->client->ps.stats[STAT_HEALTH] = ent->health = 0;
			player_die( ent, ent, ent, 100000, MOD_SWITCHTEAM );
		}
	}
	// they go to the end of the line for tournements
	if ( team == TEAM_SPECTATOR ) {
		client->sess.spectatorTime = level.time;
		if ( !client->sess.referee ) {
			client->pers.invite = 0;
		}
		/* Nico, removed multiview
		if ( team != oldTeam ) {
			G_smvAllRemoveSingleClient( ent - g_entities );
		}*/
	}

	G_LeaveTank( ent, qfalse );
	G_RemoveClientFromFireteams( clientNum, qtrue, qfalse );

	/* Nico, removed mines
	if ( g_landminetimeout.integer ) {
		G_ExplodeMines( ent );
	}*/

	G_FadeItems( ent, MOD_SATCHEL );

	// remove ourself from teamlists
	{
		int i;
		mapEntityData_t *mEnt;
		mapEntityData_Team_t *teamList;

		for ( i = 0; i < 2; i++ ) {
			teamList = &mapEntityData[i];

			if ( ( mEnt = G_FindMapEntityData( &mapEntityData[0], ent - g_entities ) ) != NULL ) {
				G_FreeMapEntityData( teamList, mEnt );
			}

			mEnt = G_FindMapEntityDataSingleClient( teamList, NULL, ent->s.number, -1 );

			while ( mEnt ) {
				mapEntityData_t *mEntFree = mEnt;

				mEnt = G_FindMapEntityDataSingleClient( teamList, mEnt, ent->s.number, -1 );

				G_FreeMapEntityData( teamList, mEntFree );
			}
		}
	}
	client->sess.spec_team = 0;
	client->sess.sessionTeam = team;
	client->sess.spectatorState = specState;
	client->sess.spectatorClient = specClient;
	client->pers.ready = qfalse;

	// (l)users will spam spec messages... honest!
	if ( team != oldTeam ) {
		gentity_t* tent = G_PopupMessage( PM_TEAM );
		tent->s.effect2Time = team;
		tent->s.effect3Time = clientNum;
		tent->s.density = 0;
	}

	if ( setweapons ) {
		G_SetClientWeapons( ent, w1, w2, qfalse );
	}

	// get and distribute relevent paramters
	G_UpdateCharacter( client );            // FIXME : doesn't ClientBegin take care of this already?
	ClientUserinfoChanged( clientNum );

	ClientBegin( clientNum );

	// ydnar: restore old respawn count (players cannot jump from team to team to regain lives)
	/* Nico, removed respawnLeft
	if ( respawnsLeft >= 0 && oldTeam != TEAM_SPECTATOR ) {
		client->ps.persistant[ PERS_RESPAWNS_LEFT ] = respawnsLeft;
	}*/

	/* Nico, removed warmup
	G_verifyMatchState( oldTeam );*/

	/* Nico, removed ws related command
	// Reset stats when changing teams
	if ( team != oldTeam ) {
		G_deleteStats( clientNum );
	}*/

	G_UpdateSpawnCounts();

	if ( g_gamestate.integer == GS_PLAYING && ( client->sess.sessionTeam == TEAM_AXIS || client->sess.sessionTeam == TEAM_ALLIES ) ) {

		/* Nico, removed LMS
		if ( g_gametype.integer == GT_WOLF_LMS && level.numTeamClients[0] > 0 && level.numTeamClients[1] > 0 ) {
			trap_SendServerCommand( clientNum, "cp \"Will spawn next round, please wait.\n\"" );
			limbo( ent, qfalse );
			return( qfalse );
		} else {*/
			int i;
			int x = client->sess.sessionTeam - TEAM_AXIS;

			for ( i = 0; i < MAX_COMMANDER_TEAM_SOUNDS; i++ ) {
				if ( level.commanderSounds[ x ][ i ].index ) {
					gentity_t* tent = G_TempEntity( client->ps.origin, EV_GLOBAL_CLIENT_SOUND );
					tent->s.eventParm = level.commanderSounds[ x ][ i ].index - 1;
					tent->s.teamNum = clientNum;
				}
			}
		// }
	}

	ent->client->pers.autofireteamCreateEndTime = 0;
	ent->client->pers.autofireteamJoinEndTime = 0;

	if ( client->sess.sessionTeam == TEAM_AXIS || client->sess.sessionTeam == TEAM_ALLIES ) {
		if ( g_autoFireteams.integer ) {
			fireteamData_t* ft = G_FindFreePublicFireteam( client->sess.sessionTeam );

			if ( ft ) {
				trap_SendServerCommand( ent - g_entities, "aftj -1" );
				ent->client->pers.autofireteamJoinEndTime = level.time + 20500;

//				G_AddClientToFireteam( ent-g_entities, ft->joinOrder[0] );
			} else {
				trap_SendServerCommand( ent - g_entities, "aftc -1" );
				ent->client->pers.autofireteamCreateEndTime = level.time + 20500;
			}
		}
	}

	return qtrue;
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
	// ATVI Wolfenstein Misc #474
	// divert behaviour if TEAM_SPECTATOR, moved the code from SpectatorThink to put back into free fly correctly
	// (I am not sure this can be called in non-TEAM_SPECTATOR situation, better be safe)
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		// drop to free floating, somewhere above the current position (that's the client you were following)
		vec3_t pos, angle;
		gclient_t   *client = ent->client;
		VectorCopy( client->ps.origin, pos );
		VectorCopy( client->ps.viewangles, angle );
		// Need this as it gets spec mode reset properly
		SetTeam( ent, "s", qtrue, -1, -1, qfalse );
		VectorCopy( pos, client->ps.origin );
		SetClientViewAngle( ent, angle );
	} else {
		// legacy code, FIXME: useless?
		// Gordon: no this is for limbo i'd guess
		ent->client->sess.spectatorState = SPECTATOR_FREE;
		ent->client->ps.clientNum = ent - g_entities;
	}
}

int G_NumPlayersWithWeapon( weapon_t weap, team_t team ) {
	int i, j, cnt = 0;

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		j = level.sortedClients[i];

		if ( level.clients[j].sess.playerType != PC_SOLDIER ) {
			continue;
		}

		if ( level.clients[j].sess.sessionTeam != team ) {
			continue;
		}

		if ( level.clients[j].sess.latchPlayerWeapon != weap && level.clients[j].sess.playerWeapon != weap ) {
			continue;
		}

		cnt++;
	}

	return cnt;
}

int G_NumPlayersOnTeam( team_t team ) {
	int i, j, cnt = 0;

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		j = level.sortedClients[i];

		if ( level.clients[j].sess.sessionTeam != team ) {
			continue;
		}

		cnt++;
	}

	return cnt;
}

qboolean G_IsHeavyWeapon( weapon_t weap ) {
	int i;

	for ( i = 0; i < NUM_HEAVY_WEAPONS; i++ ) {
		if ( bg_heavyWeapons[i] == weap ) {
			return qtrue;
		}
	}

	return qfalse;
}

int G_TeamCount( gentity_t* ent, weapon_t weap ) {
	int i, j, cnt;

	if ( weap == -1 ) { // we aint checking for a weapon, so always include ourselves
		cnt = 1;
	} else { // we ARE checking for a weapon, so ignore ourselves
		cnt = 0;
	}

	for ( i = 0; i < level.numConnectedClients; i++ ) {
		j = level.sortedClients[i];

		if ( j == ent - g_entities ) {
			continue;
		}

		if ( level.clients[j].sess.sessionTeam != ent->client->sess.sessionTeam ) {
			continue;
		}

		if ( weap != -1 ) {
			if ( level.clients[j].sess.playerWeapon != weap && level.clients[j].sess.latchPlayerWeapon != weap ) {
				continue;
			}
		}

		cnt++;
	}

	return cnt;
}

qboolean G_IsWeaponDisabled( gentity_t* ent, weapon_t weapon ) {
	int count, wcount;

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		return qtrue;
	}

	if ( !G_IsHeavyWeapon( weapon ) ) {
		return qfalse;
	}

	count =     G_TeamCount( ent, -1 );
	wcount =    G_TeamCount( ent, weapon );

	if ( wcount >= ceil( count * g_heavyWeaponRestriction.integer * 0.01f ) ) {
		return qtrue;
	}

	return qfalse;
}

void G_SetClientWeapons( gentity_t* ent, weapon_t w1, weapon_t w2, qboolean updateclient ) {
	qboolean changed = qfalse;

	if ( ent->client->sess.latchPlayerWeapon2 != w2 ) {
		ent->client->sess.latchPlayerWeapon2 = w2;
		changed = qtrue;
	}

	if ( !G_IsWeaponDisabled( ent, w1 ) ) {
		if ( ent->client->sess.latchPlayerWeapon != w1 ) {
			ent->client->sess.latchPlayerWeapon = w1;
			changed = qtrue;
		}
	} else {
		if ( ent->client->sess.latchPlayerWeapon != 0 ) {
			ent->client->sess.latchPlayerWeapon = 0;
			changed = qtrue;
		}
	}

	if ( updateclient && changed ) {
		ClientUserinfoChanged( ent - g_entities );
	}
}


/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent, unsigned int dwCommand, qboolean fValue ) {
	char s[MAX_TOKEN_CHARS];
	char ptype[4];
	char weap[4], weap2[4];
	weapon_t w, w2;

	if ( trap_Argc() < 2 ) {
		char *pszTeamName;

		switch ( ent->client->sess.sessionTeam ) {
		case TEAM_ALLIES:
			pszTeamName = "Allies";
			break;
		case TEAM_AXIS:
			pszTeamName = "Axis";
			break;
		case TEAM_SPECTATOR:
			pszTeamName = "Spectator";
			break;
		case TEAM_FREE:
		default:
			pszTeamName = "Free";
			break;
		}

		CP( va( "print \"%s team\n\"", pszTeamName ) );
		return;
	}

	trap_Argv( 1, s,        sizeof( s       ) );
	trap_Argv( 2, ptype,    sizeof( ptype   ) );
	trap_Argv( 3, weap,     sizeof( weap    ) );
	trap_Argv( 4, weap2,    sizeof( weap2   ) );

	w =     atoi( weap );
	w2 =    atoi( weap2 );

	ent->client->sess.latchPlayerType = atoi( ptype );
	if ( ent->client->sess.latchPlayerType < PC_SOLDIER || ent->client->sess.latchPlayerType > PC_COVERTOPS ) {
		ent->client->sess.latchPlayerType = PC_SOLDIER;
	}

	if ( ent->client->sess.latchPlayerType < PC_SOLDIER || ent->client->sess.latchPlayerType > PC_COVERTOPS ) {
		ent->client->sess.latchPlayerType = PC_SOLDIER;
	}

	if ( !SetTeam( ent, s, qfalse, w, w2, qtrue ) ) {
		G_SetClientWeapons( ent, w, w2, qtrue );
	}
}

void Cmd_ResetSetup_f( gentity_t* ent ) {
	qboolean changed = qfalse;

	if ( !ent || !ent->client ) {
		return;
	}

	ent->client->sess.latchPlayerType =     ent->client->sess.playerType;

	if ( ent->client->sess.latchPlayerWeapon != ent->client->sess.playerWeapon ) {
		ent->client->sess.latchPlayerWeapon = ent->client->sess.playerWeapon;
		changed = qtrue;
	}

	if ( ent->client->sess.latchPlayerWeapon2 != ent->client->sess.playerWeapon2 ) {
		ent->client->sess.latchPlayerWeapon2 =  ent->client->sess.playerWeapon2;
		changed = qtrue;
	}

	if ( changed ) {
		ClientUserinfoChanged( ent - g_entities );
	}
}

void Cmd_SetClass_f( gentity_t* ent, unsigned int dwCommand, qboolean fValue ) {
}

void Cmd_SetWeapons_f( gentity_t* ent, unsigned int dwCommand, qboolean fValue ) {
}

// END Mad Doc - TDF
/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent, unsigned int dwCommand, qboolean fValue ) {
	int i;
	char arg[MAX_TOKEN_CHARS];

	if ( trap_Argc() != 2 ) {
		if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
			StopFollowing( ent );
		}
		return;
	}

	if ( ent->client->ps.pm_flags & PMF_LIMBO ) {
		// Nico, replaced cpm by print to display into console
		// http://games.chruker.dk/enemy_territory/modding_project_bugfix.php?bug_id=065
		CP( "print \"Can't issue a follow command while in limbo.\n\"" );
		CP( "print \"Hit FIRE to switch between teammates.\n\"" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if ( i == -1 ) {
		if ( !Q_stricmp( arg, "allies" ) ) {
			i = TEAM_ALLIES;
		} else if ( !Q_stricmp( arg, "axis" ) ) {
			i = TEAM_AXIS;
		} else { return;}

		if ( !TeamCount( ent - g_entities, i ) ) {
			CP( va( "print \"The %s team %s empty!  Follow command ignored.\n\"", aTeams[i],
					( ( ent->client->sess.sessionTeam != i ) ? "is" : "would be" ) ) );
			return;
		}

		// Allow for simple toggle
		if ( ent->client->sess.spec_team != i ) {
			if ( teamInfo[i].spec_lock && !( ent->client->sess.spec_invite & i ) ) {
				CP( va( "print \"Sorry, the %s team is locked from spectators.\n\"", aTeams[i] ) );
			} else {
				ent->client->sess.spec_team = i;
				CP( va( "print \"Spectator follow is now locked on the %s team.\n\"", aTeams[i] ) );
				Cmd_FollowCycle_f( ent, 1 );
			}
		} else {
			ent->client->sess.spec_team = 0;
			CP( va( "print \"%s team spectating is now disabled.\n\"", aTeams[i] ) );
		}

		return;
	}

	// can't follow self
	if ( &level.clients[ i ] == ent->client ) {
		return;
	}

	// can't follow another spectator
	if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
		return;
	}
	if ( level.clients[ i ].ps.pm_flags & PMF_LIMBO ) {
		return;
	}

	// OSP - can't follow a player on a speclocked team, unless allowed
	if ( !G_allowFollow( ent, level.clients[i].sess.sessionTeam ) ) {
		CP( va( "print \"Sorry, the %s team is locked from spectators.\n\"", aTeams[level.clients[i].sess.sessionTeam] ) );
		return;
	}

	// first set them to spectator
	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		SetTeam( ent, "spectator", qfalse, -1, -1, qfalse );
	}

	ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
	ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
	int clientnum;
	int original;

	// first set them to spectator
	if ( ( ent->client->sess.spectatorState == SPECTATOR_NOT ) && ( !( ent->client->ps.pm_flags & PMF_LIMBO ) ) ) { // JPW NERVE for limbo state
		SetTeam( ent, "spectator", qfalse, -1, -1, qfalse );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	clientnum = ent->client->sess.spectatorClient;
	original = clientnum;
	do {
		clientnum += dir;
		if ( clientnum >= level.maxclients ) {
			clientnum = 0;
		}
		if ( clientnum < 0 ) {
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
			continue;
		}

		// can't follow another spectator
		if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
			continue;
		}

		// JPW NERVE -- couple extra checks for limbo mode
		if ( ent->client->ps.pm_flags & PMF_LIMBO ) {
			if ( level.clients[clientnum].ps.pm_flags & PMF_LIMBO ) {
				continue;
			}
			if ( level.clients[clientnum].sess.sessionTeam != ent->client->sess.sessionTeam ) {
				continue;
			}
		}

		if ( level.clients[clientnum].ps.pm_flags & PMF_LIMBO ) {
			continue;
		}

		// OSP
		if ( !G_desiredFollow( ent, level.clients[clientnum].sess.sessionTeam ) ) {
			continue;
		}

		// this is good, we can use it
		ent->client->sess.spectatorClient = clientnum;
		ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}


/*======================
G_EntitySound
	Mad Doc xkan, 11/06/2002 -

	Plays a sound (wav file or sound script) on this entity

	Note that calling G_AddEvent(..., EV_GENERAL_SOUND, ...) has the danger of
	the event never getting through to the client because the entity might not
	be visible (unless it has the SVF_BROADCAST flag), so if you want to make sure
	the sound is heard, call this function instead.
======================*/
void G_EntitySound(
	gentity_t *ent,         // entity to play the sound on
	const char *soundId,    // sound file name or sound script ID
	int volume ) {           // sound volume, only applies to sound file name call
							 //   for sound script, volume is currently always 127.
	trap_SendServerCommand( -1, va( "entitySound %d %s %d %i %i %i normal", ent->s.number, soundId, volume,
									(int)ent->s.pos.trBase[0], (int)ent->s.pos.trBase[1], (int)ent->s.pos.trBase[2] ) );
}

/*======================
G_EntitySoundNoCut
	Mad Doc xkan, 1/16/2003 -

	Similar to G_EntitySound, but do not cut this sound off

======================*/
void G_EntitySoundNoCut(
	gentity_t *ent,         // entity to play the sound on
	const char *soundId,    // sound file name or sound script ID
	int volume ) {           // sound volume, only applies to sound file name call
							 //   for sound script, volume is currently always 127.
	trap_SendServerCommand( -1, va( "entitySound %d %s %d %i %i %i noCut", ent->s.number, soundId, volume,
									(int)ent->s.pos.trBase[0], (int)ent->s.pos.trBase[1], (int)ent->s.pos.trBase[2] ) );
}


/*
==================
G_Say
==================
*/
#define MAX_SAY_TEXT    150

void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message, qboolean localize ) {
	if ( !other || !other->inuse || !other->client ) {
		return;
	}
	if ( ( mode == SAY_TEAM || mode == SAY_TEAMNL ) && !OnSameTeam( ent, other ) ) {
		return;
	}

	// NERVE - SMF - if spectator, no chatting to players in WolfMP
	/* Nico, removed match_* cvars
	if ( match_mutespecs.integer > 0 && ent->client->sess.referee == 0 &&   // OSP
		 ( ( ent->client->sess.sessionTeam == TEAM_FREE && other->client->sess.sessionTeam != TEAM_FREE ) ||
		   ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && other->client->sess.sessionTeam != TEAM_SPECTATOR ) ) ) {*/

	/* Nico, enable voice chat for spectators
	if ( ent->client->sess.referee == 0 &&   // OSP
		 ( ( ent->client->sess.sessionTeam == TEAM_FREE && other->client->sess.sessionTeam != TEAM_FREE ) ||
		   ( ent->client->sess.sessionTeam == TEAM_SPECTATOR && other->client->sess.sessionTeam != TEAM_SPECTATOR ) ) ) {*/
	if ( ent->client->sess.referee == 0 &&   // OSP
		 ( ( ent->client->sess.sessionTeam == TEAM_FREE && other->client->sess.sessionTeam != TEAM_FREE ) ) ) {
		return;
	} else {
		if ( mode == SAY_BUDDY ) {  // send only to people who have the sender on their buddy list
			if ( ent->s.clientNum != other->s.clientNum ) {
				fireteamData_t *ft1, *ft2;
				if ( !G_IsOnFireteam( other - g_entities, &ft1 ) ) {
					return;
				}
				if ( !G_IsOnFireteam( ent - g_entities, &ft2 ) ) {
					return;
				}
				if ( ft1 != ft2 ) {
					return;
				}
			}
		}

		trap_SendServerCommand( other - g_entities, va( "%s \"%s%c%c%s\" %i %i", mode == SAY_TEAM || mode == SAY_BUDDY ? "tchat" : "chat", name, Q_COLOR_ESCAPE, color, message, ent - g_entities, localize ) );
	}
}

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
	int j;
	gentity_t   *other;
	int color;
	char name[64];
	// don't let text be too long for malicious reasons
	char text[MAX_SAY_TEXT];
	qboolean localize = qfalse;
	char        *loc;

	switch ( mode ) {
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf( name, sizeof( name ), "%s%c%c: ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_BUDDY:
		localize = qtrue;
		G_LogPrintf( "saybuddy: %s: %s\n", ent->client->pers.netname, chatText );
		loc = BG_GetLocationString( ent->r.currentOrigin );
		Com_sprintf( name, sizeof( name ), "[lof](%s%c%c) (%s): ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, loc );
		color = COLOR_YELLOW;
		break;
	case SAY_TEAM:
		localize = qtrue;
		G_LogPrintf( "sayteam: %s: %s\n", ent->client->pers.netname, chatText );
		loc = BG_GetLocationString( ent->r.currentOrigin );
		Com_sprintf( name, sizeof( name ), "[lof](%s%c%c) (%s): ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, loc );
		color = COLOR_CYAN;
		break;
	case SAY_TEAMNL:
		G_LogPrintf( "sayteamnl: %s: %s\n", ent->client->pers.netname, chatText );
		Com_sprintf( name, sizeof( name ), "(%s^7): ", ent->client->pers.netname );
		color = COLOR_CYAN;
		break;
	}

	Q_strncpyz( text, chatText, sizeof( text ) );

	if ( target ) {
		if ( !COM_BitCheck( target->client->sess.ignoreClients, ent - g_entities ) ) {
			G_SayTo( ent, target, mode, color, name, text, localize );
		}
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "%s%s\n", name, text );
	}

	// send it to all the apropriate clients
	for ( j = 0; j < level.numConnectedClients; j++ ) {
		other = &g_entities[level.sortedClients[j]];
		if ( !COM_BitCheck( other->client->sess.ignoreClients, ent - g_entities ) ) {
			G_SayTo( ent, other, mode, color, name, text, localize );
		}
	}
}


/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
	if ( trap_Argc() < 2 && !arg0 ) {
		return;
	}
	G_Say( ent, NULL, mode, ConcatArgs( ( ( arg0 ) ? 0 : 1 ) ) );
}

// NERVE - SMF
void G_VoiceTo( gentity_t *ent, gentity_t *other, int mode, const char *id, qboolean voiceonly ) {
	int color;
	char *cmd;

	if ( !other ) {
		return;
	}
	if ( !other->inuse ) {
		return;
	}
	if ( !other->client ) {
		return;
	}
	if ( mode == SAY_TEAM && !OnSameTeam( ent, other ) ) {
		return;
	}

	// OSP - spec vchat rules follow the same as normal chatting rules
	/* Nico, removed match_* cvars
	if ( match_mutespecs.integer > 0 && ent->client->sess.referee == 0 &&
		 ent->client->sess.sessionTeam == TEAM_SPECTATOR && other->client->sess.sessionTeam != TEAM_SPECTATOR ) {*/
	if ( ent->client->sess.referee == 0 &&
		 ent->client->sess.sessionTeam == TEAM_SPECTATOR && other->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		return;
	}

	// send only to people who have the sender on their buddy list
	if ( mode == SAY_BUDDY ) {
		if ( ent->s.clientNum != other->s.clientNum ) {
			fireteamData_t *ft1, *ft2;
			if ( !G_IsOnFireteam( other - g_entities, &ft1 ) ) {
				return;
			}
			if ( !G_IsOnFireteam( ent - g_entities, &ft2 ) ) {
				return;
			}
			if ( ft1 != ft2 ) {
				return;
			}
		}
	}

	if ( mode == SAY_TEAM ) {
		color = COLOR_CYAN;
		cmd = "vtchat";
	} else if ( mode == SAY_BUDDY ) {
		color = COLOR_YELLOW;
		cmd = "vbchat";
	} else {
		color = COLOR_GREEN;
		cmd = "vchat";
	}

	if ( voiceonly == 2 ) {
		voiceonly = qfalse;
	}

	if ( mode == SAY_TEAM || mode == SAY_BUDDY ) {
		CPx( other - g_entities, va( "%s %d %d %d %s %i %i %i", cmd, voiceonly, ent - g_entities, color, id, (int)ent->s.pos.trBase[0], (int)ent->s.pos.trBase[1], (int)ent->s.pos.trBase[2] ) );
	} else {
		CPx( other - g_entities, va( "%s %d %d %d %s", cmd, voiceonly, ent - g_entities, color, id ) );
	}
}

void G_Voice( gentity_t *ent, gentity_t *target, int mode, const char *id, qboolean voiceonly ) {
	int j;

	// DHM - Nerve :: Don't allow excessive spamming of voice chats
	ent->voiceChatSquelch -= ( level.time - ent->voiceChatPreviousTime );
	ent->voiceChatPreviousTime = level.time;

	if ( ent->voiceChatSquelch < 0 ) {
		ent->voiceChatSquelch = 0;
	}

	// Only do the spam check for MP
	if ( ent->voiceChatSquelch >= 30000 ) {
		// Nico, voicechat spam protection was cluttering the popups message
		// http://games.chruker.dk/enemy_territory/modding_project_bugfix.php?bug_id=066
		trap_SendServerCommand( ent - g_entities, "cp \"^1Spam Protection^7: VoiceChat ignored\"" );
		return;
	}

	if ( g_voiceChatsAllowed.integer ) {
		ent->voiceChatSquelch += ( 34000 / g_voiceChatsAllowed.integer );
	} else {
		return;
	}
	// dhm

	// OSP - Charge for the lame spam!
	/*if(mode == SAY_ALL && (!Q_stricmp(id, "DynamiteDefused") || !Q_stricmp(id, "DynamitePlanted"))) {
		return;
	}*/

	if ( target ) {
		G_VoiceTo( ent, target, mode, id, voiceonly );
		return;
	}

	// echo the text to the console
	if ( g_dedicated.integer ) {
		G_Printf( "voice: %s %s\n", ent->client->pers.netname, id );
	}

	if ( mode == SAY_BUDDY ) {
		char buffer[32];
		int cls = -1, i, cnt, num;
		qboolean allowclients[MAX_CLIENTS];

		memset( allowclients, 0, sizeof( allowclients ) );

		trap_Argv( 1, buffer, 32 );

		cls = atoi( buffer );

		trap_Argv( 2, buffer, 32 );
		cnt = atoi( buffer );
		if ( cnt > MAX_CLIENTS ) {
			cnt = MAX_CLIENTS;
		}

		for ( i = 0; i < cnt; i++ ) {
			trap_Argv( 3 + i, buffer, 32 );

			num = atoi( buffer );
			if ( num < 0 ) {
				continue;
			}
			if ( num >= MAX_CLIENTS ) {
				continue;
			}

			allowclients[ num ] = qtrue;
		}

		for ( j = 0; j < level.numConnectedClients; j++ ) {

			if ( level.sortedClients[j] != ent->s.clientNum ) {
				if ( cls != -1 && cls != level.clients[level.sortedClients[j]].sess.playerType ) {
					continue;
				}
			}

			if ( cnt ) {
				if ( !allowclients[ level.sortedClients[j] ] ) {
					continue;
				}
			}

			G_VoiceTo( ent, &g_entities[level.sortedClients[j]], mode, id, voiceonly );
		}
	} else {

		// send it to all the apropriate clients
		for ( j = 0; j < level.numConnectedClients; j++ ) {
			G_VoiceTo( ent, &g_entities[level.sortedClients[j]], mode, id, voiceonly );
		}
	}
}

/*
==================
Cmd_Voice_f
==================
*/
static void Cmd_Voice_f( gentity_t *ent, int mode, qboolean arg0, qboolean voiceonly ) {
	if ( mode != SAY_BUDDY ) {
		if ( trap_Argc() < 2 && !arg0 ) {
			return;
		}
		G_Voice( ent, NULL, mode, ConcatArgs( ( ( arg0 ) ? 0 : 1 ) ), voiceonly );
	} else {
		char buffer[16];
		int index;

		trap_Argv( 2, buffer, sizeof( buffer ) );
		index = atoi( buffer );
		if ( index < 0 ) {
			index = 0;
		}

		if ( trap_Argc() < 3 + index && !arg0 ) {
			return;
		}
		G_Voice( ent, NULL, mode, ConcatArgs( ( ( arg0 ) ? 2 + index : 3 + index ) ), voiceonly );
	}
}

/*
Nico, commented this because it's no longer used
==================
Cmd_Where_f
==================

void Cmd_Where_f( gentity_t *ent ) {
	// Nico, bugfix, where command doesn't work as expected
	// trap_SendServerCommand( ent - g_entities, va( "print \"%s\n\"", vtos( ent->s.origin ) ) );
	trap_SendServerCommand( ent - g_entities, va( "print \"%s\n\"", vtos( ent->r.currentOrigin ) ) );
}*/

/*
==================
Cmd_CallVote_f
==================
*/
qboolean Cmd_CallVote_f( gentity_t *ent, unsigned int dwCommand, qboolean fRefCommand ) {
	int i;
	char arg1[MAX_STRING_TOKENS];
	char arg2[MAX_STRING_TOKENS];

	// Normal checks, if its not being issued as a referee command
	// Nico, moved 'callvote' command erros from popup messages to center print and console
	// http://games.chruker.dk/enemy_territory/modding_project_bugfix.php?bug_id=067
	if ( !fRefCommand ) {
		if ( level.voteInfo.voteTime ) {
			G_printFull("A vote is already in progress.", ent);
			return qfalse;
		}
		/* Nico, removed intermission
		else if ( level.intermissiontime ) {
			G_printFull("Cannot callvote during intermission.", ent);
			return qfalse;
		}*/
		else if ( !ent->client->sess.referee ) {
			if ( voteFlags.integer == VOTING_DISABLED ) {
				G_printFull("Voting not enabled on this server.", ent);
				return qfalse;
			} else if ( vote_limit.integer > 0 && ent->client->pers.voteCount >= vote_limit.integer ) {
				G_printFull(va("You have already called the maximum number of votes (%d).", vote_limit.integer), ent);
				return qfalse;
			} else if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
				G_printFull("Not allowed to call a vote as a spectator.", ent);
				return qfalse;
			}
		}
	}

	// make sure it is a valid command to vote on
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );


	// Nico, bugfix: callvote exploit fixed
	// http://aluigi.freeforums.org/quake3-engine-callvote-bug-t686.html
	if (strchr( arg1, ';' ) || strchr( arg2, ';' ) ||
		strchr( arg1, '\r' ) || strchr( arg2, '\r' ) ||
		strchr( arg1, '\n' ) || strchr( arg2, '\n' )) {
		char *strCmdBase = ( !fRefCommand ) ? "vote" : "ref command";

		G_refPrintf( ent, "Invalid %s string.", strCmdBase );
		return( qfalse );
	}


	if ( trap_Argc() > 1 && ( i = G_voteCmdCheck( ent, arg1, arg2, fRefCommand ) ) != G_NOTFOUND ) {   //  --OSP
		if ( i != G_OK ) {
			if ( i == G_NOTFOUND ) {
				return( qfalse );               // Command error
			} else { return( qtrue );}
		}
	} else {
		if ( !fRefCommand ) {
			CP( va( "print \"\n^3>>> Unknown vote command: ^7%s %s\n\"", arg1, arg2 ) );
			G_voteHelp( ent, qtrue );
		}
		return( qfalse );
	}

	Com_sprintf( level.voteInfo.voteString, sizeof( level.voteInfo.voteString ), "%s %s", arg1, arg2 );

	// start the voting, the caller automatically votes yes
	// If a referee, vote automatically passes.	// OSP
	if ( fRefCommand ) {
//		level.voteInfo.voteYes = level.voteInfo.numVotingClients + 10;	// JIC :)
		// Don't announce some votes, as in comp mode, it is generally a ref
		// who is policing people who shouldn't be joining and players don't want
		// this sort of spam in the console
		if ( level.voteInfo.vote_fn != G_Kick_v && level.voteInfo.vote_fn != G_Mute_v ) {
			AP( "cp \"^1** Referee Server Setting Change **\n\"" );
		}

		// Gordon: just call the stupid thing.... don't bother with the voting faff
		level.voteInfo.vote_fn( NULL, 0, NULL, NULL, qfalse );

		G_globalSound( "sound/misc/referee.wav" );
	} else {
		level.voteInfo.voteYes = 1;
		AP( va( "print \"[lof]%s^7 [lon]called a vote.[lof]  Voting for: %s\n\"", ent->client->pers.netname, level.voteInfo.voteString ) );
		AP( va( "cp \"[lof]%s\n^7[lon]called a vote.\n\"", ent->client->pers.netname ) );
		G_globalSound( "sound/misc/vote.wav" );
	}

	level.voteInfo.voteTime = level.time;
	level.voteInfo.voteNo = 0;

	// Don't send the vote info if a ref initiates (as it will automatically pass)
	if ( !fRefCommand ) {
		for ( i = 0; i < level.numConnectedClients; i++ ) {
			level.clients[level.sortedClients[i]].ps.eFlags &= ~EF_VOTED;
		}

		ent->client->pers.voteCount++;
		ent->client->ps.eFlags |= EF_VOTED;

		trap_SetConfigstring( CS_VOTE_YES,    va( "%i", level.voteInfo.voteYes ) );
		trap_SetConfigstring( CS_VOTE_NO,     va( "%i", level.voteInfo.voteNo ) );
		trap_SetConfigstring( CS_VOTE_STRING, level.voteInfo.voteString );
		trap_SetConfigstring( CS_VOTE_TIME,   va( "%i", level.voteInfo.voteTime ) );
	}

	return( qtrue );
}

qboolean StringToFilter( const char *s, ipFilter_t *f );

/* Nico, removed complaints
qboolean G_FindFreeComplainIP( gclient_t* cl, ipFilter_t* ip ) {
	int i = 0;

	if ( !g_ipcomplaintlimit.integer ) {
		return qtrue;
	}

	for ( i = 0; i < MAX_COMPLAINTIPS && i < g_ipcomplaintlimit.integer; i++ ) {
		if ( !cl->pers.complaintips[i].compare && !cl->pers.complaintips[i].mask ) {
			cl->pers.complaintips[i].compare = ip->compare;
			cl->pers.complaintips[i].mask = ip->mask;
			return qtrue;
		}
		if ( ( cl->pers.complaintips[i].compare & cl->pers.complaintips[i].mask ) == ( ip->compare & ip->mask ) ) {
			return qtrue;
		}
	}
	return qfalse;
}*/

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
	char msg[64];
	// int num; Nico, unused warning fix

	/* Nico, removed complaints
	// DHM - Nerve :: Complaints supercede voting (and share command)
	if ( ent->client->pers.complaintEndTime > level.time && g_gamestate.integer == GS_PLAYING && g_complaintlimit.integer ) {

		gentity_t* other = &g_entities[ ent->client->pers.complaintClient ];
		gclient_t *cl = other->client;
		if ( !cl ) {
			return;
		}
		if ( cl->pers.connected != CON_CONNECTED ) {
			return;
		}
		if ( cl->pers.localClient ) {
			trap_SendServerCommand( ent - g_entities, "complaint -3" );
			return;
		}

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			// Increase their complaint counter
			cl->pers.complaints++;

			num = g_complaintlimit.integer - cl->pers.complaints;

			if ( !cl->pers.localClient ) {

				const char* value;
				char userinfo[MAX_INFO_STRING];
				ipFilter_t ip;

				trap_GetUserinfo( ent - g_entities, userinfo, sizeof( userinfo ) );
				value = Info_ValueForKey( userinfo, "ip" );

				StringToFilter( value, &ip );

				if ( num <= 0 || !G_FindFreeComplainIP( cl, &ip ) ) {
					trap_DropClient( cl - level.clients, "kicked after too many complaints.", cl->sess.referee ? 0 : 300 );
					trap_SendServerCommand( ent - g_entities, "complaint -1" );
					return;
				}
			}

			trap_SendServerCommand( ent->client->pers.complaintClient, va( "cpm \"^1Warning^7: Complaint filed against you by %s^* You have Lost XP.\n\"", ent->client->pers.netname ) );
			trap_SendServerCommand( ent - g_entities, "complaint -1" );

			AddScore( other, WOLF_FRIENDLY_PENALTY );

			// Nico, removed g_stats.c
			// G_LoseKillSkillPoints( other, ent->sound2to1, ent->sound1to2, ent->sound2to3 ? qtrue : qfalse );
		} else {
			trap_SendServerCommand( ent->client->pers.complaintClient, "cpm \"No complaint filed against you.\n\"" );
			trap_SendServerCommand( ent - g_entities, "complaint -2" );
		}

		// Reset this ent's complainEndTime so they can't send multiple complaints
		ent->client->pers.complaintEndTime = -1;
		ent->client->pers.complaintClient = -1;

		return;
	}*/
	// dhm

	if ( ent->client->pers.applicationEndTime > level.time ) {

		gclient_t *cl = g_entities[ ent->client->pers.applicationClient ].client;
		if ( !cl ) {
			return;
		}
		if ( cl->pers.connected != CON_CONNECTED ) {
			return;
		}

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			trap_SendServerCommand( ent - g_entities, "application -4" );
			trap_SendServerCommand( ent->client->pers.applicationClient, "application -3" );

			G_AddClientToFireteam( ent->client->pers.applicationClient, ent - g_entities );
		} else {
			trap_SendServerCommand( ent - g_entities, "application -4" );
			trap_SendServerCommand( ent->client->pers.applicationClient, "application -2" );
		}

		ent->client->pers.applicationEndTime = 0;
		ent->client->pers.applicationClient = -1;

		return;
	}

	ent->client->pers.applicationEndTime = 0;
	ent->client->pers.applicationClient = -1;

	if ( ent->client->pers.invitationEndTime > level.time ) {

		gclient_t *cl = g_entities[ ent->client->pers.invitationClient ].client;
		if ( !cl ) {
			return;
		}
		if ( cl->pers.connected != CON_CONNECTED ) {
			return;
		}

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			trap_SendServerCommand( ent - g_entities, "invitation -4" );
			trap_SendServerCommand( ent->client->pers.invitationClient, "invitation -3" );

			G_AddClientToFireteam( ent - g_entities, ent->client->pers.invitationClient );
		} else {
			trap_SendServerCommand( ent - g_entities, "invitation -4" );
			trap_SendServerCommand( ent->client->pers.invitationClient, "invitation -2" );
		}

		ent->client->pers.invitationEndTime = 0;
		ent->client->pers.invitationClient = -1;

		return;
	}

	ent->client->pers.invitationEndTime = 0;
	ent->client->pers.invitationClient = -1;

	if ( ent->client->pers.propositionEndTime > level.time ) {
		gclient_t *cl = g_entities[ ent->client->pers.propositionClient ].client;
		if ( !cl ) {
			return;
		}
		if ( cl->pers.connected != CON_CONNECTED ) {
			return;
		}

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			trap_SendServerCommand( ent - g_entities, "proposition -4" );
			trap_SendServerCommand( ent->client->pers.propositionClient2, "proposition -3" );

			G_InviteToFireTeam( ent - g_entities, ent->client->pers.propositionClient );
		} else {
			trap_SendServerCommand( ent - g_entities, "proposition -4" );
			trap_SendServerCommand( ent->client->pers.propositionClient2, "proposition -2" );
		}

		ent->client->pers.propositionEndTime = 0;
		ent->client->pers.propositionClient = -1;
		ent->client->pers.propositionClient2 = -1;

		return;
	}

	if ( ent->client->pers.autofireteamEndTime > level.time ) {
		fireteamData_t* ft;

		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			trap_SendServerCommand( ent - g_entities, "aft -2" );

			if ( G_IsFireteamLeader( ent - g_entities, &ft ) ) {
				ft->priv = qtrue;
			}
		} else {
			trap_SendServerCommand( ent - g_entities, "aft -2" );
		}

		ent->client->pers.autofireteamEndTime = 0;

		return;
	}

	if ( ent->client->pers.autofireteamCreateEndTime > level.time ) {
		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			trap_SendServerCommand( ent - g_entities, "aftc -2" );

			G_RegisterFireteam( ent - g_entities );
		} else {
			trap_SendServerCommand( ent - g_entities, "aftc -2" );
		}

		ent->client->pers.autofireteamCreateEndTime = 0;

		return;
	}

	if ( ent->client->pers.autofireteamJoinEndTime > level.time ) {
		trap_Argv( 1, msg, sizeof( msg ) );

		if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
			fireteamData_t* ft;

			trap_SendServerCommand( ent - g_entities, "aftj -2" );


			ft = G_FindFreePublicFireteam( ent->client->sess.sessionTeam );
			if ( ft ) {
				G_AddClientToFireteam( ent - g_entities, ft->joinOrder[0] );
			}
		} else {
			trap_SendServerCommand( ent - g_entities, "aftj -2" );
		}

		ent->client->pers.autofireteamCreateEndTime = 0;

		return;
	}

	ent->client->pers.propositionEndTime = 0;
	ent->client->pers.propositionClient = -1;
	ent->client->pers.propositionClient2 = -1;

	// dhm
	// Reset this ent's complainEndTime so they can't send multiple complaints
	/* Nico, removed complaints
	ent->client->pers.complaintEndTime = -1;
	ent->client->pers.complaintClient = -1;*/

	if ( !level.voteInfo.voteTime ) {
		trap_SendServerCommand( ent - g_entities, "print \"No vote in progress.\n\"" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		trap_SendServerCommand( ent - g_entities, "print \"Vote already cast.\n\"" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		trap_SendServerCommand( ent - g_entities, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	if ( level.voteInfo.vote_fn == G_Kick_v ) {
		int pid = atoi( level.voteInfo.vote_value );
		if ( !g_entities[ pid ].client ) {
			return;
		}

		if ( g_entities[ pid ].client->sess.sessionTeam != TEAM_SPECTATOR && ent->client->sess.sessionTeam != g_entities[ pid ].client->sess.sessionTeam ) {
			trap_SendServerCommand( ent - g_entities, "print \"Cannot vote to kick player on opposing team.\n\"" );
			return;
		}
	}

	trap_SendServerCommand( ent - g_entities, "print \"Vote cast.\n\"" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
		level.voteInfo.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteInfo.voteYes ) );
	} else {
		level.voteInfo.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteInfo.voteNo ) );
	}

	// a majority will be determined in G_CheckVote, which will also account
	// for players entering or leaving
}


qboolean G_canPickupMelee( gentity_t *ent ) {
// JPW NERVE -- no "melee" weapons in net play
	return qfalse;
}
// jpw




/*
Nico, commented this because it's no longer used
=================
Cmd_SetViewpos_f
=================

void Cmd_SetViewpos_f( gentity_t *ent ) {
	vec3_t origin, angles;
	char buffer[MAX_TOKEN_CHARS];
	int i;

	if ( !g_cheats.integer ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"Cheats are not enabled on this server.\n\"" ) );
		return;
	}
	if ( trap_Argc() != 5 ) {
		trap_SendServerCommand( ent - g_entities, va( "print \"usage: setviewpos x y z yaw\n\"" ) );
		return;
	}

	VectorClear( angles );
	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	trap_Argv( 4, buffer, sizeof( buffer ) );
	angles[YAW] = atof( buffer );

	TeleportPlayer( ent, origin, angles );
}*/

/*
=================
Cmd_StartCamera_f
=================
*/
void Cmd_StartCamera_f( gentity_t *ent ) {

	if ( ent->client->cameraPortal  ) {
		G_FreeEntity( ent->client->cameraPortal );
	}
	ent->client->cameraPortal = G_Spawn();

	ent->client->cameraPortal->s.eType = ET_CAMERA;
	ent->client->cameraPortal->s.apos.trType = TR_STATIONARY;
	ent->client->cameraPortal->s.apos.trTime = 0;
	ent->client->cameraPortal->s.apos.trDuration = 0;
	VectorClear( ent->client->cameraPortal->s.angles );
	VectorClear( ent->client->cameraPortal->s.apos.trDelta );
	G_SetOrigin( ent->client->cameraPortal, ent->r.currentOrigin );
	VectorCopy( ent->r.currentOrigin, ent->client->cameraPortal->s.origin2 );

	ent->client->cameraPortal->s.frame = 0;

	ent->client->cameraPortal->r.svFlags |= ( SVF_PORTAL | SVF_SINGLECLIENT );
	ent->client->cameraPortal->r.singleClient = ent->client->ps.clientNum;

	ent->client->ps.eFlags |= EF_VIEWING_CAMERA;
	ent->s.eFlags |= EF_VIEWING_CAMERA;

	VectorCopy( ent->r.currentOrigin, ent->client->cameraOrigin );  // backup our origin
}

/*
=================
Cmd_StopCamera_f
=================
*/
void Cmd_StopCamera_f( gentity_t *ent ) {
	if ( ent->client->cameraPortal && ( ent->client->ps.eFlags & EF_VIEWING_CAMERA ) ) {

		// go back into noclient mode
		G_FreeEntity( ent->client->cameraPortal );
		ent->client->cameraPortal = NULL;

		ent->s.eFlags &= ~EF_VIEWING_CAMERA;
		ent->client->ps.eFlags &= ~EF_VIEWING_CAMERA;
	}
}

/*
=================
Cmd_SetCameraOrigin_f
=================
*/
void Cmd_SetCameraOrigin_f( gentity_t *ent ) {
	char buffer[MAX_TOKEN_CHARS];
	int i;
	vec3_t origin;

	if ( trap_Argc() != 4 ) {
		return;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		trap_Argv( i + 1, buffer, sizeof( buffer ) );
		origin[i] = atof( buffer );
	}

	if ( ent->client->cameraPortal ) {
		VectorCopy( origin, ent->client->cameraPortal->s.origin2 );
		trap_LinkEntity( ent->client->cameraPortal );
	}
}

extern vec3_t playerMins;
extern vec3_t playerMaxs;

qboolean G_TankIsOccupied( gentity_t* ent ) {
	if ( !ent->tankLink ) {
		return qfalse;
	}

	return qtrue;
}

qboolean G_TankIsMountable( gentity_t* ent, gentity_t* other ) {
	if ( !( ent->spawnflags & 128 ) ) {
		return qfalse;
	}

	if ( level.disableTankEnter ) {
		return qfalse;
	}

	if ( G_TankIsOccupied( ent ) ) {
		return qfalse;
	}

	if ( ent->health <= 0 ) {
		return qfalse;
	}

	if ( other->client->ps.weaponDelay ) {
		return qfalse;
	}

	return qtrue;
}

// Rafael
/*
==================
Cmd_Activate_f
==================
*/
qboolean Do_Activate2_f( gentity_t *ent, gentity_t *traceEnt ) {
	qboolean found = qfalse;

	/* Nico, removed disguise stuff TEST
	if ( ent->client->sess.playerType == PC_COVERTOPS && !ent->client->ps.powerups[PW_OPS_DISGUISED] && ent->health > 0 ) {
		if ( !ent->client->ps.powerups[PW_BLUEFLAG] && !ent->client->ps.powerups[PW_REDFLAG] ) {
			if ( traceEnt->s.eType == ET_CORPSE ) {
				if ( BODY_TEAM( traceEnt ) < 4 && BODY_TEAM( traceEnt ) != ent->client->sess.sessionTeam ) {
					found = qtrue;

					if ( BODY_VALUE( traceEnt ) >= 250 ) {

						traceEnt->nextthink = traceEnt->timestamp + BODY_TIME( BODY_TEAM( traceEnt ) );

						ent->client->ps.powerups[PW_OPS_DISGUISED] = 1;
						ent->client->ps.powerups[PW_OPS_CLASS_1] = BODY_CLASS( traceEnt ) & 1;
						ent->client->ps.powerups[PW_OPS_CLASS_2] = BODY_CLASS( traceEnt ) & 2;
						ent->client->ps.powerups[PW_OPS_CLASS_3] = BODY_CLASS( traceEnt ) & 4;

						BODY_TEAM( traceEnt ) += 4;
						traceEnt->activator = ent;

						traceEnt->s.time2 = 1;

						// sound effect
						G_AddEvent( ent, EV_DISGUISE_SOUND, 0 );

						// Nico, removed g_stats.c
						// G_AddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5.f );
						// G_DebugAddSkillPoints( ent, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 5, "stealing uniform" );

						Q_strncpyz( ent->client->disguiseNetname, g_entities[traceEnt->s.clientNum].client->pers.netname, sizeof( ent->client->disguiseNetname ) );

						// Nico, removed rank
						// ent->client->disguiseRank = g_entities[traceEnt->s.clientNum].client ? g_entities[traceEnt->s.clientNum].client->sess.rank : 0;
						ent->client->disguiseRank = 0;

						ClientUserinfoChanged( ent->s.clientNum );
					} else {
						BODY_VALUE( traceEnt ) += 5;
					}
				}
			}
		}
	}*/

	return found;
}

// TAT 1/14/2003 - extracted out the functionality of Cmd_Activate_f from finding the object to use
//		so we can force bots to use items, without worrying that they are looking EXACTLY at the target
qboolean Do_Activate_f( gentity_t *ent, gentity_t *traceEnt ) {
	qboolean found = qfalse;
	qboolean walking = qfalse;
	vec3_t forward;         //, offset, end;
	//trace_t		tr;

	// Arnout: invisible entities can't be used

	if ( traceEnt->entstate == STATE_INVISIBLE || traceEnt->entstate == STATE_UNDERCONSTRUCTION ) {
		return qfalse;
	}

	if ( ent->client->pers.cmd.buttons & BUTTON_WALKING ) {
		walking = qtrue;
	}

	if ( traceEnt->classname ) {
		traceEnt->flags &= ~FL_SOFTACTIVATE;    // FL_SOFTACTIVATE will be set if the user is holding 'walk' key

		if ( traceEnt->s.eType == ET_ALARMBOX ) {
			trace_t trace;

			if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
				return qfalse;
			}

			memset( &trace, 0, sizeof( trace ) );

			if ( traceEnt->use ) {
				G_UseEntity( traceEnt, ent, 0 );
			}
			found = qtrue;
		} else if ( traceEnt->s.eType == ET_ITEM )     {
			trace_t trace;

			if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
				return qfalse;
			}

			memset( &trace, 0, sizeof( trace ) );

			if ( traceEnt->touch ) {
				if ( ent->client->pers.autoActivate == PICKUP_ACTIVATE ) {
					ent->client->pers.autoActivate = PICKUP_FORCE;      //----(SA) force pickup
				}
				traceEnt->active = qtrue;
				traceEnt->touch( traceEnt, ent, &trace );
			}

			found = qtrue;
		} else if ( traceEnt->s.eType == ET_MOVER && G_TankIsMountable( traceEnt, ent ) ) {
			G_Script_ScriptEvent( traceEnt, "mg42", "mount" );
			ent->tagParent = traceEnt->nextTrain;
			Q_strncpyz( ent->tagName, "tag_player", MAX_QPATH );
			ent->backupWeaponTime = ent->client->ps.weaponTime;
			ent->client->ps.weaponTime = traceEnt->backupWeaponTime;
			ent->client->ps.weapHeat[WP_DUMMY_MG42] = traceEnt->mg42weapHeat;

			ent->tankLink = traceEnt;
			traceEnt->tankLink = ent;

			G_ProcessTagConnect( ent, qtrue );
			found = qtrue;
		} else if ( G_EmplacedGunIsMountable( traceEnt, ent ) ) {
			gclient_t* cl = &level.clients[ ent->s.clientNum ];
			vec3_t point;

			AngleVectors( traceEnt->s.apos.trBase, forward, NULL, NULL );
			VectorMA( traceEnt->r.currentOrigin, -36, forward, point );
			point[2] = ent->r.currentOrigin[2];

			// Save initial position
			VectorCopy( point, ent->TargetAngles );

			// Zero out velocity
			VectorCopy( vec3_origin, ent->client->ps.velocity );
			VectorCopy( vec3_origin, ent->s.pos.trDelta );

			traceEnt->active = qtrue;
			ent->active = qtrue;
			traceEnt->r.ownerNum = ent->s.number;
			VectorCopy( traceEnt->s.angles, traceEnt->TargetAngles );
			traceEnt->s.otherEntityNum = ent->s.number;

			cl->pmext.harc = traceEnt->harc;
			cl->pmext.varc = traceEnt->varc;
			VectorCopy( traceEnt->s.angles, cl->pmext.centerangles );
			cl->pmext.centerangles[PITCH] = AngleNormalize180( cl->pmext.centerangles[PITCH] );
			cl->pmext.centerangles[YAW] = AngleNormalize180( cl->pmext.centerangles[YAW] );
			cl->pmext.centerangles[ROLL] = AngleNormalize180( cl->pmext.centerangles[ROLL] );

			ent->backupWeaponTime = ent->client->ps.weaponTime;
			ent->client->ps.weaponTime = traceEnt->backupWeaponTime;
			ent->client->ps.weapHeat[WP_DUMMY_MG42] = traceEnt->mg42weapHeat;

			G_UseTargets( traceEnt, ent );   //----(SA)	added for Mike so mounting an MG42 can be a trigger event (let me know if there's any issues with this)
			found = qtrue;
		} else if ( ( ( Q_stricmp( traceEnt->classname, "func_door" ) == 0 ) || ( Q_stricmp( traceEnt->classname, "func_door_rotating" ) == 0 ) ) ) {
			if ( walking ) {
				traceEnt->flags |= FL_SOFTACTIVATE;     // no noise
			}
			G_TryDoor( traceEnt, ent, ent );      // (door,other,activator)
			found = qtrue;
		} else if ( ( Q_stricmp( traceEnt->classname, "team_WOLF_checkpoint" ) == 0 ) ) {
			if ( traceEnt->count != ent->client->sess.sessionTeam ) {
				traceEnt->health++;
			}
			found = qtrue;
		} else if ( ( Q_stricmp( traceEnt->classname, "func_button" ) == 0 ) && ( traceEnt->s.apos.trType == TR_STATIONARY && traceEnt->s.pos.trType == TR_STATIONARY ) && traceEnt->active == qfalse ) {
			Use_BinaryMover( traceEnt, ent, ent );
			traceEnt->active = qtrue;
			found = qtrue;
		} else if ( !Q_stricmp( traceEnt->classname, "func_invisible_user" ) ) {
			if ( walking ) {
				traceEnt->flags |= FL_SOFTACTIVATE;     // no noise
			}
			G_UseEntity( traceEnt, ent, ent );
			found = qtrue;
		} else if ( !Q_stricmp( traceEnt->classname, "props_footlocker" ) ) {
			G_UseEntity( traceEnt, ent, ent );
			found = qtrue;
		}
	}

	return found;
}

void G_LeaveTank( gentity_t* ent, qboolean position ) {
	gentity_t* tank;

	// found our tank (or whatever)
	vec3_t axis[3];
	vec3_t pos;
	trace_t tr;

	tank = ent->tankLink;
	if ( !tank ) {
		return;
	}

	if ( position ) {

		AnglesToAxis( tank->s.angles, axis );

		VectorMA( ent->client->ps.origin, 128, axis[1], pos );
		trap_Trace( &tr, pos, playerMins, playerMaxs, pos, -1, CONTENTS_SOLID );

		if ( tr.startsolid ) {
			// try right
			VectorMA( ent->client->ps.origin, -128, axis[1], pos );
			trap_Trace( &tr, pos, playerMins, playerMaxs, pos, -1, CONTENTS_SOLID );

			if ( tr.startsolid ) {
				// try back
				VectorMA( ent->client->ps.origin, -224, axis[0], pos );
				trap_Trace( &tr, pos, playerMins, playerMaxs, pos, -1, CONTENTS_SOLID );

				if ( tr.startsolid ) {
					// try front
					VectorMA( ent->client->ps.origin, 224, axis[0], pos );
					trap_Trace( &tr, pos, playerMins, playerMaxs, pos, -1, CONTENTS_SOLID );

					if ( tr.startsolid ) {
						// give up
						return;
					}
				}
			}
		}

		VectorClear( ent->client->ps.velocity ); // Gordon: dont want them to fly away ;D
		TeleportPlayer( ent, pos, ent->client->ps.viewangles );
	}


	tank->mg42weapHeat = ent->client->ps.weapHeat[WP_DUMMY_MG42];
	tank->backupWeaponTime = ent->client->ps.weaponTime;
	ent->client->ps.weaponTime = ent->backupWeaponTime;

	G_Script_ScriptEvent( tank, "mg42", "unmount" );
	ent->tagParent = NULL;
	*ent->tagName = '\0';
	ent->s.eFlags &= ~EF_MOUNTEDTANK;
	ent->client->ps.eFlags &= ~EF_MOUNTEDTANK;
	tank->s.powerups = -1;

	tank->tankLink = NULL;
	ent->tankLink = NULL;
}

void Cmd_Activate_f( gentity_t *ent ) {
	trace_t tr;
	vec3_t end;
	gentity_t   *traceEnt;
	vec3_t forward, right, up, offset;
//	int			activatetime = level.time;
	qboolean found = qfalse;
	qboolean pass2 = qfalse;
	int i;

	if ( ent->health <= 0 ) {
		return;
	}

	if ( ent->s.weapon == WP_MORTAR_SET || ent->s.weapon == WP_MOBILE_MG42_SET ) {
		return;
	}

	if ( ent->active ) {
		if ( ent->client->ps.persistant[PERS_HWEAPON_USE] ) {
			// DHM - Nerve :: Restore original position if current position is bad
			trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID );
			if ( tr.startsolid ) {
				VectorCopy( ent->TargetAngles, ent->client->ps.origin );
				VectorCopy( ent->TargetAngles, ent->r.currentOrigin );
				ent->r.contents = CONTENTS_CORPSE;      // DHM - this will correct itself in ClientEndFrame
			}

			ent->client->ps.eFlags &= ~EF_MG42_ACTIVE;          // DHM - Nerve :: unset flag
			ent->client->ps.eFlags &= ~EF_AAGUN_ACTIVE;

			ent->client->ps.persistant[PERS_HWEAPON_USE] = 0;
			ent->active = qfalse;

			for ( i = 0; i < level.num_entities; i++ ) {
				if ( g_entities[i].s.eType == ET_MG42_BARREL && g_entities[i].r.ownerNum == ent->s.number ) {
					g_entities[i].mg42weapHeat = ent->client->ps.weapHeat[WP_DUMMY_MG42];
					g_entities[i].backupWeaponTime = ent->client->ps.weaponTime;
					break;
				}
			}
			ent->client->ps.weaponTime = ent->backupWeaponTime;
		} else {
			ent->active = qfalse;
		}
		return;
	} else if ( ent->client->ps.eFlags & EF_MOUNTEDTANK && ent->s.eFlags & EF_MOUNTEDTANK && !level.disableTankExit ) {
		G_LeaveTank( ent, qtrue );
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, right, up );

	VectorCopy( ent->client->ps.origin, offset );
	offset[2] += ent->client->ps.viewheight;

	// lean
	if ( ent->client->ps.leanf ) {
		VectorMA( offset, ent->client->ps.leanf, right, offset );
	}

	//VectorMA( offset, 256, forward, end );
	VectorMA( offset, 96, forward, end );

	trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_MISSILECLIP | CONTENTS_BODY | CONTENTS_CORPSE )  );

	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.entityNum == ENTITYNUM_WORLD ) {
		trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_MISSILECLIP | CONTENTS_TRIGGER ) );
		pass2 = qtrue;
	}

tryagain:

	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.entityNum == ENTITYNUM_WORLD ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	found = Do_Activate_f( ent, traceEnt );

	if ( !found && !pass2 ) {
		pass2 = qtrue;
		trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_MISSILECLIP | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER ) );
		goto tryagain;
	}
}


void Cmd_Activate2_f( gentity_t *ent ) {
	trace_t tr;
	vec3_t end;
	gentity_t   *traceEnt;
	vec3_t forward, right, up, offset;
//	int			activatetime = level.time;
	qboolean found = qfalse;
	qboolean pass2 = qfalse;

	if ( ent->client->sess.playerType != PC_COVERTOPS ) {
		return;
	}

	AngleVectors( ent->client->ps.viewangles, forward, right, up );
	CalcMuzzlePointForActivate( ent, forward, right, up, offset );
	VectorMA( offset, 96, forward, end );

	trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE ) );

	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.entityNum == ENTITYNUM_WORLD ) {
		trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER ) );
		pass2 = qtrue;
	}

tryagain:

	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.entityNum == ENTITYNUM_WORLD ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	found = Do_Activate2_f( ent, traceEnt );

	if ( !found && !pass2 ) {
		pass2 = qtrue;
		trap_Trace( &tr, offset, NULL, NULL, end, ent->s.number, ( CONTENTS_SOLID | CONTENTS_BODY | CONTENTS_CORPSE | CONTENTS_TRIGGER ) );
		goto tryagain;
	}
}

void G_UpdateSpawnCounts( void ) {
	int i, j;
	char cs[MAX_STRING_CHARS];
	int current, count, team;

	for ( i = 0; i < level.numspawntargets; i++ ) {
		trap_GetConfigstring( CS_MULTI_SPAWNTARGETS + i, cs, sizeof( cs ) );

		current = atoi( Info_ValueForKey( cs, "c" ) );
		team = atoi( Info_ValueForKey( cs, "t" ) ) & ~256;

		count = 0;
		for ( j = 0; j < level.numConnectedClients; j++ ) {
			gclient_t* client = &level.clients[ level.sortedClients[ j ] ];

			if ( client->sess.sessionTeam != TEAM_AXIS && client->sess.sessionTeam != TEAM_ALLIES ) {
				continue;
			}

			if ( client->sess.sessionTeam == team && client->sess.spawnObjectiveIndex == i + 1 ) {
				count++;
				continue;
			}

			if ( client->sess.spawnObjectiveIndex == 0 ) {
				if ( client->sess.sessionTeam == TEAM_AXIS ) {
					if ( level.axisAutoSpawn == i ) {
						count++;
						continue;
					}
				} else {
					if ( level.alliesAutoSpawn == i ) {
						count++;
						continue;
					}
				}
			}
		}

		if ( count == current ) {
			continue;
		}

		Info_SetValueForKey( cs, "c", va( "%i", count ) );
		trap_SetConfigstring( CS_MULTI_SPAWNTARGETS + i, cs );
	}
}

/*
============
Cmd_SetSpawnPoint_f
============
*/
void SetPlayerSpawn( gentity_t* ent, int spawn, qboolean update ) {
	ent->client->sess.spawnObjectiveIndex = spawn;
	if ( ent->client->sess.spawnObjectiveIndex >= MAX_MULTI_SPAWNTARGETS || ent->client->sess.spawnObjectiveIndex < 0 ) {
		ent->client->sess.spawnObjectiveIndex = 0;
	}

	if ( update ) {
		G_UpdateSpawnCounts();
	}
}

void Cmd_SetSpawnPoint_f( gentity_t* ent ) {
	char arg[MAX_TOKEN_CHARS];
	int val, i;

	if ( trap_Argc() != 2 ) {
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	val = atoi( arg );

	if ( ent->client ) {
		SetPlayerSpawn( ent, val, qtrue );
	}

	for ( i = 0; i < level.numLimboCams; i++ ) {
		int x = ( g_entities[level.limboCams[i].targetEnt].count - CS_MULTI_SPAWNTARGETS ) + 1;
		if ( level.limboCams[i].spawn && x == val ) {
			VectorCopy( level.limboCams[i].origin, ent->s.origin2 );
			ent->r.svFlags |= SVF_SELF_PORTAL_EXCLUSIVE;
			trap_SendServerCommand( ent - g_entities, va( "portalcampos %i %i %i %i %i %i %i %i", val - 1, (int)level.limboCams[i].origin[0], (int)level.limboCams[i].origin[1], (int)level.limboCams[i].origin[2], (int)level.limboCams[i].angles[0], (int)level.limboCams[i].angles[1], (int)level.limboCams[i].angles[2], level.limboCams[i].hasEnt ? level.limboCams[i].targetEnt : -1 ) );
			break;
		}
	}
}

/*
============
Cmd_SetSniperSpot_f
============
*/
void Cmd_SetSniperSpot_f( gentity_t *clent ) {
	gentity_t *spot;

	vmCvar_t cvar_mapname;
	char filename[MAX_QPATH];
	fileHandle_t f;
	char buf[1024];

	if ( !g_cheats.integer ) {
		return;
	}
	if ( !trap_Cvar_VariableIntegerValue( "cl_running" ) ) {
		return;                                                 // only allow locally playing client
	}
	if ( clent->s.number != 0 ) {
		return;                         // only allow locally playing client

	}
	// drop a sniper spot here
	spot = G_Spawn();
	spot->classname = "bot_sniper_spot";
	VectorCopy( clent->r.currentOrigin, spot->s.origin );
	VectorCopy( clent->client->ps.viewangles, spot->s.angles );
	spot->aiTeam = clent->client->sess.sessionTeam;

	// output to text file
	trap_Cvar_Register( &cvar_mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	Com_sprintf( filename, sizeof( filename ), "maps/%s.botents", cvar_mapname.string );
	if ( trap_FS_FOpenFile( filename, &f, FS_APPEND ) < 0 ) {
		G_Error( "Cmd_SetSniperSpot_f: cannot open %s for writing", filename );
	}

	Com_sprintf( buf, sizeof( buf ), "{\n\"classname\" \"%s\"\n\"origin\" \"%.3f %.3f %.3f\"\n\"angles\" \"%.2f %.2f %.2f\"\n\"aiTeam\" \"%i\"\n}\n\n", spot->classname, spot->s.origin[0], spot->s.origin[1], spot->s.origin[2], spot->s.angles[0], spot->s.angles[1], spot->s.angles[2], spot->aiTeam );
	trap_FS_Write( buf, strlen( buf ), f );

	trap_FS_FCloseFile( f );

	G_Printf( "dropped sniper spot\n" );

	return;
}

/* Nico, removed showstats client command
void G_PrintAccuracyLog( gentity_t *ent );*/

/* Nico, removed +topshots command
void Cmd_WeaponStat_f( gentity_t* ent ) {
	char buffer[16];
	extWeaponStats_t stat;

	if ( !ent || !ent->client ) {
		return;
	}

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, buffer, 16 );
	stat = atoi( buffer );

	trap_SendServerCommand( ent - g_entities, va( "rws %i %i", ent->client->sess.aWeaponStats[stat].atts, ent->client->sess.aWeaponStats[stat].hits ) );
}*/

/* Nico, removed intermission
void Cmd_IntermissionWeaponStats_f( gentity_t* ent ) {
	char buffer[1024];
	int i, clientNum;

	if ( !ent || !ent->client ) {
		return;
	}

	trap_Argv( 1, buffer, sizeof( buffer ) );

	clientNum = atoi( buffer );
	if ( clientNum < 0 || clientNum > MAX_CLIENTS ) {
		return;
	}

	Q_strncpyz( buffer, "imws ", sizeof( buffer ) );
	for ( i = 0; i < WS_MAX; i++ ) {
		Q_strcat( buffer, sizeof( buffer ), va( "%i %i %i ", level.clients[clientNum].sess.aWeaponStats[i].atts, level.clients[clientNum].sess.aWeaponStats[i].hits, level.clients[clientNum].sess.aWeaponStats[i].kills ) );
	}

	trap_SendServerCommand( ent - g_entities, buffer );
}*/

void G_MakeReady( gentity_t* ent ) {
	ent->client->ps.eFlags |= EF_READY;
	ent->s.eFlags |= EF_READY;
	// rain - #105 - moved this set here
	ent->client->pers.ready = qtrue;
}

void G_MakeUnready( gentity_t* ent ) {
	ent->client->ps.eFlags &= ~EF_READY;
	ent->s.eFlags &= ~EF_READY;
	// rain - #105 - moved this set here
	ent->client->pers.ready = qfalse;
}

/* Nico, removed imready client command
void Cmd_IntermissionReady_f( gentity_t* ent ) {
	if ( !ent || !ent->client ) {
		return;
	}

	G_MakeReady( ent );
}*/

/* Nico, removed intermission
void Cmd_IntermissionPlayerKillsDeaths_f( gentity_t* ent ) {
	char buffer[1024];
	int i;

	if ( !ent || !ent->client ) {
		return;
	}

	Q_strncpyz( buffer, "impkd ", sizeof( buffer ) );
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		if ( g_entities[i].inuse ) {
			Q_strcat( buffer, sizeof( buffer ), va( "%i %i ", level.clients[i].sess.kills, level.clients[i].sess.deaths ) );
		} else {
			Q_strcat( buffer, sizeof( buffer ), "0 0 " );
		}
	}

	trap_SendServerCommand( ent - g_entities, buffer );
}*/

void G_CalcClientAccuracies( void ) {
	int i;// , j; Nico, unused warning fix
	int shots, hits;

	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		shots = 0;
		hits = 0;

		if ( g_entities[i].inuse ) {
			/* Nico, removed weaponstats
			for ( j = 0; j < WS_MAX; j++ ) {
				shots += level.clients[i].sess.aWeaponStats[j].atts;
				hits += level.clients[i].sess.aWeaponStats[j].hits;
			}*/

			level.clients[ i ].acc = shots ? ( 100 * hits ) / (float)shots : 0;
		} else {
			level.clients[ i ].acc = 0;
		}
	}
}

/* Nico, removed intermission
void Cmd_IntermissionWeaponAccuracies_f( gentity_t* ent ) {
	char buffer[1024];
	int i;

	if ( !ent || !ent->client ) {
		return;
	}

	G_CalcClientAccuracies();

	Q_strncpyz( buffer, "imwa ", sizeof( buffer ) );
	for ( i = 0; i < MAX_CLIENTS; i++ ) {
		Q_strcat( buffer, sizeof( buffer ), va( "%i ", (int)level.clients[ i ].acc ) );
	}

	trap_SendServerCommand( ent - g_entities, buffer );
}*/

void Cmd_SelectedObjective_f( gentity_t* ent ) {
	int i, val;
	char buffer[16];
	vec_t dist, neardist = 0;
	int nearest = -1;


	if ( !ent || !ent->client ) {
		return;
	}

	if ( trap_Argc() != 2 ) {
		return;
	}
	trap_Argv( 1, buffer, 16 );
	val = atoi( buffer ) + 1;


	for ( i = 0; i < level.numLimboCams; i++ ) {
		if ( !level.limboCams[i].spawn && level.limboCams[i].info == val ) {
			if ( !level.limboCams[i].hasEnt ) {
				VectorCopy( level.limboCams[i].origin, ent->s.origin2 );
				ent->r.svFlags |= SVF_SELF_PORTAL_EXCLUSIVE;
				trap_SendServerCommand( ent - g_entities, va( "portalcampos %i %i %i %i %i %i %i %i", val - 1, (int)level.limboCams[i].origin[0], (int)level.limboCams[i].origin[1], (int)level.limboCams[i].origin[2], (int)level.limboCams[i].angles[0], (int)level.limboCams[i].angles[1], (int)level.limboCams[i].angles[2], level.limboCams[i].hasEnt ? level.limboCams[i].targetEnt : -1 ) );

				break;
			} else {
				dist = VectorDistanceSquared( level.limboCams[i].origin, g_entities[level.limboCams[i].targetEnt].r.currentOrigin );
				if ( nearest == -1 || dist < neardist ) {
					nearest = i;
					neardist = dist;
				}
			}
		}
	}

	if ( nearest != -1 ) {
		i = nearest;

		VectorCopy( level.limboCams[i].origin, ent->s.origin2 );
		ent->r.svFlags |= SVF_SELF_PORTAL_EXCLUSIVE;
		trap_SendServerCommand( ent - g_entities, va( "portalcampos %i %i %i %i %i %i %i %i", val - 1, (int)level.limboCams[i].origin[0], (int)level.limboCams[i].origin[1], (int)level.limboCams[i].origin[2], (int)level.limboCams[i].angles[0], (int)level.limboCams[i].angles[1], (int)level.limboCams[i].angles[2], level.limboCams[i].hasEnt ? level.limboCams[i].targetEnt : -1 ) );
	}
}

void Cmd_Ignore_f( gentity_t* ent ) {
	char cmd[MAX_TOKEN_CHARS];
	int cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( !*cmd ) {
		trap_SendServerCommand( ent - g_entities, "print \"usage: Ignore <clientname>.\n\"\n" );
		return;
	}

	cnum = G_refClientnumForName( ent, cmd );

	if ( cnum != MAX_CLIENTS ) {
		COM_BitSet( ent->client->sess.ignoreClients, cnum );
	}
}

void Cmd_UnIgnore_f( gentity_t* ent ) {
	char cmd[MAX_TOKEN_CHARS];
	int cnum;

	trap_Argv( 1, cmd, sizeof( cmd ) );

	if ( !*cmd ) {
		trap_SendServerCommand( ent - g_entities, "print \"usage: Unignore <clientname>.\n\"\n" );
		return;
	}

	cnum = G_refClientnumForName( ent, cmd );

	if ( cnum != MAX_CLIENTS ) {
		COM_BitClear( ent->client->sess.ignoreClients, cnum );
	}
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
	gentity_t *ent;
	char cmd[MAX_TOKEN_CHARS];

	ent = g_entities + clientNum;
	if ( !ent->client ) {
		return;     // not fully in game yet
	}

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "say" ) == 0 ) {
		if ( !ent->client->sess.muted ) {
			Cmd_Say_f( ent, SAY_ALL, qfalse );
		}
		return;
	}

	if ( Q_stricmp( cmd, "say_team" ) == 0 ) {

		/* Nico, enable voice chat for spectators
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->sess.sessionTeam == TEAM_FREE ) {
			trap_SendServerCommand( ent - g_entities, "print \"Can't team chat as spectator\n\"\n" );
			return;
		}*/

		if ( !ent->client->sess.muted ) {
			Cmd_Say_f( ent, SAY_TEAM, qfalse );
		}
		return;
	} else if ( Q_stricmp( cmd, "vsay" ) == 0 ) {
		if ( !ent->client->sess.muted ) {
			Cmd_Voice_f( ent, SAY_ALL, qfalse, qfalse );
		}
		return;
	} else if ( Q_stricmp( cmd, "vsay_team" ) == 0 ) {

		/* Nico, enable voice chat for spectators
		if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->sess.sessionTeam == TEAM_FREE ) {
			trap_SendServerCommand( ent - g_entities, "print \"Can't team chat as spectator\n\"\n" );
			return;
		}*/

		if ( !ent->client->sess.muted ) {
			Cmd_Voice_f( ent, SAY_TEAM, qfalse, qfalse );
		}
		return;
	} else if ( Q_stricmp( cmd, "say_buddy" ) == 0 ) {
		if ( !ent->client->sess.muted ) {
			Cmd_Say_f( ent, SAY_BUDDY, qfalse );
		}
		return;
	} else if ( Q_stricmp( cmd, "vsay_buddy" ) == 0 ) {
		if ( !ent->client->sess.muted ) {
			Cmd_Voice_f( ent, SAY_BUDDY, qfalse, qfalse );
		}
		return;
	} else if ( Q_stricmp( cmd, "score" ) == 0 ) {
		Cmd_Score_f( ent );
		return;
	} else if ( Q_stricmp( cmd, "vote" ) == 0 ) {
		Cmd_Vote_f( ent );
		return;
	} else if ( Q_stricmp( cmd, "fireteam" ) == 0 ) {
		Cmd_FireTeam_MP_f( ent );
		return;
	} 

	/* Nico, removed showstats client command
	else if ( Q_stricmp( cmd, "showstats" ) == 0 ) {
		G_PrintAccuracyLog( ent );
		return;
	}*/

	else if ( Q_stricmp( cmd, "rconAuth" ) == 0 ) {
		Cmd_AuthRcon_f( ent );
		return;
	} else if ( Q_stricmp( cmd, "ignore" ) == 0 ) {
		Cmd_Ignore_f( ent );
		return;
	} else if ( Q_stricmp( cmd, "unignore" ) == 0 ) {
		Cmd_UnIgnore_f( ent );
		return;
	} else if ( Q_stricmp( cmd, "obj" ) == 0 ) {
		Cmd_SelectedObjective_f( ent );
		return;
	}

	/* Nico, removed intermission
	else if ( !Q_stricmp( cmd, "impkd" ) ) {
		Cmd_IntermissionPlayerKillsDeaths_f( ent );
		return;
	} else if ( !Q_stricmp( cmd, "imwa" ) ) {
		Cmd_IntermissionWeaponAccuracies_f( ent );
		return;
	} else if ( !Q_stricmp( cmd, "imws" ) ) {
		Cmd_IntermissionWeaponStats_f( ent );
		return;
	}*/

	/* Nico, removed imready client command
	else if ( !Q_stricmp( cmd, "imready" ) ) {
		Cmd_IntermissionReady_f( ent );
		return;
	}*/

	/* Nico, removed ws related command
	else if ( Q_stricmp( cmd, "ws" ) == 0 ) {
		Cmd_WeaponStat_f( ent );
		return;
	}*/
	
	else if ( !Q_stricmp( cmd, "forcetapout" ) ) {
		if ( !ent || !ent->client ) {
			return;
		}

		if ( ent->client->ps.stats[STAT_HEALTH] <= 0 && ( ent->client->sess.sessionTeam == TEAM_AXIS || ent->client->sess.sessionTeam == TEAM_ALLIES ) ) {
			limbo( ent );
		}

		return;
	}

	// OSP
	// Do these outside as we don't want to advertise it in the help screen
	/* Nico, removed ws related command
	if ( !Q_stricmp( cmd, "wstats" ) ) {
		G_statsPrint( ent, 1 );
		return;
	}
	if ( !Q_stricmp( cmd, "sgstats" ) ) { // Player game stats
		G_statsPrint( ent, 2 );
		return;
	}
	if ( !Q_stricmp( cmd, "stshots" ) ) { // "Topshots" accuracy rankings
		G_weaponStatsLeaders_cmd( ent, qtrue, qtrue );
		return;
	}*/

	if ( !Q_stricmp( cmd, "rs" ) ) {
		Cmd_ResetSetup_f( ent );
		return;
	}

	if ( G_commandCheck( ent, cmd, qtrue ) ) {
		return;
	}
	// OSP

	// ignore all other commands when at intermission
	/* Nico, removed intermission
	if ( level.intermissiontime ) {
		CPx( clientNum, va( "print \"^3%s^7 not allowed during intermission.\n\"", cmd ) );
		return;
	}*/

	/* Nico, removed give command
	if ( Q_stricmp( cmd, "give" ) == 0 ) {
		Cmd_Give_f( ent );
	}*/

	/* Nico, removed god command
	else if ( Q_stricmp( cmd, "god" ) == 0 ) {
		Cmd_God_f( ent );
	} */

	/* Nico, removed client command nofatigue
	else if ( Q_stricmp( cmd, "nofatigue" ) == 0 ) {
		Cmd_Nofatigue_f( ent );
	}*/

	/* Nico, removed notarget command
	else if ( Q_stricmp( cmd, "notarget" ) == 0 ) {
		Cmd_Notarget_f( ent );
	}*/ 

	else if ( Q_stricmp( cmd, "noclip" ) == 0 ) {
		Cmd_Noclip_f( ent );
	} else if ( Q_stricmp( cmd, "kill" ) == 0 ) {
		Cmd_Kill_f( ent );
	} else if ( Q_stricmp( cmd, "follownext" ) == 0 ) {
		Cmd_FollowCycle_f( ent, 1 );
	} else if ( Q_stricmp( cmd, "followprev" ) == 0 ) {
		Cmd_FollowCycle_f( ent, -1 );
	}
	/* Nico, removed where command
	else if ( Q_stricmp( cmd, "where" ) == 0 ) {
		Cmd_Where_f( ent );
	}*/
	else if ( Q_stricmp( cmd, "stopCamera" ) == 0 ) {
		Cmd_StopCamera_f( ent );
	} else if ( Q_stricmp( cmd, "setCameraOrigin" ) == 0 ) {
		Cmd_SetCameraOrigin_f( ent );
	}
	/* Nico, removed setviewpos command
	else if ( Q_stricmp( cmd, "setviewpos" ) == 0 ) {
		Cmd_SetViewpos_f( ent );
	}*/
	else if ( Q_stricmp( cmd, "setspawnpt" ) == 0 ) {
		Cmd_SetSpawnPoint_f( ent );
	} else if ( Q_stricmp( cmd, "setsniperspot" ) == 0 ) {
		Cmd_SetSniperSpot_f( ent );
	} else if ( G_commandCheck( ent, cmd, qfalse ) ) {
		return;

	} else {
		trap_SendServerCommand( clientNum, va( "print \"unknown cmd[lof] %s\n\"", cmd ) );
	}
}


