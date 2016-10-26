#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"
#include "p2Point.h"
#include "math.h"

#ifdef _DEBUG
#pragma comment( lib, "Box2D/libx86/Debug/Box2D.lib" )
#else
#pragma comment( lib, "Box2D/libx86/Release/Box2D.lib" )
#endif

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	mouse_joint = NULL;
	debug = true;
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));
	world->SetContactListener(this);

	// needed to create joints like mouse joint
	b2BodyDef bd;
	ground = world->CreateBody(&bd);

	// we define a shape to be our walls of the pinball
	int pinball_background[54] = {
		674, 655,
		507, 55,
		496, 43,
		476, 29,
		440, 18,
		272, 18,
		238, 27,
		212, 41,
		198, 58,
		165, 174,
		189, 234,
		174, 219,
		149, 229,
		181, 343,
		154, 360,
		112, 360,
		60, 545,
		255, 659,
		315, 762,
		397, 655,
		596, 546,
		553, 357,
		513, 357,
		485, 344,
		526, 231,
		634, 653,
		688, 653
	};

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXELS_TO_METERS(0), PIXELS_TO_METERS(0));

	b2Body* pinball_walls = world->CreateBody(&body);

	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[54 / 2];

	for (uint i = 0; i < 54 / 2; ++i)
	{
		p[i].x = PIXELS_TO_METERS(pinball_background[i * 2 + 0]);
		p[i].y = PIXELS_TO_METERS(pinball_background[i * 2 + 1]);
	}

	shape.CreateLoop(p, 54 / 2);

	b2FixtureDef fixture;
	fixture.shape = &shape;
	pinball_walls->CreateFixture(&fixture);

	int right_side[34] = {
		502, 181,
		504, 164,
		481, 60,
		457, 42,
		425, 27,
		395, 27,
		395, 54,
		415, 54,
		418, 109,
		455, 167,
		463, 168,
		452, 70,
		479, 70,
		500, 170,
		470, 241,
		476, 243,
		499, 188
	};

	b2Body* right = world->CreateBody(&body);

	b2ChainShape nshape;
	b2Vec2* q = new b2Vec2[34 / 2];

	for (uint i = 0; i < 34 / 2; ++i)
	{
		q[i].x = PIXELS_TO_METERS(right_side[i * 2 + 0]);
		q[i].y = PIXELS_TO_METERS(right_side[i * 2 + 1]);
	}

	nshape.CreateLoop(q, 34 / 2);

	b2FixtureDef nfixture;
	nfixture.shape = &nshape;
	right->CreateFixture(&nfixture);

	return true;
}

// 
update_status ModulePhysics::PreUpdate()
{
	while (todelete.count() != 0)
	{
		PhysBody* body_to_del = nullptr;
		todelete.at(0, body_to_del);
		body_to_del->body->GetWorld()->DestroyBody(body_to_del->body);
		delete[] body_to_del;
		todelete.del(todelete.getFirst());

	}

	world->Step(1.0f / 60.0f, 6, 2);

	for(b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		if(c->GetFixtureA()->IsSensor() && c->IsTouching())
		{
			PhysBody* pb1 = (PhysBody*)c->GetFixtureA()->GetBody()->GetUserData();
			PhysBody* pb2 = (PhysBody*)c->GetFixtureA()->GetBody()->GetUserData();
			if(pb1 && pb2 && pb1->listener)
				pb1->listener->OnCollision(pb1, pb2);
		}
	}

	return UPDATE_CONTINUE;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius, bool dynamic)
{
	b2BodyDef body;
	if (dynamic)
		body.type = b2_dynamicBody;
	else
		body.type = b2_staticBody;
	body.position.Set(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));


	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXELS_TO_METERS(radius);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = pbody->height = radius;

	return pbody;
}

