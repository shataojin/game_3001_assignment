#include "Starship.h"
#include "TextureManager.h"
#include "Util.h"
#include "Game.h"
#include"EventManager.h"

Starship::Starship() : m_maxSpeed(20.0f),
	m_turnRate(5.0f), m_accelerationRate(2.0f), m_startPos( glm::vec2(300.0f, 500.0f) )
	, m_currentAnimationState(PlayerAnimationState::PLAYER_IDLE_RIGHT)
{
	

	TextureManager::Instance().LoadSpriteSheet(
		"../Assets/sprites/atlas.txt",
		"../Assets/sprites/24242424.png",
		"spritesheet");

	SetSpriteSheet(TextureManager::Instance().GetSpriteSheet("spritesheet"));

	// set frame width
	SetWidth(53);

	// set frame height
	SetHeight(58);

	GetTransform()->position = m_startPos;
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->acceleration = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	SetCurrentHeading(0.0f); // current facing angle
	SetLOSDistance(400.0f);
	SetWhiskerAngle(45.0f);

	SetLOSColour(glm::vec4(1, 0, 0, 1)); // default LOS colour is Red
	
	SetType(GameObjectType::AGENT);

	// New for Lab 7
	SetActionState(ActionState::NO_ACTION);
	BuildPatrolPath();

	BuildAnimations();
}

Starship::~Starship()
= default;

void Starship::Draw()
{
	// draw the player according to animation state
	switch (m_currentAnimationState)
	{
	case PlayerAnimationState::PLAYER_IDLE_RIGHT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("idle"),
			GetTransform()->position, 0.12f, 0, 255, true);
		break;
	case PlayerAnimationState::PLAYER_IDLE_LEFT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("idle"),
			GetTransform()->position, 0.12f, 0, 255, true, SDL_FLIP_HORIZONTAL);
		break;
	case PlayerAnimationState::PLAYER_RUN_RIGHT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("run"),
			GetTransform()->position, 0.25f, 0, 255, true);
		break;
	case PlayerAnimationState::PLAYER_RUN_LEFT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("run"),
			GetTransform()->position, 0.25f, 0, 255, true, SDL_FLIP_HORIZONTAL);
		break;
	default:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("idle"),
			GetTransform()->position, 0.12f, 0, 255, true);
		break;
	}

	// draw LOS
	Util::DrawLine(GetTransform()->position + GetCurrentDirection() * 0.5f * static_cast<float>( GetWidth()), 
		GetTransform()->position + GetCurrentDirection() * GetLOSDistance(), GetLOSColour());
}

void Starship::Update()
{

	if (EventManager::Instance().KeyPressed(SDL_SCANCODE_K))
	{
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("die"),
			GetTransform()->position, 0.25f, 0, 255, true);
	}
	if (EventManager::Instance().KeyPressed(SDL_SCANCODE_P))
	{
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("run"),
			GetTransform()->position, 0.25f, 0, 255, true);
	}



	// Determine which to action to perform
	switch(GetActionState())
	{
	case ActionState::PATROL:
		Move();
		break;
	case ActionState::MOVE_TO_LOS:
		// perform move to LOS action
		break;
	case ActionState::ATTACK:
		// perform attack action
		break;
	}



	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
	{
		facingleft = true;
		PlayerAnimationState::PLAYER_RUN_LEFT;
		GetTransform()->position = GetTransform()->position + glm::vec2(-player_speed, 0.0f);
	}
	else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
	{
		facingleft = false;
		PlayerAnimationState::PLAYER_RUN_LEFT;
		GetTransform()->position = GetTransform()->position + glm::vec2(player_speed, 0.0f);
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_W))
	{

		GetTransform()->position = GetTransform()->position + glm::vec2(0.0f, -player_speed);

		if (facingleft == true)
		{
			PlayerAnimationState::PLAYER_RUN_LEFT;
		}
		else
		{
			PlayerAnimationState::PLAYER_RUN_LEFT;
		}
	}
	else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_S))
	{

		GetTransform()->position = GetTransform()->position + glm::vec2(0.0f, player_speed);

		if (facingleft == true)
		{
			PlayerAnimationState::PLAYER_RUN_LEFT;
		}
		else
		{
			PlayerAnimationState::PLAYER_RUN_LEFT;
		}
	}

}

void Starship::Clean()
{

}

float Starship::GetMaxSpeed() const
{
	return m_maxSpeed;
}

