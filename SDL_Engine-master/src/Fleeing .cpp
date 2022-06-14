#include"Fleeing .h"
#include "TextureManager.h"
#include"Util.h"
#include "Game.h"


Fleeing::Fleeing() :m_startPos(glm::vec2(200.0f,200.0f)),
m_maxSpeed(20.0f), m_turnRate(5.0f), m_accelerationRate(2.0f)
{
	TextureManager::Instance().Load("../Assets/textures/d7_small.png", "d7");

	const auto size = TextureManager::Instance().GetTextureSize("d7");
	SetWidth(static_cast<int>(size.x));
	SetHeight(static_cast<int>(size.y));
	GetTransform()->position = m_startPos;
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->acceleration = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;
	SetCurrentHeading(0.0f);
	SetType(GameObjectType::FLEEING);
}

Fleeing::~Fleeing()
= default;

void Fleeing::Draw()
{
	// draw the Fleeing
	TextureManager::Instance().Draw("d7", GetTransform()->position, GetCurrentHeading(), 255, true);
}

void Fleeing::Update()
{
	Move();
	CheckBounds();
}

void Fleeing::Clean()
{
}

float Fleeing::getMaxSpeed() const
{
	return m_maxSpeed;
}

float Fleeing::getTurnRate() const
{
	return m_turnRate;
}

glm::vec2 Fleeing::getDesiredVelocity() const
{
	return m_desiredVelocity;
}

float Fleeing::getAccelerationRate() const
{
	return m_accelerationRate;
}

void Fleeing::setMaxSpeed(float speed)
{
	m_maxSpeed = speed;
}

void Fleeing::setTurnRate(float angle)
{
	m_turnRate = angle;
}

void Fleeing::setDesiredVelocity(glm::vec2 target_position)
{
	m_desiredVelocity = Util::Normalize(target_position - GetTransform()->position) * m_maxSpeed;
}

void Fleeing::setAccelerationRate(float rate)
{
	m_accelerationRate = rate;
}

void Fleeing::Move()
{
	Seek();
	auto delta_time = Game::Instance().GetDeltaTime();

	const glm::vec2 initial_positiona = GetTransform()->position;

	const glm::vec2 velocity_term = GetRigidBody()->velocity * delta_time;

	const glm::vec2 acceleration_term = GetRigidBody()->acceleration * 0.5f;

	glm::vec2 final_position = initial_positiona + velocity_term + acceleration_term;

	GetTransform()->position = final_position;

	GetRigidBody()->velocity += GetRigidBody()->acceleration;

	GetRigidBody()->velocity = Util::Clamp(GetRigidBody()->velocity, getMaxSpeed());
}


void Fleeing::CheckBounds()
{

}

void Fleeing::Reset()
{
	GetTransform()->position = m_startPos;
}

void Fleeing::LookWhereIAmGoing(glm::vec2 target_direction)
{

	const auto target_rotation = Util::SignedAngle(GetCurrentDirection(), target_direction);
	if (abs(target_rotation) > m_turnRate) {

		SetCurrentHeading(GetCurrentHeading() - getTurnRate() * Util::Sign(target_rotation));
	}

}


void Fleeing::Seek()
{
	setDesiredVelocity(GetTargetPosition());

	const glm::vec2 streering_direction = getDesiredVelocity() - GetCurrentDirection();

	LookWhereIAmGoing(streering_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * getAccelerationRate();

}

