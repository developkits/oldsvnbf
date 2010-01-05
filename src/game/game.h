#ifndef __GAME_H__
#define __GAME_H__

#include "engine.h"

#define GAMEID				"bfa"
#define GAMEVERSION			161
#define DEMO_VERSION		GAMEVERSION

#define MAXAI 256
#define MAXPLAYERS (MAXCLIENTS + MAXAI)

// network quantization scale
#define DMF 16.0f			// for world locations
#define DNF 100.0f			// for normalized vectors
#define DVELF 1.0f			// for playerspeed based velocity vectors

#ifdef GAMESERVER
#define GVAR(name)			(sv_##name)
#else
#define GVAR(name)			(name)
#define FWV(name)			(force##name >= 0 ? force##name : physics::name)
#endif
enum
{
	S_JUMP = S_GAMESPECIFIC, S_IMPULSE, S_LAND, S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6, S_DIE1, S_DIE2, S_SPLASH1, S_SPLASH2, S_UNDERWATER,
	S_SPLAT, S_SPLOSH, S_DEBRIS, S_TINK, S_RICOCHET, S_WHIZZ, S_WHIRR, S_BEEP, S_EXPLODE, S_ENERGY, S_HUM, S_BURN, S_BURNING, S_BURNFIRE, S_EXTINGUISH, S_BZAP, S_BZZT,
	S_RELOAD, S_SWITCH, S_MELEE, S_MELEE2, S_PISTOL, S_PISTOL2, S_SHOTGUN, S_SHOTGUN2, S_SMG, S_SMG2, S_GRENADE, S_GRENADE2, S_FLAMER, S_FLAMER2, S_PLASMA, S_PLASMA2, S_RIFLE, S_RIFLE2,
	S_ITEMPICKUP, S_ITEMSPAWN, S_REGEN, S_DAMAGE1, S_DAMAGE2, S_DAMAGE3, S_DAMAGE4, S_DAMAGE5, S_DAMAGE6, S_DAMAGE7, S_DAMAGE8, S_BURNDAMAGE,
	S_RESPAWN, S_CHAT, S_ERROR, S_ALARM, S_V_FLAGSECURED, S_V_FLAGOVERTHROWN, S_V_FLAGPICKUP, S_V_FLAGDROP, S_V_FLAGRETURN, S_V_FLAGSCORE, S_V_FLAGRESET,
	S_V_FIGHT, S_V_CHECKPOINT, S_V_ONEMINUTE, S_V_HEADSHOT, S_V_SPREE1, S_V_SPREE2, S_V_SPREE3, S_V_SPREE4, S_V_SPREE5, S_V_SPREE6, S_V_MKILL1, S_V_MKILL2, S_V_MKILL3,
	S_V_REVENGE, S_V_DOMINATE, S_V_YOUWIN, S_V_YOULOSE, S_V_MCOMPLETE, S_V_FRAGGED, S_V_OWNED, S_V_DENIED,
	S_MAX
};

enum								// entity types
{
	NOTUSED = ET_EMPTY, LIGHT = ET_LIGHT, MAPMODEL = ET_MAPMODEL, PLAYERSTART = ET_PLAYERSTART, ENVMAP = ET_ENVMAP, PARTICLES = ET_PARTICLES,
	MAPSOUND = ET_SOUND, LIGHTFX = ET_LIGHTFX, SUNLIGHT = ET_SUNLIGHT, WEAPON = ET_GAMESPECIFIC,
	TELEPORT, ACTOR, TRIGGER, PUSHER, FLAG, CHECKPOINT, CAMERA, WAYPOINT, MAXENTTYPES
};

enum { EU_NONE = 0, EU_ITEM, EU_AUTO, EU_ACT, EU_MAX };

enum { TR_TOGGLE = 0, TR_LINK, TR_SCRIPT, TR_ONCE, TR_EXIT, TR_MAX };
enum { TA_MANUAL = 0, TA_AUTO, TA_ACTION, TA_MAX };
#define TRIGGERIDS		16
#define TRIGSTATE(a,b)	(b%2 ? !a : a)

enum { CP_RESPAWN = 0, CP_START, CP_FINISH, CP_LAST, CP_MAX };
enum { WP_COMMON = 0, WP_PLAYER, WP_ENEMY, WP_LINKED, WP_CAMERA, WP_MAX };
enum { WP_S_NONE = 0, WP_S_DEFEND, WP_S_PROJECT, WP_S_MAX };

struct enttypes
{
	int	type,			priority, links,	radius,	usetype,	numattrs,
			canlink,
			reclink;
	bool	noisy;	const char *name,			*attrs[6];
};
#ifdef GAMESERVER
enttypes enttype[] = {
	{
		NOTUSED,		-1,  0,		0,		EU_NONE,	0,
			0,
			0,
			true,				"none",			{ "",		"",			"",			"",			"",			"" }
	},
	{
		LIGHT,			1,  59,		0,		EU_NONE,	4,
			(1<<LIGHTFX),
			(1<<LIGHTFX),
			false,				"light",		{ "radius",	"red",		"green",	"blue",		"",			"" }
	},
	{
		MAPMODEL,		1,  58,		0,		EU_NONE,	6,
			(1<<TRIGGER),
			(1<<TRIGGER),
			false,				"mapmodel",		{ "type",	"yaw",		"rot",		"blend",	"scale",	"flags" }
	},
	{
		PLAYERSTART,	1,  59,		0,		EU_NONE,	5,
			0,
			0,
			false,				"playerstart",	{ "team",	"yaw",		"pitch",	"mode",		"id",		"" }
	},
	{
		ENVMAP,			1,  0,		0,		EU_NONE,	1,
			0,
			0,
			false,				"envmap",		{ "radius",	"",			"",			"",			"",			"" }
	},
	{
		PARTICLES,		1,  59,		0,		EU_NONE,	5,
			(1<<TELEPORT)|(1<<TRIGGER)|(1<<PUSHER),
			(1<<TRIGGER)|(1<<PUSHER),
			false,				"particles",	{ "type",	"a",		"b",		"c",		"d",		"" }
	},
	{
		MAPSOUND,		1,  58,		0,		EU_NONE,	5,
			(1<<TELEPORT)|(1<<TRIGGER)|(1<<PUSHER),
			(1<<TRIGGER)|(1<<PUSHER),
			false,				"sound",		{ "type",	"maxrad",	"minrad",	"volume",	"flags",	"" }
	},
	{
		LIGHTFX,		1,  1,		0,		EU_NONE,	5,
			(1<<LIGHT)|(1<<TELEPORT)|(1<<TRIGGER)|(1<<PUSHER),
			(1<<LIGHT)|(1<<TRIGGER)|(1<<PUSHER),
			false,				"lightfx",		{ "type",	"mod",		"min",		"max",		"flags",	"" }
	},
	{
		SUNLIGHT,		1,  160,	0,		EU_NONE,	6,
			0,
			0,
			false,				"sunlight",		{ "yaw",	"pitch",	"red",		"green",	"blue",		"offset" }
	},
	{
		WEAPON,			2,  59,		16,		EU_ITEM,	4,
			0,
			0,
			false,				"weapon",		{ "type",	"flags",	"mode",		"id",		"",			"" }
	},
	{
		TELEPORT,		1,  50,		12,		EU_AUTO,	5,
			(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX)|(1<<TELEPORT),
			(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX),
			false,				"teleport",		{ "yaw",	"pitch",	"push",		"radius",	"colour",	"" }
	},
	{
		ACTOR,			1,  59,		0,		EU_NONE,	5,
			(1<<FLAG)|(1<<WAYPOINT),
			0,
			false,				"actor",		{ "type",	"yaw",		"pitch",	"mode",		"id",		"" }
	},
	{
		TRIGGER,		1,  58,		16,		EU_AUTO,	5,
			(1<<MAPMODEL)|(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX),
			(1<<MAPMODEL)|(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX),
			false,				"trigger",		{ "id",		"type",		"action",	"radius",	"state",	"" }
	},
	{
		PUSHER,			1,  58,		12,		EU_AUTO,	5,
			(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX),
			(1<<MAPSOUND)|(1<<PARTICLES)|(1<<LIGHTFX),
			false,				"pusher",		{ "zpush",	"ypush",	"xpush",	"radius",	"min",		"" }
	},
	{
		FLAG,			1,  48,		36,		EU_NONE,	5,
			(1<<FLAG),
			0,
			false,				"flag",			{ "team",	"yaw",		"pitch",	"mode",		"id",		"" }
	},
	{
		CHECKPOINT,		1,  48,		16,		EU_AUTO,	6,
			0,
			0,
			false,				"checkpoint",	{ "radius",	"yaw",		"pitch",	"mode",		"id",		"type" }
	},
	{
		CAMERA,			1,  48,		0,		EU_NONE,	3,
			(1<<CAMERA),
			0,
			false,				"camera",		{ "type",	"mindist",	"maxdist",	"",			"",			"" }
	},
	{
		WAYPOINT,		0,  1,		16,		EU_NONE,	5,
			(1<<WAYPOINT),
			0,
			true,				"waypoint",		{ "type",	"state",	"id",		"radius",	"flags",	"" }
	}
};
#else
extern enttypes enttype[];
#endif

enum
{
	ANIM_PAIN = ANIM_GAMESPECIFIC, ANIM_JUMP,
	ANIM_IMPULSE_FORWARD, ANIM_IMPULSE_BACKWARD, ANIM_IMPULSE_LEFT, ANIM_IMPULSE_RIGHT, ANIM_IMPULSE_DASH,
	ANIM_SINK, ANIM_EDIT, ANIM_LAG, ANIM_SWITCH, ANIM_PICKUP, ANIM_WIN, ANIM_LOSE,
	ANIM_CROUCH, ANIM_CRAWL_FORWARD, ANIM_CRAWL_BACKWARD, ANIM_CRAWL_LEFT, ANIM_CRAWL_RIGHT,
	ANIM_MELEE, ANIM_MELEE_ATTACK,
	ANIM_PISTOL, ANIM_PISTOL_SHOOT, ANIM_PISTOL_RELOAD,
	ANIM_SHOTGUN, ANIM_SHOTGUN_SHOOT, ANIM_SHOTGUN_RELOAD,
	ANIM_SMG, ANIM_SMG_SHOOT, ANIM_SMG_RELOAD,
	ANIM_GRENADE, ANIM_GRENADE_THROW, ANIM_GREANDES_RELOAD, ANIM_GRENADE_POWER,
	ANIM_FLAMER, ANIM_FLAMER_SHOOT, ANIM_FLAMER_RELOAD,
	ANIM_PLASMA, ANIM_PLASMA_SHOOT, ANIM_PLASMA_RELOAD,
	ANIM_RIFLE, ANIM_RIFLE_SHOOT, ANIM_RIFLE_RELOAD,
	ANIM_VWEP, ANIM_SHIELD, ANIM_POWERUP,
	ANIM_MAX
};

