#include "StartScene.h"
#include <algorithm>
#include "Game.h"
#include "glm/gtx/string_cast.hpp"
#include "EventManager.h"
#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"
#include "InputType.h"
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"
#include"PathNode.h"
#include"PathManager.h"
#include <iostream>
#include <sstream>
#include"PathManager.h"

StartScene::StartScene()
{
	StartScene::Start();
}

StartScene::~StartScene()
= default;

void StartScene::Draw()
{
	DrawDisplayList();
}

void StartScene::Update()
{
	UpdateDisplayList();
}

void StartScene::Clean()
{
	RemoveAllChildren();
}

void StartScene::HandleEvents()
{
	EventManager::Instance().Update();

	// Keyboard Events
	if(EventManager::Instance().IsKeyDown(SDL_SCANCODE_ESCAPE))
	{
		Game::Instance().Quit();
	}

	if(EventManager::Instance().IsKeyDown(SDL_SCANCODE_1))
	{
		Game::Instance().ChangeSceneState(SceneState::PLAY);
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_H))
	{
		m_pLevel->SetLabelsEnabled(true);
		computeTileCosts();
		/*shortpath();*/
		HDown = true;
	}

	if ( EventManager::Instance().MousePressed(1))
	{
		if (HDown == true)
		{
			auto offset = glm::vec2(20, 20);
			m_pTarget->GetTransform()->position = m_pLevel->GetTile(trunc(abs((EventManager::Instance().GetMousePosition().x) / 40)),
				trunc(abs((EventManager::Instance().GetMousePosition().y) / 40)))->GetTransform()->position + offset;
			m_pTarget->SetGridPosition(trunc(abs((EventManager::Instance().GetMousePosition().x) / 40)),
				trunc(abs((EventManager::Instance().GetMousePosition().y) / 40)));
			shortpath();
			//testing reader
			std::cout << EventManager::Instance().GetMousePosition().x << std::endl;
			std::cout << EventManager::Instance().GetMousePosition().y << std::endl;
		std::cout << "mouse x :"<<trunc(abs((EventManager::Instance().GetMousePosition().x) / 40)) << std::endl;
		std::cout << "mouse y :" << trunc(abs((EventManager::Instance().GetMousePosition().x) / 40)) << std::endl;
		//

		}
	}

	if (EventManager::Instance().MousePressed(3))
	{
		if (HDown == true)
		{
			auto offset = glm::vec2(20, 20);
			m_pMegaman->GetTransform()->position = m_pLevel->GetTile(trunc(abs((EventManager::Instance().GetMousePosition().x) / 40))
				, trunc(abs((EventManager::Instance().GetMousePosition().y) / 40)))->GetTransform()->position + offset;
			m_pMegaman->SetGridPosition(trunc(abs((EventManager::Instance().GetMousePosition().x) / 40))
				, trunc(abs((EventManager::Instance().GetMousePosition().y) / 40)));
			shortpath();
		}
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_F))
	{
		if (HDown == true)
		{
			for (int i = 0; i <= 100; i++)
			{
				allpath();
				if (i == 100)
				{
					shortpath();
				}
			}
			
		}
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_M))
	{

	}



	//test
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_T))
	{
		allpath();
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_Y))
	{
		shortpath();
	}
	//
}

