#pragma once
#ifndef __TARGET__
#define __TARGET__

#include "NavigationObject.h"
#include "PlayerAnimationState.h"
#include"Sprite.h"

class Target final : public Sprite
{
public:
	Target();
	~Target() override;

	// Inherited via GameObject
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;

	void Reset();
	void SetAnimationState(PlayerAnimationState new_state);
private:
	void Move();
	void CheckBounds();
	void BuildAnimations();
	PlayerAnimationState m_currentAnimationState;
	glm::vec2 m_startPos;
};


#endif /* defined (__TARGET__) */