PhysBody* ModulePhysics::CreateBouncer(int x, int y, int radius)
{
	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_p.Set(-radius, 0);
	shape.m_radius = PIXELS_TO_METERS(radius);
	
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.restitution = 1.0f;

	b->CreateFixture(&fixture);

	shape.m_p.Set(radius, 0);

	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.restitution = 1.0f;

	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = pbody->height = radius;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int width, int height)
{
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXELS_TO_METERS(width) * 0.5f, PIXELS_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width * 0.5f;
	pbody->height = height * 0.5f;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int width, int height)
{
	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXELS_TO_METERS(width) * 0.5f, PIXELS_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.isSensor = true;

	b->CreateFixture(&fixture);

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = width;
	pbody->height = height;

	return pbody;
}

PhysBody* ModulePhysics::CreateChain(int x, int y, int* points, int size)
{
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));

	b2Body* b = world->CreateBody(&body);

	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[size / 2];

	for(uint i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXELS_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXELS_TO_METERS(points[i * 2 + 1]);
	}

	shape.CreateLoop(p, size / 2);

	b2FixtureDef fixture;
	fixture.shape = &shape;

	b->CreateFixture(&fixture);

	delete p;

	PhysBody* pbody = new PhysBody();
	pbody->body = b;
	b->SetUserData(pbody);
	pbody->width = pbody->height = 0;

	return pbody;
}

b2RevoluteJoint* ModulePhysics::CreateRevoluteJoint(PhysBody * anchor, PhysBody * body, bool enable_limit, float max_angle, float min_angle, bool enable_motor, int motor_speed, int max_torque)
{
	b2RevoluteJointDef rev_joint;
	rev_joint.bodyA = anchor->body;
	rev_joint.bodyB = body->body;
	rev_joint.collideConnected = false;
	rev_joint.type = e_revoluteJoint;
	rev_joint.enableLimit = enable_limit;
	rev_joint.enableMotor = enable_motor;
	b2Vec2 anchor_center_diff(0, 0);
	rev_joint.localAnchorA = anchor_center_diff;
	b2Vec2 body_center_diff(-PIXELS_TO_METERS(97/2), 0);
	rev_joint.localAnchorB = body_center_diff;
	if (enable_limit) {
		rev_joint.lowerAngle = DEGTORAD*min_angle;
		rev_joint.upperAngle = DEGTORAD*max_angle;
	}
	if (enable_motor) {
		rev_joint.motorSpeed = motor_speed;
		rev_joint.maxMotorTorque = max_torque;
	}

	b2RevoluteJoint* rev = (b2RevoluteJoint*)world->CreateJoint(&rev_joint);

	return rev;
}

