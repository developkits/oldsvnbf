#include "pch.h"
#include "engine.h"
#include "game.h"
namespace physics
{
	FVARW(crawlspeed,		0, 30.f, 1000);	// crawl speed
	FVARW(gravity,			0, 40.f, 1000);	// gravity
	FVARW(jumpvel,			0, 80.f, 1000);	// extra velocity to add when jumping
	FVARW(movespeed,		0, 50.f, 1000);	// speed

	FVARW(impulsevel,		0, 100.f, 1000);	// extra velocity to add when impulsing
	FVARW(impulsespeed,	0, 100.f, 1000);	// modifier of gravity that determines impulse interval
	VARW(impulsetime,		-1, 0, INT_MAX-1); // overrides impulsespeed to a specific time interval (-1 = turn off impulse, 0 = use impulsespeed)

	FVARW(liquidfric,		0, 10.f, 100);
	FVARW(liquidscale,		0, 0.9f, 100);
	FVARW(sinkfric,		0, 2.f, 100);
	FVARW(floorfric,		0, 5.f, 100);
	FVARW(airfric,			0, 25.f, 100);

	FVARW(stairheight,		0, 4.1f, 100);
	FVARW(floorz,			0, 0.867f, 1);
	FVARW(slopez,			0, 0.5f, 1);
	FVARW(wallz,			0, 0.2f, 1);
	FVARW(stepspeed,		0.5f, 1.0f, 2);

	FVARP(floatspeed,		1.f,	100.f,		1000.f);
	VARP(physframetime,	5,			5,			20);
	VARP(physinterp,       0,          1,          1);

	int spawncycle = -1, fixspawn = 4, physsteps = 0, lastphysframe = 0;

