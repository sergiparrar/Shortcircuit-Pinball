#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"
#include "Box2D/Box2D/Box2D.h"
#include "Animation.h"

class PhysBody;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status PreUpdate();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

public:
	p2List<PhysBody*> circles;
	p2List<PhysBody*> bouncers;
	p2List<PhysBody*> ricks;

	PhysBody* out_sensor;
	PhysBody* in_sensor;
	PhysBody* launcher;
	bool safeball;

	b2RevoluteJoint* left_kicker_rev;
	PhysBody* leftkicker_axis;
	PhysBody* leftkicker;

	b2RevoluteJoint* right_kicker_rev;
	PhysBody* rightkicker_axis;
	PhysBody* rightkicker;

	PhysBody* in_block;

	SDL_Texture* ball;
	SDL_Texture* box;
	SDL_Texture* rick;
	SDL_Texture* background;
	SDL_Texture* foreground;
	SDL_Texture* image;

	Animation black_hole;

	uint bonus_fx;
	p2Point<int> ray;
	bool ray_on;
	uint balls = 0;
private:
	uint32 current_time;
	uint32 close_time;
};
