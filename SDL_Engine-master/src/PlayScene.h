#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Scene.h"
#include "Target.h"
#include "Starship.h"
#include"Obstacle.h"
#include"seeking.h"
#include"Fleeing .h"
#include"TargetOutlCircle.h"
#include"Arrival.h"
#include "Label.h"

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene() override;

	// Scene LifeCycle Functions
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;
	virtual void HandleEvents() override;
	virtual void Start() override;
private:
	// IMGUI Function
	void GUI_Function();
	std::string m_guiTitle;
	
	// Lab 2 Game Objects
	Target* m_pTarget;
	TargetOutlCircle* m_Ptargetoutcaircle;
	Starship* m_pStarship;
	Obstacle* m_pObstacle;
	Seeking* m_pSeeking;
	Fleeing* m_pFleeing;
	Arrival* m_pArrival;
	Label* m_pInstructionsLabel{};
	Label* m_pInstructions{};

	// Input Control
	int m_pCurrentInputType{};
	void GetPlayerInput();
	void GetKeyboardInput();
	void DoWhiskerCollision();
};

#endif /* defined (__PLAY_SCENE__) */