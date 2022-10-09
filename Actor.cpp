#include "Actor.h"
#include "StudentWorld.h"

// Peach Methods
void Peach::doSomething() {
	if (!isAlive()) // dont do anything if we are dead
		return;

	// decrement our cooldowns and timers
	if (invincibleBoost > 0)
		invincibleBoost--;
	if (starBoost > 0)
		starBoost--;
	if (shootCooldown > 0)
		shootCooldown--;

	// check if peach is overlapping anything
	Actor* enemy = getWorld()->isBlockingObject(getX(), getY(), false, false);
	if (enemy != nullptr && enemy->isAlive() && enemy->isDamageable() && starBoost > 0) {
		// if peach has star power, kill whatever it overlaps
		getWorld()->playSound(SOUND_PLAYER_KICK);
		enemy->bonk();
	}

	if (jumpDistance > 0) { // if jumping, move peach up
		Actor* above = getWorld()->isBlockingObject(getX(), getY() + SPRITE_HEIGHT / 2);
		if (above != nullptr && above->isCollidable()) { // if she hits a ceiling, bonk the ceiling
			above->bonk();
			jumpDistance = 0;
		}
		else
		{
			moveTo(getX(), getY() + SPRITE_HEIGHT / 2); // otherwise, move her up
			jumpDistance -= 1;
		}
	}
	else { // falling down
		Actor* below = getWorld()->isBlockingObject(getX(), getY() - SPRITE_HEIGHT / 2);
		if (!(below != nullptr && below->isCollidable()))  // if there is no ground below, move down
			moveTo(getX(), getY() - SPRITE_HEIGHT / 2);
	}

	int key; // input
	Actor* collided = nullptr; // if we bump into anything
	bool jumping = false; // if we jump, we don't want to bonk the floor
	if (getWorld()->getKey(key)) {
		switch (key) {
		case (KEY_PRESS_LEFT):
			setDirection(180); // make peach face left
			collided = getWorld()->isBlockingObject(getX() - SPRITE_WIDTH / 2, getY()); // check if she would bump into anything
			if (collided == nullptr || !collided->isCollidable())
				moveTo(getX() - SPRITE_WIDTH / 2, getY()); // if not, move her left
			break;
		case (KEY_PRESS_RIGHT):
			setDirection(0); // make peach face right
			collided = getWorld()->isBlockingObject(getX() + SPRITE_WIDTH / 2, getY()); // check if she bumps into anything
			if (collided == nullptr || !collided->isCollidable())
				moveTo(getX() + SPRITE_WIDTH / 2, getY());	//if not move her right
			break;
		case (KEY_PRESS_UP):
			collided = getWorld()->isBlockingObject(getX(), getY() - SPRITE_HEIGHT / 2); // check if there is floor below her
			if (collided != nullptr && collided->isCollidable()) {
				getWorld()->playSound(SOUND_PLAYER_JUMP); // if so, we want to increment our jumpBoost
				jumping = true;
				if (jumpBoost > 0)
					jumpDistance = 12;
				else
					jumpDistance = 8;
			}
			break;
		case (KEY_PRESS_SPACE):
			if (shootBoost && shootCooldown == 0) { // if peach has a flower and has no shoot cooldown, then shoot a fireball
				getWorld()->playSound(SOUND_PLAYER_FIRE);
				shootCooldown = 8;
				getWorld()->CreateFireball(true, getX() + ((1 - getDirection() / 90) * SPRITE_HEIGHT / 2), getY(), getDirection());
			}
			break;
		}
	}

	if (!jumping && collided != nullptr && collided->isCollidable()) // if we bumped into any collidable, then bonk it
		collided->bonk();
}

void Peach::bonk() {
	if (invincibleBoost > 0 || starBoost > 0) // if peach is invincible, dont do anything
		return;
	invincibleBoost = 10; // set a temporary invincibility timer
	m_hitpoints--; // decrement hp
	// disable powerups
	shootBoost = false;
	jumpBoost = false;
	if (m_hitpoints > 0) {
		getWorld()->playSound(SOUND_PLAYER_HURT);
	}
	else {
		setAlive(false);
	}
}

