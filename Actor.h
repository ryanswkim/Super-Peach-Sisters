#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <iostream> // for debugging purposes
using namespace std;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp

class StudentWorld;

class Actor : public GraphObject {
public:
	Actor(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		GraphObject(imageID, startX, startY, startDirection, depth, size), m_alive(true), m_world(world) {}
	StudentWorld* getWorld() { return m_world; }
	void setAlive(bool status) { m_alive = status; }
	bool isAlive() { return m_alive; }
	virtual bool isCollidable() { return false; } // every actor is not collidable by default
	virtual bool isDamageable() { return false; } // any actor is not damageable by default
	virtual void doSomething() = 0; // every actor should do something every tick
	virtual void bonk() = 0; // every actor should do something like make noise when bonk()'ed
private:
	StudentWorld* m_world;
	bool m_alive;
};

class Peach : public Actor {
public:
	Peach(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		Actor(world, imageID, startX, startY, startDirection, depth, size),
		m_hitpoints(1), invincibleBoost(0), jumpBoost(false), shootBoost(false), starBoost(0), jumpDistance(0), shootCooldown(0) {}
	virtual void doSomething();
	virtual void bonk();
	void powerup(int powerup);
	bool hasJumpBoost() { return jumpBoost; }
	bool hasShootBoost() { return shootBoost; }
	bool hasStarBoost() { return starBoost > 0; }
	// NOTE: I don't override isDamageable() for Peach, simply because it should be obvious that Peach can be damaged.
	// As I have a getPeach() and isPeach() method in StudentWorld, I reasoned that these would suffice as to why I do 
	// not need Peach to actually have an isDamageable() method. This also meant I wouldn't be duplicating code because
	// Enemy already overrides isDamageable() to return true (meaning Peach's isDamageable() method would be the exact
	// same as Enemy's). While I do not know if such code is considered non-trivial or not, I decided to just be safe
	// and use this reasoning to justify why Peach retains the base Actor's isDamageable() method which returns false 
	// (though she is still damageable).

private:
	int m_hitpoints;
	bool jumpBoost, shootBoost; // powerup toggles
	int invincibleBoost, starBoost; // power-up durations
	int jumpDistance, shootCooldown; // power-up/movement values
};

// Collidable container class (pipe, block)

class Collidable : public Actor {
public:
	Collidable(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 2, double size = 1.0) :
		Actor(world, imageID, startX, startY, startDirection, depth, size) {}
	virtual bool isCollidable() { return true; } // blocks are collidable
	virtual void doSomething() { return; } // blocks don't do anything
	virtual void bonk();
private:
	virtual bool dropGoodie() { return false; } // no goodie by default
};

class Block : public Collidable {
public:
	Block(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 2, double size = 1.0, int goodie = 0) :
		Collidable(world, imageID, startX, startY, startDirection, depth, size), m_goodie(goodie) {}
private:
	int m_goodie; // denotes what goodie a block has, if any
	virtual bool dropGoodie(); // drops the goodie it stores, if it has one
};

class Pipe : public Collidable {
public:
	Pipe(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 2, double size = 1.0) :
		Collidable(world, imageID, startX, startY, startDirection, depth, size) {}
};

// Enemy container class (goomba, koopa, piranha)

class Enemy : public Actor {
public:
	Enemy(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		Actor(world, imageID, startX, startY, startDirection, depth, size) {}
	virtual bool isDamageable() { return true; }
	virtual void doSomething();
	virtual void bonk();
private:
	virtual void move1() { return; } // runs before bonking peach
	virtual void move2(); // runs after bonking peach
	virtual void die() { return; } // do nothing by default (only koopas update this function anyways)
};

class Goomba : public Enemy {
public:
	Goomba(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		Enemy(world, imageID, startX, startY, startDirection, depth, size) {}
};

class Piranha : public Enemy {
public:
	Piranha(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		Enemy(world, imageID, startX, startY, startDirection, depth, size), firingDelay(0) {}
private:
	virtual void move1() { increaseAnimationNumber(); } // simply animates itself every frame
	virtual void move2();
	int firingDelay;
};

class Koopa : public Enemy {
public:
	Koopa(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 0, double size = 1.0) :
		Enemy(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void die(); // extra code after getting bonk()'ed
};

// Projectile container class (goodies, fireballs, shell)

class Projectile : public Actor {
public:
	Projectile(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1,
		double size = 1.0, bool bounces = true, bool targetsPeach = true) :
		Actor(world, imageID, startX, startY, startDirection, depth, size), bounces(bounces),
		targetsPeach(targetsPeach) {}
	virtual void doSomething();
	virtual void bonk() { return; } // a projectile can't be bonked
private:
	bool bounces; // change directions if it hits a collidable
	bool targetsPeach; // whether or not it interacts with peach
	virtual void interact(Actor* actor) { actor->bonk(); } // simply bonks an enemy it overlaps with by default
};

class Mushroom : public Projectile {
public:
	Mushroom(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void interact(Actor* actor);
};

class Flower : public Projectile {
public:
	Flower(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void interact(Actor* actor);
};

class Star : public Projectile {
public:
	Star(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void interact(Actor* actor);
};

class PiranhaFireball : public Projectile {
public:
	PiranhaFireball(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size, false) {}
private:
	virtual void interact(Actor* actor);
};

class PeachFireball : public Projectile {
public:
	PeachFireball(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size, false, false) {}
};

class Shell : public Projectile {
public:
	Shell(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Projectile(world, imageID, startX, startY, startDirection, depth, size, false, false) {}
};

// LevelEnder class

class LevelEnder : public Actor {
public:
	LevelEnder(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		Actor(world, imageID, startX, startY, startDirection, depth, size) {}
	virtual void doSomething();
	virtual void bonk() { return; }
private:
	virtual void progress() = 0; // both Mario and Flag have its own code to move onto the next level
};

class Flag : public LevelEnder {
public:
	Flag(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		LevelEnder(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void progress();
};

class Mario : public LevelEnder {
public:
	Mario(StudentWorld* world, int imageID, int startX, int startY, int startDirection = 0, int depth = 1, double size = 1.0) :
		LevelEnder(world, imageID, startX, startY, startDirection, depth, size) {}
private:
	virtual void progress();
};


#endif // ACTOR_H_
