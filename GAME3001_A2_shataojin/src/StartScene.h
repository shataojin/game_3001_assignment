#pragma once
#ifndef __START_SCENE__
#define __START_SCENE__

#include "Scene.h"
#include "Label.h"
#include "Ship.h"
#include "Button.h"
#include "TiledLevel.h"
#include"Target.h"
#include"Megaman.h"
#include"Heuristic.h"

class StartScene final : public Scene
{
public:
	StartScene();
	~StartScene() override;

	// Inherited via Scene
	virtual void Draw() override;
	virtual void Update() override;
	virtual void Clean() override;
	virtual void HandleEvents() override;
	virtual void Start() override;
	
private:
	Label* m_pStartLabel{};
	Label* m_pInstructionsLabel{};
	Label* m_TextLabels{};
	Label* m_TextID{};
	Ship* m_pShip{};
	TiledLevel* m_pLevel;
	Button* m_pStartButton{};
	Heuristic m_currentHeristic;
	Megaman* m_pMegaman;
	Target* m_pTarget;
	void computeTileCosts();
	void shortpath();
	void allpath();
	bool HDown=false;
};

#endif /* defined (__START_SCENE__) */