float Starship::GetTurnRate() const
{
	return m_turnRate;
}

glm::vec2 Starship::GetDesiredVelocity() const
{
	return m_desiredVelocity;
}

float Starship::GetAccelerationRate() const
{
	return m_accelerationRate;
}

void Starship::SetMaxSpeed(float speed)
{
	m_maxSpeed = speed;
}

void Starship::SetTurnRate(float angle)
{
	m_turnRate = angle;
}

void Starship::SetDesiredVelocity(const glm::vec2 target_position)
{
	m_desiredVelocity = Util::Normalize(target_position - GetTransform()->position) * m_maxSpeed;
}

void Starship::SetAccelerationRate(const float rate)
{
	m_accelerationRate = rate;
}

void Starship::Seek()
{
	// Find Next Waypoint if within 10px of the current waypoint
	if(Util::Distance(m_patrolPath[m_wayPoint], GetTransform()->position) < 10)
	{
		// check to see if you are at the last point in the path
		if(++m_wayPoint == m_patrolPath.size())
		{
			// if so...reset
			m_wayPoint = 0;
		}
		SetTargetPosition(m_patrolPath[m_wayPoint]);
	}


	SetDesiredVelocity(GetTargetPosition());

	const glm::vec2 steering_direction = GetDesiredVelocity() - GetCurrentDirection();

	LookWhereIAmGoing(steering_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * GetAccelerationRate();
}

void Starship::LookWhereIAmGoing(const glm::vec2 target_direction)
{
	const auto target_rotation = Util::SignedAngle(GetCurrentDirection(), target_direction);
	if (GetCollisionWhiskers()[0])
	{
		SetCurrentHeading(GetCurrentHeading() + GetTurnRate());
	}
	else if (GetCollisionWhiskers()[2])
	{
		SetCurrentHeading(GetCurrentHeading() - GetTurnRate());
	}
	else if (abs(target_rotation) > m_turnRate)
	{
		SetCurrentHeading(GetCurrentHeading() + GetTurnRate() * Util::Sign(target_rotation));
	}
	UpdateWhiskers(GetWhiskerAngle()); // New for Lab 3.
}

void Starship::BuildPatrolPath()
{
	m_patrolPath.push_back(glm::vec2(760, 40)); // Top Right corner node
	m_patrolPath.push_back(glm::vec2(760, 560)); // Bottom Right corner node
	m_patrolPath.push_back(glm::vec2(40, 560)); // Bottom Left corner node
	m_patrolPath.push_back(glm::vec2(40, 40)); // Top Left corner node
	m_wayPoint = 0;

	SetTargetPosition(m_patrolPath[m_wayPoint]); // Top Right to Start
}

void Starship::Move()
{
	
	Seek(); // Get our target for this frame.

	// Kinematic equation: final_position = position + velocity * time + 0.5*acceleration * time*time
	const auto delta_time = Game::Instance().GetDeltaTime();

	// compute the position term
	const glm::vec2 initial_position = GetTransform()->position;

	// compute the velocity term
	const glm::vec2 velocity_term = GetRigidBody()->velocity * delta_time;

	// compute the acceleration term
	const glm::vec2 acceleration_term = GetRigidBody()->acceleration * 0.5f;// *dt;


	// compute the new position
	glm::vec2 final_position = initial_position + velocity_term + acceleration_term;

	GetTransform()->position = final_position;

	// add our acceleration to velocity
	GetRigidBody()->velocity += GetRigidBody()->acceleration;

	// clamp our velocity at max speed
	GetRigidBody()->velocity = Util::Clamp(GetRigidBody()->velocity, GetMaxSpeed());
}

void Starship::CheckBounds(){}

void Starship::Reset()
{
	GetTransform()->position = m_startPos;
}


void Starship::BuildAnimations()
{
	auto idle_animation = Animation();

	idle_animation.name = "idle";
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-0"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-1"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-2"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-3"));

	SetAnimation(idle_animation);

	auto run_animation = Animation();

	run_animation.name = "run";
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-0"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-1"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-2"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-3"));

	SetAnimation(run_animation);

	auto shot_animation = Animation();

	shot_animation.name = "shot";
	shot_animation.frames.push_back(GetSpriteSheet()->GetFrame("shot"));

	SetAnimation(run_animation);

	auto die_animation = Animation();

	die_animation.name = "die";
	die_animation.frames.push_back(GetSpriteSheet()->GetFrame("die"));

	SetAnimation(die_animation);
}