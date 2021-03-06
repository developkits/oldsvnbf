#include "game.h"
namespace entities
{
    vector<extentity *> ents;
    int lastenttype[MAXENTTYPES], lastusetype[EU_MAX];
    bool haswaypoints = false;

    VAR(IDF_PERSIST, showentdescs, 0, 2, 3);
    VAR(IDF_PERSIST, showentinfo, 0, 2, 5);
    VAR(IDF_PERSIST, showentnoisy, 0, 0, 2);
    VAR(IDF_PERSIST, showentdir, 0, 2, 3); // 0 = off, 1 = only selected, 2 = always when editing, 3 = always in editmode
    VAR(IDF_PERSIST, showentradius, 0, 1, 3);
    VAR(IDF_PERSIST, showentlinks, 0, 1, 3);
    VAR(IDF_PERSIST, showlighting, 0, 0, 1);
    VAR(0, dropwaypoints, 0, 0, 2); // drop waypoints during play, 2 = render as well

    bool waypointdrop(bool render = false)
    {
        return dropwaypoints >= (render ? 2 : 1) || (!render && !haswaypoints && m_play(game::gamemode));
    }

    vector<extentity *> &getents() { return ents; }
     int lastent(int type) { return lastenttype[type]; }

    int triggertime(extentity &e)
    {
        switch(e.type)
        {
            case TRIGGER: case MAPMODEL: case PARTICLES: case MAPSOUND: case LIGHTFX: case TELEPORT: case PUSHER: return 1000; break;
            default: break;
        }
        return 0;
    }

