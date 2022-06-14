#include "Starship.h"
#include "TextureManager.h"
#include"Util.h"
#include "Game.h"


Starship::Starship() :m_startPos(glm::vec2(400.0f, 400.0f)),
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
	SetType(GameObjectType::SPACE_SHIP);
}

Starship::~Starship()
= default;

void Starship::Draw()
{
	// draw the Starship
	TextureManager::Instance().Draw("d7", GetTransform()->position, GetCurrentHeading(), 255, true);
}

void Starship::Update()
{
	Move();
	CheckBounds();
}

void Starship::Clean()
{
}

float Starship::getMaxSpeed() const
{
	return m_maxSpeed;
}

float Starship::getTurnRate() const
{
	return m_turnRate;
}

glm::vec2 Starship::getDesiredVelocity() const
{
	return m_desiredVelocity;
}

float Starship::getAccelerationRate() const
{
	return m_accelerationRate;
}

void Starship::setMaxSpeed(float speed)
{
	m_maxSpeed = speed;
}

void Starship::setTurnRate(float angle)
{
	m_turnRate = angle;
}

void Starship::setDesiredVelocity(glm::vec2 target_position)
{
	m_desiredVelocity = Util::Normalize(target_position - GetTransform()->position) * m_maxSpeed;
}

void Starship::setAccelerationRate(float rate)
{
	m_accelerationRate = rate;
}

void Starship::Move()
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


void Starship::CheckBounds()
{

}

void Starship::Reset()
{
	GetTransform()->position = m_startPos;
}

void Starship::LookWhereIAmGoing(glm::vec2 target_direction)
{

	const auto target_rotation = Util::SignedAngle(GetCurrentDirection(), target_direction);
	if(abs(target_rotation) > m_turnRate) {

		SetCurrentHeading(GetCurrentHeading() + getTurnRate() * Util::Sign(target_rotation));
	}

}


void Starship::Seek()
{
	setDesiredVelocity(GetTargetPosition());

	const glm::vec2 streering_direction = getDesiredVelocity() - GetCurrentDirection();

	LookWhereIAmGoing(streering_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * getAccelerationRate();

}