#define WEAPSWITCHDELAY	PHYSMILLIS*2
#define WEAPPICKUPDELAY	PHYSMILLIS*2
#define EXPLOSIONSCALE	16.f

enum
{
	WEAP_MELEE = 0, WEAP_PISTOL, WEAP_OFFSET, // end of unselectable weapon set
	WEAP_SHOTGUN = WEAP_OFFSET, WEAP_SMG, WEAP_FLAMER, WEAP_PLASMA, WEAP_RIFLE, WEAP_GRENADE, WEAP_SUPER, // end of item weapon set
	WEAP_INSTA = WEAP_SUPER, WEAP_TOTAL, // end of selectable weapon set
	WEAP_GIBS = WEAP_TOTAL, WEAP_MAX
};
#define isweap(a)		(a > -1 && a < WEAP_MAX)

enum { WEAP_F_NONE = 0, WEAP_F_FORCED = 1<<0 };
enum { WEAP_S_IDLE = 0, WEAP_S_SHOOT, WEAP_S_RELOAD, WEAP_S_POWER, WEAP_S_SWITCH, WEAP_S_PICKUP, WEAP_S_WAIT };

enum
{
	IMPACT_GEOM = 1<<0, BOUNCE_GEOM = 1<<1, IMPACT_PLAYER = 1<<2, BOUNCE_PLAYER = 1<<3, RADIAL_PLAYER = 1<<4,
	COLLIDE_TRACE = 1<<5, COLLIDE_OWNER = 1<<6, COLLIDE_CONT = 1<<7, COLLIDE_STICK = 1<<8,
	COLLIDE_GEOM = IMPACT_GEOM|BOUNCE_GEOM, COLLIDE_PLAYER = IMPACT_PLAYER|BOUNCE_PLAYER, HIT_PLAYER = IMPACT_PLAYER|BOUNCE_PLAYER|RADIAL_PLAYER
};