	#define imov(name,v,d,s,os) \
		void do##name(bool down) \
		{ \
			world::player1->s = down && world::allowmove(world::player1); \
			world::player1->v = world::player1->s ? d : (world::player1->os ? -(d) : 0); \
		} \
		ICOMMAND(name, "D", (int *down), { do##name(*down!=0); });

	imov(backward, move,   -1, k_down,  k_up);
	imov(forward,  move,    1, k_up,    k_down);
	imov(left,     strafe,  1, k_left,  k_right);
	imov(right,    strafe, -1, k_right, k_left);

	// inputs
	#define iput(x,y,t,z,a,q) \
		void do##x(bool down) \
		{ \
			if(!q) \
			{ \
				world::player1->y = false; \
				if(z && down) world::respawn(world::player1); \
			} \
			else \
			{ \
				if((a && world::player1->y != down) || (!a && down)) \
					world::player1->t = lastmillis; \
				world::player1->y = down; \
			} \
		} \
		ICOMMAND(x, "D", (int *down), { do##x(*down!=0); });

	iput(crouch,	crouching,	crouchtime,	false,	true,	world::allowmove(world::player1));
	iput(jump,		jumping,	jumptime,	false,	false,	world::allowmove(world::player1));
	iput(attack,	attacking,	attacktime,	true,	false,	world::allowmove(world::player1));
	iput(reload,	reloading,	reloadtime,	true,	false,	world::allowmove(world::player1));
	iput(action,	useaction,	usetime,	true,	false,	world::allowmove(world::player1));

	void taunt(gameent *d)
	{
        if(d->state!=CS_ALIVE || d->physstate<PHYS_SLOPE) return;
		if(lastmillis-d->lasttaunt<1000) return;
		d->lasttaunt = lastmillis;
		client::addmsg(SV_TAUNT, "ri", d->clientnum);
	}
	ICOMMAND(taunt, "", (), { taunt(world::player1); });

	bool iscrouching(physent *d)
	{
		return d->crouching || lastmillis-d->crouchtime <= 200;
	}

	float jumpvelocity(physent *d)
	{
		return jumpvel*(d->weight/100.f)*(d->inliquid ? liquidscale : 1.f)*jumpscale;
	}

	float impulsevelocity(physent *d)
	{
		return impulsevel*(d->weight/100.f)*jumpscale;
	}

	float gravityforce(physent *d)
	{
		return gravity*(d->weight/100.f)*gravityscale;
	}

	bool canimpulse(physent *d)
	{
		int timelen = impulsetime ? impulsetime : int(gravityforce(d)*impulsespeed);
		if(timelen > 0) return lastmillis-d->lastimpulse > timelen;
		return false;
	}

	float maxspeed(physent *d)
	{
		if(d->type == ENT_PLAYER && d->state != CS_SPECTATOR && d->state != CS_WAITING && d->state != CS_EDITING)
		{
			return d->maxspeed*(float(iscrouching(d) ? crawlspeed : movespeed)/100.f)*(float(d->weight)/100.f)*speedscale;
		}
		return d->maxspeed*(float(movespeed)/100.f)*(float(d->weight)/100.f)*speedscale;
	}

	bool movepitch(physent *d)
	{
		return d->type == ENT_CAMERA || d->state == CS_SPECTATOR || d->state == CS_WAITING || d->state == CS_EDITING;
	}

    void recalcdir(physent *d, const vec &oldvel, vec &dir)
    {
        float speed = oldvel.magnitude();
        if(speed > 1e-6f)
        {
            float step = dir.magnitude();
            dir = d->vel;
            dir.add(d->falling);
            dir.mul(step/speed);
        }
    }

    void slideagainst(physent *d, vec &dir, const vec &obstacle, bool foundfloor)
    {
        vec wall(obstacle);
        if(foundfloor && wall.z)
        {
            wall.z = 0;
            if(!wall.iszero()) wall.normalize();
        }
        vec oldvel(d->vel);
        oldvel.add(d->falling);
        d->vel.project(wall);
        d->falling.project(wall);
        recalcdir(d, oldvel, dir);
    }

    void switchfloor(physent *d, vec &dir, const vec &floor)
    {
        if(floor.z >= floorz) d->falling = vec(0, 0, 0);

        vec oldvel(d->vel);
        oldvel.add(d->falling);
        if(dir.dot(floor) >= 0)
        {
            if(d->physstate < PHYS_SLIDE || fabs(dir.dot(d->floor)) > 0.01f*dir.magnitude()) return;
            d->vel.projectxy(floor, 0.0f);
        }
        else d->vel.projectxy(floor);
        d->falling.project(floor);
        recalcdir(d, oldvel, dir);
    }

	bool trystepup(physent *d, vec &dir, float maxstep)
	{
		vec old(d->o);
		/* check if there is space atop the stair to move to */
		if(d->physstate != PHYS_STEP_UP)
		{
			d->o.add(dir);
			d->o.z += maxstep + 0.1f;
			if(!collide(d))
			{
				d->o = old;
				return false;
			}
		}

       /* try stepping up half as much as forward */
        d->o = old;
        vec smoothdir(dir.x, dir.y, 0);
        if(fabs(smoothdir.x) > 1e-9f || fabs(smoothdir.y) > 1e-9f) smoothdir.normalize();
        smoothdir.z = 0.5f;
        smoothdir.mul(dir.magnitude()*stepspeed/smoothdir.magnitude());
        d->o.add(smoothdir);
        d->o.z += maxstep + 0.1f;
        if(collide(d, smoothdir))
        {
            d->o.z -= maxstep + 0.1f;
            if(d->physstate == PHYS_FALL)
            {
                d->timeinair = 0;
                d->floor = vec(0, 0, 1);
                switchfloor(d, dir, d->floor);
            }
            d->physstate = PHYS_STEP_UP;
            return true;
        }

		/* try stepping up */
		d->o = old;
		d->o.z += dir.magnitude()*stepspeed;
		if(collide(d, vec(0, 0, 1)))
		{
			if(d->physstate == PHYS_FALL)
			{
				d->timeinair = 0;
				d->floor = vec(0, 0, 1);
				switchfloor(d, dir, d->floor);
			}
			d->physstate = PHYS_STEP_UP;
			return true;
		}
		d->o = old;
		return false;
	}

	#if 0
	bool trystepdown(physent *d, vec &dir, float step, float a, float b)
	{
		vec old(d->o);
		vec dv(dir.x*a, dir.y*a, -step*b), v(dv);
		v.mul(stairheight/(step*b));
		d->o.add(v);
		if(!collide(d, vec(0, 0, -1), slopez))
		{
			d->o = old;
			d->o.add(dv);
			if(collide(d, vec(0, 0, -1))) return true;
		}
		d->o = old;
		return false;
	}
	#endif

    void falling(physent *d, vec &dir, const vec &floor)
	{
	#if 0
		if(d->physstate >= PHYS_FLOOR && (floor.z == 0.0f || floor.z == 1.0f))
		{
			vec moved(d->o);
			d->o.z -= stairheight+0.1f;
			if(!collide(d, vec(0, 0, -1), slopez))
			{
				d->o = moved;
				d->physstate = PHYS_STEP_DOWN;
				return;
			}
			else d->o = moved;
		}
	#endif
        if(floor.z > 0.0f && floor.z < slopez)
        {
            if(floor.z >= wallz) switchfloor(d, dir, floor);
            d->timeinair = 0;
            d->physstate = PHYS_SLIDE;
            d->floor = floor;
        }
		else d->physstate = PHYS_FALL;
	}

    void landing(physent *d, vec &dir, const vec &floor)
	{
    #if 0
        if(d->physstate == PHYS_FALL)
        {
            d->timeinair = 0;
            if(dir.z < 0.0f) dir.z = d->vel.z = 0.0f;
        }
    #endif
        switchfloor(d, dir, floor);
        d->timeinair = 0;

		if(floor.z >= floorz) d->physstate = PHYS_FLOOR;
		else d->physstate = PHYS_SLOPE;
		d->floor = floor;
	}

	bool findfloor(physent *d, bool collided, const vec &obstacle, bool &slide, vec &floor)
	{
		bool found = false;
		vec moved(d->o);
		d->o.z -= 0.1f;
		if(!collide(d, vec(0, 0, -1), d->physstate == PHYS_SLOPE ? slopez : floorz))
		{
			floor = wall;
			found = true;
		}
		else if(collided && obstacle.z >= slopez)
		{
			floor = obstacle;
			found = true;
			slide = false;
		}
        else if(d->physstate == PHYS_STEP_UP || d->physstate == PHYS_SLIDE)
        {
            if(!collide(d, vec(0, 0, -1)) && wall.z > 0.0f)
            {
                floor = wall;
                if(floor.z >= slopez) found = true;
            }
        }
        else if(d->physstate >= PHYS_SLOPE && d->floor.z < 1.0f)
        {
            if(!collide(d, vec(d->floor).neg(), 0.95f) || !collide(d, vec(0, 0, -1)))
            {
                floor = wall;
                if(floor.z >= slopez && floor.z < 1.0f) found = true;
            }
        }
        if(collided && (!found || obstacle.z > floor.z))
        {
            floor = obstacle;
            slide = !found && (floor.z < wallz || floor.z >= slopez);
        }
		d->o = moved;
		return found;
	}

	bool move(physent *d, vec &dir)
	{
		vec old(d->o);
		#if 0
		if(d->physstate == PHYS_STEP_DOWN && dir.z <= 0.0f && world::allowmove(d) && (d->move || d->strafe))
		{
			float step = dir.magnitude()*stepspeed;
			if(trystepdown(d, dir, step, 0.75f, 0.25f)) return true;
			if(trystepdown(d, dir, step, 0.5f, 0.5f)) return true;
			if(trystepdown(d, dir, step, 0.25f, 0.75f)) return true;
			d->o.z -= step;
			if(collide(d, vec(0, 0, -1))) return true;
			d->o = old;
		}
		#endif
		bool collided = false, slidecollide = false;
		vec obstacle;
		d->o.add(dir);
		if(!collide(d, d->type!=ENT_CAMERA ? dir : vec(0, 0, 0)) || (d->type==ENT_AI && !collide(d)))
		{
            obstacle = wall;
            /* check to see if there is an obstacle that would prevent this one from being used as a floor */
            if(d->type==ENT_PLAYER && ((wall.z>=slopez && dir.z<0) || (wall.z<=-slopez && dir.z>0)) && (dir.x || dir.y) && !collide(d, vec(dir.x, dir.y, 0)))
            {
                if(wall.dot(dir) >= 0) slidecollide = true;
                obstacle = wall;
            }

            d->o = old;
            if(d->type == ENT_CAMERA) return false;
            d->o.z -= stairheight;
            d->zmargin = -stairheight;
            if(d->physstate == PHYS_SLOPE || d->physstate == PHYS_FLOOR || (!collide(d, vec(0, 0, -1), slopez) && (d->physstate==PHYS_STEP_UP || wall.z>=floorz)))
            {
                d->o = old;
                d->zmargin = 0;
                if(trystepup(d, dir, stairheight)) return true;
            }
            else
            {
                d->o = old;
                d->zmargin = 0;
            }

			/* can't step over the obstacle, so just slide against it */
			collided = true;
		}
        else if(d->physstate == PHYS_STEP_UP)
        {
            if(!collide(d, vec(0, 0, -1), slopez))
            {
                d->o = old;
                if(trystepup(d, dir, stairheight)) return true;
                d->o.add(dir);
            }
        }

		vec floor(0, 0, 0);
		bool slide = collided,
			 found = findfloor(d, collided, obstacle, slide, floor);
        if(slide || (!collided && floor.z > 0 && floor.z < wallz))
        {
            slideagainst(d, dir, slide ? obstacle : floor, found || slidecollide);
			if(d->type == ENT_AI || d->type == ENT_INANIMATE) d->blocked = true;
		}
		if(found)
		{
			if(d->type == ENT_CAMERA) return false;
			landing(d, dir, floor);
		}
		else falling(d, dir, floor);
		return !collided;
	}

	void modifyvelocity(physent *pl, bool local, bool floating, int millis)
	{
		if(floating)
		{
			pl->lastimpulse = 0;
			if(pl->jumping)
			{
				pl->vel.z = max(pl->vel.z, 0.f) + jumpvelocity(pl);
				pl->jumping = false;
			}
		}
		else if(pl->physstate >= PHYS_SLOPE || pl->inliquid)
		{
			pl->lastimpulse = 0;
			if(pl->jumping)
			{
				pl->vel.z = max(pl->vel.z, 0.f) + jumpvelocity(pl);
				if(pl->inliquid) { pl->vel.x *= liquidscale; pl->vel.y *= liquidscale; }
				playsound(S_JUMP, pl->o, pl);
				pl->jumping = false;
			}
		}
		else if(pl->jumping && canimpulse(pl))
		{
			vec dir;
			vecfromyawpitch(pl->yaw, pl->move || pl->strafe ? pl->pitch : 90.f, pl->move || pl->strafe ? pl->move : 1, pl->strafe, dir);
			dir.normalize();
			dir.mul(impulsevelocity(pl));
			pl->vel.add(dir);
			pl->lastimpulse = lastmillis;
			pl->jumping = false;
		}
        if(pl->physstate == PHYS_FALL) pl->timeinair += curtime;

		vec m(0.0f, 0.0f, 0.0f);
        bool wantsmove = world::allowmove(pl) && (pl->move || pl->strafe);
		if(m.iszero() && wantsmove)
		{
			vecfromyawpitch(pl->aimyaw, floating || pl->inliquid || movepitch(pl) ? pl->aimpitch : 0, pl->move, pl->strafe, m);

			if(!floating && pl->physstate >= PHYS_SLIDE)
			{
				/* move up or down slopes in air
				 * but only move up slopes in liquid
				 */
				float dz = -(m.x*pl->floor.x + m.y*pl->floor.y)/pl->floor.z;
				if(pl->inliquid) m.z = max(m.z, dz);
				else if(pl->floor.z >= wallz) m.z = dz;
			}

			m.normalize();
		}

		vec d(m);
		d.mul(maxspeed(pl));
        if(pl->type==ENT_PLAYER)
        {
		    if(floating) { if(local) d.mul(floatspeed/100.0f); }
		    else if(!pl->inliquid) d.mul((wantsmove ? 1.3f : 1.0f) * (pl->physstate < PHYS_SLOPE ? 1.3f : 1.0f)); // EXPERIMENTAL
        }
		float friction = pl->inliquid && !floating ? liquidfric : (pl->physstate >= PHYS_SLOPE || floating ? floorfric : airfric);
		float fpsfric = max(friction/millis*20.0f, 1.0f);

        pl->vel.mul(fpsfric-1);
        pl->vel.add(d);
        pl->vel.div(fpsfric);
	}

    void modifygravity(physent *pl, int curtime)
    {
        float secs = curtime/1000.0f;
        vec g(0, 0, 0);
        if(pl->physstate == PHYS_FALL) g.z -= gravityforce(pl)*secs;
        else if(pl->floor.z > 0 && pl->floor.z < floorz)
        {
            g.z = -1;
            g.project(pl->floor);
            g.normalize();
            g.mul(gravityforce(pl)*secs);
        }
        if(!pl->inliquid || (!pl->move && !pl->strafe)) pl->falling.add(g);

        if(pl->inliquid || pl->physstate >= PHYS_SLOPE)
        {
            float friction = pl->inliquid ? sinkfric : floorfric,
                  fpsfric = friction/curtime*20.0f,
                  c = pl->inliquid ? 1.0f : clamp((pl->floor.z - slopez)/(floorz-slopez), 0.0f, 1.0f);
            pl->falling.mul(1 - c/fpsfric);
        }
    }

    void updatematerial(physent *pl, bool local, bool floating)
    {
		vec v = pl->type == ENT_PLAYER ? world::feetpos(pl, 1.f) : pl->o;
		int material = lookupmaterial(v);
		int curmat = material&MATF_VOLUME, oldmat = pl->inmaterial&MATF_VOLUME;

		if(!floating && curmat != oldmat)
		{
			uchar mcol[3] = { 255, 255, 255 };
			#define mattrig(mo,mf,mz,ms,mt,mw) \
			{ \
				mf; \
				int col = (mcol[2] + (mcol[1] << 8) + (mcol[0] << 16)); \
				world::spawneffect(mo, col, mt, ms); \
				if(mw >= 0) playsound(mw, mo, pl); \
			}
			if(curmat == MAT_WATER || oldmat == MAT_WATER)
				mattrig(v, getwatercolour(mcol), PART_WATER, 2.f, 4, curmat != MAT_WATER ? S_SPLASH1 : S_SPLASH2);
			if(curmat == MAT_LAVA) mattrig(vec(pl->o).sub(vec(0, 0, pl->height/2.f)), getlavacolour(mcol), PART_FIREBALL, 2.f, int(pl->height/2.f), S_BURNING);

			if(local)
			{
				if(!isliquid(curmat) && isliquid(oldmat))
					pl->vel.z = max(pl->vel.z, jumpvelocity(pl));
				else if(isliquid(curmat) && !isliquid(oldmat))
					pl->vel.mul(liquidscale);

				if(pl->type == ENT_PLAYER && pl->state == CS_ALIVE && isdeadly(curmat))
					world::suicide((gameent *)pl, curmat == MAT_LAVA ? HIT_MELT : 0);
			}
		}
		pl->inmaterial = material;
		pl->inliquid = !floating && isliquid(curmat);
    }

	// main physics routine, moves a player/monster for a time step
	// moveres indicated the physics precision (which is lower for monsters and multiplayer prediction)
	// local is false for multiplayer prediction

	bool moveplayer(physent *pl, int moveres, bool local, int millis)
	{
		bool floating = pl->type == ENT_PLAYER && (pl->state == CS_EDITING || pl->state == CS_SPECTATOR || pl->state == CS_WAITING);
		float secs = millis/1000.f;

		if(pl->type!=ENT_CAMERA) updatematerial(pl, local, floating);

        // apply gravity
        if(!floating && pl->type!=ENT_CAMERA) modifygravity(pl, millis);
		// apply any player generated changes in velocity
		modifyvelocity(pl, local, floating, millis);

		vec d(pl->vel), oldpos(pl->o);
        if(!floating && pl->type!=ENT_CAMERA && pl->inliquid) d.mul(0.5f);
        d.add(pl->falling);
		d.mul(secs);

		pl->blocked = false;
		pl->moving = true;
		pl->onplayer = NULL;

		if(floating)				// just apply velocity
		{
			if(pl->physstate != PHYS_FLOAT)
			{
				pl->physstate = PHYS_FLOAT;
				pl->timeinair = 0;
                pl->falling = vec(0, 0, 0);
			}
			pl->o.add(d);
		}
		else						// apply velocity with collision
		{
			const float f = 1.0f/moveres;
			int collisions = 0;
			vec vel(pl->vel);

			d.mul(f);
			loopi(moveres) if(!move(pl, d)) { if(pl->type==ENT_CAMERA) return false; if(++collisions<5) i--; } // discrete steps collision detection & sliding
			if(!pl->timeinair && vel.z <= -64) // if we land after long time must have been a high jump, make thud sound
			{
				playsound(S_LAND, pl->o, pl);
			}
		}

		if(pl->type!=ENT_CAMERA && pl->state==CS_ALIVE) updatedynentcache(pl);

		if(!pl->timeinair && pl->physstate >= PHYS_FLOOR && pl->vel.squaredlen() < 1e-4f)
			pl->moving = false;

		pl->lastmoveattempt = lastmillis;
		if(pl->o!=oldpos) pl->lastmove = lastmillis;

        if((pl->type==ENT_PLAYER || pl->type==ENT_AI) && local && pl->o.z < 0 && pl->state == CS_ALIVE)
        {
            world::suicide((gameent *)pl, HIT_FALL);
            return false;
        }

		return true;
	}

    void interppos(physent *d)
    {
        d->o = d->newpos;
        d->o.z += d->height;

        int diff = lastphysframe - lastmillis;
        if(diff <= 0 || !physinterp) return;

        vec deltapos(d->deltapos);
        deltapos.mul(min(diff, physframetime)/float(physframetime));
        d->o.add(deltapos);
    }

	void move(physent *d, int moveres, bool local)
	{
        if(physsteps <= 0)
        {
            if(local) interppos(d);
            return;
        }

        if(local)
        {
            d->o = d->newpos;
            d->o.z += d->height;
        }
        loopi(physsteps-1) moveplayer(d, moveres, local, physframetime);
        if(local) d->deltapos = d->o;
        moveplayer(d, moveres, local, physframetime);
        if(local)
        {
            d->newpos = d->o;
            d->deltapos.sub(d->newpos);
            d->newpos.z -= d->height;
            interppos(d);
        }
	}

	void avoidcollision(physent *d, const vec &dir, physent *obstacle, float space)
	{
		float rad = obstacle->radius+d->radius;
		vec bbmin(obstacle->o);
		bbmin.x -= rad;
		bbmin.y -= rad;
		bbmin.z -= obstacle->height+d->aboveeye;
		bbmin.sub(space);
		vec bbmax(obstacle->o);
		bbmax.x += rad;
		bbmax.y += rad;
		bbmax.z += obstacle->aboveeye+d->height;
		bbmax.add(space);

		loopi(3) if(d->o[i] <= bbmin[i] || d->o[i] >= bbmax[i]) return;

		float mindist = 1e16f;
		loopi(3) if(dir[i] != 0)
		{
			float dist = ((dir[i] > 0 ? bbmax[i] : bbmin[i]) - d->o[i]) / dir[i];
			mindist = min(mindist, dist);
		}
		if(mindist >= 0.0f && mindist < 1e15f) d->o.add(vec(dir).mul(mindist));
	}

	void updatephysstate(physent *d)
	{
		if(d->physstate == PHYS_FALL) return;
		d->timeinair = 0;
		vec old(d->o);
		/* Attempt to reconstruct the floor state.
		 * May be inaccurate since movement collisions are not considered.
		 * If good floor is not found, just keep the old floor and hope it's correct enough.
		 */
		switch(d->physstate)
		{
			case PHYS_SLOPE:
			case PHYS_FLOOR:
				d->o.z -= 0.15f;
				if(!collide(d, vec(0, 0, -1), d->physstate == PHYS_SLOPE ? slopez : floorz))
					d->floor = wall;
				break;

			case PHYS_STEP_UP:
				d->o.z -= stairheight+0.15f;
				if(!collide(d, vec(0, 0, -1), slopez))
					d->floor = wall;
				break;

			case PHYS_SLIDE:
				d->o.z -= 0.15f;
				if(!collide(d, vec(0, 0, -1)) && wall.z < slopez)
					d->floor = wall;
				break;
		}
		if(d->physstate > PHYS_FALL && d->floor.z <= 0) d->floor = vec(0, 0, 1);
		d->o = old;
	}

	bool entinmap(physent *d, bool avoidplayers)
	{
		d->o.z += d->height;	 // pos specified is at feet
		vec orig = d->o;
		#define inmapchk(x,y) \
			loopi(x) \
			{ \
				if(i) \
				{ \
					d->o = orig; \
					y; \
				} \
				if(collide(d) && !inside) \
				{ \
					if(hitplayer) \
					{ \
						if(!avoidplayers) continue; \
						d->o = orig; \
                        d->resetinterp(); \
						return false; \
					} \
                    d->resetinterp(); \
					return true; \
				} \
			}

		inmapchk(10, { d->o.add(vec(d->vel).mul((rnd(21)-10)*i/5)); });
		inmapchk(10, {
				d->o.add(vec(d->vel).mul((rnd(21)-10)*i/5));
				d->o.z += fabs(d->vel.z)*((rnd(21)-10)*i/5)*2;
			});
		inmapchk(100, {
				d->o.x += (rnd(21)-10)*i/5;  // increasing distance
				d->o.y += (rnd(21)-10)*i/5;
				d->o.z += (rnd(21)-10)*i/5;
			});

		d->o = orig;
        d->resetinterp();
		return false;
	}

    VARP(smoothmove, 0, 90, 1000);
    VARP(smoothdist, 0, 64, 1024);

    void predictplayer(gameent *d, bool domove, int res = 0, bool local = false)
    {
        d->o = d->newpos;
        d->o.z += d->height;

        d->yaw = d->newyaw;
        d->pitch = d->newpitch;

        d->aimyaw = d->newaimyaw;
        d->aimpitch = d->newaimpitch;

        if(domove)
        {
            move(d, res, local);
            d->newpos = d->o;
            d->newpos.z -= d->height;
        }

        float k = 1.0f - float(lastmillis - d->smoothmillis)/float(smoothmove);
        if(k>0)
        {
            d->o.add(vec(d->deltapos).mul(k));

            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;

            d->aimyaw += d->deltaaimyaw*k;
            if(d->aimyaw<0) d->aimyaw += 360;
            else if(d->aimyaw>=360) d->aimyaw -= 360;
            d->aimpitch += d->deltaaimpitch*k;
        }
    }

	void smoothplayer(gameent *d, int res, bool local)
	{
		if(d->state==CS_ALIVE || d->state==CS_EDITING)
		{
			if(smoothmove && d->smoothmillis>0) predictplayer(d, true, res, local);
			else move(d, res, local);
		}
		else if(d->state==CS_DEAD && lastmillis-d->lastpain<2000) move(d, res, local);
	}

	bool droptofloor(vec &o, float radius, float height)
	{
		if(!insideworld(o)) return false;
		vec v(0.0001f, 0.0001f, -1);
		v.normalize();
		if(raycube(o, v, hdr.worldsize) >= hdr.worldsize) return false;
		physent d;
		d.type = ENT_CAMERA;
		d.o = o;
		d.vel = vec(0, 0, -1);
		d.radius = radius;
		d.height = height;
		d.aboveeye = radius;
		loopi(hdr.worldsize) if(!move(&d, v))
		{
			o = d.o;
			return true;
		}
		return false;
	}

	void update()
	{
        int diff = lastmillis - lastphysframe;
        if(diff <= 0) physsteps = 0;
        else
        {
            physsteps = (diff + physframetime - 1)/physframetime;
            lastphysframe += physsteps * physframetime;
        }
		cleardynentcache();
	}
}
