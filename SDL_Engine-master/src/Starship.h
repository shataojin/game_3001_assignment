#pragma once
#ifndef __Starship__
#define __Starship__

#include "Agent.h"

class Starship final : public Agent {
public:
	Starship();
	~Starship() override;

	// Inherited via GameObject
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;

	float getMaxSpeed() const;
	float getTurnRate() const;
	glm::vec2 getDesiredVelocity() const;
	float getAccelerationRate() const;

	void setMaxSpeed(float speed);
	void setTurnRate(float angle);
	void setDesiredVelocity(glm::vec2 target_position);
	void setAccelerationRate(float rate);

	void Reset();
	void LookWhereIAmGoing(glm::vec2 target_direction);
		void Seek();
private:
	void Move();
	void CheckBounds();
	float m_maxSpeed;
	float m_turnRate;
	glm::vec2 m_desiredVelocity;
	float m_accelerationRate;

	glm::vec2 m_startPos;

};


#endif /* defined (__Starship__) */