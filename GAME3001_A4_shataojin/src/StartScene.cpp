#include "StartScene.h"
#include <algorithm>
#include "Game.h"
#include "glm/gtx/string_cast.hpp"
#include "EventManager.h"

StartScene::StartScene()
{
	StartScene::start();
}

StartScene::~StartScene()
= default;

void StartScene::draw()
{
	drawDisplayList();
}

void StartScene::update()
{
	updateDisplayList();
}

void StartScene::clean()
{
	removeAllChildren();
}

void StartScene::handleEvents()
{
	EventManager::Instance().update();

	// Keyboard Events
	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(PLAY_SCENE);
	}
}

void StartScene::start()
{
	const SDL_Color black = { 0, 0, 0, 255 };


	// Start Button
	m_pStartButton = new Button();
	m_pStartButton->getTransform()->position = glm::vec2(400.0f, 400.0f);

	m_pStartButton->addEventListener(CLICK, [&]()-> void
		{
			m_pStartButton->setActive(false);
			TheGame::Instance()->changeSceneState(PLAY_SCENE);
		});

	m_pStartButton->addEventListener(MOUSE_OVER, [&]()->void
		{
			m_pStartButton->setAlpha(128);
		});

	m_pStartButton->addEventListener(MOUSE_OUT, [&]()->void
		{
			m_pStartButton->setAlpha(255);
		});
	addChild(m_pStartButton);

	const SDL_Color blue = { 0, 0, 255, 255 };
	m_pStartLabel = new Label("START SCENE", "Consolas", 80, blue, glm::vec2(400.0f, 40.0f));
	m_pStartLabel->setParent(this);
	addChild(m_pStartLabel);


	m_TextLabels = new Label("name:taojin sha", "Consolas", 20, blue, glm::vec2(90.0f, 560.0f));
	m_TextLabels->setParent(this);
	addChild(m_TextLabels);

	m_TextID = new Label("studentID: 101334639", "Consolas", 20, blue, glm::vec2(115.0f, 580.0f));
	m_TextID->setParent(this);
	addChild(m_TextID);

	auto offset = glm::vec2(20, 20);
}