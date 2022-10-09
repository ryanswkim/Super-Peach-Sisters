#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream> // for debugging purposes
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
    return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
    : GameWorld(assetPath)
{
    m_peach = nullptr;
    finishedLevel = false;
    finishedGame = false;
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()
{
    Level lev(assetPath());
    ostringstream oss;
    int level = this->getLevel();
    // once we get our current level, format our text accordingly
    if (level > 9) {
        oss << "level";
    }
    else {
        oss << "level0";
    }
    oss << level << ".txt";
    // load our level
    Level::LoadResult result = lev.loadLevel(oss.str());
    if (result == Level::load_fail_file_not_found) { // no such existing level
        return GWSTATUS_LEVEL_ERROR;
    }
    else if (result == Level::load_fail_bad_format) {
        return GWSTATUS_LEVEL_ERROR; // bad formatting 
    }
    else if (result == Level::load_success)
    {
        Level::GridEntry ge;
        for (int x = 0; x < GRID_HEIGHT; x++) {
            for (int y = 0; y < GRID_WIDTH; y++) {
                ge = lev.getContentsOf(x, y);
                // calculate our x and y positions
                int lx = x * SPRITE_WIDTH;
                int ly = y * SPRITE_HEIGHT;
                switch (ge) {
                case Level::peach:
                    m_peach = new Peach(this, IID_PEACH, lx, ly);
                    break;
                case Level::block:
                    m_actors.push_back(new Block(this, IID_BLOCK, lx, ly));
                    break;
                case Level::pipe:
                    m_actors.push_back(new Pipe(this, IID_PIPE, lx, ly));
                    break;
                case Level::goomba:
                    m_actors.push_back(new Goomba(this, IID_GOOMBA, lx, ly, randInt(0, 1) * 180));
                    break;
                case Level::koopa:
                    m_actors.push_back(new Koopa(this, IID_KOOPA, lx, ly, randInt(0, 1) * 180));
                    break;
                case Level::piranha:
                    m_actors.push_back(new Piranha(this, IID_PIRANHA, lx, ly, randInt(0, 1) * 180));
                    break;
                case Level::mushroom_goodie_block:
                    m_actors.push_back(new Block(this, IID_BLOCK, lx, ly, 0, 2, 1.0, 1));
                    break;
                case Level::flower_goodie_block:
                    m_actors.push_back(new Block(this, IID_BLOCK, lx, ly, 0, 2, 1.0, 2));
                    break;
                case Level::star_goodie_block:
                    m_actors.push_back(new Block(this, IID_BLOCK, lx, ly, 0, 2, 1.0, 3));
                    break;
                case Level::flag:
                    m_actors.push_back(new Flag(this, IID_FLAG, lx, ly));
                    break;
                case Level::mario:
                    m_actors.push_back(new Mario(this, IID_MARIO, lx, ly));
                    break;
                };

            }
        }
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    if (m_peach->isAlive()) // make peach do something first
        m_peach->doSomething();
    vector<Actor*>::iterator actor = m_actors.begin();
    while (actor != m_actors.end()) { // go through each actor
        if (!m_peach->isAlive()) { // check if one of our actors caused peach to die. if so, play dying sound and decrease lives
            playSound(SOUND_PLAYER_DIE);
            decLives();
            return GWSTATUS_PLAYER_DIED;
        }
        if ((*actor)->isAlive()) { // make our actor do something
            (*actor)->doSomething();
        }
        actor++;
    }


    if (finishedLevel) { // finished current level
        finishedLevel = false;
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }

    if (finishedGame) { // finished entier game
        finishedGame = false;
        playSound(SOUND_GAME_OVER);
        return GWSTATUS_PLAYER_WON;
    }

    actor = m_actors.begin(); // go through each actor to see if they died
    while (actor != m_actors.end()) {
        if (!(*actor)->isAlive()) { // if they did die, delete them and clean them from the vector
            delete (*actor);
            actor = m_actors.erase(actor);
        }
        else {
            actor++;
        }
    }


    ostringstream oss;
    // set up our display text to display lives, levels, and points
    oss << "Lives: " << getLives();
    oss.fill('0');
    oss << "  Level: " << setw(2) << getLevel();
    oss.fill('0');
    oss << "  Points: " << setw(6) << getScore();

    // append any powerup text
    if (getPeach()->hasStarBoost())
        oss << " StarPower!";
    if (getPeach()->hasShootBoost())
        oss << " ShootPower!";
    if (getPeach()->hasJumpBoost())
        oss << " JumpPower!";

    // display our text
    setGameStatText(oss.str());

    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
    // delete our peach first
    delete m_peach;
    m_peach = nullptr;

    vector<Actor*>::iterator actor = m_actors.begin();
    // delete every actor in our vector next
    while (actor != m_actors.end()) {
        delete (*actor);
        actor = m_actors.erase(actor);
    }
}

Actor* StudentWorld::isBlockingObject(int x, int y, bool includePeach, bool moving) {
    // if we are not peach (enemy), the first thing we want to look for is peach to attack her
    if (includePeach && !moving) { // we only want to attack if we overlap
        int curX = m_peach->getX();
        int curY = m_peach->getY();

        // all of the code essentially checks if our peach sprite contains the coordinate
        bool containsLeft = (x >= curX && x <= curX + SPRITE_WIDTH - 1); // see if peach contains the left side of our sprite
        bool containsBottom = (y >= curY && y <= curY + SPRITE_HEIGHT - 1); // see if peach contains the bottom side of our sprite
        bool containsRight = (x + SPRITE_WIDTH - 1 >= curX && x + SPRITE_WIDTH - 1 <= curX + SPRITE_WIDTH - 1); // see if peach contains the right side of our sprite
        bool containsTop = (y + SPRITE_HEIGHT - 1 >= curY && y + SPRITE_HEIGHT - 1 <= curY + SPRITE_HEIGHT - 1); // see if peach contains the top side of our sprite

        if ((containsRight || containsLeft) && (containsBottom || containsTop)) { // it only needs to contain either the right or left and either the top or bottom
            return m_peach;
        }
    }

    // initialize iterator to go through our other actors
    vector<Actor*>::iterator actor = m_actors.begin();
    while (actor != m_actors.end()) {
        if (moving && !(*actor)->isCollidable()) { // if we are moving, we only want to go through collidable actors
            actor++;
            continue;
        }
        // this is the exact same code as above for peach
        int curX = (*actor)->getX();
        int curY = (*actor)->getY();

        bool containsLeft = (x >= curX && x <= curX + SPRITE_WIDTH - 1);
        bool containsBottom = (y >= curY && y <= curY + SPRITE_HEIGHT - 1);
        bool containsRight = (x + SPRITE_WIDTH - 1 >= curX && x + SPRITE_WIDTH - 1 <= curX + SPRITE_WIDTH - 1);
        bool containsTop = (y + SPRITE_HEIGHT - 1 >= curY && y + SPRITE_HEIGHT - 1 <= curY + SPRITE_HEIGHT - 1);

        if ((containsRight || containsLeft) && (containsBottom || containsTop)) {
            // again, if our current actor contains either the left or right and either top or bottom of our sprite, then we return it
            return *actor;
        }

        actor++;
    }

    return nullptr;
}

void StudentWorld::CreatePowerup(int goodie, int x, int y) {
    switch (goodie) { // int goodie represents what goodie should be created
    case 1: // mushroom
        m_actors.push_back(new Mushroom(this, IID_MUSHROOM, x, y));
        break;
    case 2: // flower
        m_actors.push_back(new Flower(this, IID_FLOWER, x, y));
        break;
    case 3: // star
        m_actors.push_back(new Star(this, IID_STAR, x, y));
        break;
    }
}

void StudentWorld::CreateFireball(bool peach, int x, int y, int dir) {
    if (peach) // if peach is shooting, we make a peachfireball. otherwise, make a piranhafireball
        m_actors.push_back(new PeachFireball(this, IID_PEACH_FIRE, x, y, dir));
    else
        m_actors.push_back(new PiranhaFireball(this, IID_PIRANHA_FIRE, x, y, dir));
}

void StudentWorld::CreateShell(int x, int y, int dir) {
    m_actors.push_back(new Shell(this, IID_SHELL, x, y, dir));
}

void StudentWorld::NextLevel(bool mario) {
    if (mario) { // touched mario
        finishedGame = true;
    }
    else { // touched flag
        finishedLevel = true;
    }

}