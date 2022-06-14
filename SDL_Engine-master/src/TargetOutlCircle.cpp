#include "TargetOutlCircle.h"
#include "TextureManager.h"


TargetOutlCircle::TargetOutlCircle():m_startPos(glm::vec2(100.0f, 100.0f))
{
	TextureManager::Instance().Load("../Assets/textures/1.png","circlex");

	const auto size = TextureManager::Instance().GetTextureSize("circlex");
	SetWidth(static_cast<int>(size.x));
	SetHeight(static_cast<int>(size.y));
	GetTransform()->position = m_startPos;
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	SetType(GameObjectType::TARGETOUT);
}

TargetOutlCircle::~TargetOutlCircle()
= default;

void TargetOutlCircle::Draw()
{
	
	TextureManager::Instance().Draw("circlex", GetTransform()->position, 0, 255, true);
}

void TargetOutlCircle::Update()
{
	Move();
	CheckBounds();
}

void TargetOutlCircle::Clean()
{
}

void TargetOutlCircle::Move()
{
	GetTransform()->position = GetTransform()->position + GetRigidBody()->velocity * 5.0f;
}

void TargetOutlCircle::CheckBounds()
{

}

void TargetOutlCircle::Reset()
{
	GetTransform()->position = m_startPos;
}
