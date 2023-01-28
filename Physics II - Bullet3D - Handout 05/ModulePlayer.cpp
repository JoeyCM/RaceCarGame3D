#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "PhysBody3D.h"

ModulePlayer::ModulePlayer(Application* app, bool start_enabled) : Module(app, start_enabled), vehicle(NULL)
{
	turn = acceleration = brake = 0.0f;
}

ModulePlayer::~ModulePlayer()
{}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading player");

	VehicleInfo car;

	// Car chasis --------------------------------------------
	// Main car structure
	car.chassis_size.Set(2, .3f, 5);
	car.chassis_offset.Set(0, 1.5, 0);

	// FrontWing structure
	car.chassis2_size.Set(1.0f, .5f, 1.5f);
	car.chassis2_offset.Set(0, 1.1f, 3.2);

	// FrontWing part
	car.chassis3_size.Set(3.0f, .25f, .75f);
	car.chassis3_offset.Set(0, .9f, 4.2);

	// RearWing stucture 1
	car.chassis4_size.Set(.25f, 1.0f, .75f);
	car.chassis4_offset.Set(.82f, 2.1f, -2.1);

	// RearWing stucture 2
	car.chassis5_size.Set(.25f, 1.0f, .75f);
	car.chassis5_offset.Set(-.82f, 2.1f, -2.1);

	// Car properties ----------------------------------------
	car.mass = 500.0f;
	car.suspensionStiffness = 20.88f;
	car.suspensionCompression = 0.83f;
	car.suspensionDamping = 1.23f;
	car.maxSuspensionTravelCm = 1000.0f;
	car.frictionSlip = 0.9f;
	car.maxSuspensionForce = 6000.0f;

	// Wheel properties ---------------------------------------
	float connection_height = 1.5f;
	float wheel_radius = 0.5f;
	float wheel_width = 0.65f;
	float suspensionRestLength = 0.6f;

	// Don't change anything below this line ------------------

	float half_width = car.chassis_size.x*0.5f;
	float half_length = car.chassis_size.z*0.5f;
	
	vec3 direction(0,-1,0);
	vec3 axis(-1,0,0);
	
	car.num_wheels = 4;
	car.wheels = new Wheel[4];

	// FRONT-LEFT ------------------------
	car.wheels[0].connection.Set(half_width + 0.5f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[0].direction = direction;
	car.wheels[0].axis = axis;
	car.wheels[0].suspensionRestLength = suspensionRestLength;
	car.wheels[0].radius = wheel_radius;
	car.wheels[0].width = wheel_width;
	car.wheels[0].front = true;
	car.wheels[0].drive = true;
	car.wheels[0].brake = false;
	car.wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car.wheels[1].connection.Set(-half_width - 0.5f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[1].direction = direction;
	car.wheels[1].axis = axis;
	car.wheels[1].suspensionRestLength = suspensionRestLength;
	car.wheels[1].radius = wheel_radius;
	car.wheels[1].width = wheel_width;
	car.wheels[1].front = true;
	car.wheels[1].drive = true;
	car.wheels[1].brake = false;
	car.wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car.wheels[2].connection.Set(half_width + 0.5f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[2].direction = direction;
	car.wheels[2].axis = axis;
	car.wheels[2].suspensionRestLength = suspensionRestLength;
	car.wheels[2].radius = wheel_radius;
	car.wheels[2].width = wheel_width;
	car.wheels[2].front = false;
	car.wheels[2].drive = false;
	car.wheels[2].brake = true;
	car.wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car.wheels[3].connection.Set(-half_width - 0.5f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[3].direction = direction;
	car.wheels[3].axis = axis;
	car.wheels[3].suspensionRestLength = suspensionRestLength;
	car.wheels[3].radius = wheel_radius;
	car.wheels[3].width = wheel_width;
	car.wheels[3].front = false;
	car.wheels[3].drive = false;
	car.wheels[3].brake = true;
	car.wheels[3].steering = false;

	vehicle = App->physics->AddVehicle(car);
	vehicle->SetPos(0, 0, 0);
	
	vehicle->collision_listeners.add(this);
	vehicle->SetId(1);

	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	return true;
}

// Update: draw background
update_status ModulePlayer::Update(float dt)
{
	turn = acceleration = brake = 0.0f;

	// Drag force applied to the vehicle
	App->physics->DragForce(vehicle, 30);

	position.setValue(vehicle->GetPos().getX(), vehicle->GetPos().getY(), vehicle->GetPos().getZ());

	// Accelerate
	if(App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
	{
		acceleration = MAX_ACCELERATION;
	}

	// Turn Left
	if(App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	{
		if(turn < TURN_DEGREES)
			turn +=  TURN_DEGREES;
	}

	// Turn Right
	if(App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{
		if(turn > -TURN_DEGREES)
			turn -= TURN_DEGREES;
	}

	// Brake
	if(App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
	{
		brake = BRAKE_POWER;
	}

	// Car Jumping
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) 
	{
		vehicle->Push(0, 3000, 0);
	}

	// Brake if there's no throttle
	if (App->input->GetKey(SDL_SCANCODE_S) != KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_W) != KEY_REPEAT) 
	{
		brake = BRAKE_POWER / 100;

		if (vehicle->GetKmh() > 100) 
		{
			brake = BRAKE_POWER / 30;
		}
	}


	vehicle->ApplyEngineForce(acceleration);
	vehicle->Turn(turn);
	vehicle->Brake(brake);


	/* POST UPDATE */
	vehicle->Render();
	
	// Set window title
	char title[80];
	sprintf_s(title, "%.1f Km/h", vehicle->GetKmh());
	App->window->SetTitle(title);

	return UPDATE_CONTINUE;
}

vec3 ModulePlayer::GetVehicleForwardVector() 
{
	return vehicle->GetForwardVector();
}


