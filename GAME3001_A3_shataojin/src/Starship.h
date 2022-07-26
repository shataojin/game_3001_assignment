#pragma once
#ifndef __STARSHIP__
#define __STARSHIP__

#include "Agent.h"
#include "PlayerAnimationState.h"
class Starship final : public Agent
{
public:
	Starship();
	~Starship() override;

	// Inherited via GameObject
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;

	// Getters and Setters
	float GetMaxSpeed() const;
	float GetTurnRate() const;
	glm::vec2 GetDesiredVelocity() const;
	float GetAccelerationRate() const;

	void SetMaxSpeed(float speed);
	void SetTurnRate(float angle);
	void SetDesiredVelocity(glm::vec2 target_position);
	void SetAccelerationRate(float rate);

	// Others
	void Seek();
	void LookWhereIAmGoing(glm::vec2 target_direction);
	void Reset();
	void SetAnimationState(PlayerAnimationState new_state);
	bool line = false;
	int hp = 10;
private:
	float m_maxSpeed;
	float m_turnRate;
	glm::vec2 m_desiredVelocity;
	float m_accelerationRate;
	glm::vec2 m_startPos;

	void Move();
	void CheckBounds();

	std::vector<glm::vec2> m_patrolPath;
	int m_waypoint;
	PlayerAnimationState m_currentAnimationState;

	void BuildAnimations();
	bool move = false;
};


#endif /* defined (__STARSHIP__) */