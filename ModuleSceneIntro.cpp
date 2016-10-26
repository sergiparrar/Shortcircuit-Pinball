#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	ball = box = rick = NULL;
	ray_on = false;
	sensed = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	ball = App->textures->Load("pinball/pinball_ball.png"); 
	box = App->textures->Load("pinball/crate.png");
	rick = App->textures->Load("pinball/rick_head.png");
	background = App->textures->Load("pinball/background.png");
	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");

	out_sensor = App->physics->CreateRectangleSensor(326, SCREEN_HEIGHT + 50, 150, 50);
	out_sensor->body->SetSleepingAllowed(false);
	launcher = App->physics->CreateRectangleSensor(702, 645, 60, 15);
	launcher->body->SetSleepingAllowed(false);

	// DANI --> NEED TO FIX THIS ASAP
	/*b2Body* leftkickaxis = (b2Body*)App->physics->CreateCircle(216, 558, 14);
	PhysBody* rightkickaxis;
	PhysBody* smallkickaxis;
	b2Body* leftkick = (b2Body*)App->physics->CreateRectangle(216, 558, 97, 24);
	PhysBody* rightkick;
	PhysBody* smallkick;

	leftkicker.bodyA = leftkickaxis;
	leftkicker.bodyB = leftkick;
	leftkicker.collideConnected = true;
	leftkicker.enableLimit = true;
	leftkicker.lowerAngle = -45;
	leftkicker.upperAngle = 45;*/

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::PreUpdate()
{
	launcher->body->SetTransform(b2Vec2(PIXELS_TO_METERS(702), PIXELS_TO_METERS(645)), 0);

	return UPDATE_CONTINUE;
}

update_status ModuleSceneIntro::Update()
{
	//Draw background
	SDL_Rect back;
	back.x = 0;
	back.y = 0;
	back.w = SCREEN_WIDTH;
	back.h = SCREEN_HEIGHT;

	App->renderer->Blit(background, 0, 0, &back);

	if (balls == 0) { //DANI --> CREATES BALLS BY DEFAULT
		circles.add(App->physics->CreateCircle(642, 613, 12));
		circles.getLast()->data->listener = this;
		circles.getLast()->data->body->SetSleepingAllowed(false);
		balls++;
	}

	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		ray_on = !ray_on;
		ray.x = App->input->GetMouseX();
		ray.y = App->input->GetMouseY();
	}

	if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		circles.add(App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 10));
		circles.getLast()->data->listener = this;
	}

	if(App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
		boxes.add(App->physics->CreateRectangle(App->input->GetMouseX(), App->input->GetMouseY(), 100, 50));
	}

	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
	{
		launcher->body->SetTransform(b2Vec2(PIXELS_TO_METERS(642), PIXELS_TO_METERS(645)), 0);
	}

	/*if (SDL_SCANCODE_BACKSPACE == KEY_DOWN && App->scene_intro->launcher->listener->OnCollision == true) 
	{
		//DANI --> Don't know how to place the restitucion
	}*/
	// Prepare for raycast ------------------------------------------------------
	
	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	fVector normal(0.0f, 0.0f);

	// All draw functions ------------------------------------------------------
	p2List_item<PhysBody*>* c = circles.getFirst();

	while(c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		SDL_Rect size;
		size.w = size.h = 30 + 0.027*(y - 95); //SERGI: formula for dynamic resizing of the sprite, may need tweaking
		size.x = x;
		size.y = y;
		App->renderer->Blit(ball, x, y, NULL, 1.0f, 0, &size);
		c = c->next;
	}

	c = boxes.getFirst();

	while(c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		App->renderer->Blit(box, x, y, NULL, 1.0f, c->data->GetRotation());
		if(ray_on)
		{
			int hit = c->data->RayCast(ray.x, ray.y, mouse.x, mouse.y, normal.x, normal.y);
			if(hit >= 0)
				ray_hit = hit;
		}
		c = c->next;
	}

	c = ricks.getFirst();

	while(c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		App->renderer->Blit(rick, x, y, NULL, 1.0f, c->data->GetRotation());
		c = c->next;
	}

	// ray -----------------
	if(ray_on == true)
	{
		fVector destination(mouse.x-ray.x, mouse.y-ray.y);
		destination.Normalize();
		destination *= ray_hit;

		App->renderer->DrawLine(ray.x, ray.y, ray.x + destination.x, ray.y + destination.y, 255, 255, 255);

		if(normal.x != 0.0f)
			App->renderer->DrawLine(ray.x + destination.x, ray.y + destination.y, ray.x + destination.x + normal.x * 25.0f, ray.y + destination.y + normal.y * 25.0f, 100, 255, 100);
	}

	App->renderer->Blit(foreground, 0, 0, &back);

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	int x, y;

	//

	if (bodyB == out_sensor)
	{
		circles.del(circles.findNode(bodyA));
		App->physics->todelete.add(bodyA);
		balls--;
		App->audio->PlayFx(bonus_fx);
		//SERGI -> Call here function to start new round
	}
	if (bodyB == launcher)
	{
		bodyA->body->SetAwake(true);
		//bodyA->body->SetLinearVelocity(b2Vec2(0, 2000));
		bodyA->body->ApplyForceToCenter(b2Vec2(0, 1999999), true);
		App->audio->PlayFx(bonus_fx);

	}
}