void StartScene::Start()
{

	m_pLevel = new TiledLevel("../Assets/data/starlevel.txt", "../Assets/data/leveldata.txt",
		"../Assets/textures/Tiles.png", "tiles", { 32,32 }, { 40,40 }, 15, 20, true, true);
	AddChild(m_pLevel);

	const SDL_Color blue = { 0, 0, 255, 255 };
	m_pStartLabel = new Label("START SCENE", "Consolas", 60, blue, glm::vec2(400.0f, 40.0f));
	m_pStartLabel->SetParent(this);
	AddChild(m_pStartLabel);

	m_pInstructionsLabel = new Label("press H for debug view", "Consolas", 17, blue, glm::vec2(400.0f, 500.0f));
	m_pInstructionsLabel->SetParent(this);
	AddChild(m_pInstructionsLabel);

	//
	m_TextLabels = new Label("name:taojin sha", "Consolas", 20, blue, glm::vec2(90.0f, 560.0f));
	m_TextLabels->SetParent(this);
	AddChild(m_TextLabels);

	m_TextID = new Label("studentID: 101334639", "Consolas", 20, blue, glm::vec2(115.0f, 580.0f));
	m_TextID->SetParent(this);
	AddChild(m_TextID);

	auto offset = glm::vec2(20, 20);

	m_pTarget = new Target();
	m_pTarget->GetTransform()->position = m_pLevel->GetTile(15, 11)->GetTransform()->position + offset;
	m_pTarget->SetGridPosition(15.0f, 11.0f);
	AddChild(m_pTarget);

	m_pMegaman = new Megaman();
	m_pMegaman->GetTransform()->position = m_pLevel->GetTile(1, 3)->GetTransform()->position + offset;
	m_pMegaman->SetGridPosition(1.0f, 3.0f);
	AddChild(m_pMegaman);

	

	//
	// Start Button

	// Start Button
	m_pStartButton = new Button();
	m_pStartButton->GetTransform()->position = glm::vec2(400.0f, 400.0f); 

	m_pStartButton->AddEventListener(Event::CLICK, [&]()-> void
	{
		m_pStartButton->SetActive(false);
		Game::Instance().ChangeSceneState(SceneState::PLAY);
	});
	
	m_pStartButton->AddEventListener(Event::MOUSE_OVER, [&]()->void
	{
		m_pStartButton->SetAlpha(128);
	});

	m_pStartButton->AddEventListener(Event::MOUSE_OUT, [&]()->void
	{
		m_pStartButton->SetAlpha(255);
	});
	AddChild(m_pStartButton);

	ImGuiWindowFrame::Instance().SetDefaultGuiFunction();
}



void StartScene::computeTileCosts()
{
	float distance = 0.0f;
	float dx, dy;
	dx = dy = 0.0f;
	for (auto tile : m_pLevel->GetLevel())
	{
		if (tile->GetTileType() == TileType::IMPASSABLE)continue;
		switch (m_currentHeristic)
		{
		case Heuristic::MANHATTAN:
			dx = abs(tile->GetGridPosition().x - m_pTarget->GetGridPosition().x);
			dy = abs(tile->GetGridPosition().y - m_pTarget->GetGridPosition().y);
			distance = dx + dy;
			break;
		case Heuristic::EUCLIDEAN:
			distance = Util::Distance(tile->GetGridPosition(), m_pTarget->GetGridPosition());
			break;
		}
		tile->SetTileCost(distance);
		std::cout << "(" << tile->GetGridPosition().x << ", " << tile->GetGridPosition().y << ")" << std::endl;


	}
}


void StartScene::shortpath()
{
	PathNode* startNode = m_pLevel->GetTile(m_pMegaman->GetGridPosition())->GetNode();
	PathNode* goalNode = m_pLevel->GetTile(m_pTarget->GetGridPosition())->GetNode();
	if (startNode != nullptr && goalNode != nullptr)
	{
		PathManager::GetShortestPath(startNode, goalNode);

	}
	else
	{
		std::cout << "cant get shortest path. one or more nodes are null" << std::endl;
	}
}


void StartScene::allpath()
{
	PathNode* startNode = m_pLevel->GetTile(m_pMegaman->GetGridPosition())->GetNode();
	PathNode* goalNode = m_pLevel->GetTile(m_pTarget->GetGridPosition())->GetNode();
	if (startNode != nullptr && goalNode != nullptr)
	{
		PathManager::GetAllPath(startNode, goalNode);

	}
	else
	{
		std::cout << "cant get shortest path. one or more nodes are null" << std::endl;
	}
}