void Peach::powerup(int powerup) {
	switch (powerup) {
	case 1: // mushroom
		m_hitpoints = 2;
		jumpBoost = true;
		break;
	case 2: // flower
		m_hitpoints = 2;
		shootBoost = true;
		break;
	case 3: // star
		starBoost = 150;
		break;
	}
}

// Collidable Methods

void Collidable::bonk() {
	// if the block holds a goodie, drop it. otherwise, just make the bonk noise
	if (!dropGoodie())
		getWorld()->playSound(SOUND_PLAYER_BONK);
	else
		getWorld()->playSound(SOUND_POWERUP_APPEARS);
}

bool Block::dropGoodie() {
	if (m_goodie == 0) // if there is no goodie, return false
		return false;
	// otherwise, make it no longer have a goodie and drop its current goodie, return true
	getWorld()->CreatePowerup(m_goodie, getX(), getY() + SPRITE_HEIGHT);
	m_goodie = 0;
	return true;
}

// Enemy Methods
void Enemy::bonk() {
	setAlive(false);
	getWorld()->increaseScore(100); // increment score and kill itself
	die(); // call our private die() method
}

void Enemy::doSomething() {
	if (!isAlive()) // if we are dead, dont do anything
		return;

	move1();

	Actor* peach = getWorld()->isBlockingObject(getX(), getY(), true, false);
	if (peach != nullptr && getWorld()->isPeach(peach)) { // check if peach overlaps. if she does, then we want to bonk her
		peach->bonk();
		return;
	}

	move2();
}

void Enemy::move2() {
	// NOTE: So I originally followed the spec for goomba/koopa movement, but I realized that it actually differs quite significantly
	// with the movement in the sample. thus, I added a bunch of simple extraneous math (which makes it now exactly mirror the movement
	// in the sample). Specifically, the sample actually checks one pixel more to the right than it actually should (so if a koopa was
	// 1 pixel to the left of a block, it would turn around instead of going 1 more pixel to the right).

	int dir = getDirection();
	int dx = 1 - dir / 90; // delta x that we want to move in
	int cx = dx;
	// this is one example of above note (goombas and koopas in the sample turned from right to left one pixel earlier than they should)
	// when going left, cx will simply equal dx, so it will actually work as intended. however, it will now turn around one pixel earlier
	// when it is going right. pretty much, cx = dx when facing left and cx = dx + 1 when facing right
	if (dir == 0) // if we are facing right, then add 1 to cx
		cx++;
	Actor* collided = getWorld()->isBlockingObject(getX() + cx, getY());
	Actor* floor = getWorld()->isBlockingObject(getX() + (dx * (SPRITE_WIDTH - 1)) + cx, getY() - 1);
	if (collided != nullptr || floor == nullptr) // if there is a blocking object or no floor in front, we swap directions
		setDirection((dir + 180) % 360);

	// recalculate in our new direction to see if there is a blocking object
	dir = getDirection();
	dx = 1 - dir / 90; // delta x that we want to move in
	cx = dx;
	if (dir == 0) // if we are facing right, then add 1 to cx
		cx++;
	collided = getWorld()->isBlockingObject(getX() + cx, getY());
	if (collided != nullptr) // similar to the sample, we only check for a blocking object (meaning we can still move forward for 1 frame without a floor)
		return;
	moveTo(getX() + dx, getY()); // move our enemy

	// NOTE: I know that this is quite different from the spec, but I toyed with this function for a couple hours to perfectly imitate
	// the sample. I wasn't sure whether to follow the spec or the sample, but the spec said that it should be fine if our code works the
	// same as the sample. Hence, all of this extra code such as including cx and moving forward without an existing floor was added to
	// simply mimic the sample.
}