// 
update_status ModulePhysics::PostUpdate()
{
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(!debug)
		return UPDATE_CONTINUE;

	mouse_position.x = PIXELS_TO_METERS(App->input->GetMouseX());
	mouse_position.y = PIXELS_TO_METERS(App->input->GetMouseY());

	// Bonus code: this will iterate all objects in the world and draw the circles
	// You need to provide your own macro to translate meters to pixels
	for(b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for(b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			switch(f->GetType())
			{
				// Draw circles ------------------------------------------------
				case b2Shape::e_circle:
				{
					b2CircleShape* shape = (b2CircleShape*)f->GetShape();
					b2Vec2 pos = f->GetBody()->GetPosition();
					App->renderer->DrawCircle(METERS_TO_PIXELS(pos.x), METERS_TO_PIXELS(pos.y), METERS_TO_PIXELS(shape->m_radius), 255, 255, 255);
				}
				break;

				// Draw polygons ------------------------------------------------
				case b2Shape::e_polygon:
				{
					b2PolygonShape* polygonShape = (b2PolygonShape*)f->GetShape();
					int32 count = polygonShape->GetVertexCount();
					b2Vec2 prev, v;

					for(int32 i = 0; i < count; ++i)
					{
						v = b->GetWorldPoint(polygonShape->GetVertex(i));
						if(i > 0)
							App->renderer->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), 255, 100, 100);

						prev = v;
					}

					v = b->GetWorldPoint(polygonShape->GetVertex(0));
					App->renderer->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), 255, 100, 100);
				}
				break;

				// Draw chains contour -------------------------------------------
				case b2Shape::e_chain:
				{
					b2ChainShape* shape = (b2ChainShape*)f->GetShape();
					b2Vec2 prev, v;

					for(int32 i = 0; i < shape->m_count; ++i)
					{
						v = b->GetWorldPoint(shape->m_vertices[i]);
						if(i > 0)
							App->renderer->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), 100, 255, 100);
						prev = v;
					}

					v = b->GetWorldPoint(shape->m_vertices[0]);
					App->renderer->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y), METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y), 100, 255, 100);
				}
				break;

				// Draw a single segment(edge) ----------------------------------
				case b2Shape::e_edge:
				{
					b2EdgeShape* shape = (b2EdgeShape*)f->GetShape();
					b2Vec2 v1, v2;

					v1 = b->GetWorldPoint(shape->m_vertex0);
					v1 = b->GetWorldPoint(shape->m_vertex1);
					App->renderer->DrawLine(METERS_TO_PIXELS(v1.x), METERS_TO_PIXELS(v1.y), METERS_TO_PIXELS(v2.x), METERS_TO_PIXELS(v2.y), 100, 100, 255);
				}
				break;
			}

			// TODO 1: If mouse button 1 is pressed ...
			// test if the current body contains mouse position
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN && !is_clicked)
			{

				if (f->GetShape()->TestPoint(b->GetTransform(), mouse_position))
				{
					is_clicked = true;
					clicked_body = b;
				}
			}

			
					
			
		}
	}

	// If a body was selected we will attach a mouse joint to it
	// so we can pull it around
	// TODO 2: If a body was selected, create a mouse joint
	// using mouse_joint class property
	if (is_clicked)
	{
		b2MouseJointDef def;
		def.bodyA = ground;
		def.bodyB = clicked_body;

		def.target = mouse_position;

		def.dampingRatio = 0.5f;

		def.frequencyHz = 2.0f;

		def.maxForce = 100.0f * clicked_body->GetMass();

		mouse_joint = (b2MouseJoint*)world->CreateJoint(&def);
	}


	// TODO 3: If the player keeps pressing the mouse button, update
	// target position and draw a red line between both anchor points
	if (is_clicked)
	{
		App->renderer->DrawLine(mouse_joint->GetAnchorA().x, mouse_joint->GetAnchorA().y, mouse_joint->GetAnchorB().x, mouse_joint->GetAnchorB().y, 255, 0, 0);
	}

	// TODO 4: If the player releases the mouse button, destroy the joint
	if (is_clicked && App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
	{
		world->DestroyJoint(mouse_joint);
		mouse_joint = nullptr;
		is_clicked = false;
	}

	return UPDATE_CONTINUE;
}


// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	// Delete the whole physics world!
	delete world;

	return true;
}

void PhysBody::GetPosition(int& x, int &y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x) - (width);
	y = METERS_TO_PIXELS(pos.y) - (height);
}

float PhysBody::GetRotation() const
{
	return RADTODEG * body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXELS_TO_METERS(x), PIXELS_TO_METERS(y));

	const b2Fixture* fixture = body->GetFixtureList();

	while(fixture != NULL)
	{
		if(fixture->GetShape()->TestPoint(body->GetTransform(), p) == true)
			return true;
		fixture = fixture->GetNext();
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	int ret = -1;

	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXELS_TO_METERS(x1), PIXELS_TO_METERS(y1));
	input.p2.Set(PIXELS_TO_METERS(x2), PIXELS_TO_METERS(y2));
	input.maxFraction = 1.0f;

	const b2Fixture* fixture = body->GetFixtureList();

	while(fixture != NULL)
	{
		if(fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0) == true)
		{
			// do we want the normal ?

			float fx = x2 - x1;
			float fy = y2 - y1;
			float dist = sqrtf((fx*fx) + (fy*fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return output.fraction * dist;
		}
		fixture = fixture->GetNext();
	}

	return ret;
}

void ModulePhysics::BeginContact(b2Contact* contact)
{
	PhysBody* physA = (PhysBody*)contact->GetFixtureA()->GetBody()->GetUserData();
	PhysBody* physB = (PhysBody*)contact->GetFixtureB()->GetBody()->GetUserData();

	if(physA && physA->listener != NULL)
		physA->listener->OnCollision(physA, physB);

	if(physB && physB->listener != NULL)
		physB->listener->OnCollision(physB, physA);
}