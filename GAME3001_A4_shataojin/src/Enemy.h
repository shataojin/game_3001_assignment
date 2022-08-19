#pragma once
#ifndef __ENEMY__
#define __ENEMY__
#include "DecisionTree.h"
#include "NavigationAgent.h"
#include "Line.h"

class Enemy : public NavigationAgent
{
public:
	// constructors
	Enemy(std::string texture, std::string key);
	
	// destructor
	~Enemy();
	
	// life cycle functions
	void draw() override;
	void update() override;
	void clean() override;

	// getters and setters
	void setDestination(glm::vec2 destination);
	glm::vec2 getDestination();
	void setMaxSpeed(float speed);
	float getRotation() const;
	void setRotation(float angle);
	float getTurnRate() const;
	void setTurnRate(float rate);
	float getAccelerationRate() const;
	void setAccelerationRate(float rate);
	void setDetectionRadius(float stopR);
	float getDetectionRadius()const;
	void turnLeft();
	void setCurrentHp(int n);
	int getCurrentHp() const;

	void Test();
	std::string m_key;
	
	//CheckPoints
	bool location0 = false;
	bool location1 = false;
	bool location2 = false;
	bool location3 = false;
	bool location4 = false;
	bool AddTest ;

	bool avoidance = false;
	float AttackCd = 0;
	float CoveringTime = 0;
	int wait = 0;
	float avocd = 0;

	bool patrol = true;
	bool search = false;
	bool isHit = false;
	bool isCovering = false;
	
	bool ENEMY1X = false;
	bool ENEMY2X = false;

	bool active = false;
	
	bool move = true;
	bool moveWithAvoidance = false;
	bool flee = false;
	int moveCounterShip = 0;
	int ColObsL = 100;
	int ColObsR = 100;

	Line m_RWhishker;
	Line m_LWhishker;

	bool tLeft = false;
	bool tRight = false;
	//DecisionTree* decisionTree;
private:

	glm::vec2 m_destination;
	glm::vec2 m_targetDirection;
	glm::vec2 m_orientation;
	float m_rotationAngle;
	float m_maxSpeed;
	float m_turnRate;
	float m_accelerationRate;
	float m_detectionRadius;
	
	int currentHp;
	
	//Line m_RWhishker;
	//Line m_LWhishker;

	//DecisionTree* decisionTree;
	
	// private function
	void m_Move();
private:

	const float speed = 2.0f;
};

#endif /* defined (__OBSTACLE__) */
