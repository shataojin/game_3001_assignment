#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Scene.h"
#include "TiledLevel.h"
#include"Target.h"
#include"Megaman.h"
#include"Heuristic.h"
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

	// Lab 4 GameObjects
	TiledLevel* m_pLevel;
	Megaman* m_pMegaman;
	Target* m_pTarget;

	// Input Control
	int m_pCurrentInputType{};
	void GetPlayerInput();
	void GetKeyboardInput();

	void computeTileCosts();

	int tilecost = 0;
	Heuristic m_currentHeristic;


	void m_displayPathList();
};

#endif /* defined (__PLAY_SCENE__) */