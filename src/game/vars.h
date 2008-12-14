#ifdef STANDALONE
SVARG(defaultmap, "eight");
VARG(defaultmode, G_LOBBY, G_DEATHMATCH, G_MAX-1);
#else
SVARG(defaultmap, "overseer");
VARG(defaultmode, G_LOBBY, G_LOBBY, G_MAX-1);
#endif
VARG(defaultmuts, G_M_NONE, G_M_NONE, G_M_ALL);

SVARG(lobbymaps, "overseer");
SVARG(missionmaps, "mpspbf1 mpspbf2"); // remember, for these the rotation starts at defaultmap
SVARG(mainmaps, "eight warground warehouse tower rivals refuge citadel storage");
SVARG(ctfmaps, "eight warground warehouse tower rivals refuge citadel storage siege");
SVARG(stfmaps, "eight warground warehouse tower rivals refuge citadel storage siege");
VARG(maprotate, 0, 1, 2); // 0 = off, 1 = sequence, 2 = random

VARG(itemsallowed, 0, 1, 2); // 0 = never, 1 = all but instagib, 2 = always
VARG(itemdropping, 0, 1, 1); // 0 = never, 1 = yes
VARG(itemspawntime, 1, 30, 3600); // secs when items respawn
VARG(kamakaze, 0, 1, 3); // 0 = never, 1 = holding grenade, 2 = have grenades, 3 = always

VARG(timelimit, 0, 15, 60);
VARG(intermlimit, 0, 10, 120); // secs before vote menu comes up
VARG(votelimit, 0, 20, 120); // secs before vote passes by default

VARG(teamdamage, 0, 1, 1); // damage team mates
VARG(ctflimit, 0, 0, INT_MAX-1); // finish when score is this or more
VARG(stflimit, 0, 0, INT_MAX-1); // finish when score is this or more
VARG(stffinish, 0, 0, 1); // finish when all bases captured

VARG(spawngun, 0, GUN_PLASMA, GUN_MAX-1);
VARG(instaspawngun, 0, GUN_RIFLE, GUN_MAX-1);

VARG(botbalance, 0, 1, 1);
VARG(botratio, 0, 2, 10);
VARG(botminamt, 0, 2, MAXCLIENTS-1);
VARG(botmaxamt, 0, 12, MAXCLIENTS-1);
VARG(botminskill, 0, 80, 100);
VARG(botmaxskill, 0, 100, 100);

FVARG(damagescale, 0.1f, 1.f, 10);
FVARG(gravityscale, 0.1f, 1.f, 10);
FVARG(jumpscale, 0.1f, 1.f, 10);
FVARG(speedscale, 0.1f, 1.f, 10);
