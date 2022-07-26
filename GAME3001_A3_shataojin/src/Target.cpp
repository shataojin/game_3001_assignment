#include "Target.h"
#include "TextureManager.h"
#include"EventManager.h"

Target::Target() : m_startPos( glm::vec2(600.0f, 100.0f) )
{
	TextureManager::Instance().Load("../Assets/textures/Circle.png","circle");

	const auto size = TextureManager::Instance().GetTextureSize("circle");
	SetWidth(static_cast<int>(size.x));
	SetHeight(static_cast<int>(size.y));
	GetTransform()->position = m_startPos;
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	SetType(GameObjectType::TARGET);
}

Target::~Target()
= default;

void Target::Draw()
{
	// draw the target
	TextureManager::Instance().Draw("circle", GetTransform()->position, 0, 255, true);
}

void Target::Update()
{
	Move();
	CheckBounds();
}

void Target::Clean()
{
}

void Target::Move()
{
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
	{
		GetTransform()->position.x = GetTransform()->position.x - 5.0f;
	}
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
	{
		GetTransform()->position.x = GetTransform()->position.x +  5.0f;
	}
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_W))
	{
		GetTransform()->position.y = GetTransform()->position.y -  5.0f;
	}
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_S))
	{
		GetTransform()->position.y = GetTransform()->position.y +  5.0f;
	}
}

void Target::CheckBounds()
{

}

void Target::Reset()
{
	GetTransform()->position = m_startPos;
}
