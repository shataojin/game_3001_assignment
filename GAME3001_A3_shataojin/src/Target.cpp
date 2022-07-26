#include "Target.h"
#include "TextureManager.h"
#include"EventManager.h"

Target::Target(): m_currentAnimationState(PlayerAnimationState::PLAYER_IDLE_RIGHT)
{
	TextureManager::Instance().LoadSpriteSheet(
		"../Assets/sprites/atlas.txt",
		"../Assets/sprites/24242424.png",
		"spritesheet");

	SetSpriteSheet(TextureManager::Instance().GetSpriteSheet("spritesheet"));

	// set frame width
	SetWidth(53);

	// set frame height
	SetHeight(58);

	GetTransform()->position = glm::vec2(400.0f, 300.0f);
	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	SetType(GameObjectType::TARGET);
	BuildAnimations();
}

Target::~Target()
= default;

void Target::Draw()
{
	// draw the player according to animation state
	switch (m_currentAnimationState)
	{
	case PlayerAnimationState::PLAYER_IDLE_RIGHT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("idle"),
			GetTransform()->position, 0.12f, 0, 255, true);
		break;
	case PlayerAnimationState::PLAYER_IDLE_LEFT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("idle"),
			GetTransform()->position, 0.12f, 0, 255, true, SDL_FLIP_HORIZONTAL);
		break;
	case PlayerAnimationState::PLAYER_RUN_RIGHT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("run"),
			GetTransform()->position, 0.25f, 0, 255, true);
		break;
	case PlayerAnimationState::PLAYER_RUN_LEFT:
		TextureManager::Instance().PlayAnimation("spritesheet", GetAnimation("run"),
			GetTransform()->position, 0.25f, 0, 255, true, SDL_FLIP_HORIZONTAL);
		break;
	default:
		break;
	}
}

void Target::Update()
{
	Move();
	CheckBounds();
}

void Target::Clean()
{
}

void Target::SetAnimationState(const PlayerAnimationState new_state)
{
	m_currentAnimationState = new_state;
}

void Target::BuildAnimations()
{
	auto idle_animation = Animation();

	idle_animation.name = "idle";
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-0"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-1"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-2"));
	idle_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-idle-3"));

	SetAnimation(idle_animation);

	auto run_animation = Animation();

	run_animation.name = "run";
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-0"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-1"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-2"));
	run_animation.frames.push_back(GetSpriteSheet()->GetFrame("megaman-run-3"));

	SetAnimation(run_animation);
}


void Target::Move()
{
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_A))
	{
		SetAnimationState(PlayerAnimationState::PLAYER_RUN_LEFT);
		GetTransform()->position = GetTransform()->position + glm::vec2(-player_speed, 0.0f);
	}
	else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_D))
	{
		SetAnimationState(PlayerAnimationState::PLAYER_RUN_RIGHT);
		GetTransform()->position = GetTransform()->position + glm::vec2(player_speed, 0.0f);
	}
	
	if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_W))
	{
		
		GetTransform()->position = GetTransform()->position + glm::vec2(0.0f, -player_speed);

		/*if (m_playerFacingRight)
		{
			m_pTarget->SetAnimationState(PlayerAnimationState::PLAYER_RUN_RIGHT);
		}
		else
		{
			m_pTarget->SetAnimationState(PlayerAnimationState::PLAYER_RUN_LEFT);
		}*/
	}
	else if (EventManager::Instance().IsKeyDown(SDL_SCANCODE_S))
	{
		
		GetTransform()->position = GetTransform()->position + glm::vec2(0.0f, player_speed);

	/*	if (m_playerFacingRight)
		{
			m_pTarget->SetAnimationState(PlayerAnimationState::PLAYER_RUN_RIGHT);
		}
		else
		{
			m_pTarget->SetAnimationState(PlayerAnimationState::PLAYER_RUN_LEFT);
		}*/
	}
}

void Target::CheckBounds()
{

}

void Target::Reset()
{
	GetTransform()->position = m_startPos;
}
