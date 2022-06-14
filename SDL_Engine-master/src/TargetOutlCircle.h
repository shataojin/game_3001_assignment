#pragma once
#ifndef __TargetOutlCircle__
#define __TargetOutlCircle__

#include "DisplayObject.h"

class TargetOutlCircle final : public DisplayObject {
public:
	TargetOutlCircle();
	~TargetOutlCircle() override;

	// Inherited via GameObject
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;
	void Reset();
private:
	void Move();
	void CheckBounds();
	glm::vec2 m_startPos;
};


#endif /* defined (__TargetOutlCircle__) */