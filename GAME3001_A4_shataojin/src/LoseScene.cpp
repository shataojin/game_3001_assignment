#include "LoseScene.h"
#include <algorithm>
#include "Game.h"
#include "glm/gtx/string_cast.hpp"
#include "EventManager.h"

LoseScene::LoseScene()
{
	LoseScene::start();
	SoundManager::Instance().load("../Assets/audio/Bgm_2.mp3", "Bgm", SOUND_MUSIC);
	SoundManager::Instance().playMusic("Bgm", -1, 0);

}

LoseScene::~LoseScene()
= default;

void LoseScene::draw()
{
	TextureManager::Instance()->draw("backgroundLose", 400, 300, 0, 255, true);

	drawDisplayList();
}

void LoseScene::update()
{
	updateDisplayList();
}

void LoseScene::clean()
{
	removeAllChildren();
}

void LoseScene::handleEvents()
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

void LoseScene::start()
{
	TextureManager::Instance()->load("../Assets/textures/x.jpg", "backgroundLose");

	m_pStartButton = new Button("../Assets/textures/restartButton.png", "restart");
	m_pStartButton->getTransform()->position = glm::vec2(200.0f, 300.0f);

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

	m_pExitButton->addEventListener(CLICK, [&]()-> void
		{
			m_pExitButton->setActive(false);
			TheGame::Instance()->changeSceneState(START_SCENE);
		});

	m_pExitButton->addEventListener(MOUSE_OVER, [&]()->void
		{
			m_pExitButton->setAlpha(128);
		});

	m_pExitButton->addEventListener(MOUSE_OUT, [&]()->void
		{
			m_pExitButton->setAlpha(255);
		});
	addChild(m_pExitButton);


}