#define WEAPON(name,w0,w1,w2,w3,w4,w5,w6,w7,w8,w9,wa,wb,wc,wd,we,wf,wg,wh,wi,wj,wk,wl,wm,wn,wo,wp,wq,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,y0,y1,y2,y3,y4,y5,y6,y7,y8,y9,ya,yb,yc,yd,ye,yf,yg,yh) \
	VARG(name##add, 1, w0, w1);					VARG(name##max, 1, w1, w1); \
	VARG(name##sub1, 0, w2, w1);				VARG(name##sub2, 0, w3, w1); \
	VARG(name##adelay1, 0, w4, INT_MAX-1);		VARG(name##adelay2, 0, w5, INT_MAX-1);		VARG(name##rdelay, 0, w6, INT_MAX-1); \
	VARG(name##damage1, 0, w7, INT_MAX-1);		VARG(name##damage2, 0, w8, INT_MAX-1); \
	VARG(name##speed1, 0, w9, INT_MAX-1);		VARG(name##speed2, 0, wa, INT_MAX-1); \
	VARG(name##power, 0, wb, INT_MAX-1); \
	VARG(name##time1, 0, wc, INT_MAX-1);		VARG(name##time2, 0, wd, INT_MAX-1); \
	VARG(name##pdelay, 0, we, INT_MAX-1); \
	VARG(name##explode1, 0, wf, INT_MAX-1);		VARG(name##explode2, 0, wg, INT_MAX-1); \
	VARG(name##rays1, 1, wh, INT_MAX-1);		VARG(name##rays2, 1, wi, INT_MAX-1); \
	VARG(name##spread1, 0, wj, INT_MAX-1);		VARG(name##spread2, 0, wk, INT_MAX-1); \
	VARG(name##zdiv1, 0, wl, INT_MAX-1);		VARG(name##zdiv2, 0, wm, INT_MAX-1); \
	VARG(name##aiskew1, 0, wn, INT_MAX-1);		VARG(name##aiskew2, 0, wo, INT_MAX-1); \
	VARG(name##collide1, 0, wp, INT_MAX-1);		VARG(name##collide2, 0, wq, INT_MAX-1); \
	VARG(name##taper1, 0, x0, 1);				VARG(name##taper2, 0, x1, 1); \
	VARG(name##extinguish1, 0, x2, 1);			VARG(name##extinguish2, 0, x3, 1); \
	VARG(name##radial1, 0, x4, 1);				VARG(name##radial2, 0, x5, 1); \
	VARG(name##burns1, 0, x6, 1);				VARG(name##burns2, 0, x7, 1); \
	VARG(name##reloads, 0, x8, 1);				VARG(name##zooms, 0, x9, 1); \
	VARG(name##fullauto1, 0, xa, 1);			VARG(name##fullauto2, 0, xb, 1); \
	FVARG(name##elasticity1, 0, y0, 1);			FVARG(name##elasticity2, 0, y1, 1); \
	FVARG(name##reflectivity1, 0, y2, 360);		FVARG(name##reflectivity2, 0, y3, 360); \
	FVARG(name##relativity1, 0, y4, 10000);		FVARG(name##relativity2, 0, y5, 10000); \
	FVARG(name##waterfric1, 0, y6, 10000);		FVARG(name##waterfric2, 0, y7, 10000); \
	FVARG(name##weight1, -10000, y8, 10000);	FVARG(name##weight2, -10000, y9, 10000); \
	FVARG(name##radius1, -10000, ya, 10000);	FVARG(name##radius2, -10000, yb, 10000); \
	FVARG(name##kickpush1, -10000, yc, 10000);	FVARG(name##kickpush2, -10000, yd, 10000); \
	FVARG(name##hitpush1, -10000, ye, 10000);	FVARG(name##hitpush2, -10000, yf, 10000); \
	FVARG(name##maxdist1, 0, yg, 10000);		FVARG(name##maxdist2, 0, yh, 10000);

//	add		max		sub1	sub2	adelay1		adelay2		rdelay	dam1	dam2	speed1		speed2		power	time1		time2		pdelay	expl1	expl2	rays1	rays2	sprd1	sprd2	zdiv1	zdiv2	aiskew1	aiskew2
//	collide1																collide2
//	tpr1	tpr2	ext1	ext2	radl1	radl2	brn1	brn2	rlds	zooms	fa1		fa2
//	elas1	elas2	rflt1	rflt2	relt1	relt2	wfrc1	wfrc2	wght1	wght2	rads1	rads2	kpsh1	kpsh2	hpsh1		hpsh2		mdst1	mdst2
WEAPON(melee,
	2,		2,		0,		0,		300,		300,		0,		60,		30,		150,		150,		0,		100,		100,		1,		0,		0,		1,		1,		1,		1,		1,		1,		0,		0,
	IMPACT_PLAYER,															IMPACT_PLAYER,
	1,		1,		0,		0,		0,		0,		0,		0,		0,		0,		1,		1,
	0,		0,		0,		0,		1,		1,		0,		0,		0,		0,		6,		6,		2,		4,		100,		500,		25,		25
);
WEAPON(pistol,
	10,		10,		1,		1,		100,		200,		1000,	40,		40,		3000,		3000,		0,		2000,		2000,		0,		0,		0,		1,		1,		1,		1,		1,		1,		16,		16,
	IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE,								IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		0,		0,		1,
	0,		0,		0,		0,		0.05f,		0.05f,		2,		2,		0,		0,		1,		1,		2,		2,		150,		150,		300,		300
);
WEAPON(shotgun,
	1,		8,		1,		2,		500,		750,		1000,	15,		10,		2500,		2000,		0,		300,		100,		0,		0,		0,		20,		40,		25,		20,		1,		2,		2,		2,
	BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|COLLIDE_OWNER,					IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		0,		0,		0,
	0.5f,		0.35f,		50,		50,		0.05f,		0.05f,		2,		2,		25,		25,		1,		1,		15,		15,		20,		40,		150,		300
);
WEAPON(smg,
	40,		40,		1,		5,		75,		300,		1500,		30,		25,		2000,		2000,		0,		500,		500,		0,		0,		0,		1,		5,		5,		5,		4,		2,		4,		4,
	BOUNCE_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|COLLIDE_OWNER,					IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		0,		1,		1,
	0.75f,		0.5f,		30,		30,		0.05f,		0.05f,		2,		2,		0,		0,		1,		1,		0.5f,		3,		100,		120,		300,		400
);
WEAPON(flamer,
	50,		50,		1,		5,		100,		750,		2000,	8,		5,		150,		200,		0,		400,		600,		0,		20,		24,		1,		5,		40,		20,		0,		0,		1,		2,
	BOUNCE_GEOM|IMPACT_PLAYER,												IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
	0,		0,		1,		1,		1,		1,		1,		1,		1,		0,		1,		1,
	0.15f,		0,		45,		0,		0.95f,		0.5f,		1,		1,		-300,		50,		1,		1,		0.25f,		1,		20,		40,		40,		80
);
WEAPON(plasma,
	20,		20,		1,		20,		500,		2000,		3000,	25,		25,		1500,		35,			0,		750,		5000,		0,		16,		40,		1,		1,		5,		5,		0,		0,		6,		2,
	IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,								IMPACT_GEOM|RADIAL_PLAYER|COLLIDE_OWNER|COLLIDE_STICK,
	1,		1,		1,		0,		1,		1,		0,		0,		1,		0,		1,		0,
	0,		0,		0,		0,		0.125f,		0.175f,		1,		1,		0,		0,		1,		1,		3,		6,		50,		200,		200,		50
);
WEAPON(rifle,
	5,		5,		1,		1,		750,		750,		2000,	75,		150,	5000,		50000,		0,		5000,		5000,		0,		16,		0,		1,		1,		1,		0,		0,		0,		2,		1,
	IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER|COLLIDE_TRACE,					IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|COLLIDE_CONT,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		1,		0,		0,
	0,		0,		0,		0,		1,		0,		2,		2,		0,		0,		1,		1,		5,		0,		100,		200,		600,		0
);
WEAPON(grenade,
	1,		2,		1,		1,		1500,		1500,		6000,	300,	300,	250,		250,		3000,	3000,		3000,		100,	60,		60,		1,		1,		0,		0,		0,		0,		1,		1,
	BOUNCE_GEOM|BOUNCE_PLAYER|COLLIDE_OWNER,								IMPACT_GEOM|BOUNCE_PLAYER|COLLIDE_OWNER|COLLIDE_STICK,
	0,		0,		0,		0,		0,		0,		1,		1,		0,		0,		0,		0,
	0.5f,	0,		0,		0,		1,		1,		2,		2,		64,		64,		1,		1,		5,		5,		1000,		1000,		400,		400
);
WEAPON(insta,
	5,		5,		1,		1,		750,		750,		2000,	200,	200,	10000,		50000,		0,		5000,		5000,		0,		0,		0,		1,		1,		0,		0,		0,		0,		1,		1,
	IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE,								IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_TRACE|COLLIDE_CONT,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		1,		0,		0,
	0,		0,		0,		0,		1,		0,		2,		2,		0,		0,		1,		1,		5,		0,		100,		200,		0,		0
);
WEAPON(gibs,
	2,		2,		1,		1,		500,		500,		500,	25,		25,		250,		250,		0,		1500,		1500,		100,	0,		0,		1,		1,		0,		0,		0,		0,		1,		1,
	IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,								IMPACT_GEOM|IMPACT_PLAYER|COLLIDE_OWNER,
	0,		0,		0,		0,		0,		0,		0,		0,		1,		0,		0,		0,
	0.35f,		0.35f,		0,		0,		1,		1,		2,		2,		35,		35,		2,		2,		5,		5,		100,		100,		200,		200
);

struct weaptypes
{
	int	info, 				anim,				colour,			sound, 		esound, 	fsound,		rsound,			espeed;
	bool	follows[2],			muzzle,		eject;
	float	partsize[2],		partlen[2],		thrown[2],				halo,		esize;
	const char *name, 		*text,	*item,						*vwep,						*proj,					*eprj;
};
#ifdef GAMESERVER
weaptypes weaptype[] =
{
	{
		WEAP_MELEE,			ANIM_MELEE,			0xFFFFFF,		S_MELEE,	S_RICOCHET,	-1,			-1,				1,
			{ false, false },	false,		false,
			{ 0.75f, 0.75f },	{ 0, 0 },		{ 0, 0 },				1,			0,
			"melee",		"\fd",	"",							"",							"",						""
	},
	{
		WEAP_PISTOL,		ANIM_PISTOL,		0x888888,		S_PISTOL,	S_BZAP,		S_WHIZZ,	-1,				10,
			{ true, true },		true,		true,
			{ 0.5f, 0.5f },		{ 10, 10 },		{ 0, 0 },				4,			0.35f,
			"pistol",		"\fa",	"weapons/pistol/item",		"weapons/pistol/vwep",		"",						"weapons/cartridge"
	},
	{
		WEAP_SHOTGUN,		ANIM_SHOTGUN,		0xFFFF22,		S_SHOTGUN,	S_BZAP,		S_WHIZZ,	S_RICOCHET,		10,
			{ true, true },		true,		true,
			{ 0.75f, 0.75f },	{ 50, 50 },		{ 0, 0 },				6,			0.45f,
			"shotgun",		"\fy",	"weapons/shotgun/item",		"weapons/shotgun/vwep",		"",						"weapons/shell"
	},
	{
		WEAP_SMG,			ANIM_SMG,			0xFF8822,		S_SMG,		S_BZAP,		S_WHIZZ,	S_RICOCHET,		20,
			{ true, true },		true,		true,
			{ 0.5f, 0.5f },		{ 40, 40 },		{ 0, 0 },				5,			0.35f,
			"smg",			"\fo",	"weapons/smg/item",			"weapons/smg/vwep",			"",						"weapons/cartridge"
	},
	{
		WEAP_FLAMER,		ANIM_FLAMER,		0xFF2222,		S_FLAMER,	S_BURN,		S_BURNING,	-1,				1,
			{ true, true },		true,		false,
			{ 0, 0 },			{ 0, 0 },		{ 0, 0 },				7,			0,
			"flamer",		"\fr",	"weapons/flamer/item",		"weapons/flamer/vwep",		"",						""
	},
	{
		WEAP_PLASMA,		ANIM_PLASMA,		0x22FFFF,		S_PLASMA,	S_ENERGY,	S_HUM,		-1,				1,
			{ true, true },		true,		false,
			{ 0, 0 },			{ 0, 0 },		{ 0, 0 },				5,			0,
			"plasma",		"\fc",	"weapons/plasma/item",		"weapons/plasma/vwep",		"",						""
	},
	{
		WEAP_RIFLE,			ANIM_RIFLE,			0xAA66FF,		S_RIFLE,	S_ENERGY,	S_BZZT,		-1,				1,
			{ false, false },	true,		false,
			{ 0.65f, 1.5f },	{ 512, 4096 },	{ 0, 0 },				7,			0,
			"rifle",		"\fv",	"weapons/rifle/item",		"weapons/rifle/vwep",		"",						""
	},
	{
		WEAP_GRENADE,		ANIM_GRENADE,		0x22FF22,		S_GRENADE,	S_EXPLODE,	S_BEEP,	S_TINK,				1,
			{ true, true },		false,		false,
			{ 2, 2 },			{ 0, 0 },		{ 0.0625f, 0.0625f },	3,			0,
			"grenade",		"\fg",	"weapons/grenade/item",		"weapons/grenade/vwep",		"weapons/grenade/proj",	""
	},
	{
		WEAP_INSTA,			ANIM_RIFLE,			0xAA66FF,		S_RIFLE,	S_ENERGY,	S_BZZT,		-1,				1,
			{ false, false },	true,		false,
			{ 0.65f, 1.5f },	{ 1024, 4096 },	{ 0, 0 },				7,			0,
			"rifle",		"\fv",	"weapons/rifle/item",		"weapons/rifle/vwep",		"",						""
	},
	{
		WEAP_GIBS,			ANIM_GRENADE,		0x660000,		S_SPLOSH,	S_SPLAT,	S_WHIRR,	S_SPLAT,		1,
			{ true, true },		false,		false,
			{ 2, 2 },			{ 0, 0 },		{ 0.125f, 0.125f },		4,			0,
			"gibs",			"\fw",	"gibs/gibc",				"gibs/gibc",				"gibs/gibc",			""
	},
};
#define WP(proto,name)			proto *sv_weap_stat_##name[] = {&sv_melee##name, &sv_pistol##name, &sv_shotgun##name, &sv_smg##name, &sv_flamer##name, &sv_plasma##name, &sv_rifle##name, &sv_grenade##name, &sv_insta##name, &sv_gibs##name};
#define WP2(proto,name)			proto *sv_weap_stat_##name[][2] = {{&sv_melee##name##1,&sv_melee##name##2}, {&sv_pistol##name##1,&sv_pistol##name##2}, {&sv_shotgun##name##1,&sv_shotgun##name##2}, {&sv_smg##name##1,&sv_smg##name##2}, {&sv_flamer##name##1,&sv_flamer##name##2}, {&sv_plasma##name##1,&sv_plasma##name##2}, {&sv_rifle##name##1,&sv_rifle##name##2}, {&sv_grenade##name##1,&sv_grenade##name##2}, {&sv_insta##name##1,&sv_insta##name##2}, {&sv_gibs##name##1,&sv_gibs##name##2}};
#define WPA(weap,name)			(*sv_weap_stat_##name[weap])
#define WPB(weap,name,second)	(*sv_weap_stat_##name[weap][second?1:0])
#define WPS(a,weap,attr)		defformatstring(a)("sv_%s%s", weaptype[weap].name, #attr)
#else
extern weaptypes weaptype[];
#ifdef GAMEWORLD
#define WP(proto,name)			proto *weap_stat_##name[] = {&melee##name, &pistol##name, &shotgun##name, &smg##name, &flamer##name, &plasma##name, &rifle##name, &grenade##name, &insta##name, &gibs##name};
#define WP2(proto,name)			proto *weap_stat_##name[][2] = {{&melee##name##1,&melee##name##2}, {&pistol##name##1,&pistol##name##2}, {&shotgun##name##1,&shotgun##name##2}, {&smg##name##1,&smg##name##2}, {&flamer##name##1,&flamer##name##2}, {&plasma##name##1,&plasma##name##2}, {&rifle##name##1,&rifle##name##2}, {&grenade##name##1,&grenade##name##2}, {&insta##name##1,&insta##name##2}, {&gibs##name##1,&gibs##name##2}};
#else
#define WP(proto,name)			extern proto *weap_stat_##name[];
#define WP2(proto,name)			extern proto *weap_stat_##name[][2];
#endif
#define WPA(weap,name)			(*weap_stat_##name[weap])
#define WPB(weap,name,second)	(*weap_stat_##name[weap][second?1:0])
#define WPS(a,weap,attr)		defformatstring(a)("%s%s", weaptype[weap].name, #attr)
#define FIRECOLOURS 8
const int firecols[FIRECOLOURS] = { 0xFF5808, 0x981808, 0x782808, 0x481808, 0x983818, 0x601808, 0xFF1808, 0x381808 };
#endif

WP(int, add); WP(int, max); WP2(int, sub); WP2(int, adelay); WP(int, rdelay); WP2(int, damage); WP2(int, speed); WP(int, power);
WP2(int, time); WP(int, pdelay); WP2(int, explode); WP2(int, rays); WP2(int, spread); WP2(int, zdiv); WP2(int, aiskew); WP2(int, collide);
WP2(int, taper); WP2(int, extinguish); WP2(int, radial); WP2(int, burns); WP(int, reloads); WP(int, zooms); WP2(int, fullauto);
WP2(float, elasticity); WP2(float, reflectivity); WP2(float, relativity); WP2(float, waterfric); WP2(float, weight); WP2(float, radius);
WP2(float, kickpush); WP2(float, hitpush); WP2(float, maxdist);

enum
{
	HIT_NONE = 0, HIT_ALT = 1<<0, HIT_LEGS = 1<<1, HIT_TORSO = 1<<2, HIT_HEAD = 1<<3, HIT_FULL = 1<<4, HIT_PROJ = 1<<5,
	HIT_EXPLODE = 1<<6, HIT_BURN = 1<<7, HIT_MELT = 1<<8, HIT_DEATH = 1<<9, HIT_WATER = 1<<10, HIT_WAVE = 1<<11, HIT_SPAWN = 1<<12,
	HIT_LOST = 1<<13, HIT_KILL = 1<<14, HIT_SFLAGS = HIT_KILL
};

#define hithurts(x) 	(x&HIT_BURN || x&HIT_EXPLODE || x&HIT_PROJ || x&HIT_MELT || x&HIT_DEATH || x&HIT_WATER)
#define doesburn(x,y)	(isweap(x) && WPB(x, burns, y&HIT_ALT) && y&HIT_FULL)
enum
{
	FRAG_NONE = 0, FRAG_HEADSHOT = 1<<1, FRAG_OBLITERATE = 1<<2,
	FRAG_SPREE1 = 1<<3, FRAG_SPREE2 = 1<<4, FRAG_SPREE3 = 1<<5, FRAG_SPREE4 = 1<<6, FRAG_SPREE5 = 1<<7, FRAG_SPREE6 = 1<<8,
	FRAG_MKILL1 = 1<<9, FRAG_MKILL2 = 1<<10, FRAG_MKILL3 = 1<<11, FRAG_REVENGE = 1<<12, FRAG_DOMINATE = 1<<13,
	FRAG_SPREES = 6, FRAG_SPREE = 3, FRAG_MKILL = 9,
	FRAG_CHECK = FRAG_SPREE1|FRAG_SPREE2|FRAG_SPREE3|FRAG_SPREE4|FRAG_SPREE5|FRAG_SPREE6,
	FRAG_MULTI = FRAG_MKILL1|FRAG_MKILL2|FRAG_MKILL3,
};

enum
{
	G_DEMO = 0, G_LOBBY, G_EDITMODE, G_CAMPAIGN, G_DEATHMATCH, G_STF, G_CTF, G_TRIAL, G_MAX,
	G_START = G_LOBBY, G_PLAY = G_CAMPAIGN, G_FIGHT = G_DEATHMATCH, G_RAND = G_CTF-G_DEATHMATCH+1
};
enum
{
	G_M_NONE = 0, G_M_MULTI = 1<<0, G_M_TEAM = 1<<1, G_M_INSTA = 1<<2, G_M_DUEL = 1<<3, G_M_SURVIVOR= 1<<4, G_M_ARENA = 1<<5,
	G_M_ALL = G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA, G_M_SOME = G_M_INSTA|G_M_ARENA,
	G_M_NUM = 6
};

struct gametypes
{
	int	type,			mutators,															implied;		const char *name;
};
#ifdef GAMESERVER
gametypes gametype[] = {
	{ G_DEMO,			G_M_NONE,															G_M_NONE,				"demo" },
	{ G_LOBBY,			G_M_SOME,															G_M_NONE,				"lobby" },
	{ G_EDITMODE,		G_M_SOME,															G_M_NONE,				"editing" },
	{ G_CAMPAIGN,		G_M_TEAM|G_M_SOME,													G_M_TEAM|G_M_ARENA,		"campaign" },
	{ G_DEATHMATCH,		G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA,		G_M_NONE,				"deathmatch" },
	{ G_STF,			G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA,								G_M_TEAM,				"secure-the-flag" },
	{ G_CTF,			G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA,								G_M_TEAM,				"capture-the-flag" },
	{ G_TRIAL,			G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_ARENA,								G_M_NONE,				"time-trial" },
}, mutstype[] = {
	{ G_M_MULTI,		G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA,		G_M_TEAM|G_M_MULTI,		"multi" },
	{ G_M_TEAM,			G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA,		G_M_TEAM,				"team" },
	{ G_M_INSTA,		G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA,		G_M_INSTA,				"insta" },
	{ G_M_DUEL,			G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_ARENA,					G_M_DUEL,				"duel" },
	{ G_M_SURVIVOR,		G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_SURVIVOR|G_M_ARENA,				G_M_SURVIVOR,			"survivor" },
	{ G_M_ARENA,		G_M_MULTI|G_M_TEAM|G_M_INSTA|G_M_DUEL|G_M_SURVIVOR|G_M_ARENA,		G_M_ARENA,				"arena" },
};
#else
extern gametypes gametype[], mutstype[];
#endif

#define m_game(a)			(a > -1 && a < G_MAX)
#define m_check(a,b)		(!a || (a < 0 ? -a != b : a == b))

#define m_demo(a)			(a == G_DEMO)
#define m_lobby(a)			(a == G_LOBBY)
#define m_edit(a)			(a == G_EDITMODE)
#define m_campaign(a)		(a == G_CAMPAIGN)
#define m_dm(a)				(a == G_DEATHMATCH)
#define m_stf(a)			(a == G_STF)
#define m_ctf(a)			(a == G_CTF)
#define m_trial(a)			(a == G_TRIAL)

#define m_play(a)			(a >= G_PLAY)
#define m_flag(a)			(m_stf(a) || m_ctf(a))
#define m_fight(a)			(a >= G_FIGHT)

#define m_multi(a,b)		((b & G_M_MULTI) || (gametype[a].implied & G_M_MULTI))
#define m_team(a,b)			((b & G_M_TEAM) || (gametype[a].implied & G_M_TEAM))
#define m_insta(a,b)		((b & G_M_INSTA) || (gametype[a].implied & G_M_INSTA))
#define m_duel(a,b)			((b & G_M_DUEL) || (gametype[a].implied & G_M_DUEL))
#define m_survivor(a,b)		((b & G_M_SURVIVOR) || (gametype[a].implied & G_M_SURVIVOR))
#define m_arena(a,b)		((b & G_M_ARENA) || (gametype[a].implied & G_M_ARENA))

#define m_duke(a,b)			(m_duel(a, b) || m_survivor(a, b))
#define m_regen(a,b)		(!m_duke(a,b) && !m_insta(a,b))

#define m_weapon(a,b)		(m_arena(a,b) ? -1 : (m_edit(a) || m_trial(a) ? GVAR(trialweapon) : (m_insta(a,b) ? GVAR(instaweapon) : GVAR(spawnweapon))))
#define m_delay(a,b)		(m_play(a) && !m_duke(a,b) ? (m_trial(a) ? GVAR(trialdelay) : ((m_insta(a, b) ? GVAR(instadelay) : GVAR(spawndelay)))) : 0)
#define m_protect(a,b)		(m_insta(a, b) || m_arena(a, b) ? GVAR(instaprotect) : GVAR(spawnprotect))
#define m_noitems(a,b)		(GVAR(itemsallowed) < (m_insta(a,b) ? (m_arena(a,b) ? 2 : 3) : (m_trial(a) ? 3 : 1)))
#define m_health(a,b)		(m_insta(a,b) ? 1 : GVAR(maxhealth))

#define w_reload(w1,w2)		(w1 != WEAP_MELEE && (w1 == (isweap(w1) ? w2 : WEAP_PISTOL) || (isweap(w1) && WPA(w1, reloads))))
#define w_carry(w1,w2)		(w1 != WEAP_MELEE && w1 != (isweap(w1) ? w2 : WEAP_PISTOL) && (isweap(w1) && WPA(w1, reloads)))
#define w_attr(a,w1,w2)		(m_edit(a) || (w1 >= WEAP_OFFSET && w1 != (isweap(w2) ? w2 : WEAP_PISTOL)) ? w1 : WEAP_GRENADE)

// network messages codes, c2s, c2c, s2c
enum
{
	SV_CONNECT = 0, SV_SERVERINIT, SV_WELCOME, SV_CLIENTINIT, SV_POS, SV_PHYS, SV_TEXT, SV_COMMAND, SV_ANNOUNCE, SV_DISCONNECT,
	SV_SHOOT, SV_DESTROY, SV_SUICIDE, SV_DIED, SV_POINTS, SV_DAMAGE, SV_SHOTFX,
	SV_LOADWEAP, SV_TRYSPAWN, SV_SPAWNSTATE, SV_SPAWN,
	SV_DROP, SV_WEAPSELECT,
	SV_MAPCHANGE, SV_MAPVOTE, SV_CHECKPOINT, SV_ITEMSPAWN, SV_ITEMUSE, SV_TRIGGER, SV_EXECLINK,
	SV_PING, SV_PONG, SV_CLIENTPING,
	SV_TIMEUP, SV_NEWGAME, SV_ITEMACC,
	SV_SERVMSG, SV_GAMEINFO, SV_RESUME,
	SV_EDITMODE, SV_EDITENT, SV_EDITLINK, SV_EDITVAR, SV_EDITF, SV_EDITT, SV_EDITM, SV_FLIP, SV_COPY, SV_PASTE, SV_ROTATE, SV_REPLACE, SV_DELCUBE, SV_REMIP, SV_NEWMAP,
	SV_GETMAP, SV_SENDMAP, SV_SENDMAPFILE, SV_SENDMAPSHOT, SV_SENDMAPCONFIG,
	SV_MASTERMODE, SV_KICK, SV_CLEARBANS, SV_CURRENTMASTER, SV_SPECTATOR, SV_WAITING, SV_SETMASTER, SV_SETTEAM,
	SV_FLAGS, SV_FLAGINFO,
	SV_TAKEFLAG, SV_RETURNFLAG, SV_RESETFLAG, SV_DROPFLAG, SV_SCOREFLAG, SV_INITFLAGS, SV_SCORE,
	SV_LISTDEMOS, SV_SENDDEMOLIST, SV_GETDEMO, SV_SENDDEMO,
	SV_DEMOPLAYBACK, SV_RECORDDEMO, SV_STOPDEMO, SV_CLEARDEMOS,
	SV_CLIENT, SV_RELOAD, SV_REGEN,
	SV_ADDBOT, SV_DELBOT, SV_INITAI,
    SV_MAPCRC, SV_CHECKMAPS,
	SV_SWITCHNAME, SV_SWITCHTEAM,
	SV_AUTHTRY, SV_AUTHCHAL, SV_AUTHANS,
    NUMSV
};

#ifdef GAMESERVER
char msgsizelookup(int msg)
{
	static const int msgsizes[] =				// size inclusive message token, 0 for variable or not-checked sizes
	{
		SV_CONNECT, 0, SV_SERVERINIT, 5, SV_WELCOME, 1, SV_CLIENTINIT, 0, SV_POS, 0, SV_PHYS, 0, SV_TEXT, 0, SV_COMMAND, 0,
		SV_ANNOUNCE, 0, SV_DISCONNECT, 2,
		SV_SHOOT, 0, SV_DESTROY, 0, SV_SUICIDE, 3, SV_DIED, 8, SV_POINTS, 4, SV_DAMAGE, 10, SV_SHOTFX, 0,
		SV_LOADWEAP, 0, SV_TRYSPAWN, 2, SV_SPAWNSTATE, 0, SV_SPAWN, 0,
		SV_DROP, 0, SV_WEAPSELECT, 0,
		SV_MAPCHANGE, 0, SV_MAPVOTE, 0, SV_CHECKPOINT, 0, SV_ITEMSPAWN, 2, SV_ITEMUSE, 0, SV_TRIGGER, 0, SV_EXECLINK, 3,
		SV_PING, 2, SV_PONG, 2, SV_CLIENTPING, 2,
		SV_TIMEUP, 2, SV_NEWGAME, 1, SV_ITEMACC, 0,
		SV_SERVMSG, 0, SV_GAMEINFO, 0, SV_RESUME, 0,
		SV_EDITMODE, 2, SV_EDITENT, 0, SV_EDITLINK, 4, SV_EDITVAR, 0, SV_EDITF, 16, SV_EDITT, 16, SV_EDITM, 15, SV_FLIP, 14, SV_COPY, 14, SV_PASTE, 14, SV_ROTATE, 15, SV_REPLACE, 16, SV_DELCUBE, 14, SV_REMIP, 1, SV_NEWMAP, 2,
		SV_GETMAP, 0, SV_SENDMAP, 0, SV_SENDMAPFILE, 0, SV_SENDMAPSHOT, 0, SV_SENDMAPCONFIG, 0,
		SV_MASTERMODE, 2, SV_KICK, 2, SV_CLEARBANS, 1, SV_CURRENTMASTER, 3, SV_SPECTATOR, 3, SV_WAITING, 2, SV_SETMASTER, 0, SV_SETTEAM, 0,
		SV_FLAGS, 0, SV_FLAGINFO, 0,
		SV_DROPFLAG, 0, SV_SCOREFLAG, 5, SV_RETURNFLAG, 3, SV_TAKEFLAG, 3, SV_RESETFLAG, 2, SV_INITFLAGS, 0, SV_SCORE, 0,
		SV_LISTDEMOS, 1, SV_SENDDEMOLIST, 0, SV_GETDEMO, 2, SV_SENDDEMO, 0,
		SV_DEMOPLAYBACK, 3, SV_RECORDDEMO, 2, SV_STOPDEMO, 1, SV_CLEARDEMOS, 2,
		SV_CLIENT, 0, SV_RELOAD, 0, SV_REGEN, 0,
		SV_ADDBOT, 0, SV_DELBOT, 0, SV_INITAI, 0,
        SV_MAPCRC, 0, SV_CHECKMAPS, 1,
		SV_SWITCHNAME, 0, SV_SWITCHTEAM, 0,
		SV_AUTHTRY, 0, SV_AUTHCHAL, 0, SV_AUTHANS, 0,
		-1
	};
	static int sizetable[NUMSV] = { -1 };
	if(sizetable[0] < 0)
	{
		memset(sizetable, -1, sizeof(sizetable));
		for(const int *p = msgsizes; *p >= 0; p += 2) sizetable[p[0]] = p[1];
	}
	return msg >= 0 && msg < NUMSV ? sizetable[msg] : -1;
}
#else
extern char msgsizelookup(int msg);
#endif
enum { SPHY_NONE = 0, SPHY_JUMP, SPHY_IMPULSE, SPHY_POWER, SPHY_EXTINGUISH, SPHY_MAX };
enum { CON_CHAT = CON_GAMESPECIFIC, CON_EVENT, CON_MAX, CON_LO = CON_MESG, CON_HI = CON_SELF, CON_IMPORTANT = CON_SELF };

#define DEMO_MAGIC "BFDZ"
struct demoheader
{
	char magic[16];
	int version, gamever;
};

enum
{
	TEAM_NEUTRAL = 0, TEAM_ALPHA, TEAM_BETA, TEAM_GAMMA, TEAM_DELTA, TEAM_ENEMY, TEAM_MAX,
	TEAM_FIRST = TEAM_ALPHA, TEAM_LAST = TEAM_DELTA, TEAM_NUM = TEAM_LAST-(TEAM_FIRST-1)
};
struct teamtypes
{
	int	type,			colour;	const char	*name,
		*tpmdl,								*fpmdl,
		*flag,			*icon,				*chat,		*colname;
};
#ifdef GAMESERVER
teamtypes teamtype[] = {
	{
		TEAM_NEUTRAL,	0x666666,			"neutral",
		"actors/player",					"actors/player/vwep",
		"flag",			"team",				"\fa",		"grey"
	},
	{
		TEAM_ALPHA,		0x2222AA,			"alpha",
		"actors/player/alpha",				"actors/player/alpha/vwep",
		"flag/alpha",	"teamalpha",		"\fb",		"blue"
	},
	{
		TEAM_BETA,		0xAA2222,			"beta",
		"actors/player/beta",				"actors/player/beta/vwep",
		"flag/beta",	"teambeta",			"\fr",		"red"
	},
	{
		TEAM_GAMMA,		0x22AA22,			"gamma",
		"actors/player/gamma",				"actors/player/gamma/vwep",
		"flag/gamma",	"teamgamma",		"\fg",		"green"
	},
	{
		TEAM_DELTA,		0xAAAA22,			"delta",
		"actors/player/delta",				"actors/player/delta/vwep",
		"flag/delta",	"teamdelta",		"\fy",		"yellow"
	},
	{
		TEAM_ENEMY,		0x222222,			"enemy",
		"actors/player",					"actors/player/vwep",
		"flag",			"team",				"\fd",		"dark grey"
	}
};
#else
extern teamtypes teamtype[];
#endif
enum { BASE_NONE = 0, BASE_HOME = 1<<0, BASE_FLAG = 1<<1, BASE_BOTH = BASE_HOME|BASE_FLAG };

#define numteams(a,b)	(m_fight(a) && m_team(a,b) ? (m_multi(a,b) ? TEAM_NUM : TEAM_NUM/2) : 1)
#define isteam(a,b,c,d)	(m_fight(a) && m_team(a,b) ? (c >= d && c <= numteams(a,b)) : c == TEAM_NEUTRAL)
#define valteam(a,b)	(a >= b && a <= TEAM_NUM)
#define adjustscaled(t,n,s) { \
	if(n > 0) { n = (t)(n/(1.f+sqrtf((float)curtime)/float(s))); if(n <= 0) n = (t)0; } \
	else if(n < 0) { n = (t)(n/(1.f+sqrtf((float)curtime)/float(s))); if(n >= 0) n = (t)0; } \
}

#define MAXNAMELEN 24
enum { SAY_NONE	= 0, SAY_ACTION = 1<<0, SAY_TEAM = 1<<1, SAY_NUM = 2 };
enum { PRIV_NONE = 0, PRIV_MASTER, PRIV_ADMIN, PRIV_MAX };

#define MM_MODE 0xF
#define MM_AUTOAPPROVE 0x1000
#define MM_FREESERV (MM_AUTOAPPROVE|MM_MODE)
#define MM_VETOSERV ((1<<MM_OPEN)|(1<<MM_VETO))
#define MM_COOPSERV (MM_AUTOAPPROVE|MM_VETOSERV|(1<<MM_LOCKED))
#define MM_OPENSERV (MM_AUTOAPPROVE|(1<<MM_OPEN))

enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
enum { CAMERA_NONE = 0, CAMERA_PLAYER, CAMERA_FOLLOW, CAMERA_ENTITY, CAMERA_MAX };
enum { SINFO_STATUS = 0, SINFO_NAME, SINFO_PORT, SINFO_QPORT, SINFO_DESC, SINFO_MODE, SINFO_MUTS, SINFO_MAP, SINFO_TIME, SINFO_NUMPLRS, SINFO_MAXPLRS, SINFO_PING, SINFO_MAX };
enum { SSTAT_OPEN = 0, SSTAT_LOCKED, SSTAT_PRIVATE, SSTAT_FULL, SSTAT_UNKNOWN, SSTAT_MAX };

enum { AC_ATTACK = 0, AC_ALTERNATE, AC_RELOAD, AC_USE, AC_JUMP, AC_SPRINT, AC_CROUCH, AC_SPECIAL, AC_TOTAL, AC_DASH = AC_TOTAL, AC_MAX };
enum { IM_METER = 0, IM_TYPE, IM_TIME, IM_COUNT, IM_MAX };
enum { IM_T_NONE = 0, IM_T_BOOST, IM_T_DASH, IM_T_KICK, IM_T_SKATE, IM_T_MAX, IM_T_WALL = IM_T_KICK };
#define CROUCHHEIGHT 0.7f
#define PHYSMILLIS 250

#include "ai.h"

// inherited by gameent and server clients
struct gamestate
{
	int health, ammo[WEAP_MAX], entid[WEAP_MAX];
	int lastweap, loadweap, weapselect, weapload[WEAP_MAX], weapshot[WEAP_MAX], weapstate[WEAP_MAX], weapwait[WEAP_MAX], weaplast[WEAP_MAX];
	int lastdeath, lastspawn, lastrespawn, lastpain, lastregen, lastfire;
	int aitype, aientity, ownernum, skill, points, cpmillis, cptime;

	gamestate() : loadweap(-1), weapselect(WEAP_MELEE), lastdeath(0), lastspawn(0), lastrespawn(0), lastpain(0), lastregen(0), lastfire(0),
		aitype(-1), aientity(-1), ownernum(-1), skill(0), points(0), cpmillis(0), cptime(0) {}
	~gamestate() {}

	int hasweap(int weap, int sweap, int level = 0, int exclude = -1)
	{
		if(isweap(weap) && weap != exclude)
		{
			if(ammo[weap] > 0 || (w_reload(weap, sweap) && !ammo[weap])) switch(level)
			{
				case 0: default: return true; break; // has weap at all
				case 1: if(w_carry(weap, sweap)) return true; break; // only carriable
				case 2: if(ammo[weap] > 0) return true; break; // only with actual ammo
				case 3: if(ammo[weap] > 0 && w_reload(weap, sweap)) return true; break; // only reloadable with actual ammo
				case 4: if(ammo[weap] >= (w_reload(weap, sweap) ? 0 : WPA(weap, max))) return true; break; // only reloadable or those with < max
				case 5: if(w_carry(weap, sweap) || (!w_reload(weap, sweap) && weap >= WEAP_OFFSET)) return true; break; // special case for usable weapons
			}
		}
		return false;
	}

	int bestweap(int sweap, bool last = false)
	{
		if(last && hasweap(lastweap, sweap)) return lastweap;
		loopirev(WEAP_MAX) if(hasweap(i, sweap, 3)) return i; // reloadable first
		loopirev(WEAP_MAX) if(hasweap(i, sweap, 1)) return i; // carriable second
		loopirev(WEAP_MAX) if(hasweap(i, sweap, 0)) return i; // any just to bail us out
		return weapselect;
	}

	int carry(int sweap, int level = 1, int exclude = -1)
	{
		int carry = 0;
		loopi(WEAP_MAX) if(hasweap(i, sweap, level, exclude)) carry++;
		return carry;
	}

	int drop(int sweap, int exclude = -1)
	{
		int weap = -1;
		if(hasweap(weapselect, sweap, 1, exclude)) weap = weapselect;
		else loopi(WEAP_MAX) if(hasweap(i, sweap, 1, exclude))
		{
			weap = i;
			break;
		}
		return weap;
	}

	void weapreset(bool full = false)
	{
		loopi(WEAP_MAX)
		{
			weapstate[i] = WEAP_S_IDLE;
			weapwait[i] = weaplast[i] = weapload[i] = weapshot[i] = 0;
			if(full) ammo[i] = entid[i] = -1;
		}
		if(full) { lastweap = -1; weapselect = WEAP_MELEE; }
	}

	void setweapstate(int weap, int state, int delay, int millis)
	{
		weapstate[weap] = state;
		weapwait[weap] = delay;
		weaplast[weap] = millis;
	}

	void weapswitch(int weap, int millis, int state = WEAP_S_SWITCH)
	{
		int delay = state == WEAP_S_PICKUP ? WEAPPICKUPDELAY : WEAPSWITCHDELAY;
		lastweap = weapselect;
		setweapstate(lastweap, state, delay, millis);
		weapselect = weap;
		setweapstate(weap, state, delay, millis);
	}

	bool weapwaited(int weap, int millis, int skip = 0)
	{
		if(!weapwait[weap] || weapstate[weap] == WEAP_S_IDLE || weapstate[weap] == WEAP_S_POWER || (skip && skip&(1<<weapstate[weap]))) return true;
		return millis-weaplast[weap] >= weapwait[weap];
	}

	int skipwait(int weap, int flags, int millis, int skip, bool override = false)
	{
		int skipstate = skip;
		if(WPB(weap, sub, flags&HIT_ALT) && (skip&(1<<WEAP_S_RELOAD)) && weapstate[weap] == WEAP_S_RELOAD && millis-weaplast[weap] < weapwait[weap])
		{
			if(!override && ammo[weap]-weapload[weap] < WPB(weap, sub, flags&HIT_ALT))
				skipstate &= ~(1<<WEAP_S_RELOAD);
		}
		return skipstate;
	}

	bool canswitch(int weap, int sweap, int millis, int skip = 0)
	{
		if(weap != weapselect && weapwaited(weapselect, millis, skipwait(weapselect, 0, millis, skip, true)) && hasweap(weap, sweap) && weapwaited(weap, millis, skipwait(weap, 0, millis, skip, true)))
			return true;
		return false;
	}

	bool canshoot(int weap, int flags, int sweap, int millis, int skip = 0)
	{
		if(hasweap(weap, sweap) && ammo[weap] >= WPB(weap, sub, flags&HIT_ALT) && weapwaited(weap, millis, skipwait(weap, flags, millis, skip)))
			return true;
		return false;
	}

	bool canreload(int weap, int sweap, int millis)
	{
		if(weap == weapselect && w_reload(weap, sweap) && hasweap(weap, sweap) && ammo[weap] < WPA(weap, max) && weapwaited(weap, millis))
			return true;
		return false;
	}

	bool canuse(int type, int attr, vector<int> &attrs, int sweap, int millis, int skip = 0)
	{
		//if((type != TRIGGER || attrs[2] == TA_AUTO) && enttype[type].usetype == EU_AUTO) return true;
		//if(weapwaited(weapselect, millis, skipwait(weapselect, 0, millis, skip)))
		switch(enttype[type].usetype)
		{
			case EU_AUTO: case EU_ACT: return true; break;
			case EU_ITEM:
			{ // can't use when reloading or firing
				if(isweap(attr) && !hasweap(attr, sweap, 4) && weapwaited(weapselect, millis, skipwait(weapselect, 0, millis, skip, true)))
					return true;
				break;
			}
			default: break;
		}
		return false;
	}

	void useitem(int id, int type, int attr, vector<int> &attrs, int sweap, int millis)
	{
		switch(type)
		{
			case TRIGGER: break;
			case WEAPON:
			{
				int prev = ammo[attr];
				weapswitch(attr, millis, hasweap(attr, sweap) ? (weapselect != attr ? WEAP_S_SWITCH : WEAP_S_RELOAD) : WEAP_S_PICKUP);
				ammo[attr] = clamp((ammo[attr] > 0 ? ammo[attr] : 0)+WPA(attr, add), 1, WPA(attr, max));
				weapload[attr] = ammo[attr]-prev;
				entid[attr] = id;
				break;
			}
			default: break;
		}
	}

	void clearstate()
	{
		lastdeath = lastpain = lastregen = lastfire = 0;
		lastrespawn = -1;
	}

	void mapchange()
	{
		points = cpmillis = cptime = 0;
	}

	void respawn(int millis, int heal)
	{
		health = heal;
		lastspawn = millis;
		clearstate();
		weapreset(true);
	}

	void spawnstate(int sweap, int heal, bool arena = false, bool grenades = false)
	{
		health = heal;
		weapreset(true);
		if(!isweap(sweap)) sweap = WEAP_PISTOL;
		if(sweap != WEAP_MELEE) ammo[WEAP_MELEE] = WPA(WEAP_MELEE, max);
		ammo[sweap] = WPA(sweap, reloads) ? WPA(sweap, add) : WPA(sweap, max);
		if(arena)
		{
			int aweap = loadweap;
			while(aweap < WEAP_OFFSET || aweap >= WEAP_SUPER || aweap == WEAP_GRENADE) aweap = rnd(WEAP_SUPER-WEAP_OFFSET)+WEAP_OFFSET; // pistol = random
			ammo[aweap] = WPA(aweap, reloads) ? WPA(aweap, add) : WPA(aweap, max);
			lastweap = weapselect = aweap;
		}
		else
		{
			loadweap = -1;
			lastweap = weapselect = sweap;
		}
		if(grenades && sweap != WEAP_GRENADE) ammo[WEAP_GRENADE] = WPA(WEAP_GRENADE, max);
	}

	void editspawn(int millis, int sweap, int heal, bool arena = false, bool grenades = false)
	{
		clearstate();
		spawnstate(sweap, heal, arena, grenades);
	}

	int respawnwait(int millis, int delay)
	{
		return lastdeath ? max(0, delay-(millis-lastdeath)) : 0;
	}

	// just subtract damage here, can set death, etc. later in code calling this
	void dodamage(int heal)
	{
		lastregen = 0;
		health = heal;
	}

	int protect(int millis, int delay)
	{
		int amt = 0;
		if(lastspawn && delay && millis-lastspawn <= delay) amt = delay-(millis-lastspawn);
		return amt;
	}
};

#if !defined(GAMESERVER) && !defined(STANDALONE)
struct gameentity : extentity
{
	int schan;
	int lastuse, lastspawn;
	int mark;
	vector<int> kin;

	gameentity() : schan(-1), lastuse(0), lastspawn(0), mark(0) { kin.setsize(0); }
	~gameentity()
	{
		if(issound(schan)) removesound(schan);
		schan = -1;
		kin.setsize(0);
	}
};

enum
{
	ST_NONE		= 0,
	ST_CAMERA	= 1<<0,
	ST_CURSOR	= 1<<1,
	ST_GAME		= 1<<2,
	ST_SPAWNS	= 1<<3,
	ST_DEFAULT	= ST_CAMERA|ST_CURSOR|ST_GAME,
	ST_VIEW		= ST_CURSOR|ST_CAMERA,
	ST_ALL		= ST_CAMERA|ST_CURSOR|ST_GAME|ST_SPAWNS,
};

enum { ITEM_ENT = 0, ITEM_PROJ, ITEM_MAX };
struct actitem
{
	int type, target;
	float score;

	actitem() : type(ITEM_ENT), target(-1), score(0.f) {}
	~actitem() {}
};
#ifdef GAMEWORLD
const char *animnames[] =
{
	"idle", "forward", "backward", "left", "right", "dead", "dying", "swim",
	"mapmodel", "trigger on", "trigger off", "pain", "jump",
	"impulse forward", "impulse backward", "impulse left", "impulse right", "impulse dash",
	"sink", "edit", "lag", "switch", "pickup", "win", "lose",
	"crouch", "crawl forward", "crawl backward", "crawl left", "crawl right",
	"melee", "melee attack",
	"pistol", "pistol shoot", "pistol reload",
	"shotgun", "shotgun shoot", "shotgun reload",
	"smg", "smg shoot", "smg reload",
	"grenade", "grenade throw", "grenade reload", "grenade power",
	"flamer", "flamer shoot", "flamer reload",
	"plasma", "plasma shoot", "plasma reload",
	"rifle", "rifle shoot", "rifle reload",
	"vwep", "shield", "powerup",
	""
};
#else
extern const char *animnames[];
#endif

struct gameent : dynent, gamestate
{
	editinfo *edit; ai::aiinfo *ai;
	int team, clientnum, privilege, lastnode, checkpoint, cplast, respawned, suicided, lastupdate, lastpredict, plag, ping, lastflag, frags, deaths, totaldamage, totalshots,
		actiontime[AC_MAX], impulse[IM_MAX], smoothmillis, turnmillis, turnside, aschan, vschan, wschan, fschan, lasthit, lastkill, lastattacker, lastpoints, quake, lastpush;
	float deltayaw, deltapitch, newyaw, newpitch, deltaaimyaw, deltaaimpitch, newaimyaw, newaimpitch, turnyaw, turnroll;
	vec head, torso, muzzle, eject, melee, waist, lfoot, rfoot, legs, hrad, trad, lrad;
	bool action[AC_MAX], conopen, dominating, dominated, k_up, k_down, k_left, k_right;
	string name, info, obit;
	vector<int> airnodes;

	gameent() : edit(NULL), ai(NULL), team(TEAM_NEUTRAL), clientnum(-1), privilege(PRIV_NONE), checkpoint(-1), cplast(0), lastupdate(0), lastpredict(0), plag(0), ping(0),
		frags(0), deaths(0), totaldamage(0), totalshots(0), smoothmillis(-1), turnmillis(0), aschan(-1), vschan(-1), wschan(-1), fschan(-1),
		lastattacker(-1), lastpoints(0), quake(0), lastpush(0),
		head(-1, -1, -1), torso(-1, -1, -1), muzzle(-1, -1, -1), eject(-1, -1, -1), melee(-1, -1, -1), waist(-1, -1, -1),
		lfoot(-1, -1, -1), rfoot(-1, -1, -1), legs(-1, -1, -1), hrad(-1, -1, -1), trad(-1, -1, -1), lrad(-1, -1, -1),
		conopen(false), dominating(false), dominated(false), k_up(false), k_down(false), k_left(false), k_right(false)
	{
		name[0] = info[0] = obit[0] = 0;
		weight = 200; // so we can control the 'gravity' feel
		maxspeed = 50; // ditto for movement
		checktags();
		respawn(-1, 100);
	}
	~gameent()
	{
		removesounds();
		freeeditinfo(edit);
		if(ai) delete ai;
		removetrackedparticles(this);
		removetrackedsounds(this);
	}

	void removesounds()
	{
		if(issound(aschan)) removesound(aschan);
		if(issound(vschan)) removesound(vschan);
		if(issound(wschan)) removesound(wschan);
		if(issound(fschan)) removesound(fschan);
		aschan = vschan = wschan = fschan = -1;
	}

	void stopmoving(bool full)
	{
		if(full) move = strafe = 0;
		loopi(AC_MAX)
		{
			action[i] = false;
			actiontime[i] = 0;
		}
	}

	void clearstate()
	{
		loopi(IM_MAX) impulse[i] = 0;
		cplast = lasthit = lastkill = quake = lastpush = turnmillis = turnside = 0;
		turnroll = turnyaw = 0;
		lastflag = respawned = suicided = lastnode = -1;
		obit[0] = 0;
	}

	void respawn(int millis, int heal)
	{
		stopmoving(true);
		removesounds();
		clearstate();
		physent::reset();
		gamestate::respawn(millis, heal);
		airnodes.setsizenodelete(0);
	}

	void editspawn(int millis, int sweap, int heal, bool arena = false, bool grenades = false)
	{
		stopmoving(true);
		clearstate();
		inmaterial = timeinair = 0;
		inliquid = onladder = false;
		strafe = move = 0;
		physstate = PHYS_FALL;
		vel = falling = vec(0, 0, 0);
		floor = vec(0, 0, 1);
		resetinterp();
		gamestate::editspawn(millis, sweap, heal, arena, grenades);
		airnodes.setsizenodelete(0);
	}

	void resetstate(int millis, int heal)
	{
		respawn(millis, heal);
		frags = deaths = totaldamage = totalshots = 0;
	}

	void mapchange(int millis, int heal)
	{
		checkpoint = -1;
		dominating = dominated = false;
		resetstate(millis, heal);
		gamestate::mapchange();
	}

	void cleartags() { head = torso = muzzle = eject = melee = waist = lfoot = rfoot = vec(-1, -1, -1); }

	void checkmeleepos()
	{
		if(melee == vec(-1, -1, -1))
		{
			vec dir; vecfromyawpitch(yaw, pitch, 1, 0, dir);
			dir.mul(radius); dir.z -= height*0.0625f;
			melee = vec(o).add(dir);
		}
	}

	void checkmuzzlepos()
	{
		if(muzzle == vec(-1, -1, -1))
		{
			vec dir, right; vecfromyawpitch(yaw, pitch, 1, 0, dir); vecfromyawpitch(yaw, pitch, 0, -1, right);
			dir.mul(radius); right.mul(radius); dir.z -= height*0.0625f;
			muzzle = vec(o).add(dir).add(right);
		}
	}

	vec &muzzlepos(int weap)
	{
		if(isweap(weap) && weaptype[weap].muzzle)
		{
			checkmuzzlepos();
			return muzzle;
		}
		checkmeleepos();
		return melee;
	}

	void checkejectpos()
	{
		if(eject == vec(-1, -1, -1))
		{
			checkmuzzlepos();
			eject = muzzle;
		}
	}

	vec &ejectpos(int weap)
	{
		if(isweap(weap) && weaptype[weap].eject)
		{
			checkejectpos();
			return eject;
		}
		return muzzlepos(weap);
	}

	void checkhitboxes()
	{
		float hsize = max(xradius*0.45f, yradius*0.45f); if(head == vec(-1, -1, -1)) { torso = head; head = o; head.z -= hsize; }
		vec dir; vecfromyawpitch(yaw, pitch+90, 1, 0, dir); dir.mul(hsize); head.add(dir); hrad = vec(xradius*0.45f, yradius*0.45f, hsize);
		if(torso == vec(-1, -1, -1)) { torso = o; torso.z -= height*0.5f; } torso.z += hsize*0.5f;
		float tsize = (head.z-hrad.z)-torso.z; trad = vec(xradius, yradius, tsize);
		float lsize = ((torso.z-trad.z)-(o.z-height))*0.5f; legs = torso; legs.z -= trad.z+lsize; lrad = vec(xradius*0.8f, yradius*0.8f, lsize);
		if(waist == vec(-1, -1, -1))
		{
			vecfromyawpitch(yaw, 0, -1, 0, dir); dir.mul(radius*1.15f); dir.z -= height*0.5f;
			waist = vec(o).add(dir);
		}
		if(lfoot == vec(-1, -1, -1))
		{
			vecfromyawpitch(yaw, 0, 0, -1, dir); dir.mul(radius); dir.z -= height;
			lfoot = vec(o).add(dir);
		}
		if(rfoot == vec(-1, -1, -1))
		{
			vecfromyawpitch(yaw, 0, 0, 1, dir); dir.mul(radius); dir.z -= height;
			rfoot = vec(o).add(dir);
		}
	}

	bool wantshitbox() { return type == ENT_PLAYER || (type == ENT_AI && (!isaitype(aitype) || aistyle[aitype].maxspeed)); }

	void checktags()
	{
		checkmeleepos();
		checkmuzzlepos();
		checkejectpos();
		if(wantshitbox()) checkhitboxes();
	}

	float calcroll(bool crouch)
	{
		float r = roll, wobble = float(rnd(15)-7)*(float(min(quake, 100))/100.f);
		switch(state)
		{
			case CS_SPECTATOR: case CS_WAITING: r = wobble*0.5f; break;
			case CS_ALIVE: if(crouch) wobble *= 0.5f;
			case CS_DEAD: r += wobble; break;
			default: break;
		}
		return r;
	}

	void doimpulse(int cost, int type, int millis)
	{
		impulse[IM_METER] += cost;
		impulse[IM_TYPE] = type;
		impulse[IM_TIME] = millis;
		impulse[IM_COUNT]++;
		resetphys();
	}

	void dojumpreset()
	{
		timeinair = turnside = impulse[IM_COUNT] = impulse[IM_TYPE] = 0;
	}
};

enum { PRJ_SHOT = 0, PRJ_GIBS, PRJ_DEBRIS, PRJ_EJECT, PRJ_ENT, PRJ_MAX };

struct projent : dynent
{
	vec from, to, norm;
	int addtime, lifetime, lifemillis, waittime, spawntime, fadetime, lastradial, lasteffect, lastbounce, beenused;
	float movement, roll, lifespan, lifesize;
	bool local, extinguish, limited, stuck, escaped;
	int projtype, projcollide;
	float elasticity, reflectivity, relativity, waterfric;
	int schan, id, weap, flags, hitflags;
	entitylight light;
	gameent *owner;
	physent *hit;
	const char *mdl;

	projent() : norm(0, 0, 1), projtype(PRJ_SHOT), id(-1), hitflags(HITFLAG_NONE), owner(NULL), hit(NULL), mdl(NULL) { reset(); }
	~projent()
	{
		removetrackedparticles(this);
		removetrackedsounds(this);
		if(issound(schan)) removesound(schan);
		schan = -1;
	}

	void reset()
	{
		physent::reset();
		type = ENT_PROJ;
		state = CS_ALIVE;
		norm = vec(0, 0, 1);
		addtime = lifetime = lifemillis = waittime = spawntime = fadetime = lastradial = lasteffect = lastbounce = beenused = flags = 0;
		schan = id = weap = -1;
		movement = roll = lifespan = lifesize = 0.f;
		extinguish = limited = stuck = escaped = false;
		projcollide = BOUNCE_GEOM|BOUNCE_PLAYER;
	}

	bool ready(bool used = true)
	{
		if(owner && (!used || !beenused) && waittime <= 0 && state != CS_DEAD)
			return true;
		return false;
	}
};

namespace server
{
	extern void stopdemo();
	extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
}

namespace client
{
	extern bool demoplayback, sendinfo, sendcrc;
	extern void clearvotes(gameent *d);
	extern void addmsg(int type, const char *fmt = NULL, ...);
	extern void c2sinfo();
}

namespace physics
{
	extern float gravity, jumpspeed, movespeed, movecrawl, impulsespeed, impulseregen, liquidspeed, liquidcurb, floorcurb, aircurb;
	extern int impulsestyle, impulsemeter, impulsecost, impulsecount, impulseskate;
	extern int smoothmove, smoothdist;
	extern bool canimpulse(physent *d, int cost = 0);
	extern bool movecamera(physent *pl, const vec &dir, float dist, float stepdist);
	extern void smoothplayer(gameent *d, int res, bool local);
	extern void update();
}

namespace projs
{
	extern vector<projent *> projs;

	extern void reset();
	extern void update();
	extern void create(vec &from, vec &to, bool local, gameent *d, int type, int lifetime, int lifemillis, int waittime, int speed, int id = 0, int weap = -1, int flags = 0);
	extern void preload();
	extern void remove(gameent *owner);
	extern void shootv(int weap, int flags, int power, vec &from, vector<vec> &locs, gameent *d, bool local);
	extern void drop(gameent *d, int g, int n, bool local = true);
	extern void adddynlights();
	extern void render();
}

namespace weapons
{
	extern int autoreloading;
	extern bool weapselect(gameent *d, int weap, bool local = true);
	extern bool weapreload(gameent *d, int weap, int load = -1, int ammo = -1, bool local = true);
	extern void reload(gameent *d);
	extern void shoot(gameent *d, vec &targ, int force = 0);
	extern void preload(int weap = -1);
}

namespace hud
{
	extern char *conopentex, *playertex, *deadtex, *dominatingtex, *dominatedtex, *inputtex, *bliptex, *cardtex, *flagtex, *arrowtex;
	extern int hudwidth, hudsize, lastteam, lastnewgame, damageresidue, damageresiduefade, shownotices, radarflagnames, inventorygame, teamkillnum;
	extern float noticescale, inventoryblend, inventoryskew, inventorygrow, radarflagblend, radarblipblend, radarflagsize;
	extern vector<int> teamkills;
	extern bool chkcond(int val, bool cond);
	extern char *timetostr(int millis, bool limited = false);
	extern void drawquad(float x, float y, float w, float h, float tx1 = 0, float ty1 = 0, float tx2 = 1, float ty2 = 1);
	extern void drawtex(float x, float y, float w, float h, float tx = 0, float ty = 0, float tw = 1, float th = 1);
	extern void drawsized(float x, float y, float s);
	extern void colourskew(float &r, float &g, float &b, float skew = 1.f);
	extern void healthskew(int &s, float &r, float &g, float &b, float &fade, float ss, bool throb = 1.f);
	extern void skewcolour(float &r, float &g, float &b, bool t = false);
	extern void skewcolour(int &r, int &g, int &b, bool t = false);
	extern void drawindicator(int weap, int x, int y, int s);
	extern void drawclip(int weap, int x, int y, float s);
	extern void drawpointerindex(int index, int x, int y, int s, float r = 1.f, float g = 1.f, float b = 1.f, float fade = 1.f);
	extern void drawpointer(int w, int h, int index);
	extern int numteamkills();
	extern float radarrange();
	extern void drawblip(const char *tex, float area, int w, int h, float s, float blend, vec &dir, float r = 1.f, float g = 1.f, float b = 1.f, const char *font = "sub", const char *text = NULL, ...);
	extern int drawprogress(int x, int y, float start, float length, float size, bool left, float r = 1.f, float g = 1.f, float b = 1.f, float fade = 1.f, float skew = 1.f, const char *font = NULL, const char *text = NULL, ...);
	extern int drawitem(const char *tex, int x, int y, float size, bool left = false, float r = 1.f, float g = 1.f, float b = 1.f, float fade = 1.f, float skew = 1.f, const char *font = NULL, const char *text = NULL, ...);
	extern int drawitemsubtext(int x, int y, float size, int align = TEXT_RIGHT_UP, float skew = 1.f, const char *font = NULL, float blend = 1.f, const char *text = NULL, ...);
	extern int drawweapons(int x, int y, int s, float blend = 1.f);
	extern int drawhealth(int x, int y, int s, float blend = 1.f);
	extern void drawinventory(int w, int h, int edge, float blend = 1.f);
	extern void damage(int n, const vec &loc, gameent *actor, int weap);
	extern const char *teamtex(int team = TEAM_NEUTRAL);
	extern const char *itemtex(int type, int stype);
}

namespace game
{
	extern int numplayers, gamemode, mutators, nextmode, nextmuts, minremain, maptime,
			zoomtime, lastzoom, lasttvcam, lasttvchg, spectvtime, waittvtime, showplayerinfo,
				bloodfade, gibfade, fogdist, aboveheadfade, announcefilter, dynlighteffects;
	extern float bloodscale, gibscale;
	extern bool intermission, zooming;
	extern vec swaypush, swaydir;
    extern string clientmap;

	extern gameent *player1;
	extern vector<gameent *> players;

	extern gameent *newclient(int cn);
	extern gameent *getclient(int cn);
	extern gameent *intersectclosest(vec &from, vec &to, gameent *at);
	extern void clientdisconnected(int cn);
	extern char *colorname(gameent *d, char *name = NULL, const char *prefix = "", bool team = true, bool dupname = true);
	extern void announce(int idx, int targ, gameent *d, const char *msg, ...);
	extern void respawn(gameent *d);
	extern void impulseeffect(gameent *d, bool effect);
	extern void suicide(gameent *d, int flags);
	extern void fixrange(float &yaw, float &pitch);
	extern void fixfullrange(float &yaw, float &pitch, float &roll, bool full);
	extern void getyawpitch(const vec &from, const vec &pos, float &yaw, float &pitch);
	extern void scaleyawpitch(float &yaw, float &pitch, float targyaw, float targpitch, float frame = 1.f, float scale = 1.f);
	extern bool allowmove(physent *d);
	extern int mousestyle();
	extern int deadzone();
	extern int panspeed();
	extern void checkzoom();
	extern bool inzoom();
	extern bool inzoomswitch();
	extern void zoomview(bool down);
	extern bool tvmode();
	extern void resetcamera();
	extern void resetworld();
	extern void resetstate();
	extern void hiteffect(int weap, int flags, int damage, gameent *d, gameent *actor, vec &dir, bool local = false);
	extern void damaged(int weap, int flags, int damage, int health, gameent *d, gameent *actor, int millis, vec &dir);
	extern void killed(int weap, int flags, int damage, gameent *d, gameent *actor, int style);
	extern void timeupdate(int timeremain);
}

namespace entities
{
	extern int showentdescs;
	extern vector<extentity *> ents;
	extern int lastenttype[MAXENTTYPES], lastusetype[EU_MAX];
	extern void clearentcache();
	extern int closestent(int type, const vec &pos, float mindist, bool links = false, gameent *d = NULL);
	extern bool collateitems(gameent *d, vector<actitem> &actitems);
	extern void checkitems(gameent *d);
	extern void putitems(packetbuf &p);
	extern void execlink(gameent *d, int index, bool local, int ignore = -1);
	extern void setspawn(int n, int m);
	extern bool tryspawn(dynent *d, const vec &o, short yaw = 0, short pitch = 0);
	extern void spawnplayer(gameent *d, int ent = -1, bool suicide = false);
	extern const char *entinfo(int type, vector<int> &attr, bool full = false);
	extern void useeffects(gameent *d, int n, bool s, int g, int r);
	extern const char *entmdlname(int type, vector<int> &attr);
	extern bool clipped(const vec &o, bool aiclip = false);
	extern void preload();
	extern void edittoggled(bool edit);
	extern const char *findname(int type);
	extern void adddynlights();
	extern void render();
	extern void update();
	struct avoidset
	{
		struct obstacle
		{
			dynent *ent;
			int numentities;
			float above;

			obstacle(dynent *ent) : ent(ent), numentities(0), above(-1) {}
		};

		vector<obstacle> obstacles;
		vector<int> entities;

		void clear()
		{
			obstacles.setsizenodelete(0);
			entities.setsizenodelete(0);
		}

		void add(dynent *ent)
		{
			obstacle &ob = obstacles.add(obstacle(ent));
			if(!ent) ob.above = enttype[WAYPOINT].radius;
			else switch(ent->type)
			{
				case ENT_PLAYER: case ENT_AI:
				{
					gameent *e = (gameent *)ent;
					ob.above = e->abovehead().z;
					break;
				}
				case ENT_PROJ:
				{
					projent *p = (projent *)ob.ent;
					if(p->projtype == PRJ_SHOT && WPB(p->weap, explode, p->flags&HIT_ALT))
						ob.above = p->o.z+(WPB(p->weap, explode, p->flags&HIT_ALT)*p->lifesize)+1.f;
					break;
				}
			}
		}

		void add(dynent *ent, int entity)
		{
			if(obstacles.empty() || ent!=obstacles.last().ent) add(ent);
			obstacles.last().numentities++;
			entities.add(entity);
		}

		void avoidnear(dynent *d, const vec &pos, float limit);

		#define loopavoid(v, d, body) \
			if(!(v).obstacles.empty()) \
			{ \
				int cur = 0; \
				loopv((v).obstacles) \
				{ \
					const entities::avoidset::obstacle &ob = (v).obstacles[i]; \
					int next = cur + ob.numentities; \
					if(!ob.ent || ob.ent != (d)) \
					{ \
						for(; cur < next; cur++) \
						{ \
							int ent = (v).entities[cur]; \
							body; \
						} \
					} \
					cur = next; \
				} \
			}

		bool find(int entity, gameent *d) const
		{
			loopavoid(*this, d, { if(ent == entity) return true; });
			return false;
		}

		int remap(gameent *d, int n, vec &pos)
		{
			if(!obstacles.empty())
			{
				int cur = 0;
				loopv(obstacles)
				{
					obstacle &ob = obstacles[i];
					int next = cur + ob.numentities;
					if(!ob.ent || ob.ent != d)
					{
						for(; cur < next; cur++) if(entities[cur] == n)
						{
							if(ob.above < 0) return -1;
							vec above(pos.x, pos.y, ob.above);
							if(above.z-d->o.z >= ai::JUMPMAX)
								return -1; // too much scotty
							int node = closestent(WAYPOINT, above, ai::SIGHTMIN, true, d);
							if(ents.inrange(node) && node != n)
							{ // try to reroute above their head?
								if(!find(node, d))
								{
									pos = ents[node]->o;
									return node;
								}
								else return -1;
							}
							else
							{
								vec old = d->o;
								d->o = vec(above).add(vec(0, 0, d->height));
								bool col = collide(d, vec(0, 0, 1));
								d->o = old;
								if(col)
								{
									pos = above;
									return n;
								}
								else return -1;
							}
						}
					}
					cur = next;
				}
			}
			return n;
		}
	};
	extern void findentswithin(int type, const vec &pos, float mindist, float maxdist, vector<int> &results);
	extern float route(int node, int goal, vector<int> &route, const avoidset &obstacles, gameent *d = NULL, bool check = true);
}
#elif defined(GAMESERVER)
namespace client
{
	extern const char *getname();
}
#endif
#include "ctf.h"
#include "stf.h"
#include "vars.h"
#ifndef GAMESERVER
#include "scoreboard.h"
#endif

#endif

