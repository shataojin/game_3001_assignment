#include "SpaceShip.h"

#include "Game.h"
#include "TextureManager.h"
#include "Util.h"

SpaceShip::SpaceShip()
{
	TextureManager::Instance().Load("../Assets/textures/reliant_small.png", "space_ship");

	const auto size = TextureManager::Instance().GetTextureSize("space_ship");
	SetWidth(size.x);
	SetHeight(size.y);
	
	
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->acceleration = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	// starting motion properties
	SetCurrentHeading(0.0f);// current facing angle
	SetCurrentDirection(glm::vec2(1.0f, 0.0f)); // facing right
	m_maxSpeed = 10.0f; // a maximum number of pixels moved per frame
	m_turnRate = 5.0f; // a maximum number of degrees to turn each time-step
	m_accelerationRate = 2.0f; // a maximum number of pixels to add to the velocity each frame
	
	SetLOSDistance(400.0f); // 5 ppf x 80 feet
	SetLOSColour(glm::vec4(1, 0, 0, 1));



	// Set patrol path
	m_patrolPath.push_back(glm::vec2(760, 40)); // Top Right 
	m_patrolPath.push_back(glm::vec2(760, 560)); // Bottom Right 
	m_patrolPath.push_back(glm::vec2(40, 560)); // Bottom Left 
	m_patrolPath.push_back(glm::vec2(40, 40)); // Top left
	m_waypoint = 0;

	SetTargetPosition(m_patrolPath[m_waypoint]); // Top Right
	SetType(GameObjectType::AGENT);
}

SpaceShip::~SpaceShip() 
= default;

void SpaceShip::Draw()
{
	// alias for x and y
	const auto x = GetTransform()->position.x;
	const auto y = GetTransform()->position.y;

	// draw the ship
	TextureManager::Instance().Draw("space_ship", x, y, GetCurrentHeading(), 255, isCentered());

	// draw LOS
	Util::DrawLine(GetTransform()->position, GetTransform()->position + GetCurrentDirection() * GetLOSDistance(), GetLOSColour());
}

void SpaceShip::Update()
{

}

void SpaceShip::Clean()
{
}

float SpaceShip::GetMaxSpeed() const
{
	return m_maxSpeed;
}

float SpaceShip::GetTurnRate() const
{
	return m_turnRate;
}

float SpaceShip::GetAccelerationRate() const
{
	return m_accelerationRate;
}

glm::vec2 SpaceShip::GetDesiredVelocity() const
{
	return m_desiredVelocity;
}

void SpaceShip::SetMaxSpeed(const float speed)
{
	m_maxSpeed = speed;
}

void SpaceShip::SetTurnRate(const float angle)
{
	m_turnRate = angle;
}

void SpaceShip::SetAccelerationRate(const float rate)
{
	m_accelerationRate = rate;
}

void SpaceShip::SetDesiredVelocity(const glm::vec2 tarGet_position)
{
	m_desiredVelocity = Util::Normalize(tarGet_position - GetTransform()->position);
}

void SpaceShip::Seek()
{
	// Find next waypoint:
	if(Util::Distance(m_patrolPath[m_waypoint], GetTransform()->position) < 10)
	{
		// check to see if you are at the last point in the path 
		if(++m_waypoint == m_patrolPath.size())
		{
			// if so...reSet
			m_waypoint = 0;
		}
		SetTargetPosition(m_patrolPath[m_waypoint]);
	}
	
	SetDesiredVelocity(GetTargetPosition());

	const glm::vec2 steering_direction = GetDesiredVelocity() - GetCurrentDirection();

	LookWhereYoureGoing(steering_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * GetAccelerationRate();
}

void SpaceShip::LookWhereYoureGoing(const glm::vec2 tarGet_direction)
{
	float tarGet_rotation = Util::SignedAngle(GetCurrentDirection(), tarGet_direction) - 90;

	const float turn_sensitivity = 3.0f;

	if (GetCollisionWhiskers()[0])
	{
		SetCurrentHeading(GetCurrentHeading() + GetTurnRate());
	}
	else if (GetCollisionWhiskers()[2])
	{
		SetCurrentHeading(GetCurrentHeading() - GetTurnRate());
	}
	else if (abs(tarGet_rotation) > turn_sensitivity)
	{
		if (tarGet_rotation > 0.0f)
		{
			SetCurrentHeading(GetCurrentHeading() + GetTurnRate());
		}
		else if (tarGet_rotation < 0.0f)
		{
			SetCurrentHeading(GetCurrentHeading() - GetTurnRate());
		}
	}

	UpdateWhiskers(GetWhiskerAngle());
}

void SpaceShip::m_move()
{
	Seek();
	
	//                                   final Position     position term    velocity term     acceleration term
	// kinematic equation for motion --> Pf            =      Pi     +     Vi*(time)    +   (0.5)*Ai*(time * time)

	const float dt = Game::Instance().GetDeltaTime();

	// compute the position term
	const glm::vec2 initial_position = GetTransform()->position;

	// compute the velocity term
	const glm::vec2 velocity_term = GetRigidBody()->velocity * dt;

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
