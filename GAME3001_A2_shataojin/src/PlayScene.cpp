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


PlayScene::PlayScene()
{
	PlayScene::Start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::Draw()
{
	DrawDisplayList();
	SDL_SetRenderDrawColor(Renderer::Instance().GetRenderer(), 255, 255, 255, 255);
}

void PlayScene::Update()
{
	UpdateDisplayList();
}

void PlayScene::Clean()
{
	RemoveAllChildren();
}


void PlayScene::HandleEvents()
{
	EventManager::Instance().Update();

	GetPlayerInput();

	GetKeyboardInput();
}

void PlayScene::GetPlayerInput()
{
	switch (m_pCurrentInputType)
	{
	case static_cast<int>(InputType::GAME_CONTROLLER):
	{
		// handle player movement with GameController
		if (SDL_NumJoysticks() > 0)
		{
			if (EventManager::Instance().GetGameController(0) != nullptr)
			{
				constexpr auto dead_zone = 10000;
				if (EventManager::Instance().GetGameController(0)->STICK_LEFT_HORIZONTAL > dead_zone)
				{
					
				}
				else if (EventManager::Instance().GetGameController(0)->STICK_LEFT_HORIZONTAL < -dead_zone)
				{
					
				}
				else
				{
					
				}
			}
		}
	}
	break;
	case static_cast<int>(InputType::KEYBOARD_MOUSE):
	{
		// handle player movement with mouse and keyboard
		if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
		{
			
		}
		else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
		{
			
		}
		else
		{
			
		}
	}
	break;
	case static_cast<int>(InputType::ALL):
	{
		if (SDL_NumJoysticks() > 0)
		{
			if (EventManager::Instance().GetGameController(0) != nullptr)
			{
				constexpr auto dead_zone = 10000;
				if (EventManager::Instance().GetGameController(0)->STICK_LEFT_HORIZONTAL > dead_zone
					|| EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
				{
					
				}
				else if (EventManager::Instance().GetGameController(0)->STICK_LEFT_HORIZONTAL < -dead_zone
					|| EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
				{
					
				}
				else
				{
					
				}
			}
		}
		else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
		{
			
		}
		else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
		{
			
		}
		else
		{
			
		}
	}
	break;
	}
}

void PlayScene::GetKeyboardInput()
{
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_ESCAPE))
	{
		Game::Instance().Quit();
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_1))
	{
		Game::Instance().ChangeSceneState(SceneState::START);
	}

	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_2))
	{
		Game::Instance().ChangeSceneState(SceneState::END);
	}
}

void PlayScene::computeTileCosts()
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
		/*tilecost++;
		std::cout << tilecost << std::endl;*/
		std::cout<<distance<<std::endl;
		/* std::cout<< abs(tile->GetGridPosition().x) << " , " << abs(tile->GetGridPosition().y) << std::endl;*/
		std::cout<< dx <<" , "<< dy << std::endl;
	}
}

void PlayScene::Start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";

	// Set Input Type
	m_pCurrentInputType = static_cast<int>(InputType::KEYBOARD_MOUSE);

	// Create GameObjects
	m_pLevel = new TiledLevel("../Assets/data/level.txt", "../Assets/data/leveldata.txt",
		"../Assets/textures/Tiles.png", "tiles", { 32,32 }, { 40,40 }, 15, 20, true,true);
	AddChild(m_pLevel);

	auto offset = glm::vec2(20, 20);

	m_pTarget = new Target();
	m_pTarget->GetTransform()->position = m_pLevel->GetTile(15, 11)->GetTransform()->position + offset;
	m_pTarget->SetGridPosition(15.0f, 11.0f);
	AddChild(m_pTarget);

	m_pMegaman = new Megaman();
	m_pMegaman->GetTransform()->position = m_pLevel->GetTile(1, 3)->GetTransform()->position + offset;
	m_pMegaman->SetGridPosition(1.0f, 3.0f);
	AddChild(m_pMegaman);

	/* DO NOT REMOVE */
	ImGuiWindowFrame::Instance().SetGuiFunction([this] { GUI_Function(); });
}



void PlayScene::GUI_Function() 
{
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin(m_guiTitle.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

	ImGui::Text("Player Input");
	ImGui::RadioButton("Keyboard / Mouse", &m_pCurrentInputType, static_cast<int>(InputType::KEYBOARD_MOUSE)); ImGui::SameLine();
	ImGui::RadioButton("Game Controller", &m_pCurrentInputType, static_cast<int>(InputType::GAME_CONTROLLER)); ImGui::SameLine();
	ImGui::RadioButton("Both", &m_pCurrentInputType, static_cast<int>(InputType::ALL));

	ImGui::Separator();

	static bool toggle_grid = false;
	if (ImGui::Checkbox("Toggle Grid", &toggle_grid))
	{
		m_pLevel->SetLabelsEnabled(toggle_grid);
	}

	ImGui::Separator();

	auto offset = glm::vec2(20, 20);

	static int start_position[2] = { (int)m_pMegaman->GetGridPosition().x,(int)m_pMegaman->GetGridPosition().y };
	if (ImGui::SliderInt2("Start Position", start_position, 0, 19))
	{
		if (start_position[1] > 14) start_position[1] = 14;

		m_pMegaman->GetTransform()->position = m_pLevel->GetTile(start_position[0],
			start_position[1])->GetTransform()->position + offset;
		m_pMegaman->SetGridPosition(start_position[0], start_position[1]);
	}

	static int goal_position[2] = { (int)m_pTarget->GetGridPosition().x,(int)m_pTarget->GetGridPosition().y };

	if (ImGui::SliderInt2("goal Position", goal_position, 0, 19))
	{
		if (goal_position[1] > 14) goal_position[1] = 14;

		m_pTarget->GetTransform()->position = m_pLevel->GetTile(goal_position[0],
			goal_position[1])->GetTransform()->position + offset;
		m_pTarget->SetGridPosition(goal_position[0], goal_position[1]);
	}

	ImGui::Separator();
	static int radio = static_cast<int>(m_currentHeristic);
	ImGui::Text("heuristic type");
	ImGui::RadioButton("manhattan", &radio, static_cast<int>(Heuristic::MANHATTAN));
	ImGui::SameLine();
	ImGui::RadioButton("euclidean", &radio, static_cast<int>(Heuristic::EUCLIDEAN));

	if (ImGui::Button("computer tile costs", { 208,20 }))
	{
		computeTileCosts();
	}

	ImGui::Separator();

	if (ImGui::Button("get shortest path", { 208,20 })&& m_pLevel->HasNavigation())
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

	ImGui::SameLine();

	if (ImGui::Button("clear path", { 104,20 })&&m_pLevel->HasNavigation())
	{
		PathManager::ClearPath();
		for (const auto tile : m_pLevel->GetLevel())
		{
			if (tile->GetTileType() == TileType::IMPASSABLE)continue;
			tile->SetTileStatus(TileStatus::UNVISITED);
		}
	}
	ImGui::End();
}


//1，显示COST上的总和或每一步？
//2，增加柔顺移动 lookingwhereiamgoing在坦克里应该有对应
//3，在start 界面增加操作（查询A*在C++中作用）
//4,增加背景音乐

