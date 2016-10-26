#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

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
	PhysBody* launcher;
	bool safeball;

	/*b2RevoluteJointDef leftkicker;
	b2RevoluteJointDef rightkicker;
	b2RevoluteJointDef smallkicker;*/

	SDL_Texture* ball;
	SDL_Texture* box;
	SDL_Texture* rick;
	SDL_Texture* background;
	SDL_Texture* foreground;
	uint bonus_fx;
	p2Point<int> ray;
	bool ray_on;
	uint balls = 0;
};