void Piranha::move2() {
	if (abs(getY() - getWorld()->getPeach()->getY()) > 1.5 * SPRITE_HEIGHT) { // check if peach is on the same level. if not, return
		return;
	}

	if (getX() < getWorld()->getPeach()->getX()) { // make our piranha face whatever direction peach is in
		setDirection(0);
	}
	else {
		setDirection(180);
	}

	if (firingDelay > 0) { // if we are on firing cooldown, decrement it every tick
		firingDelay--;
		return;
	}

	if (abs(getX() - getWorld()->getPeach()->getX()) < 8 * SPRITE_WIDTH) { // if peach is close enough, shoot and reset our firing cooldown
		getWorld()->playSound(SOUND_PIRANHA_FIRE);
		getWorld()->CreateFireball(false, getX(), getY(), getDirection());
		firingDelay = 40;
	}
}

void Koopa::die() {
	getWorld()->CreateShell(getX(), getY(), getDirection()); // make a shell wherever a koopa dies
}

// Projectile Methods
void Projectile::doSomething() {
	if (!isAlive()) { // if we are dead, dont do anything
		return;
	}
	Actor* overlap = getWorld()->isBlockingObject(getX(), getY(), targetsPeach, false);
	// we want to see if we are currently overlapping a valid target
	if (overlap != nullptr && overlap->isAlive() && ((targetsPeach && getWorld()->isPeach(overlap)) || (!targetsPeach && overlap->isDamageable()))) {
		interact(overlap); // interact based on whatever projectile we are
		setAlive(false); // disappear
		return;
	}

	// check 2 pixels below
	int newY = getY();
	Actor* below = getWorld()->isBlockingObject(getX(), getY() - SPRITE_HEIGHT / 4);
	if (below == nullptr || !below->isCollidable())  // if there is no ground below, we want to move down
		newY = getY() - SPRITE_HEIGHT / 4;

	// check 2 pixels forward (assume that we moved down, but we have not actually yet)
	int dir = getDirection();
	int newX = getX() + ((1 - dir / 90) * SPRITE_HEIGHT / 4);
	Actor* collided = getWorld()->isBlockingObject(newX, newY);

	// see if we collided with anything
	if (collided != nullptr)
		if (bounces) { // if we did and we bounce, then we flip
			moveTo(getX(), newY); // i added this just to mimic the sample code, as it seems to still move one before it bounces
			setDirection((dir + 180) % 360);
		}
		else // if we dont bounce, disappear
			setAlive(false);
	else // if we did not collide, then just move forward
		moveTo(newX, newY);
}

void Mushroom::interact(Actor* actor) {
	// return if we are not peach
	if (actor != getWorld()->getPeach())
		return;
	// increment points accordingly, playing the sound, and give peach the jump powerup
	getWorld()->increaseScore(75);
	getWorld()->getPeach()->powerup(1);
	getWorld()->playSound(SOUND_PLAYER_POWERUP);
}

void Flower::interact(Actor* actor) {
	// return if we are not peach
	if (actor != getWorld()->getPeach())
		return;
	// increment points accordingly, playing the sound, and give peach the shoot powerup
	getWorld()->increaseScore(50);
	getWorld()->getPeach()->powerup(2);
	getWorld()->playSound(SOUND_PLAYER_POWERUP);
}

void Star::interact(Actor* actor) {
	// return if we are not peach
	if (actor != getWorld()->getPeach())
		return;
	// increment points accordingly, playing the sound, and give peach the star powerup
	getWorld()->increaseScore(100);
	getWorld()->getPeach()->powerup(3);
	getWorld()->playSound(SOUND_PLAYER_POWERUP);
}

void PiranhaFireball::interact(Actor* actor) {
	// return if we are not peach
	if (actor != getWorld()->getPeach())
		return;
	getWorld()->getPeach()->bonk(); // bonk peach
}

// LevelEnder methods

void LevelEnder::doSomething() {
	if (!isAlive()) // dont do anything if we are dead
		return;

	Actor* peach = getWorld()->isBlockingObject(getX(), getY(), true, false);
	if (peach != nullptr && getWorld()->isPeach(peach)) { // if we overlap with peach
		getWorld()->increaseScore(1000); // increase our score
		setAlive(false);
		progress();
	}
}

// these call the same function, but mario simply passes true while a flag passes false
void Flag::progress() { getWorld()->NextLevel(false); }
void Mario::progress() { getWorld()->NextLevel(true); }