    const char *entinfo(int type, vector<int> &attr, bool full)
    {
        static string entinfostr; entinfostr[0] = 0;
        #define addentinfo(s) if(*(s)) { \
            if(entinfostr[0]) concatstring(entinfostr, ", "); \
            concatstring(entinfostr, s); \
        }
        switch(type)
        {
            case PARTICLES:
            {
                switch(attr[0])
                {
                    case 0: addentinfo("fire plume"); break;
                    case 1: addentinfo("smoke vent"); break;
                    case 2: addentinfo("water fountain"); break;
                    case 3: addentinfo("fireball"); break;
                    case 4: addentinfo("tape"); break;
                    case 7: addentinfo("lightning"); break;
                    case 8: addentinfo("fire"); break;
                    case 9: addentinfo("smoke"); break;
                    case 10: addentinfo("water"); break;
                    case 11: addentinfo("plasma"); break;
                    case 12: addentinfo("snow"); break;
                    case 13: addentinfo("sparks"); break;
                    case 14: addentinfo("flames"); break;
                    case 15: addentinfo("smoke plume"); break;
                    case 6: addentinfo("progress versus"); break;
                    case 5: addentinfo("progress"); break;
                    case 32: addentinfo("lensflare (plain)"); break;
                    case 33: addentinfo("lensflare (sparkle)"); break;
                    case 34: addentinfo("lensflare (sun)"); break;
                    case 35: addentinfo("lensflare (sparklesun)"); break;
                    default: break;
                }
                switch(attr[0])
                {
                    case 4: case 7: case 8: case 9: case 10: case 11: case 12: case 13:
                    {
                        if(attr[1] >= 256)
                        {
                            int val = attr[1]-256;
                            switch(val%32)
                            {
                                case 0: case 1: case 2: addentinfo("circle"); break;
                                case 3: case 4: case 5: addentinfo("cylinder"); break;
                                case 6: case 7: case 8: case 9: case 10: case 11: addentinfo("cone"); break;
                                case 12: case 13: case 14: addentinfo("plane"); break;
                                case 15: case 16: case 17: case 18: case 19: case 20: addentinfo("line"); break;
                                case 21: default: addentinfo("sphere"); break;
                            }
                            switch(val%3)
                            {
                                case 0: addentinfo("x-axis"); break;
                                case 1: addentinfo("y-axis"); break;
                                case 2: addentinfo("z-axis"); break;
                            }
                            if(val%64 >= 32) addentinfo("inverted");
                            break;
                        }
                        // fall through
                    }
                    case 1: case 2:
                    {
                        switch(attr[1]%3)
                        {
                            case 0: addentinfo("x-axis"); break;
                            case 1: addentinfo("y-axis"); break;
                            case 2: addentinfo("z-axis"); break;
                        }
                        if(attr[1]%6 >= 3) addentinfo("inverted");
                        break;
                    }
                    default: break;
                }
                break;
            }
            case PLAYERSTART: case FLAG: case CHECKPOINT:
            {
                if(type != CHECKPOINT)
                {
                    if(valteam(attr[0], TEAM_FIRST))
                    {
                        defformatstring(str)("team %s", teamtype[attr[0]].name);
                        addentinfo(str);
                    }
                }
                else
                {
                    const char *cpnames[CP_MAX+1] = { "respawn", "start", "finish", "last", "" };
                    addentinfo(cpnames[attr[5] < 0 || attr[5] >= CP_MAX ? CP_MAX : attr[5]]);
                }
                if(attr[3] && attr[3] > -G_MAX && attr[3] < G_MAX)
                {
                    string ds;
                    if(attr[3]<0) formatstring(ds)("not %s", gametype[-attr[3]].name);
                    else formatstring(ds)("%s", gametype[attr[3]].name);
                    addentinfo(ds);
                }
                break;
            }
            case LIGHTFX:
            {
                if(full)
                {
                    const char *lfxnames[LFX_MAX+1] = { "spotlight", "dynlight", "flicker", "pulse", "glow", "" };
                    addentinfo(lfxnames[attr[0] < 0 || attr[0] >= LFX_MAX ? LFX_MAX : attr[0]]);
                    loopi(LFX_MAX-1) if(attr[4]&(1<<(LFX_S_MAX+i))) { defformatstring(ds)("+%s", lfxnames[i+1]); addentinfo(ds); break; }
                    if(attr[4]&LFX_S_RAND1) addentinfo("rnd-min");
                    if(attr[4]&LFX_S_RAND2) addentinfo("rnd-max");
                }
                break;
            }
            case ACTOR:
            {
                if(full && attr[0] >= AI_START && attr[0] < AI_MAX)
                {
                    addentinfo(aistyle[attr[0]].name);
                    if(attr[3] && attr[3] > -G_MAX && attr[3] < G_MAX)
                    {
                        string ds;
                        if(attr[3]<0) formatstring(ds)("not %s", gametype[-attr[3]].name);
                        else formatstring(ds)("%s", gametype[attr[3]].name);
                        addentinfo(ds);
                    }
                    addentinfo(weaptype[attr[5] > 0 && attr[5] <= WEAP_MAX ? attr[5]-1 : aistyle[attr[0]].weap].name);
                }
                break;
            }
            case WEAPON:
            {
                int sweap = m_weapon(game::gamemode, game::mutators), attr1 = w_attr(game::gamemode, attr[0], sweap);
                if(isweap(attr1))
                {
                    defformatstring(str)("\fs%s%s\fS", weaptype[attr1].text, weaptype[attr1].name);
                    addentinfo(str);
                    if(full)
                    {
                        if(attr[2] && attr[2] > -G_MAX && attr[2] < G_MAX)
                        {
                            string ds;
                            if(attr[2]<0) formatstring(ds)("not %s", gametype[-attr[2]].name);
                            else formatstring(ds)("%s", gametype[attr[2]].name);
                            addentinfo(ds);
                        }
                        if(attr[1]&WEAP_F_FORCED) addentinfo("forced");
                    }
                }
                break;
            }
            case MAPMODEL:
            {
                if(mapmodels.inrange(attr[0])) addentinfo(mapmodels[attr[0]].name);
                if(full)
                {
                    if(attr[5]&MMT_HIDE) addentinfo("hide");
                    if(attr[5]&MMT_NOCLIP) addentinfo("noclip");
                    if(attr[5]&MMT_NOSHADOW) addentinfo("noshadow");
                    if(attr[5]&MMT_NODYNSHADOW) addentinfo("nodynshadow");
                }
                break;
            }
            case MAPSOUND:
            {
                if(mapsounds.inrange(attr[0])) addentinfo(mapsounds[attr[0]].sample->name);
                if(full)
                {
                    if(attr[4]&SND_NOATTEN) addentinfo("noatten");
                    if(attr[4]&SND_NODELAY) addentinfo("nodelay");
                    if(attr[4]&SND_NOCULL) addentinfo("nocull");
                    if(attr[4]&SND_NOPAN) addentinfo("nopan");
                    if(attr[4]&SND_NODIST) addentinfo("nodist");
                    if(attr[4]&SND_NOQUIET) addentinfo("noquiet");
                }
                break;
            }
            case TRIGGER:
            {
                if(full)
                {
                    const char *trgnames[TR_MAX+1] = { "toggle", "link", "script", "once", "exit", "" }, *actnames[TA_MAX+1] = { "manual", "proximity", "action", "" };
                    addentinfo(trgnames[attr[1] < 0 || attr[1] >= TR_MAX ? TR_MAX : attr[1]]);
                    addentinfo(actnames[attr[2] < 0 || attr[2] >= TA_MAX ? TA_MAX : attr[2]]);
                    if(attr[4] >= 2) addentinfo(attr[4] ? "routed" : "inert");
                    addentinfo(attr[4]%2 ? "on" : "off");
                }
                break;
            }
            case PUSHER:
            {
                if(full) switch(attr[5])
                {
                    case 0: addentinfo("conditional"); break;
                    case 1: addentinfo("additional"); break;
                    case 2: addentinfo("redirectional"); break;
                    case 3: addentinfo("absolute"); break;
                    default: break;
                }
                break;
            }
            case WAYPOINT:
            {
                if(full && attr[0]&WP_F_CROUCH) addentinfo("crouch");
                break;
            }
            default: break;
        }
        return entinfostr[0] ? entinfostr : "";
    }

    const char *entmdlname(int type, vector<int> &attr)
    {
        switch(type)
        {
            case PLAYERSTART: return teamtype[attr[0]].tpmdl;
            case WEAPON:
            {
                int sweap = m_weapon(game::gamemode, game::mutators), attr1 = w_attr(game::gamemode, attr[0], sweap);
                return weaptype[attr1].item;
            }
            case FLAG: return teamtype[attr[0]].flag;
            case ACTOR: if(attr[0] >= AI_START && attr[0] < AI_MAX) return aistyle[attr[0]].tpmdl;
            default: break;
        }
        return "";
    }

    // these two functions are called when the server acknowledges that you really
    // picked up the item (in multiplayer someone may grab it before you).

    void useeffects(gameent *d, int n, int c, bool s, int g, int r, int v)
    {
        gameentity &e = *(gameentity *)ents[n];
        int sweap = m_weapon(game::gamemode, game::mutators), attr = e.type == WEAPON ? w_attr(game::gamemode, e.attrs[0], sweap) : e.attrs[0],
            colour = e.type == WEAPON ? weaptype[attr].colour : 0xFFFFFF;
        if(showentdescs)
        {
            const char *texname = showentdescs >= 2 ? hud::itemtex(e.type, attr) : NULL;
            if(texname && *texname) part_icon(d->abovehead(), textureload(texname, 3), 2, 1, -10, 0, game::aboveheadfade, colour, 0, 1, d);
            else
            {
                const char *item = entinfo(e.type, e.attrs, false);
                if(item && *item)
                {
                    defformatstring(ds)("<super>%s (%d)", item, e.type);
                    part_textcopy(d->abovehead(), ds, PART_TEXT, game::aboveheadfade, 0xFFFFFF, 2, 1, -10, 0, d);
                }
            }
        }
        if(isweap(g))
        {
            d->setweapstate(g, WEAP_S_SWITCH, WEAPSWITCHDELAY, lastmillis);
            d->ammo[g] = -1;
            if(d->weapselect != g)
            {
                d->lastweap = d->weapselect;
                d->weapselect = g;
            }
        }
        d->useitem(n, e.type, attr, c, sweap, lastmillis);
        if(issound(d->wschan)) removesound(d->wschan);
        playsound(S_ITEMPICKUP, d->o, d, 0, -1, -1, -1, &d->wschan);
        if(game::dynlighteffects) adddynlight(d->o, enttype[e.type].radius*2, vec(colour>>16, (colour>>8)&0xFF, colour&0xFF).mul(2.f/0xFF), 250, 250);
        if(ents.inrange(r) && ents[r]->type == WEAPON)
        {
            gameentity &f = *(gameentity *)ents[r];
            attr = w_attr(game::gamemode, f.attrs[0], sweap);
            if(isweap(attr)) projs::drop(d, attr, r, v, d == game::player1 || d->ai);
        }
        if(e.spawned != s)
        {
            e.spawned = s;
            e.lastuse = lastmillis;
        }
        loopv(projs::projs)
        {
            projent &proj = *projs::projs[i];
            if(proj.projtype != PRJ_ENT || proj.id != n) continue;
            proj.beenused = 2;
            proj.lifetime = min(proj.lifetime, proj.fadetime);
        }
    }

    struct entcachenode
    {
        float split[2];
        uint child[2];

        int axis() const { return child[0]>>30; }
        int childindex(int which) const { return child[which]&0x3FFFFFFF; }
        bool isleaf(int which) const { return (child[1]&(1<<(30+which)))!=0; }
    };

    vector<entcachenode> entcache;
    int entcachedepth = -1;
    vec entcachemin(1e16f, 1e16f, 1e16f), entcachemax(-1e16f, -1e16f, -1e16f);

    float calcentcacheradius(extentity &e)
    {
        switch(e.type)
        {
            case WAYPOINT: return 0;
            case TRIGGER: case TELEPORT: case PUSHER: case CHECKPOINT:
                if(e.attrs[e.type == CHECKPOINT ? 0 : 3]) return e.attrs[e.type == CHECKPOINT ? 0 : 3]; // fall through
            default: return enttype[e.type].radius;
        }
    }

    static void buildentcache(int *indices, int numindices, const vec &vmin, const vec &vmax, int depth = 1)
    {
        int axis = 2;
        loopk(2) if(vmax[k] - vmin[k] > vmax[axis] - vmin[axis]) axis = k;

        vec leftmin(1e16f, 1e16f, 1e16f), leftmax(-1e16f, -1e16f, -1e16f), rightmin(1e16f, 1e16f, 1e16f), rightmax(-1e16f, -1e16f, -1e16f);
        float split = 0.5f*(vmax[axis] + vmin[axis]), splitleft = -1e16f, splitright = 1e16f;
        int left, right;
        for(left = 0, right = numindices; left < right;) if(ents.inrange(indices[left]))
        {
            extentity &e = *ents[indices[left]];
            float radius = calcentcacheradius(e);
            if(max(split - (e.o[axis]-radius), 0.0f) > max((e.o[axis]+radius) - split, 0.0f))
            {
                ++left;
                splitleft = max(splitleft, e.o[axis]+radius);
                leftmin.min(vec(e.o).sub(radius));
                leftmax.max(vec(e.o).add(radius));
            }
            else
            {
                --right;
                swap(indices[left], indices[right]);
                splitright = min(splitright, e.o[axis]-radius);
                rightmin.min(vec(e.o).sub(radius));
                rightmax.max(vec(e.o).add(radius));
            }
        }

        if(!left || right==numindices)
        {
            leftmin = rightmin = vec(1e16f, 1e16f, 1e16f);
            leftmax = rightmax = vec(-1e16f, -1e16f, -1e16f);
            left = right = numindices/2;
            splitleft = -1e16f;
            splitright = 1e16f;
            loopi(numindices) if(ents.inrange(indices[i]))
            {
                extentity &e = *ents[indices[i]];
                float radius = calcentcacheradius(e);
                if(i < left)
                {
                    splitleft = max(splitleft, e.o[axis]+radius);
                    leftmin.min(vec(e.o).sub(radius));
                    leftmax.max(vec(e.o).add(radius));
                }
                else
                {
                    splitright = min(splitright, e.o[axis]-radius);
                    rightmin.min(vec(e.o).sub(radius));
                    rightmax.max(vec(e.o).add(radius));
                }
            }
        }

        int node = entcache.length();
        entcache.add();
        entcache[node].split[0] = splitleft;
        entcache[node].split[1] = splitright;

        if(left==1) entcache[node].child[0] = (axis<<30) | indices[0];
        else
        {
            entcache[node].child[0] = (axis<<30) | entcache.length();
            if(left) buildentcache(indices, left, leftmin, leftmax, depth+1);
        }

        if(numindices-right==1) entcache[node].child[1] = (1<<31) | (left==1 ? 1<<30 : 0) | indices[right];
        else
        {
            entcache[node].child[1] = (left==1 ? 1<<30 : 0) | entcache.length();
            if(numindices-right) buildentcache(&indices[right], numindices-right, rightmin, rightmax, depth+1);
        }

        entcachedepth = max(entcachedepth, depth);
    }

    bool docacheclear = false;
    void clearentcache()
    {
        entcache.setsize(0);
        entcachedepth = -1;
        entcachemin = vec(1e16f, 1e16f, 1e16f);
        entcachemax = vec(-1e16f, -1e16f, -1e16f);
    }
    ICOMMAND(0, clearentcache, "", (void), clearentcache());

    void buildentcache()
    {
        entcache.setsize(0);
        vector<int> indices;
        entcachemin = vec(1e16f, 1e16f, 1e16f);
        entcachemax = vec(-1e16f, -1e16f, -1e16f);
        loopv(ents)
        {
            extentity &e = *ents[i];
            if(e.type==WAYPOINT || enttype[e.type].usetype != EU_NONE)
            {
                indices.add(i);
                float radius = calcentcacheradius(e);
                entcachemin.min(vec(e.o).sub(radius));
                entcachemax.max(vec(e.o).add(radius));
            }
        }
        buildentcache(indices.getbuf(), indices.length(), entcachemin, entcachemax);
    }

    struct entcachestack
    {
        entcachenode *node;
        float tmin, tmax;
    };

    vector<entcachenode *> entcachestack;

    int closestent(int type, const vec &pos, float mindist, bool links)
    {
        if(entcachedepth<0) buildentcache();

        entcachestack.setsize(0);

        #define CHECKCLOSEST(branch) do { \
            int n = curnode->childindex(branch); \
            if(ents.inrange(n)) { \
                extentity &e = *ents[n]; \
                if(e.type == type && (!links || !e.links.empty())) \
                { \
                    float dist = e.o.squaredist(pos); \
                    if(dist < mindist*mindist) { closest = n; mindist = sqrtf(dist); } \
                } \
            } \
        } while(0)
        int closest = -1;
        loop(force, 2) for(entcachenode *curnode = &entcache[0];;)
        {
            int axis = curnode->axis();
            float dist1 = pos[axis] - curnode->split[0], dist2 = curnode->split[1] - pos[axis];
            if(dist1 >= mindist)
            {
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKCLOSEST(1);
                }
            }
            else if(curnode->isleaf(0))
            {
                CHECKCLOSEST(0);
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKCLOSEST(1);
                }
            }
            else
            {
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) entcachestack.add(&entcache[curnode->childindex(1)]);
                    else CHECKCLOSEST(1);
                }
                curnode = &entcache[curnode->childindex(0)];
                continue;
            }
            if(entcachestack.empty()) { if(closest >= 0) return closest; else break; }
            curnode = entcachestack.pop();
        }
        return -1;
    }

    void findentswithin(int type, const vec &pos, float mindist, float maxdist, vector<int> &results)
    {
        float mindist2 = mindist*mindist, maxdist2 = maxdist*maxdist;

        if(entcachedepth<0) buildentcache();

        entcachestack.setsize(0);

        entcachenode *curnode = &entcache[0];
        #define CHECKWITHIN(branch) do { \
            int n = curnode->childindex(branch); \
            if(ents.inrange(n)) { \
                extentity &e = *ents[n]; \
                if(e.type == type) \
                { \
                    float dist = e.o.squaredist(pos); \
                    if(dist > mindist2 && dist < maxdist2) results.add(n); \
                } \
            } \
        } while(0)
        for(;;)
        {
            int axis = curnode->axis();
            float dist1 = pos[axis] - curnode->split[0], dist2 = curnode->split[1] - pos[axis];
            if(dist1 >= maxdist)
            {
                if(dist2 < maxdist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKWITHIN(1);
                }
            }
            else if(curnode->isleaf(0))
            {
                CHECKWITHIN(0);
                if(dist2 < maxdist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKWITHIN(1);
                }
            }
            else
            {
                if(dist2 < maxdist)
                {
                    if(!curnode->isleaf(1)) entcachestack.add(&entcache[curnode->childindex(1)]);
                    else CHECKWITHIN(1);
                }
                curnode = &entcache[curnode->childindex(0)];
                continue;
            }
            if(entcachestack.empty()) return;
            curnode = entcachestack.pop();
        }
    }

    void avoidset::avoidnear(dynent *d, const vec &pos, float limit)
    {
        float limit2 = limit*limit;

        if(entcachedepth<0) buildentcache();

        entcachestack.setsize(0);

        entcachenode *curnode = &entcache[0];
        #define CHECKNEAR(branch) do { \
            int n = curnode->childindex(branch); \
            if(ents.inrange(n)) { \
                extentity &e = *ents[n]; \
                if(e.type == WAYPOINT && e.o.squaredist(pos) < limit2) add(d, n); \
            } \
        } while(0)
        for(;;)
        {
            int axis = curnode->axis();
            float dist1 = pos[axis] - curnode->split[0], dist2 = curnode->split[1] - pos[axis];
            if(dist1 >= limit)
            {
                if(dist2 < limit)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKNEAR(1);
                }
            }
            else if(curnode->isleaf(0))
            {
                CHECKNEAR(0);
                if(dist2 < limit)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKNEAR(1);
                }
            }
            else
            {
                if(dist2 < limit)
                {
                    if(!curnode->isleaf(1)) entcachestack.add(&entcache[curnode->childindex(1)]);
                    else CHECKNEAR(1);
                }
                curnode = &entcache[curnode->childindex(0)];
                continue;
            }
            if(entcachestack.empty()) return;
            curnode = entcachestack.pop();
        }
    }

    void collateents(const vec &pos, float xyrad, float zrad, vector<actitem> &actitems)
    {
        if(entcachedepth<0) buildentcache();

        entcachestack.setsize(0);

        entcachenode *curnode = &entcache[0];
        #define CHECKITEM(branch) do { \
            int n = curnode->childindex(branch); \
            if(ents.inrange(n)) { \
                extentity &e = *ents[n]; \
                if(enttype[e.type].usetype != EU_NONE && (enttype[e.type].usetype!=EU_ITEM || e.spawned)) \
                { \
                    float radius = (e.type == TRIGGER || e.type == TELEPORT || e.type == PUSHER || e.type == CHECKPOINT) && e.attrs[e.type == CHECKPOINT ? 0 : 3] ? e.attrs[e.type == CHECKPOINT ? 0 : 3] : enttype[e.type].radius; \
                    if(overlapsbox(pos, zrad, xyrad, e.o, radius, radius)) \
                    { \
                        actitem &t = actitems.add(); \
                        t.type = ITEM_ENT; \
                        t.target = n; \
                        t.score = pos.squaredist(e.o); \
                    } \
                } \
            } \
        } while(0)
        for(;;)
        {
            int axis = curnode->axis();
            float mindist = axis<2 ? xyrad : zrad, dist1 = pos[axis] - curnode->split[0], dist2 = curnode->split[1] - pos[axis];
            if(dist1 >= mindist)
            {
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKITEM(1);
                }
            }
            else if(curnode->isleaf(0))
            {
                CHECKITEM(0);
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) { curnode = &entcache[curnode->childindex(1)]; continue; }
                    CHECKITEM(1);
                }
            }
            else
            {
                if(dist2 < mindist)
                {
                    if(!curnode->isleaf(1)) entcachestack.add(&entcache[curnode->childindex(1)]);
                    else CHECKITEM(1);
                }
                curnode = &entcache[curnode->childindex(0)];
                continue;
            }
            if(entcachestack.empty()) return;
            curnode = entcachestack.pop();
        }
    }

    static int sortitems(const actitem *a, const actitem *b)
    {
        if(a->score > b->score) return -1;
        if(a->score < b->score) return 1;
        return 0;
    }

    bool collateitems(gameent *d, vector<actitem> &actitems)
    {
        float eye = d->height*0.5f;
        vec m = vec(d->o).sub(vec(0, 0, eye));

        collateents(m, d->radius, eye, actitems);
        loopv(projs::projs)
        {
            projent &proj = *projs::projs[i];
            if(proj.projtype != PRJ_ENT || !proj.ready()) continue;
            if(!ents.inrange(proj.id) || enttype[ents[proj.id]->type].usetype != EU_ITEM) continue;
            if(!overlapsbox(m, eye, d->radius, proj.o, enttype[ents[proj.id]->type].radius, enttype[ents[proj.id]->type].radius))
                continue;
            actitem &t = actitems.add();
            t.type = ITEM_PROJ;
            t.target = i;
            t.score = m.squaredist(proj.o);
        }
        if(!actitems.empty())
        {
            actitems.sort(sortitems); // sort items so last is closest
            return true;
        }
        return false;
    }

    gameent *trigger = NULL;
    ICOMMAND(0, triggerclientnum, "", (), intret(trigger ? trigger->clientnum : -1));

    void runtrigger(int n, gameent *d, bool act = true)
    {
        gameentity &e = *(gameentity *)ents[n];
        if(lastmillis-e.lastuse >= triggertime(e)/2)
        {
            e.lastuse = lastmillis;
            switch(e.attrs[1])
            {
                case TR_TOGGLE: case TR_LINK: case TR_ONCE: case TR_EXIT:
                { // wait for ack
                    if(e.attrs[1] == TR_EXIT && (d->aitype >= AI_BOT || !m_campaign(game::gamemode))) break;
                    client::addmsg(N_TRIGGER, "ri2", d->clientnum, n);
                    break;
                }
                case TR_SCRIPT:
                {
                    if(d == game::player1)
                    {
                        defformatstring(s)("on_trigger_%d", e.attrs[0]);
                        trigger = d; RUNWORLD(s); trigger = NULL;
                    }
                    break;
                }
                default: break;
            }
            if(act && e.attrs[2] == TA_ACTION) d->action[AC_USE] = false;
        }
    }

    void runtriggers(int n, gameent *d)
    {
        loopi(lastenttype[TRIGGER]) if(ents[i]->type == TRIGGER && ents[i]->attrs[0] == n && ents[i]->attrs[2] == TA_MANUAL) runtrigger(i, d, false);
    }
    ICOMMAND(0, exectrigger, "i", (int *n), if(worldidents) runtriggers(*n, trigger ? trigger : game::player1));

    void execitem(int n, gameent *d, bool &tried)
    {
        gameentity &e = *(gameentity *)ents[n];
        switch(enttype[e.type].usetype)
        {
            case EU_ITEM: if(d->action[AC_USE])
            {
                if(game::allowmove(d))
                {
                    int sweap = m_weapon(game::gamemode, game::mutators), attr = e.type == WEAPON ? w_attr(game::gamemode, e.attrs[0], sweap) : e.attrs[0];
                    if(d->canuse(e.type, attr, e.attrs, sweap, lastmillis, (1<<WEAP_S_RELOAD)|(1<<WEAP_S_SWITCH)))
                    {
                        client::addmsg(N_ITEMUSE, "ri3", d->clientnum, lastmillis-game::maptime, n);
                        d->setweapstate(d->weapselect, WEAP_S_WAIT, WEAPSWITCHDELAY, lastmillis);
                        d->action[AC_USE] = false;
#if 0
                        e.spawned = false;
                        e.lastuse = lastmillis;
                        loopv(projs::projs)
                        {
                            projent &proj = *projs::projs[i];
                            if(proj.projtype != PRJ_ENT || proj.id != n || !proj.ready()) continue;
                            proj.beenused = 2;
                            proj.lifetime = min(proj.lifetime, proj.fadetime);
                        }
#endif
                    }
                    else tried = true;
                }
                else tried = true;
            } break;
            case EU_AUTO: switch(e.type)
            {
                case TELEPORT:
                {
                    if(lastmillis-e.lastuse >= triggertime(e)/2)
                    {
                        e.lastuse = e.lastemit = lastmillis;
                        static vector<int> teleports;
                        teleports.shrink(0);
                        loopv(e.links)
                            if(ents.inrange(e.links[i]) && ents[e.links[i]]->type == e.type)
                                teleports.add(e.links[i]);
                        if(!teleports.empty())
                        {
                            bool teleported = false;
                            while(!teleports.empty())
                            {
                                int r = e.type == TELEPORT ? rnd(teleports.length()) : 0;
                                gameentity &f = *(gameentity *)ents[teleports[r]];
                                d->dojumpreset();
                                d->o = vec(f.o).add(vec(0, 0, d->height*0.5f));
                                d->yaw = f.attrs[0] < 0 ? (lastmillis/5)%360 : f.attrs[0];
                                d->pitch = f.attrs[1];
                                if(physics::entinmap(d, true))
                                {
                                    float mag = max(d->vel.magnitude(), f.attrs[2] ? float(f.attrs[2]) : 50.f);
                                    vecfromyawpitch(d->yaw, d->pitch, 1, 0, d->vel);
                                    d->vel.mul(mag);
                                    game::fixfullrange(d->yaw, d->pitch, d->roll, true);
                                    f.lastuse = f.lastemit = e.lastemit;
                                    if(d == game::focus) game::resetcamera();
                                    execlink(d, n, true); execlink(d, teleports[r], true);
                                    teleported = true;
                                    break;
                                }
                                teleports.remove(r); // must've really sucked, try another one
                            }
                            if(!teleported) game::suicide(d, HIT_SPAWN);
                        }
                    }
                    break;
                }
                case PUSHER:
                {
                    float mag = max(e.attrs[2], 1);
                    if(e.attrs[4] && e.attrs[4] < e.attrs[3])
                    {
                        vec m = vec(d->o).sub(vec(0, 0, d->height*0.5f));
                        float dist = m.dist(e.o);
                        if(dist >= e.attrs[4]) mag *= 1.f-clamp((dist-e.attrs[4])/float(e.attrs[3]-e.attrs[4]), 0.f, 1.f);
                    }
                    vec dir; vecfromyawpitch(e.attrs[0], e.attrs[1], 1, 0, dir); dir.normalize().mul(mag);
                    if(d->ai) d->ai->becareful = true;
                    d->dojumpreset();
                    switch(e.attrs[5])
                    {
                        case 0:
                        {
                            loopk(3)
                            {
                                if((d->vel.v[k] > 1e-1f && dir.v[k] < -1e-1f) || (d->vel.v[k] < -1e-1f && dir.v[k] > 1e-1f) || (fabs(dir.v[k]) > fabs(d->vel.v[k])))
                                    d->vel.v[k] = dir.v[k];
                            }
                            break;
                        }
                        case 1: d->vel.add(dir); break;
                        case 2: dir.add(vec(dir).normalize().mul(d->vel.magnitude())); // fall through
                        case 3: d->vel = dir; break;
                        default: break;
                    }
                    e.lastuse = e.lastemit = lastmillis; execlink(d, n, true);
                    break;
                }
                case TRIGGER:
                {
                    if((e.attrs[2] == TA_ACTION && d->action[AC_USE] && d == game::player1) || e.attrs[2] == TA_AUTO) runtrigger(n, d);
                    break;
                }
                case CHECKPOINT:
                {
                    if(!m_check(e.attrs[3], game::gamemode) || (!m_campaign(game::gamemode) && !m_trial(game::gamemode))) break;
                    if(d->checkpoint != n)
                    {
                        client::addmsg(N_TRIGGER, "ri2", d->clientnum, n);
                        d->checkpoint = n;
                        if(!d->cpmillis || e.attrs[5] == CP_START) d->cpmillis = lastmillis;
                    }
                }
            } break;
        }
    }

    void checkitems(gameent *d)
    {
        static vector<actitem> actitems;
        actitems.setsize(0);
        if(collateitems(d, actitems))
        {
            bool tried = false;
            while(!actitems.empty())
            {
                actitem &t = actitems.last();
                int ent = -1;
                switch(t.type)
                {
                    case ITEM_ENT:
                    {
                        if(!ents.inrange(t.target)) break;
                        ent = t.target;
                        break;
                    }
                    case ITEM_PROJ:
                    {
                        if(!projs::projs.inrange(t.target)) break;
                        projent &proj = *projs::projs[t.target];
                        ent = proj.id;
                        break;
                    }
                    default: break;
                }
                if(ents.inrange(ent)) execitem(ent, d, tried);
                actitems.pop();
            }
            if(tried && d->action[AC_USE] && d == game::player1)
            {
                playsound(S_ERROR, d->o, d);
                d->action[AC_USE] = false;
            }
        }
        if(m_ctf(game::gamemode)) ctf::checkflags(d);
    }

    void putitems(packetbuf &p)
    {
        loopv(ents) if(enttype[ents[i]->type].usetype == EU_ITEM || ents[i]->type == PLAYERSTART || ents[i]->type == CHECKPOINT || ents[i]->type == ACTOR || ents[i]->type == TRIGGER)
        {
            gameentity &e = *(gameentity *)ents[i];
            putint(p, i);
            putint(p, int(e.type));
            putint(p, e.attrs.length());
            putint(p, e.kin.length());
            loopvj(e.attrs) putint(p, e.attrs[j]);
            loopvj(e.kin) putint(p, e.kin[j]);
        }
    }

    void setspawn(int n, int m)
    {
        if(ents.inrange(n))
        {
            gameentity &e = *(gameentity *)ents[n];
            bool on = m%2, spawned = e.spawned;
            if((e.spawned = on) == true) e.lastspawn = lastmillis;
            if(e.type == TRIGGER)
            {
                if((m >= 2 || e.lastemit <= 0 || e.spawned != spawned) && (e.attrs[1] == TR_TOGGLE || e.attrs[1] == TR_LINK || e.attrs[1] == TR_ONCE))
                {
                    e.lastemit = m <= 1 ? lastmillis-(e.lastemit ? max(triggertime(e)-(lastmillis-e.lastemit), 0) : triggertime(e)) : -1;
                    execlink(NULL, n, false);
                    loopv(e.kin) if(ents.inrange(e.kin[i]))
                    {
                        gameentity &f = *(gameentity *)ents[e.kin[i]];
                        f.spawned = e.spawned; f.lastemit = e.lastemit;
                        execlink(NULL, e.kin[i], false, n);
                    }
                }
            }
            else if(enttype[e.type].usetype == EU_ITEM)
            {
                loopv(projs::projs)
                {
                    projent &proj = *projs::projs[i];
                    if(proj.projtype != PRJ_ENT || proj.id != n) continue;
                    proj.beenused = 2;
                    proj.lifetime = min(proj.lifetime, proj.fadetime);
                }
            }
        }
    }

    extentity *newent() { return new gameentity; }
    void deleteent(extentity *e) { delete (gameentity *)e; }

    void clearents()
    {
        clearentcache();
        while(ents.length()) deleteent(ents.pop());
        memset(lastenttype, 0, sizeof(lastenttype));
        memset(lastusetype, 0, sizeof(lastusetype));
    }

    bool cansee(extentity &e)
    {
        return (showentinfo || game::player1->state == CS_EDITING) && (!enttype[e.type].noisy || showentnoisy >= 2 || (showentnoisy && game::player1->state == CS_EDITING));
    }

    void fixentity(int n, bool recurse, bool create)
    {
        gameentity &e = *(gameentity *)ents[n];
        int num = max(5, enttype[e.type].numattrs);
        while(e.attrs.length() < num) e.attrs.add(0);
        while(e.attrs.length() > num) e.attrs.pop();
        loopvrev(e.links)
        {
            int ent = e.links[i];
            if(!canlink(n, ent, verbose >= 2)) e.links.remove(i);
            else if(ents.inrange(ent))
            {
                gameentity &f = *(gameentity *)ents[ent];
                if(((enttype[e.type].reclink&(1<<f.type)) || (enttype[f.type].reclink&(1<<e.type))) && f.links.find(n) < 0)
                {
                    f.links.add(n);
                    if(verbose) conoutf("\frWARNING: automatic reciprocal link between %d and %d added", n, ent);
                }
                else continue;
                if(recurse || ent < n) fixentity(ent, false);
            }
            else continue;
        }
        if(issound(e.schan))
        {
            removesound(e.schan); e.schan = -1; // prevent clipping when moving around
            if(e.type == MAPSOUND) e.lastemit = lastmillis+1000;
        }
        switch(e.type)
        {
            case MAPMODEL:
                while(e.attrs[1] < 0) e.attrs[1] += 360;
                while(e.attrs[1] >= 360) e.attrs[1] -= 360;
                while(e.attrs[2] < -180) e.attrs[2] += 360;
                while(e.attrs[2] > 180) e.attrs[2] -= 360;
                while(e.attrs[3] < 0) e.attrs[3] += 101;
                while(e.attrs[3] >= 101) e.attrs[3] -= 101;
                if(e.attrs[4] < 0) e.attrs[4] = 0;
            case PARTICLES:
            case MAPSOUND:
            case LIGHTFX:
            {
                loopv(e.links) if(ents.inrange(e.links[i]) && ents[e.links[i]]->type == TRIGGER)
                {
                    e.lastemit = ents[e.links[i]]->lastemit;
                    e.spawned = TRIGSTATE(ents[e.links[i]]->spawned, ents[e.links[i]]->attrs[4]);
                }
                break;
            }
            case PUSHER:
            {
                while(e.attrs[0] < 0) e.attrs[0] += 360;
                while(e.attrs[0] >= 360) e.attrs[0] -= 360;
                while(e.attrs[1] < -90) e.attrs[1] += 180;
                while(e.attrs[1] > 90) e.attrs[1] -= 180;
                if(e.attrs[2] < 1) e.attrs[2] = 1;
                if(e.attrs[3] < 0) e.attrs[3] = 0;
                if(e.attrs[4] < 0 || e.attrs[4] >= (e.attrs[3] ? e.attrs[3] : enttype[PUSHER].radius)) e.attrs[4] = 0;
                if(e.attrs[5] < 0) e.attrs[5] += 3;
                if(e.attrs[5] >= 3) e.attrs[5] -= 3;
                break;
            }
            case TRIGGER:
            {
                while(e.attrs[1] < 0) e.attrs[1] += TR_MAX;
                while(e.attrs[1] >= TR_MAX) e.attrs[1] -= TR_MAX;
                while(e.attrs[2] < 0) e.attrs[2] += TA_MAX;
                while(e.attrs[2] >= TA_MAX) e.attrs[2] -= TA_MAX;
                while(e.attrs[4] < 0) e.attrs[4] += 4;
                while(e.attrs[4] >= 4) e.attrs[4] -= 4;
                if(e.attrs[4] >= 2)
                {
                    while(e.attrs[0] < 0) e.attrs[0] += TRIGGERIDS+1;
                    while(e.attrs[0] > TRIGGERIDS) e.attrs[0] -= TRIGGERIDS+1;
                }
                loopv(e.links) if(ents.inrange(e.links[i]) && (ents[e.links[i]]->type == MAPMODEL || ents[e.links[i]]->type == PARTICLES || ents[e.links[i]]->type == MAPSOUND || ents[e.links[i]]->type == LIGHTFX))
                {
                    ents[e.links[i]]->lastemit = e.lastemit;
                    ents[e.links[i]]->spawned = TRIGSTATE(e.spawned, e.attrs[4]);
                }
                break;
            }
            case WEAPON:
                if(create && (e.attrs[0] < WEAP_OFFSET || e.attrs[0] >= WEAP_MAX)) e.attrs[0] = WEAP_OFFSET; // don't be stupid when creating the entity
                while(e.attrs[0] < WEAP_OFFSET) e.attrs[0] += WEAP_MAX-WEAP_OFFSET; // don't allow superimposed weaps
                while(e.attrs[0] >= WEAP_MAX) e.attrs[0] -= WEAP_MAX-WEAP_OFFSET;
                while(e.attrs[2] <= -G_MAX) e.attrs[2] += G_MAX*2;
                while(e.attrs[2] >= G_MAX) e.attrs[2] -= G_MAX*2;
                break;
            case PLAYERSTART:
                while(e.attrs[0] < 0) e.attrs[0] += TEAM_COUNT;
                while(e.attrs[0] >= TEAM_COUNT) e.attrs[0] -= TEAM_COUNT;
            case CHECKPOINT:
                while(e.attrs[1] < 0) e.attrs[1] += 360;
                while(e.attrs[1] >= 360) e.attrs[1] -= 360;
                while(e.attrs[2] < -90) e.attrs[2] += 180;
                while(e.attrs[2] > 90) e.attrs[2] -= 180;
                while(e.attrs[3] <= -G_MAX) e.attrs[3] += G_MAX*2;
                while(e.attrs[3] >= G_MAX) e.attrs[3] -= G_MAX*2;
                if(e.type == CHECKPOINT)
                {
                    while(e.attrs[5] < 0) e.attrs[5] += CP_MAX;
                    while(e.attrs[5] >= CP_MAX) e.attrs[5] -= CP_MAX;
                }
                break;
            case ACTOR:
                while(e.attrs[0] <= 0) e.attrs[0] += AI_MAX-1;
                while(e.attrs[0] >= AI_MAX) e.attrs[0] -= AI_MAX-1;
                while(e.attrs[1] < 0) e.attrs[1] += 360;
                while(e.attrs[1] >= 360) e.attrs[1] -= 360;
                while(e.attrs[2] < -90) e.attrs[2] += 180;
                while(e.attrs[2] > 90) e.attrs[2] -= 180;
                while(e.attrs[3] <= -G_MAX) e.attrs[3] += G_MAX*2;
                while(e.attrs[3] >= G_MAX) e.attrs[3] -= G_MAX*2;
                while(e.attrs[4] < 0) e.attrs[4] += TRIGGERIDS+1;
                while(e.attrs[4] > TRIGGERIDS) e.attrs[4] -= TRIGGERIDS+1;
                while(e.attrs[5] < 0) e.attrs[5] += WEAP_MAX+1; // allow any weapon
                while(e.attrs[5] > WEAP_MAX) e.attrs[5] -= WEAP_MAX+1;
                if(e.attrs[6] < 0) e.attrs[6] = 0;
                if(e.attrs[7] < 0) e.attrs[7] = 0;
                break;
            case FLAG:
                while(e.attrs[0] < 0) e.attrs[0] += TEAM_COUNT;
                while(e.attrs[0] >= TEAM_COUNT) e.attrs[0] -= TEAM_COUNT;
                while(e.attrs[1] < 0) e.attrs[1] += 360;
                while(e.attrs[1] >= 360) e.attrs[1] -= 360;
                while(e.attrs[2] < -90) e.attrs[2] += 180;
                while(e.attrs[2] > 90) e.attrs[2] -= 180;
                while(e.attrs[3] <= -G_MAX) e.attrs[3] += G_MAX*2;
                while(e.attrs[3] >= G_MAX) e.attrs[3] -= G_MAX*2;
                break;
            case TELEPORT:
                while(e.attrs[0] < -1) e.attrs[0] += 361;
                while(e.attrs[0] >= 360) e.attrs[0] -= 360;
                while(e.attrs[2] < -90) e.attrs[2] += 180;
                while(e.attrs[2] > 90) e.attrs[2] -= 180;
                break;
            default:
                break;
        }
    }

    const char *findname(int type)
    {
        if(type >= NOTUSED && type < MAXENTTYPES) return enttype[type].name;
        return "";
    }

    int findtype(char *type)
    {
        loopi(MAXENTTYPES) if(!strcmp(type, enttype[i].name)) return i;
        return NOTUSED;
    }

    // these functions are called when the client touches the item
    void execlink(gameent *d, int index, bool local, int ignore)
    {
        if(ents.inrange(index) && maylink(ents[index]->type))
        {
            bool commit = false;
                int numents = max(lastenttype[MAPMODEL], max(lastenttype[LIGHTFX], max(lastenttype[PARTICLES], lastenttype[MAPSOUND])));
            loopi(numents) if(ents[i]->links.find(index) >= 0)
            {
                if(ents.inrange(ignore) && ents[ignore]->links.find(index) >= 0) continue;
                bool both = ents[index]->links.find(i) >= 0;
                switch(ents[i]->type)
                {
                    case MAPMODEL:
                    {
                        ents[i]->lastemit = ents[index]->lastemit;
                        if(ents[index]->type == TRIGGER) ents[i]->spawned = TRIGSTATE(ents[index]->spawned, ents[index]->attrs[4]);
                        break;
                    }
                    case LIGHTFX:
                    case PARTICLES:
                    {
                        ents[i]->lastemit = ents[index]->lastemit;
                        if(ents[index]->type == TRIGGER) ents[i]->spawned = TRIGSTATE(ents[index]->spawned, ents[index]->attrs[4]);
                        else if(local) commit = true;
                        break;
                    }
                    case MAPSOUND:
                    {
                        ents[i]->lastemit = ents[index]->lastemit;
                        if(ents[index]->type == TRIGGER) ents[i]->spawned = TRIGSTATE(ents[index]->spawned, ents[index]->attrs[4]);
                        else if(local) commit = true;
                        if(mapsounds.inrange(ents[i]->attrs[0]) && !issound(((gameentity *)ents[i])->schan))
                        {
                            int flags = SND_MAP;
                            loopk(SND_LAST)  if(ents[i]->attrs[4]&(1<<k)) flags |= 1<<k;
                            playsound(ents[i]->attrs[0], both ? ents[i]->o : ents[index]->o, NULL, flags, ents[i]->attrs[3], ents[i]->attrs[1], ents[i]->attrs[2], &((gameentity *)ents[i])->schan);
                        }
                        break;
                    }
                    default: break;
                }
            }
            if(d && commit) client::addmsg(N_EXECLINK, "ri2", d->clientnum, index);
        }
    }

    bool tryspawn(dynent *d, const vec &o, short yaw, short pitch)
    {
        d->yaw = yaw;
        d->pitch = pitch;
        d->roll = 0;
        d->o = vec(o).add(vec(0, 0, d->height+1));
        game::fixrange(d->yaw, d->pitch);
        return physics::entinmap(d, true);
    }

    void spawnplayer(gameent *d, int ent, bool suicide)
    {
        if(ent >= 0 && ents.inrange(ent))
        {
            switch(ents[ent]->type)
            {
                case ACTOR: if(d->type == ENT_PLAYER) break;
                case PLAYERSTART: case CHECKPOINT: if(tryspawn(d, ents[ent]->o, ents[ent]->attrs[1], ents[ent]->attrs[2])) return;
                default: if(tryspawn(d, ents[ent]->o, rnd(360), 0)) return;
            }
        }
        else if(d->type == ENT_PLAYER)
        {
            loopk(3)
            {
                vector<int> spawns;
                switch(k)
                {
                    case 0: if(m_fight(game::gamemode) && m_team(game::gamemode, game::mutators))
                                loopi(lastenttype[PLAYERSTART]) if(ents[i]->type == PLAYERSTART && ents[i]->attrs[0] == d->team && m_check(ents[i]->attrs[3], game::gamemode)) spawns.add(i);
                    case 1: if(spawns.empty()) loopi(lastenttype[PLAYERSTART]) if(ents[i]->type == PLAYERSTART && m_check(ents[i]->attrs[3], game::gamemode)) spawns.add(i);
                    case 2: if(spawns.empty()) loopi(lastenttype[WEAPON]) if(ents[i]->type == WEAPON && m_check(ents[i]->attrs[2], game::gamemode)) spawns.add(i);
                    default: break;
                }
                while(!spawns.empty())
                {
                    int r = rnd(spawns.length());
                    gameentity &e = *(gameentity *)ents[spawns[r]];
                    if(tryspawn(d, e.o, e.type == PLAYERSTART ? e.attrs[1] : rnd(360), e.type == PLAYERSTART ? e.attrs[2] : 0))
                        return;
                    spawns.remove(r); // must've really sucked, try another one
                }
            }
            d->yaw = d->pitch = d->roll = 0;
            d->o.x = d->o.y = d->o.z = getworldsize();
            d->o.x *= 0.5f; d->o.y *= 0.5f;
            if(physics::entinmap(d, true)) return;
        }
        if(!m_edit(game::gamemode) && suicide) game::suicide(d, HIT_SPAWN);
    }

    void editent(int i)
    {
        extentity &e = *ents[i];
        fixentity(i, true);
        if(m_edit(game::gamemode) && game::player1->state == CS_EDITING)
            client::addmsg(N_EDITENT, "ri5iv", i, (int)(e.o.x*DMF), (int)(e.o.y*DMF), (int)(e.o.z*DMF), e.type, e.attrs.length(), e.attrs.length(), e.attrs.getbuf()); // FIXME
        if(e.type >= NOTUSED && e.type < MAXENTTYPES)
        {
            lastenttype[e.type] = max(lastenttype[e.type], i+1);
            lastusetype[enttype[e.type].usetype] = max(lastusetype[enttype[e.type].usetype], i+1);
        }
        docacheclear = true;
    }

    float dropheight(extentity &e)
    {
        if(e.type==MAPMODEL || e.type==FLAG) return 0.0f;
        return 4.0f;
    }

    bool maylink(int type, int ver)
    {
        if(enttype[type].links && enttype[type].links <= (ver ? ver : GAMEVERSION))
                return true;
        return false;
    }

    bool canlink(int index, int node, bool msg)
    {
        if(ents.inrange(index) && ents.inrange(node))
        {
            if(index != node && maylink(ents[index]->type) && maylink(ents[node]->type) &&
                    (enttype[ents[index]->type].canlink&(1<<ents[node]->type)))
                        return true;
            if(msg)
                conoutf("\frentity %s (%d) and %s (%d) are not linkable", enttype[ents[index]->type].name, index, enttype[ents[node]->type].name, node);

            return false;
        }
        if(msg) conoutf("\frentity %d and %d are unable to be linked as one does not seem to exist", index, node);
        return false;
    }

    bool linkents(int index, int node, bool add, bool local, bool toggle)
    {
        if(ents.inrange(index) && ents.inrange(node) && index != node && canlink(index, node, local && verbose))
        {
            gameentity &e = *(gameentity *)ents[index], &f = *(gameentity *)ents[node];
            bool recip = (enttype[e.type].reclink&(1<<f.type)) || (enttype[f.type].reclink&(1<<e.type));
            int g = -1, h = -1;
            if((toggle || !add) && (g = e.links.find(node)) >= 0)
            {
                if(!add || (toggle && (!canlink(node, index) || (h = f.links.find(index)) >= 0)))
                {
                    e.links.remove(g);
                    if(recip) f.links.remove(h);
                    fixentity(index, true);
                    if(local && m_edit(game::gamemode)) client::addmsg(N_EDITLINK, "ri3", 0, index, node);
                    if(verbose > 2) conoutf("\faentity %s (%d) and %s (%d) delinked", enttype[ents[index]->type].name, index, enttype[ents[node]->type].name, node);
                    return true;
                }
                else if(toggle && canlink(node, index))
                {
                    f.links.add(index);
                    if(recip && (h = e.links.find(node)) < 0) e.links.add(node);
                    fixentity(node, true);
                    if(local && m_edit(game::gamemode)) client::addmsg(N_EDITLINK, "ri3", 1, node, index);
                    if(verbose > 2) conoutf("\faentity %s (%d) and %s (%d) linked", enttype[ents[node]->type].name, node, enttype[ents[index]->type].name, index);
                    return true;
                }
            }
            else if(toggle && canlink(node, index) && (g = f.links.find(index)) >= 0)
            {
                f.links.remove(g);
                if(recip && (h = e.links.find(node)) >= 0) e.links.remove(h);
                fixentity(node, true);
                if(local && m_edit(game::gamemode)) client::addmsg(N_EDITLINK, "ri3", 0, node, index);
                if(verbose > 2) conoutf("\faentity %s (%d) and %s (%d) delinked", enttype[ents[node]->type].name, node, enttype[ents[index]->type].name, index);
                return true;
            }
            else if(toggle || add)
            {
                e.links.add(node);
                if(recip && (h = f.links.find(index)) < 0) f.links.add(index);
                fixentity(index, true);
                if(local && m_edit(game::gamemode)) client::addmsg(N_EDITLINK, "ri3", 1, index, node);
                if(verbose > 2) conoutf("\faentity %s (%d) and %s (%d) linked", enttype[ents[index]->type].name, index, enttype[ents[node]->type].name, node);
                return true;
            }
        }
        if(verbose > 2)
            conoutf("\frentity %s (%d) and %s (%d) failed linking", enttype[ents[index]->type].name, index, enttype[ents[node]->type].name, node);
        return false;
    }

    struct linkq
    {
        uint id;
        float curscore, estscore;
        linkq *prev;

        linkq() : id(0), curscore(0.f), estscore(0.f), prev(NULL) {}

        float score() const { return curscore + estscore; }
    };

    static inline float heapscore(linkq *q) { return q->score(); }

    float route(int node, int goal, vector<int> &route, const avoidset &obstacles, gameent *d, bool check)
    {
        if(!ents.inrange(node) || !ents.inrange(goal) || ents[goal]->type != ents[node]->type || goal == node || ents[node]->links.empty())
            return 0;

        static uint routeid = 1;
        static vector<linkq> nodes;
        static vector<linkq *> queue;

        int routestart = verbose >= 3 ? SDL_GetTicks() : 0;

        if(!routeid)
        {
            loopv(nodes) nodes[i].id = 0;
            routeid = 1;
        }
        while(nodes.length() < ents.length()) nodes.add();

        if(d)
        {
            if(d->ai) loopi(ai::NUMPREVNODES) if(d->ai->prevnodes[i] != node && nodes.inrange(d->ai->prevnodes[i]))
            {
                nodes[d->ai->prevnodes[i]].id = routeid;
                nodes[d->ai->prevnodes[i]].curscore = -1.f;
                nodes[d->ai->prevnodes[i]].estscore = 0.f;
            }
            if(check)
            {
                loopavoid(obstacles, d, { if(ents.inrange(ent) && ents[ent]->type == ents[node]->type)
                {
                    if(ent != node && ents[node]->links.find(ent) < 0)
                    {
                        nodes[ent].id = routeid;
                        nodes[ent].curscore = -1.f;
                        nodes[ent].estscore = 0.f;
                    }
                }});
            }
        }

        nodes[node].id = routeid;
        nodes[node].curscore = 0.f;
        nodes[node].estscore = 0.f;
        nodes[node].prev = NULL;
        queue.setsize(0);
        queue.add(&nodes[node]);
        route.setsize(0);

        int lowest = -1;
        while(!queue.empty())
        {
            linkq *m = queue.removeheap();
            float prevscore = m->curscore;
            m->curscore = -1.f;
            int current = int(m-&nodes[0]);
            if(!ents.inrange(current)) continue;
            extentity &ent = *ents[current];
            vector<int> &links = ent.links;
            loopv(links)
            {
                int link = links[i];
                if(ents.inrange(link) && ents[link]->type == ents[node]->type && (link == node || link == goal || !ents[link]->links.empty()))
                {
                    linkq &n = nodes[link];
                    float curscore = prevscore + ents[link]->o.dist(ent.o);
                    if(n.id == routeid && curscore >= n.curscore) continue;
                    n.curscore = curscore;
                    n.prev = m;
                    if(n.id != routeid)
                    {
                        n.estscore = ents[link]->o.dist(ents[goal]->o);
                        if(n.estscore <= float(enttype[ents[link]->type].radius*4) && (lowest < 0 || n.estscore < nodes[lowest].estscore))
                            lowest = link;
                        n.id = routeid;
                        if(link == goal) goto foundgoal;
                        queue.addheap(&n);
                    }
                    else loopvj(queue) if(queue[j] == &n) { queue.upheap(j); break; }
                }
            }
        }
        foundgoal:

        routeid++;
        float score = 0;
        if(lowest >= 0) // otherwise nothing got there
        {
            for(linkq *m = &nodes[lowest]; m != NULL; m = m->prev)
                route.add(m - &nodes[0]); // just keep it stored backward
            if(!route.empty()) score = nodes[lowest].score();
        }

        if(verbose >= 3)
            conoutf("\faroute %d to %d (%d) generated %d nodes (%d queued) in %.3fs", node, goal, lowest, route.length(), queue.length(), (SDL_GetTicks()-routestart)/1000.0f);

        return score;
    }

    void entitylink(int index, int node, bool both = true)
    {
        if(ents.inrange(index) && ents.inrange(node))
        {
            gameentity &e = *(gameentity *)ents[index], &f = *(gameentity *)ents[node];
            if(e.links.find(node) < 0) linkents(index, node, true, true, false);
            if(both && f.links.find(index) < 0) linkents(node, index, true, true, false);
        }
    }

    bool clipped(const vec &o, bool aiclip)
    {
        int material = lookupmaterial(o), clipmat = material&MATF_CLIP;
        if(clipmat == MAT_CLIP || (aiclip && clipmat == MAT_AICLIP)) return true;
        if(material&MAT_DEATH) return true;
        if((material&MATF_VOLUME) == MAT_LAVA) return true;
        return false;
    }

    void entitycheck(gameent *d)
    {
        int cleanairnodes = 0;
        if(d->state == CS_ALIVE)
        {
            vec v = d->feetpos();
            bool clip = clipped(v, true), shoulddrop = waypointdrop() && !d->ai && !clip;
            float dist = float(shoulddrop ? enttype[WAYPOINT].radius : (d->ai ? ai::JUMPMIN : ai::SIGHTMIN));
            int curnode = closestent(WAYPOINT, v, dist, false), prevnode = d->lastnode;

            if(!ents.inrange(curnode) && shoulddrop)
            {
                int cmds = WP_F_NONE;
                if(physics::iscrouching(d)) cmds |= WP_F_CROUCH;
                curnode = ents.length();
                static vector<int> wpattrs;
                wpattrs.setsize(0);
                wpattrs.add(cmds);
                newentity(v, WAYPOINT, wpattrs);
                if(d->physstate == PHYS_FALL) d->airnodes.add(curnode);
            }

            if(ents.inrange(curnode))
            {
                if(shoulddrop && ents.inrange(d->lastnode) && d->lastnode != curnode)
                    entitylink(d->lastnode, curnode, d->physstate != PHYS_FALL && !d->onladder);
                d->lastnode = curnode;
            }
            else if(!ents.inrange(d->lastnode) || ents[d->lastnode]->o.squaredist(v) > ai::CLOSEDIST*ai::CLOSEDIST)
                d->lastnode = closestent(WAYPOINT, v, ai::SIGHTMAX, false);

            if(clip) cleanairnodes = 2;
            else if(d->physstate != PHYS_FALL) cleanairnodes = 1;
            if(d->ai && ents.inrange(prevnode) && d->lastnode != prevnode) d->ai->addprevnode(prevnode);
        }
        else
        {
            d->lastnode = -1;
            cleanairnodes = 2;
        }

        if(cleanairnodes && !d->airnodes.empty())
        {
            if(cleanairnodes > 1)
            {
                loopv(d->airnodes) if(ents.inrange(d->airnodes[i]))
                    ents[d->airnodes[i]]->type = NOTUSED;
                loopvk(ents) if(!ents[k]->links.empty())
                {
                    loopvrev(ents[k]->links) if(d->airnodes.find(ents[k]->links[i]) >= 0)
                        ents[k]->links.remove(i);
                }
            }
            d->airnodes.setsize(0);
        }
    }

    void readent(stream *g, int mtype, int mver, char *gid, int gver, int id)
    {
        gameentity &f = *(gameentity *)ents[id];
        f.mark = 0;
        if(mtype == MAP_OCTA)
        {
            // translate into our format
            switch(f.type)
            {
                // LIGHT            -   LIGHT
                // MAPMODEL         -   MAPMODEL
                // PLAYERSTART      -   PLAYERSTART
                // ENVMAP           -   ENVMAP
                // PARTICLES        -   PARTICLES
                // MAPSOUND         -   MAPSOUND
                // SPOTLIGHT        -   LIGHTFX
                //                  -   SUNLIGHT
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
                {
                    break;
                }

                // I_SHELLS         -   WEAPON      WEAP_SHOTGUN
                // I_BULLETS        -   WEAPON      WEAP_SMG
                // I_ROCKETS        -   WEAPON      WEAP_PLASMA
                // I_ROUNDS         -   WEAPON      WEAP_RIFLE
                // I_GL             -   WEAPON      WEAP_GRENADE
                // I_CARTRIDGES     -   WEAPON      WEAP_PISTOL
                case 9: case 10: case 11: case 12: case 13: case 14:
                {
                    int weap = f.type-8, weapmap[6] = {
                        WEAP_SHOTGUN, WEAP_SMG, WEAP_PLASMA, WEAP_RIFLE, WEAP_GRENADE, WEAP_SWORD
                    };

                    if(weap >= 0 && weap <= 5)
                    {
                        f.type = WEAPON;
                        f.attrs[0] = weapmap[weap];
                        f.attrs[1] = 0;
                    }
                    else f.type = NOTUSED;
                    break;
                }
                // I_QUAD           -   WEAPON      WEAP_FLAMER
                case 19:
                {
                    f.type = WEAPON;
                    f.attrs[0] = WEAP_FLAMER;
                    f.attrs[1] = 0;
                    break;
                }

                // TELEPORT         -   TELEPORT
                // TELEDEST         -   TELEPORT (linked)
                case 20: case 21:
                {
                    if(f.type == 21)
                    {
                        f.mark = f.attrs[1]+1; // needs translating later
                        f.attrs[1] = -1;
                    }
                    else
                    {
                        f.mark = -(f.attrs[0]+1);
                        f.attrs[0] = -1;
                    }
                    f.attrs[2] = f.attrs[3] = f.attrs[4] = 0;
                    f.type = TELEPORT;
                    break;
                }
                // MONSTER          -   NOTUSED
                case 22:
                {
                    f.type = NOTUSED;
                    break;
                }
                // CARROT           -   TRIGGER     0
                case 23:
                {
                    f.type = NOTUSED;
                    f.attrs[0] = f.attrs[1] = f.attrs[2] = f.attrs[3] = f.attrs[4] = 0;
                    break;
                }
                // JUMPPAD          -   PUSHER
                case 24:
                {
                    f.type = PUSHER;
                    break;
                }
                // BASE             -   FLAG        1:idx       TEAM_NEUTRAL
                case 25:
                {
                    f.type = FLAG;
                    if(f.attrs[0] < 0) f.attrs[0] = 0;
                    f.attrs[1] = TEAM_NEUTRAL; // spawn as neutrals
                    break;
                }
                // RESPAWNPOINT     -   CHECKPOINT
                case 26:
                {
                    f.type = CHECKPOINT;
                    break;
                }
                // FLAG             -   FLAG        #           2:team
                case 31:
                {
                    f.type = FLAG;
                    f.attrs[0] = 0;
                    if(f.attrs[1] <= 0) f.attrs[1] = -1; // needs a team
                    break;
                }

                // I_HEALTH         -   NOTUSED
                // I_BOOST          -   NOTUSED
                // I_GREENARMOUR    -   NOTUSED
                // I_YELLOWARMOUR   -   NOTUSED
                // BOX              -   NOTUSED
                // BARREL           -   NOTUSED
                // PLATFORM     -   NOTUSED
                // ELEVATOR     -   NOTUSED
                default:
                {
                    if(verbose) conoutf("\frWARNING: ignoring entity %d type %d", id, f.type);
                    f.type = NOTUSED;
                    break;
                }
            }
        }
    }

    void writeent(stream *g, int id)
    {
    }

    void remapents(vector<int> &idxs)
    {
        int numents[MAXENTTYPES], numinvalid = 0;
        memset(numents, 0, sizeof(numents));
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type >= NOTUSED && e.type < MAXENTTYPES) numents[e.type]++;
            else numinvalid++;
        }
        int offsets[MAXENTTYPES];
        memset(offsets, -1, sizeof(offsets));
        int priority = INT_MIN, nextpriority = INT_MIN;
        loopi(MAXENTTYPES) nextpriority = max(nextpriority, enttype[i].priority);
        int offset = 0;
        do
        {
            priority = nextpriority;
            nextpriority = INT_MIN;
            loopi(MAXENTTYPES) if(offsets[i] < 0)
            {
                if(enttype[i].priority >= priority) { offsets[i] = offset; offset += numents[i]; }
                else nextpriority = max(nextpriority, enttype[i].priority);
            }
        } while(nextpriority < priority);
        idxs.setsize(0);
        idxs.reserve(offset + numinvalid);
        while(idxs.length() < offset + numinvalid) idxs.add(-1);
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            idxs[e.type >= NOTUSED && e.type < MAXENTTYPES ? offsets[e.type]++ : offset++] = i;
        }
    }

    void importentities(int mtype, int mver, int gver)
    {
        int flag = 0, teams[TEAM_NUM] = {0};
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(verbose) progress(float(i)/float(ents.length()), "importing entities...");

            switch(e.type)
            {
                case TELEPORT:
                {
                    if(e.mark > 0) // translate teledest to teleport and link them appropriately
                    {
                        loopvj(ents) if(j != i) // find linked teleport(s)
                        {
                            gameentity &f = *(gameentity *)ents[j];
                            if(f.type == TELEPORT && -f.mark == e.mark)
                            {
                                if(verbose) conoutf("\frWARNING: teledest %d and teleport %d linked automatically", i, j);
                                f.links.add(i);
                            }
                        }
                    }
                    break;
                }
                case WEAPON:
                {
                    float mindist = float(enttype[WEAPON].radius*enttype[WEAPON].radius*6);
                    int weaps[WEAP_MAX];
                    loopj(WEAP_MAX) weaps[j] = j != e.attrs[0] ? 0 : 1;
                    loopvj(ents) if(j != i)
                    {
                        gameentity &f = *(gameentity *)ents[j];
                        if(f.type == WEAPON && e.o.squaredist(f.o) <= mindist && isweap(f.attrs[0]))
                        {
                            weaps[f.attrs[0]]++;
                            f.type = NOTUSED;
                            if(verbose) conoutf("\frWARNING: culled tightly packed weapon %d [%d]", j, f.attrs[0]);
                        }
                    }
                    int best = e.attrs[0];
                    loopj(WEAP_MAX) if(weaps[j] > weaps[best]) best = j;
                    e.attrs[0] = best;
                    break;
                }
                case FLAG: // replace bases/neutral flags near team flags
                {
                    if(valteam(e.attrs[1], TEAM_FIRST)) teams[e.attrs[1]-TEAM_FIRST]++;
                    else if(e.attrs[1] == TEAM_NEUTRAL)
                    {
                        int dest = -1;

                        loopvj(ents) if(j != i)
                        {
                            gameentity &f = *(gameentity *)ents[j];

                            if(f.type == FLAG && f.attrs[1] != TEAM_NEUTRAL &&
                                (!ents.inrange(dest) || e.o.dist(f.o) < ents[dest]->o.dist(f.o)) &&
                                    e.o.dist(f.o) <= enttype[FLAG].radius*4.f)
                                        dest = j;
                        }

                        if(ents.inrange(dest))
                        {
                            gameentity &f = *(gameentity *)ents[dest];
                            if(verbose) conoutf("\frWARNING: old base %d (%d, %d) replaced with flag %d (%d, %d)", i, e.attrs[0], e.attrs[1], dest, f.attrs[0], f.attrs[1]);
                            if(!f.attrs[0]) f.attrs[0] = e.attrs[0]; // give it the old base idx
                            e.type = NOTUSED;
                        }
                        else if(e.attrs[0] > flag) flag = e.attrs[0]; // find the highest idx
                    }
                    break;
                }
            }
        }

        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];

            switch(e.type)
            {
                case TELEPORT:
                {
                    if(e.mark > 0) // second pass teledest translation
                    {
                        int dest = -1;
                        loopvj(ents) if(j != i) // see if this teledest is sitting on top of a teleport already
                        {
                            gameentity &f = *(gameentity *)ents[j];

                            if(f.type == TELEPORT && f.mark < 0 &&
                                (!ents.inrange(dest) || e.o.dist(f.o) < ents[dest]->o.dist(f.o)) &&
                                    e.o.dist(f.o) <= enttype[TELEPORT].radius*4.f)
                                        dest = j;
                        }
                        if(ents.inrange(dest))
                        {
                            gameentity &f = *(gameentity *)ents[dest];
                            if(verbose) conoutf("\frWARNING: replaced teledest %d with closest teleport %d", i, dest);
                            f.attrs[0] = e.attrs[0]; // copy the yaw
                            loopvk(e.links) if(f.links.find(e.links[k]) < 0) f.links.add(e.links[k]);
                            loopvj(ents) if(j != i && j != dest)
                            {
                                gameentity &g = *(gameentity *)ents[j];
                                if(g.type == TELEPORT)
                                {
                                    int link = g.links.find(i);
                                    if(link >= 0)
                                    {
                                        g.links.remove(link);
                                        if(g.links.find(dest) < 0) g.links.add(dest);
                                        if(verbose) conoutf("\frWARNING: imported link to teledest %d to teleport %d", i, j);
                                    }
                                }
                            }
                            e.type = NOTUSED; // get rid of ye olde teledest
                            e.links.shrink(0);
                            break;
                        }
                        else if(verbose) conoutf("\frWARNING: teledest %d has become a teleport", i);
                    }
                    break;
                }
                case FLAG:
                {
                    if(!e.attrs[0]) e.attrs[0] = ++flag; // assign a sane idx
                    if(!valteam(e.attrs[1], TEAM_NEUTRAL)) // assign a team
                    {
                        int lowest = -1;
                        loopk(TEAM_NUM) if(lowest<0 || teams[k] < teams[lowest]) lowest = i;
                        e.attrs[1] = lowest+TEAM_FIRST;
                        teams[lowest]++;
                    }
                    break;
                }
            }
        }
    }

    void updateoldentities(int mtype, int mver, int gver)
    {
        loopvj(ents)
        {
            gameentity &e = *(gameentity *)ents[j];
            if(verbose) progress(float(j)/float(ents.length()), "updating old entities...");
            switch(e.type)
            {
                case LIGHTFX:
                {
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 159))
                    {
                        e.attrs[1] = e.attrs[0];
                        e.attrs[0] = LFX_SPOTLIGHT;
                        e.attrs[2] = e.attrs[3] = e.attrs[4] = 0;
                    }
                    break;
                }
                case PLAYERSTART:
                {
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 158))
                    {
                        short yaw = e.attrs[0];
                        e.attrs[0] = e.attrs[1];
                        e.attrs[1] = yaw;
                        e.attrs[2] = e.attrs[3] = e.attrs[4] = 0;
                    }
                    if(mtype == MAP_BFGZ && gver <= 164 && e.attrs[0] > TEAM_LAST) e.attrs[0] = TEAM_NEUTRAL;
                    break;
                }
                case PARTICLES:
                {
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && mver <= 36))
                    {
                        switch(e.attrs[0])
                        {
                            case 0: if(e.attrs[3] <= 0) break;
                            case 4: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 5: case 6:
                                e.attrs[3] = (((e.attrs[3]&0xF)<<4)|((e.attrs[3]&0xF0)<<8)|((e.attrs[3]&0xF00)<<12))+0x0F0F0F;
                                if(e.attrs[0] != 5 && e.attrs[0] != 6) break;
                            case 3:
                                e.attrs[2] = (((e.attrs[2]&0xF)<<4)|((e.attrs[2]&0xF0)<<8)|((e.attrs[2]&0xF00)<<12))+0x0F0F0F; break;
                            default: break;
                        }
                    }
                    break;
                }
                case TELEPORT:
                {
                    if(mtype == MAP_OCTA)
                    {
                        e.attrs[2] = 100; // give a push
                        e.attrs[4] = e.attrs[1] >= 0 ? 0x2CE : 0;
                        e.attrs[1] = e.attrs[3] = 0;
                        e.o.z += 8; // teleport in BF is at middle
                        if(e.attrs[0] >= 0 && clipped(e.o))
                        {
                            vec dir;
                            vecfromyawpitch(e.attrs[0], e.attrs[1], 1, 0, dir);
                            e.o.add(dir);
                        }
                        e.mark = 0;
                    }
                    if(e.attrs[0] >= 0 && ((mtype == MAP_OCTA && mver <= 30) || (mtype == MAP_BFGZ && mver <= 39))) e.attrs[0] = (e.attrs[0] + 180)%360;
                    break;
                }
                case WEAPON:
                {
                    if(mtype == MAP_BFGZ && gver <= 90)
                    { // move grenade to the end of the weapon array
                        if(e.attrs[0] >= 4) e.attrs[0]--;
                        else if(e.attrs[0] == 3) e.attrs[0] = WEAP_GRENADE;
                    }
                    if(mtype == MAP_BFGZ && gver <= 97 && e.attrs[0] >= 4)
                        e.attrs[0]++; // add in pistol
                    if(mtype != MAP_BFGZ || gver <= 112) e.attrs[1] = 0;
                    if(mtype == MAP_BFGZ && gver <= 160)
                    {
                        e.attrs[0]++; // add in melee
                        if(e.attrs[0] < WEAP_OFFSET) e.attrs[0] = WEAP_GRENADE; // cleanup for fixentity
                    }
                    if(mtype == MAP_BFGZ && gver <= 163) e.attrs[0]++; // add in impulse
                    break;
                }
                case TRIGGER:
                {
                    if(mtype == MAP_BFGZ && gver <= 158) e.attrs[4] = 0;
                    break;
                }
                case PUSHER:
                {
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 95)) e.attrs[0] = int(e.attrs[0]*1.25f);
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 162))
                    {
                        vec dir = vec(e.attrs[2], e.attrs[1], e.attrs[0]).mul(10);
                        float yaw = 0, pitch = 0;
                        vectoyawpitch(vec(dir).normalize(), yaw, pitch);
                        e.attrs[0] = int(yaw);
                        e.attrs[1] = int(pitch);
                        e.attrs[2] = int(dir.magnitude());
                        e.attrs[5] = 0;
                    }
                    else if(mtype == MAP_BFGZ && mver <= 39) e.attrs[0] = (e.attrs[0] + 180)%360;
                    break;
                }
                case FLAG:
                {
                    if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 158))
                    {
                        e.attrs[0] = e.attrs[1];
                        e.attrs[1] = e.attrs[2];
                        e.attrs[2] = e.attrs[3];
                        e.attrs[3] = e.attrs[4] = 0;
                    }
                    if(mtype == MAP_BFGZ && gver <= 164 && e.attrs[0] > TEAM_LAST) e.attrs[0] = TEAM_NEUTRAL;
                    if((mtype == MAP_OCTA && mver <= 30) || (mtype == MAP_BFGZ && mver <= 39)) e.attrs[1] = (e.attrs[1] + 180)%360;
                    break;
                }
                case WAYPOINT:
                {
                    if(!m_edit(game::gamemode) && clipped(e.o, true)) e.type = NOTUSED;
                    else if(mtype == MAP_BFGZ)
                    {
                        if(gver <= 90) e.attrs[0] = e.attrs[1] = e.attrs[2] = e.attrs[3] = e.attrs[4] = 0;
                        if(gver <= 165 && gver >= 160)
                        {
                            e.attrs[0] = e.attrs[4]; // for a short while we had a mess of attributes
                            e.attrs[1] = e.attrs[2] = e.attrs[3] = e.attrs[4] = 0;
                        }
                    }
                    break;
                }
                case CHECKPOINT: case ACTOR:
                    if((mtype == MAP_OCTA && mver <= 30) || (mtype == MAP_BFGZ && mver <= 39)) e.attrs[1] = (e.attrs[1] + 180)%360;
                    break;
                default: break;
            }
        }
    }

    void importwaypoints(int mtype, int mver, int gver)
    {
        const char *mname = mapname;
        if(!mname || !*mname) return;
        string wptname;
        formatstring(wptname)("%s.wpt", mname);

        stream *f = opengzfile(wptname, "rb");
        if(!f) return;
        char magic[4];
        if(f->read(magic, 4) < 4 || memcmp(magic, "OWPT", 4)) { delete f; return; }

        int numents = ents.length()-1; // -1 because OCTA waypoints count from 1 upward
        ushort numwp = f->getlil<ushort>();
        loopi(numwp)
        {
            if(f->end()) break;
            vec o;
            o.x = f->getlil<float>();
            o.y = f->getlil<float>();
            o.z = f->getlil<float>();
            extentity *e = NULL;
            e = newent();
            ents.add(e);
            loopk(5) e->attrs.add(0);
            e->type = !m_edit(game::gamemode) && clipped(o, true) ? NOTUSED : WAYPOINT;
            e->o = o;
            int numlinks = clamp(f->getchar(), 0, 6);
            loopi(numlinks) e->links.add(numents+f->getlil<ushort>());
        }
        delete f;
        conoutf("loaded %d waypoints from %s", numwp, wptname);
    }

    void initents(stream *g, int mtype, int mver, char *gid, int gver)
    {
        haswaypoints = false;
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            int num = max(5, enttype[e.type].numattrs);
            while(e.attrs.length() < num) e.attrs.add(0);
            while(e.attrs.length() > num) e.attrs.pop();
        }
        if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver <= 49)) importentities(mtype, mver, gver);
        if(mtype == MAP_OCTA || (mtype == MAP_BFGZ && gver < GAMEVERSION)) updateoldentities(mtype, mver, gver);
        if(mtype == MAP_OCTA) importwaypoints(mtype, mver, gver);
        loopv(ents)
        {
            fixentity(i, false);
            if(!haswaypoints && ents[i]->type == WAYPOINT) haswaypoints = true;
        }
        memset(lastenttype, 0, sizeof(lastenttype));
        memset(lastusetype, 0, sizeof(lastusetype));
        loopv(ents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type >= NOTUSED && e.type < MAXENTTYPES)
            {
                lastenttype[e.type] = max(lastenttype[e.type], i+1);
                lastusetype[enttype[e.type].usetype] = max(lastusetype[enttype[e.type].usetype], i+1);
            }
            if(enttype[e.type].usetype == EU_ITEM || e.type == TRIGGER)
            {
                setspawn(i, 0);
                if(e.type == TRIGGER) // find shared kin
                {
                    loopvj(e.links) if(ents.inrange(e.links[j]))
                    {
                        loopvk(ents) if(ents[k]->type == TRIGGER && ents[k]->links.find(e.links[j]) >= 0)
                        {
                            if(((gameentity *)ents[i])->kin.find(k) < 0) ((gameentity *)ents[i])->kin.add(k);
                            if(((gameentity *)ents[k])->kin.find(i) < 0) ((gameentity *)ents[k])->kin.add(i);
                        }
                    }
                }
            }
        }
        clearentcache();
    }

    void edittoggled(bool edit) { clearentcache(); }

    #define renderfocus(i,f) { gameentity &e = *(gameentity *)ents[i]; f; }

    void renderlinked(gameentity &e, int idx)
    {
        loopv(e.links)
        {
            int index = e.links[i];
            if(ents.inrange(index))
            {
                gameentity &f = *(gameentity *)ents[index];
                bool both = false;
                loopvj(f.links) if(f.links[j] == idx)
                {
                    both = true;
                    break;
                }
                part_trace(e.o, f.o, 1, 1, 1, both ? 0xAA44CC : 0x660088);
            }
        }
    }

    bool shouldshowents(int level)
    {
        return max(showentradius, max(showentdir, showentlinks)) >= level || dropwaypoints >= 2 || ai::aidebug >= 6;
    }

    void renderentshow(gameentity &e, int idx, int level)
    {
        if(e.o.squaredist(camera1->o) > maxparticledistance*maxparticledistance) return;
        #define entdirpart(o,yaw,pitch,size,fade,colour) { vec pos = o; part_dir(pos, yaw, pitch, size, 1, fade, colour); pos.z -= 0.1f; part_dir(pos, yaw, pitch, size, 1, fade, 0x000000); }
        if(showentradius >= level)
        {
            switch(e.type)
            {
                case PLAYERSTART:
                {
                    part_radius(vec(e.o).add(vec(0, 0, game::player1->zradius/2)), vec(game::player1->xradius, game::player1->yradius, game::player1->zradius/2), 1, 1, 1, teamtype[e.type == PLAYERSTART ? e.attrs[0] : TEAM_NEUTRAL].colour);
                    break;
                }
                case ACTOR:
                {
                    part_radius(vec(e.o).add(vec(0, 0, aistyle[e.attrs[0]].height/2)), vec(aistyle[e.attrs[0]].xradius, aistyle[e.attrs[0]].height/2), 1, 1, 1, 0x888888);
                    part_radius(e.o, vec(ai::SIGHTMAX*2, ai::SIGHTMAX*2, ai::SIGHTMAX*2), 1, 1, 1, 0x888888);
                    break;
                }
                case MAPSOUND:
                {
                    part_radius(e.o, vec(e.attrs[1], e.attrs[1], e.attrs[1]), 1, 1, 1, 0x00FFFF);
                    part_radius(e.o, vec(e.attrs[2], e.attrs[2], e.attrs[2]), 1, 1, 1, 0x00FFFF);
                    break;
                }
                case ENVMAP:
                {
                    int s = e.attrs[0] ? clamp(e.attrs[0], 0, 10000) : envmapradius;
                    part_radius(e.o, vec(s, s, s), 1, 1, 1, 0x00FFFF);
                    break;
                }
                case LIGHT:
                {
                    int s = e.attrs[0] ? e.attrs[0] : hdr.worldsize,
                        colour = ((e.attrs[1])<<16)|((e.attrs[2])<<8)|(e.attrs[3]);
                    part_radius(e.o, vec(s, s, s), 1, 1, 1, colour);
                    break;
                }
                case LIGHTFX:
                {
                    if(e.attrs[0] == LFX_SPOTLIGHT) loopv(e.links) if(ents.inrange(e.links[i]) && ents[e.links[i]]->type == LIGHT)
                    {
                        gameentity &f = *(gameentity *)ents[e.links[i]];
                        float radius = f.attrs[0];
                        if(!radius) radius = 2*e.o.dist(f.o);
                        vec dir = vec(e.o).sub(f.o).normalize();
                        float angle = max(1, min(90, int(e.attrs[1])));
                        int colour = ((f.attrs[1]/2)<<16)|((f.attrs[2]/2)<<8)|(f.attrs[3]/2);
                        part_cone(f.o, dir, radius, angle, 1, 1, 1, colour);
                        break;
                    }
                    break;
                }
                case SUNLIGHT:
                {
                    int colour = ((e.attrs[2]/2)<<16)|((e.attrs[3]/2)<<8)|(e.attrs[4]/2), offset = e.attrs[5] ? e.attrs[5] : 10, yaw = e.attrs[0], pitch = e.attrs[1]+90;
                    vec dir(yaw*RAD, pitch*RAD);
                    static const float offsets[9][2] = { { 0, 0 }, { 0, 1 }, { 90, 1 }, { 180, 1 }, { 270, 1 }, { 45, 0.5f }, { 135, 0.5f }, { 225, 0.5f }, { 315, 0.5f } };
                    loopk(9)
                    {
                        vec spoke(yaw*RAD, (pitch + offset*offsets[k][1])*RAD);
                        spoke.rotate(offsets[k][0]*RAD, dir);
                        float syaw, spitch;
                        vectoyawpitch(spoke, syaw, spitch);
                        entdirpart(e.o, syaw, spitch, getworldsize()*2, 1, colour);
                    }
                    break;
                }
                case FLAG:
                {
                    float radius = (float)enttype[e.type].radius;
                    part_radius(e.o, vec(radius, radius, radius), 1, 1, 1, teamtype[e.attrs[0]].colour);
                    radius = radius*2/3; // ctf pickup dist
                    part_radius(e.o, vec(radius, radius, radius), 1, 1, 1, teamtype[e.attrs[0]].colour);
                    break;
                }
                case WAYPOINT:
                {
                    part_radius(e.o, vec(enttype[e.type].radius, enttype[e.type].radius, enttype[e.type].radius), 1, 1, 1, 0x008888);
                    break;
                }
                default:
                {
                    float radius = (float)enttype[e.type].radius;
                    if((e.type == TRIGGER || e.type == TELEPORT || e.type == PUSHER || e.type == CHECKPOINT) && e.attrs[e.type == CHECKPOINT ? 0 : 3])
                        radius = (float)e.attrs[e.type == CHECKPOINT ? 0 : 3];
                    if(radius > 0.f) part_radius(e.o, vec(radius, radius, radius), 1, 1, 1, 0x00FFFF);
                    if(e.type == PUSHER && e.attrs[4] && e.attrs[4] < e.attrs[3])
                        part_radius(e.o, vec(e.attrs[4], e.attrs[4], e.attrs[4]), 1, 1, 1, 0x00FFFF);
                    break;
                }
            }
        }

        if(showentdir >= level)
        {
            switch(e.type)
            {
                case PLAYERSTART: case CHECKPOINT:
                {
                    entdirpart(e.o, e.attrs[1], e.attrs[2], 4.f, 1, teamtype[e.type == PLAYERSTART ? e.attrs[0] : TEAM_NEUTRAL].colour);
                    break;
                }
                case MAPMODEL:
                {
                    entdirpart(e.o, e.attrs[1], 360-e.attrs[2], 4.f, 1, 0x00FFFF);
                    break;
                }
                case ACTOR:
                {
                    entdirpart(e.o, e.attrs[1], e.attrs[2], 4.f, 1, 0xAAAAAA);
                    break;
                }
                case TELEPORT:
                case CAMERA:
                {
                    if(e.attrs[0] < 0) { entdirpart(e.o, (lastmillis/5)%360, e.attrs[1], 4.f, 1, 0x00FFFF); }
                    else { entdirpart(e.o, e.attrs[0], e.attrs[1], 8.f, 1, 0x00FFFF); }
                    break;
                }
                case PUSHER:
                {
                    entdirpart(e.o, e.attrs[0], e.attrs[1], 4.f+e.attrs[2], 1, 0x00FFFF);
                    break;
                }
                default: break;
            }
        }
        if(enttype[e.type].links && (showentlinks >= level || (e.type == WAYPOINT && (dropwaypoints >= 2 || ai::aidebug >= 6))))
            renderlinked(e, idx);
    }

    void renderentlight(gameentity &e)
    {
        adddynlight(vec(e.o), float(e.attrs[0] ? e.attrs[0] : hdr.worldsize)*0.75f, vec(e.attrs[1], e.attrs[2], e.attrs[3]).div(383.f), 0, 0, DL_KEEP);
    }

    void adddynlights()
    {
        if(game::player1->state == CS_EDITING && showlighting)
        {
            #define islightable(q) ((q)->type == LIGHT && (q)->attrs[0] > 0 && !(q)->links.length())
            loopv(entgroup)
            {
                int n = entgroup[i];
                if(ents.inrange(n) && islightable(ents[n]) && n != enthover)
                    renderfocus(n, renderentlight(e));
            }
            if(ents.inrange(enthover) && islightable(ents[enthover]))
                renderfocus(enthover, renderentlight(e));
        }
        loopi(lastenttype[LIGHTFX]) if(ents[i]->type == LIGHTFX && ents[i]->attrs[0] != LFX_SPOTLIGHT)
        {
            if(ents[i]->spawned || ents[i]->lastemit)
            {
                if(!ents[i]->spawned && ents[i]->lastemit > 0 && lastmillis-ents[i]->lastemit > triggertime(*ents[i])/2)
                    continue;
            }
            else
            {
                bool lonely = true;
                loopvk(ents[i]->links) if(ents.inrange(ents[i]->links[k]) && ents[ents[i]->links[k]]->type != LIGHT) { lonely = false; break; }
                if(!lonely) continue;
            }
            loopvk(ents[i]->links) if(ents.inrange(ents[i]->links[k]) && ents[ents[i]->links[k]]->type == LIGHT)
                makelightfx(*ents[i], *ents[ents[i]->links[k]]);
        }
    }

    void update()
    {
        entitycheck(game::player1);
        loopv(game::players) if(game::players[i]) entitycheck(game::players[i]);
        loopi(lastenttype[MAPSOUND])
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type == MAPSOUND && e.links.empty() && mapsounds.inrange(e.attrs[0]) && !issound(e.schan))
            {
                int flags = SND_MAP|SND_LOOP; // ambient sounds loop
                loopk(SND_LAST)  if(e.attrs[4]&(1<<k)) flags |= 1<<k;
                playsound(e.attrs[0], e.o, NULL, flags, e.attrs[3], e.attrs[1], e.attrs[2], &e.schan);
            }
        }
        if(docacheclear)
        {
            clearentcache();
            docacheclear = false;
        }
    }

    void render()
    {
        if(rendermainview && shouldshowents(game::player1->state == CS_EDITING ? 1 : 3)) loopv(ents) // important, don't render lines and stuff otherwise!
            renderfocus(i, renderentshow(e, i, game::player1->state == CS_EDITING ? ((entgroup.find(i) >= 0 || enthover == i) ? 1 : 2) : 3));
        if(!envmapping)
        {
            int numents = m_edit(game::gamemode) ? ents.length() : lastusetype[EU_ITEM];
            loopi(numents)
            {
                gameentity &e = *(gameentity *)ents[i];
                if(e.type <= NOTUSED || e.type >= MAXENTTYPES) continue;
                bool active = enttype[e.type].usetype == EU_ITEM && (e.spawned || (e.lastuse && lastmillis-e.lastuse < 500));
                if(m_edit(game::gamemode) || active)
                {
                    const char *mdlname = entmdlname(e.type, e.attrs);
                    vec pos = e.o;
                    if(mdlname && *mdlname)
                    {
                        int flags = MDL_SHADOW|MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
                        float fade = 1, yaw = 0, pitch = 0, size = 1;
                        if(!active)
                        {
                            fade = 0.5f;
                            if(e.type == FLAG || e.type == PLAYERSTART) { yaw = e.attrs[1]+(e.type == PLAYERSTART ? 90 : 0); pitch = e.attrs[2]; }
                            else if(e.type == ACTOR) { yaw = e.attrs[1]+90; pitch = e.attrs[2]; }
                        }
                        else if(e.spawned)
                        {
                            int millis = lastmillis-e.lastspawn;
                            if(millis < 500) size = fade = float(millis)/500.f;
                        }
                        else if(e.lastuse)
                        {
                            int millis = lastmillis-e.lastuse;
                            if(millis < 500) size = fade = 1.f-(float(millis)/500.f);
                        }
                        rendermodel(&e.light, mdlname, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, 0.f, flags, NULL, NULL, 0, 0, fade, size);
                    }
                }
            }
        }
    }

    void maketeleport(gameentity &e)
    {
        float yaw = e.attrs[0] < 0 ? (lastmillis/5)%360 : e.attrs[0], radius = float(e.attrs[3] ? e.attrs[3] : enttype[e.type].radius);
        int attr = int(e.attrs[4]), colour = (((attr&0xF)<<4)|((attr&0xF0)<<8)|((attr&0xF00)<<12))+0x0F0F0F;
        part_portal(e.o, radius, 1, yaw, e.attrs[1], PART_TELEPORT, 0, colour);
    }

    void drawparticle(gameentity &e, const vec &o, int idx, bool spawned, float skew)
    {
        switch(e.type)
        {
            case PARTICLES:
                if(idx < 0 || e.links.empty()) makeparticles(e);
                else if(e.spawned || (e.lastemit > 0 && lastmillis-e.lastemit <= triggertime(e)/2))
                    makeparticle(o, e.attrs);
                break;

            case TELEPORT:
                if(e.attrs[4]) maketeleport(e);
                break;
        }

        vec off(0, 0, 2.f), pos(o);
        if(enttype[e.type].usetype == EU_ITEM) pos.add(off);
        bool edit = m_edit(game::gamemode) && cansee(e), isedit = edit && game::player1->state == CS_EDITING,
                item = enttype[e.type].usetype == EU_ITEM && (spawned || (e.lastuse && lastmillis-e.lastuse < 500)),
                hasent = isedit && idx >= 0 && (entgroup.find(idx) >= 0 || enthover == idx);
        int sweap = m_weapon(game::gamemode, game::mutators), attr = e.type == WEAPON ? w_attr(game::gamemode, e.attrs[0], sweap) : e.attrs[0],
            colour = e.type == WEAPON ? weaptype[attr].colour : 0xFFFFFF, interval = lastmillis%1000;
        float fluc = interval >= 500 ? (1500-interval)/1000.f : (500+interval)/1000.f;
        if(item)
        {
            float radius = max(((e.type == WEAPON ? weaptype[attr].halo : enttype[e.type].radius*0.5f)+(fluc*0.5f))*skew, 0.125f);
            part_create(PART_HINT_SOFT, 1, o, colour, radius, fluc*skew);
        }
        if(isedit ? (showentinfo >= (hasent ? 2 : 3)) : (item && showentdescs >= 3))
        {
            const char *itxt = entinfo(e.type, e.attrs, isedit);
            if(itxt && *itxt)
            {
                defformatstring(ds)("<emphasis>%s", itxt);
                part_textcopy(pos.add(off), ds, hasent ? PART_TEXT_ONTOP : PART_TEXT, 1, 0xFFFFFF);
            }
        }
        if(edit)
        {
            part_create(hasent ? PART_EDIT_ONTOP : PART_EDIT, 1, o, hasent ? 0xAA22FF : 0x441188, hasent ? 2.f : 1.f);
            if(showentinfo >= (isedit ? 2 : 4))
            {
                defformatstring(s)("<super>%s%s (%d)", hasent ? "\fp" : "\fv", enttype[e.type].name, idx >= 0 ? idx : 0);
                part_textcopy(pos.add(off), s, hasent ? PART_TEXT_ONTOP : PART_TEXT);
                if(showentinfo >= (isedit ? (hasent ? 2 : 3) : 5)) loopk(enttype[e.type].numattrs)
                {
                    const char *attrname = enttype[e.type].attrs[k];
                    if(e.type == PARTICLES && k) switch(e.attrs[0])
                    {
                        case 0: switch(k) { case 1: attrname = "length"; break; case 2: attrname = "height"; break; case 3: attrname = "colour"; break; case 4: attrname = "fade"; break; default: attrname = ""; } break;
                        case 1: switch(k) { case 1: attrname = "dir"; break; default: attrname = ""; } break;
                        case 2: switch(k) { case 1: attrname = "dir"; break; default: attrname = ""; } break;
                        case 3: switch(k) { case 1: attrname = "size"; break; case 2: attrname = "colour"; break; default: attrname = ""; } break;
                        case 4: case 7: switch(k) { case 1: attrname = "dir"; break; case 2: attrname = "length"; break; case 3: attrname = "colour"; break; case 4: attrname = "fade"; break; case 5: attrname = "size"; break; default: attrname = ""; break; } break;
                        case 8: case 9: case 10: case 11: case 12: case 13: switch(k) { case 1: attrname = "dir"; break; case 2: attrname = "length"; break; case 3: attrname = "colour"; break; case 4: attrname = "fade"; break; case 5: attrname = "size"; break; case 6: attrname = "decal"; break; case 7: attrname = "gravity"; break; case 8: attrname = "velocity"; break; default: attrname = ""; break; } break;
                        case 14: case 15: switch(k) { case 1: attrname = "radius"; break; case 2: attrname = "height"; break; case 3: attrname = "colour"; break; case 4: attrname = "fade"; break; case 5: attrname = "size"; break; case 6: attrname = "gravity"; break; case 7: attrname = "velocity"; break; default: attrname = ""; } break;
                        case 6: switch(k) { case 1: attrname = "amt"; break; case 2: attrname = "colour"; break; case 3: attrname = "colour2"; break; default: attrname = ""; } break;
                        case 5: switch(k) { case 1: attrname = "amt"; break; case 2: attrname = "colour"; break; default: attrname = ""; } break;
                        case 32: case 33: case 34: case 35: switch(k) { case 1: attrname = "red"; break; case 2: attrname = "green"; break; case 3: attrname = "blue"; break; default: attrname = ""; } break;
                        default: attrname = ""; break;
                    }
                    if(attrname && *attrname)
                    {
                        formatstring(s)("%s%s:%d", hasent ? "\fw" : "\fd", attrname, e.attrs[k]);
                        part_textcopy(pos.add(off), s, hasent ? PART_TEXT_ONTOP : PART_TEXT);
                    }
                }
            }
        }
    }

    void drawparticles()
    {
        float maxdist = float(maxparticledistance)*float(maxparticledistance);
        int numents = m_edit(game::gamemode) ? ents.length() : max(lastusetype[EU_ITEM], max(lastenttype[PARTICLES], lastenttype[TELEPORT]));
        loopi(numents)
        {
            gameentity &e = *(gameentity *)ents[i];
            if(e.type != PARTICLES && e.type != TELEPORT && !m_edit(game::gamemode) && enttype[e.type].usetype != EU_ITEM) continue;
            if(e.o.squaredist(camera1->o) > maxdist) continue;
            float skew = 1;
            if(e.spawned)
            {
                int millis = lastmillis-e.lastspawn;
                if(millis < 500) skew = float(millis)/500.f;
            }
            else if(e.lastuse)
            {
                int millis = lastmillis-e.lastuse;
                if(millis < 500) skew = 1.f-(float(millis)/500.f);
            }
            drawparticle(e, e.o, i, e.spawned, skew);
        }
        loopv(projs::projs)
        {
            projent &proj = *projs::projs[i];
            if(proj.projtype != PRJ_ENT || !ents.inrange(proj.id)) continue;
            gameentity &e = *(gameentity *)ents[proj.id];
            float skew = 1;
            if(proj.fadetime && proj.lifemillis)
            {
                int interval = min(proj.lifemillis, proj.fadetime);
                if(proj.lifetime < interval) skew = float(proj.lifetime)/float(interval);
                else if(proj.lifemillis > interval)
                {
                    interval = min(proj.lifemillis-interval, proj.fadetime);
                    if(proj.lifemillis-proj.lifetime < interval) skew = float(proj.lifemillis-proj.lifetime)/float(interval);
                }
            }
            drawparticle(e, proj.o, -1, proj.ready(), skew);
        }
    }
}
