#pragma once
#ifndef __SPACE_SHIP__
#define __SPACE_SHIP__

#include "Agent.h"

class SpaceShip final : public Agent
{
public:
	SpaceShip();
	~SpaceShip();

	// Inherited via GameObject
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;

	// Getters and Setters (accessors and mutators)
	float GetMaxSpeed() const;
	float GetTurnRate() const;
	float GetAccelerationRate() const;
	glm::vec2 GetDesiredVelocity() const;

	void SetMaxSpeed(float speed);
	void SetTurnRate(float angle);
	void SetAccelerationRate(float rate);
	void SetDesiredVelocity(glm::vec2 tarGet_position);

	// public functions
	void Seek();
	void LookWhereYoureGoing(glm::vec2 tarGet_direction);

private:
	// private movement variables
	float m_maxSpeed;
	float m_turnRate;
	float m_accelerationRate;
	// new for Lab 7
	std::vector<glm::vec2> m_patrolPath;
	int m_waypoint;

	// where we want to go
	glm::vec2 m_desiredVelocity;

	// private functions
	void m_move();
};


#endif /* defined (__SPACE_SHIP__) */