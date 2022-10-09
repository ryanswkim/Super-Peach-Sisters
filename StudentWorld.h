#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include <vector>
#include <string>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetPath);
	~StudentWorld();
	virtual int init();
	virtual int move();
	virtual void cleanUp();
	Actor* isBlockingObject(int x, int y, bool includePeach = false, bool moving = true);
	Peach* getPeach() { return m_peach; }
	bool isPeach(Actor* unknown) { return unknown == m_peach; }
	void CreatePowerup(int goodie, int x, int y);
	void CreateFireball(bool peach, int x, int y, int dir);
	void CreateShell(int x, int y, int dir);
	void NextLevel(bool mario);

private:
	Peach* m_peach;
	std::vector<Actor*> m_actors;
	bool finishedLevel; // denotes whether we finished our current level
	bool finishedGame; // denotes whether we finished the entire game
};

#endif // STUDENTWORLD_H_
