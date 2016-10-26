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
	safeball = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	close_time_in = 0;
	close_time_top = 0;


	App->renderer->camera.x = App->renderer->camera.y = 0;

	ball = App->textures->Load("pinball/pinball_ball.png"); 
	box = App->textures->Load("pinball/crate.png");
	rick = App->textures->Load("pinball/rick_head.png");
	background = App->textures->Load("pinball/background.png");
	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");
	image = App->textures->Load("pinball/images.png");
	lkicker = App->textures->Load("pinball/left_kicker.png");
	rkicker = App->textures->Load("pinball/right_kicker.png");

	out_sensor = App->physics->CreateRectangleSensor(326, SCREEN_HEIGHT + 50, 150, 50);
	out_sensor->body->SetSleepingAllowed(false);
	launcher = App->physics->CreateRectangleSensor(702, 645, 60, 35);
	launcher->body->SetSleepingAllowed(false);
	in_block = App->physics->CreateCircle(545, 235, 14, 1);
	in_sensor = App->physics->CreateRectangleSensor(545, 271, 32, 20);
	top_block = App->physics->CreateRectangle(303, 17, 9, 22, false);
	top_sensor = App->physics->CreateRectangleSensor(280, 17, 9, 22);


	top_screen_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH/2, 50, SCREEN_WIDTH, 100);
	mid_screen_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, 150, SCREEN_WIDTH, 450);

	bouncers.add(App->physics->CreateBouncer(306, 100, 8));
	// DANI --> NEED TO FIX THIS ASAP
	leftkicker_axis = App->physics->CreateCircle(218, 568, 14, 1);
	leftkicker = App->physics->CreateRectangle(218, 568, 87, 24);
	left_kicker_rev = App->physics->CreateRevoluteJoint(leftkicker_axis, leftkicker, 87, true, 25, -45, false, 0, 200, true);

	rightkicker_axis = App->physics->CreateCircle(430, 568, 14, 1);
	rightkicker = App->physics->CreateRectangle(315, 568, 87, 24);
	right_kicker_rev = App->physics->CreateRevoluteJoint(rightkicker_axis, rightkicker, 87, true, 45, -25, false, 0, 800, false);
	
	/*
	PhysBody* rightkickaxis;
	PhysBody* smallkickaxis;
	
	PhysBody* rightkick;
	PhysBody* smallkick;


	*/
	black_hole.PushBack({ 4, 3, 90, 47 });
	black_hole.PushBack({ 4, 49, 90, 47 });
	black_hole.PushBack({ 4, 95, 90, 47 });
	black_hole.PushBack({ 4, 141, 90, 47 });
	black_hole.PushBack({ 4, 187, 90, 47 });
	black_hole.loop = true;
	black_hole.speed = 0.08f;

	foreground = App->textures->Load("pinball/foreground.png");
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
	launcher->body->SetTransform(b2Vec2(PIXELS_TO_METERS(712), PIXELS_TO_METERS(645)), 0);
	current_time = SDL_GetTicks();
	while (bodies_to_copy.count() != 0)
	{
		App->physics->ChangeRadius(bodies_to_copy.getFirst()->data, radius_to_copy.getFirst()->data);
		
		bodies_to_copy.del(bodies_to_copy.getFirst());
		radius_to_copy.del(radius_to_copy.getFirst());
	}

	return UPDATE_CONTINUE;
}

update_status ModuleSceneIntro::Update()
{
	if (current_time > close_time_in)
		in_block->body->SetActive(true);

	if (current_time > close_time_top)
		top_block->body->SetActive(true);
	//Draw background
	SDL_Rect back;
	back.x = 0;
	back.y = 0;
	back.w = SCREEN_WIDTH;
	back.h = SCREEN_HEIGHT;

	App->renderer->Blit(background, 0, 0, &back);

	if (balls == 0) { //DANI --> CREATES BALLS BY DEFAULT
		circles.add(App->physics->CreateCircle(642, 613, 16)); //Sergi had it at 14
		circles.getLast()->data->listener = this;
		circles.getLast()->data->body->SetSleepingAllowed(false);
		circles.getLast()->data->body->SetFixedRotation(0);

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
		circles.add(App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 8));
		circles.getLast()->data->listener = this;
	}

	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_REPEAT)
	{
		launcher->body->SetTransform(b2Vec2(PIXELS_TO_METERS(642), PIXELS_TO_METERS(645)), 0);
	}

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		leftkicker->body->ApplyTorque(-300, true);
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		rightkicker->body->ApplyTorque(300, true);
	}
	
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

	App->renderer->Blit(image, 292, 113, &(black_hole.GetCurrentFrame()), 1.0f);
	
	App->renderer->Blit(lkicker, 216, 557, NULL, 1.0f, leftkicker->GetRotation(), NULL, left_kicker_rev->GetAnchorA().x, left_kicker_rev->GetAnchorA().y);
	App->renderer->Blit(rkicker, 342, 557, NULL, 1.0f, rightkicker->GetRotation(), NULL, 95, 14);
	
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
		bodyA->body->SetAwake(false);
		bodyA->body->SetLinearVelocity(b2Vec2(0, -700));
		App->audio->PlayFx(bonus_fx);

	}

	if (bodyB == in_sensor)
	{
		in_block->body->SetActive(false);
		close_time_in = SDL_GetTicks() + 1000;
	}

	if (bodyB == top_sensor)
	{
		top_block->body->SetActive(false);
		close_time_top = SDL_GetTicks() + 1000;
	}
	
	if (bodyB == top_screen_sensor  && bodyA->width != 8)
	{
		bodies_to_copy.add(bodyA);
		radius_to_copy.add(8);
	}
	if (bodyB == mid_screen_sensor && bodyA->width != 14)
	{
		bodies_to_copy.add(bodyA);
		radius_to_copy.add(14);
	}
}
