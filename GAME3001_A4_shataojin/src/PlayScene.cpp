#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"
#include"DecisionTree.h"


// required for IMGUI
#include "CloseCombatAttack.h"
#include "Flee.h"
#include "imgui.h"
#include "imgui_sdl.h"
#include "MoveBehindCover.h"
#include "MoveToLOS.h"
#include "MoveToPlayer.h"
#include "Patrol.h"
#include "RangedAttack.h"
#include "Renderer.h"
#include "Transition.h"
#include "Util.h"
#include "WaitBehindCover.h"

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{

	if (EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();
	}

	drawDisplayList();
	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{	
	auto deltaTime = TheGame::Instance()->getDeltaTime();
	updateDisplayList();

	//LOS,CloseCombatRange
	for (int i = 0; i < Enemies; i++)
	{
		if(m_pEnemy[i]->isEnabled())
			m_CheckAgentLOS(m_pEnemy[i],m_pPlayer);
		if (m_pEnemyDebug[i]->isEnabled())
			m_CheckAgentLOS(m_pEnemyDebug[i],m_pPlayer);
		if (m_pEnemyDebug[i]->isEnabled())
			m_CheckCloseCombat(m_pEnemyDebug[i], m_pPlayer);
		if (m_pEnemyDebug[i]->isEnabled())
			m_CheckRangedCombat(m_pEnemyDebug[i], m_pPlayer);
		
		m_CheckCloseCombat(m_pPlayer,m_pEnemy[i]);
	}
	//CloseCombatRange DestObj
	for (int i = 0; i < damgobj; i++)
	{
		m_CheckCloseCombat(m_pPlayer,m_dDamgobj[i]);
	}
	//TilesLos
	m_CheckPathNodeLOS();

	//State Machine Conditions and Updates
	for (int i = 0; i < Enemies; i++)
	{
		////StateMachineCloseCombat1
		if(i==0)
		{
			// Set conditions
			m_pCCHasLOSCondition->SetCondition(m_pEnemy[i]->hasLOS());
			m_pCCLostLOSCondition->SetCondition(!m_pEnemy[i]->hasLOS());
			m_pCCIsWithinDetectionRadiusCondition->SetCondition(
			Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) <= m_pEnemy[i]->getDetectionRadius()
			);
			m_pCCIsNotWithinDetectionRadiusCondition->SetCondition(
				Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) > m_pEnemy[i]->getDetectionRadius()
			);
			m_pCCIsWithinCombatRangeCondition->SetCondition(
				Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) <= m_pEnemy[i]->getCloseCombatDistance()
			);
			m_pCCLifeIsLow->SetCondition(m_pEnemy[i]->getCurrentHp() == 1);
			m_pCCNotWithinCombatRangeCondition->SetCondition(Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) > m_pEnemy[i]->getCloseCombatDistance());
			m_pCCStateMachine->Update();
		}
		////StateMachineRanged1
		else if(i==1)
		{
			// Set conditions
			m_pRCHasLOSCondition->SetCondition(m_pEnemy[i]->hasLOS());
			m_pRCLostLOSCondition->SetCondition(!m_pEnemy[i]->hasLOS());
			m_pRCIsWithinDetectionRadiusCondition->SetCondition(
				Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) <= m_pEnemy[i]->getDetectionRadius()
			);
			m_pRCIsNotWithinDetectionRadiusCondition->SetCondition(
				Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) > m_pEnemy[i]->getDetectionRadius()
			);
			m_pRCIsWithinCombatRangeCondition->SetCondition(
				Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) <= m_pEnemy[i]->getRangedCombatDistance()
			);
			m_pRCLifeIsLow->SetCondition(m_pEnemy[i]->getCurrentHp() == 1);

			m_pRCNotWithinCombatRangeCondition->SetCondition(Util::distance(m_pEnemy[i]->getTransform()->position, m_pPlayer->getTransform()->position) > m_pEnemy[i]->getRangedCombatDistance());
			m_pRCIsHit->SetCondition(m_pEnemy[i]->isHit == true);
			m_pRCCoverWait->SetCondition(m_pEnemy[i]->isCovering == true);
			m_pRCCoverOut->SetCondition(m_pEnemy[i]->isCovering == false);
			m_pRCStateMachine->Update();
		}
	}
	
	//Enemy movevents
	m_move();

	//Timer for Cooldowns
	{
		GameTimer += 1 * deltaTime;
		ButtonCD += 1 * deltaTime;
		CheckCD += 1 * deltaTime;
		ShootCD += 1 * deltaTime;
		RespawnCD += 1 * deltaTime;
		
		m_pEnemy[0]->AttackCd += 1 * deltaTime;
		
		m_pEnemy[1]->AttackCd += 1 * deltaTime;
		m_pEnemy[1]->CoveringTime += 1 * deltaTime;
	}

	for (auto i = 0; i < Enemies; i++)
	{
		//m_pEnemy[i]->cd += 1 * deltaTime;
		m_pEnemy[i]->avocd += 1 * deltaTime;
	}

	//Enemies Debug bind
	for (int i = 0; i < Enemies; i++)
	{
		m_pEnemyDebug[i]->getTransform()->position = m_pEnemy[i]->getTransform()->position;
		m_pEnemyDebug[i]->setDestination(m_pEnemy[i]->getDestination());
	}


	//Health Bind
	{
		// Enemy0
		Enemy0[0]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x,m_pEnemy[0]->getTransform()->position.y - 40 };
		Enemy0[1]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x + 10,m_pEnemy[0]->getTransform()->position.y - 40 };
		Enemy0[2]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x - 10,m_pEnemy[0]->getTransform()->position.y - 40 };
		Enemy0[3]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x - 20,m_pEnemy[0]->getTransform()->position.y - 40 };

		// Enemy1
		Enemy1[0]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x,m_pEnemy[1]->getTransform()->position.y - 40 };
		Enemy1[1]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x + 10,m_pEnemy[1]->getTransform()->position.y - 40 };
		Enemy1[2]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x - 10,m_pEnemy[1]->getTransform()->position.y - 40 };
		Enemy1[3]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x - 20,m_pEnemy[1]->getTransform()->position.y - 40 };

		//Player Health bind
		PlayerHp[0]->getTransform()->position = { m_pPlayer->getTransform()->position.x,m_pPlayer->getTransform()->position.y - 40 };
		PlayerHp[1]->getTransform()->position = { m_pPlayer->getTransform()->position.x + 10,m_pPlayer->getTransform()->position.y - 40 };
		PlayerHp[2]->getTransform()->position = { m_pPlayer->getTransform()->position.x - 10,m_pPlayer->getTransform()->position.y - 40 };
		PlayerHp[3]->getTransform()->position = { m_pPlayer->getTransform()->position.x - 20,m_pPlayer->getTransform()->position.y - 40 };
		PlayerHp[4]->getTransform()->position = { m_pPlayer->getTransform()->position.x + 20,m_pPlayer->getTransform()->position.y - 40 };
	}
	
	//Set Player destiantion
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	m_pPlayer->setDestination(glm::vec2(mx, my));

	//Player Bullet Off Screen
	for (int i = 0; i < m_pBullet.size(); i++)
	{
		if (m_pBullet[i]->getTransform()->position.x >= 800.0f ||
			m_pBullet[i]->getTransform()->position.x <= 0.0f ||
			m_pBullet[i]->getTransform()->position.y >= 600.0f ||
			m_pBullet[i]->getTransform()->position.y <= 0)
		{
			m_pBullet[i]->setEnabled(false);
			//AddChild(m_pBullet[i].back(), 2);
			break;
		}
	}

	//Collisions

	//Player and stage Collision
	if(m_pPlayer->isEnabled())
	{
		for(int i=0;i<obstacles;i++)
		{
			if (CollisionManager::CircleAABBCheck(m_pPlayer,m_field[i]))
			{
				float x = m_field[i]->getTransform()->position.x - m_pPlayer->getTransform()->position.x;// right 
				float x2 = m_pPlayer->getTransform()->position.x - m_field[i]->getTransform()->position.x;// left
				float y = m_field[i]->getTransform()->position.y - m_pPlayer->getTransform()->position.y;// down
				float y2 = m_pPlayer->getTransform()->position.y - m_field[i]->getTransform()->position.y;// up
				
				//RightCollision
				if((x>x2)&&(x>y)&&(x>y2))
					m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x - 3.0f;
				//LeftCollision
				else if ((x2 > x) && (x2 > y) && (x2 > y2))
					m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x + 3.0f;
				//DownCollision
				else if ((y > x) && (y >x2 ) && (y > y2))
					m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y - 3.0f;
				//UpCollision
				else if ((y2 > x) && (y2 > x2) && (y2 > y))
					m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y + 3.0f;
			}
		}
		for (int i = 0; i < damgobj; i++)
		{
			if (m_dDamgobj[i]->isEnabled())
			{
				if (CollisionManager::CircleAABBCheck(m_pPlayer, m_dDamgobj[i]))
				{
					float x = m_dDamgobj[i]->getTransform()->position.x - m_pPlayer->getTransform()->position.x;// right
					float x2 = m_pPlayer->getTransform()->position.x - m_dDamgobj[i]->getTransform()->position.x;// left
					float y = m_dDamgobj[i]->getTransform()->position.y - m_pPlayer->getTransform()->position.y;// down
					float y2 = m_pPlayer->getTransform()->position.y - m_dDamgobj[i]->getTransform()->position.y;// up

					//RightCollision
					if ((x > x2) && (x > y) && (x > y2))
						m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x - 5.0f;
					//LeftCollision
					else if ((x2 > x) && (x2 > y) && (x2 > y2))
						m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x + 5.0f;
					//DownCollision
					else if ((y > x) && (y> x2) && (y> y2))
						m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y - 5.0f;
					//UpCollision
					else if ((y2 > x) && (y2 > x2) && (y2 > y))
						m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y + 5.0f;
				}
			}
		}
	}

	//Enemy and stage Collision
	for (int t = 0; t < Enemies;t++)
	{
		if (m_pPlayer->isEnabled())
		{
			for (int i = 0; i < obstacles; i++)
			{
				if (CollisionManager::CircleAABBCheck(m_pEnemy[t], m_field[i]))
				{
					float x = m_field[i]->getTransform()->position.x - m_pEnemy[t]->getTransform()->position.x;//right
					float x2 = m_pEnemy[t]->getTransform()->position.x - m_field[i]->getTransform()->position.x;// left
					float y = m_field[i]->getTransform()->position.y - m_pEnemy[t]->getTransform()->position.y;//down
					float y2 = m_pEnemy[t]->getTransform()->position.y - m_field[i]->getTransform()->position.y;// up

					//RightCollision
					if ((x > x2) && (x > y) && (x > y2))
						m_pEnemy[t]->getTransform()->position.x = m_pEnemy[t]->getTransform()->position.x - 5.0f;
					//LeftCollision
					else if ((x2 > x) && (x2 > y) && (x2 > y2))
						m_pEnemy[t]->getTransform()->position.x = m_pEnemy[t]->getTransform()->position.x + 5.0f;
					//DownCollision
					else if ((y > x) && (y > x2) && (y > y2))
						m_pEnemy[t]->getTransform()->position.y = m_pEnemy[t]->getTransform()->position.y - 5.0f;
					//UpCollision
					else if ((y2 > x) && (y2 > x2) && (y2 > y))
						m_pEnemy[t]->getTransform()->position.y = m_pEnemy[t]->getTransform()->position.y + 5.0f;
				}
			}
			for (int i = 0; i < damgobj; i++)
			{
				if (m_dDamgobj[i]->isEnabled())
				{
					if (CollisionManager::CircleAABBCheck(m_pEnemy[t], m_dDamgobj[i]))
					{
						float x = m_dDamgobj[i]->getTransform()->position.x - m_pEnemy[t]->getTransform()->position.x;//right
						float x2 = m_pEnemy[t]->getTransform()->position.x - m_dDamgobj[i]->getTransform()->position.x;// left
						float y = m_dDamgobj[i]->getTransform()->position.y - m_pEnemy[t]->getTransform()->position.y;//down
						float y2 = m_pEnemy[t]->getTransform()->position.y - m_dDamgobj[i]->getTransform()->position.y;// up

						//RightCollision
						if ((x> x2) && (x > y) && (x > y2))
							m_pEnemy[t]->getTransform()->position.x = m_pEnemy[t]->getTransform()->position.x - 5.0f;
						//LeftCollision
						else if ((x2 > x) && (x2 > y) && (x2 > y2))
							m_pEnemy[t]->getTransform()->position.x = m_pEnemy[t]->getTransform()->position.x + 5.0f;
						//DownCollision
						else if ((y> x) && (y > x2) && (y > y2))
							m_pEnemy[t]->getTransform()->position.y = m_pEnemy[t]->getTransform()->position.y - 5.0f;
						//UpCollision
						else if ((y2 > x) && (y2 > x2) && (y2 > y))
							m_pEnemy[t]->getTransform()->position.y = m_pEnemy[t]->getTransform()->position.y + 5.0f;
					}
				}
			}
		}
	}
	
	//Player and enemy Collision
	if (m_pPlayer->isEnabled())
	{
		for (int i = 0; i < Enemies; i++)
		{
			if (CollisionManager::CircleAABBCheck(m_pPlayer, m_pEnemy[i]))
			{
				float x = m_pEnemy[i]->getTransform()->position.x - m_pPlayer->getTransform()->position.x;//collision right
				float x2 = m_pPlayer->getTransform()->position.x - m_pEnemy[i]->getTransform()->position.x;//collision left
				float y = m_pEnemy[i]->getTransform()->position.y - m_pPlayer->getTransform()->position.y;//collision down
				float y2 = m_pPlayer->getTransform()->position.y - m_pEnemy[i]->getTransform()->position.y;//collision up

				//RightCollision
				if ((x > x2) && (x > y) && (x > y2))
				{
					m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x - 5.0f;
					m_pEnemy[i]->getTransform()->position.x = m_pEnemy[i]->getTransform()->position.x + 5.0f;
				}
				//LeftCollision
				else if ((x2 > x) && (x2 > y) && (x2 > y2))
				{
					m_pPlayer->getTransform()->position.x = m_pPlayer->getTransform()->position.x + 5.0f;
					m_pEnemy[i]->getTransform()->position.x = m_pEnemy[i]->getTransform()->position.x - 5.0f;
				}
				//DownCollision
				else if ((y > x) && (y > x2) && (y > y2))
				{
					m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y - 5.0f;
					m_pEnemy[i]->getTransform()->position.y = m_pEnemy[i]->getTransform()->position.y + 5.0f;
				}
				//UpCollision
				else if ((y2 > x) && (y2 > x2) && (y2 > y))
				{
					m_pPlayer->getTransform()->position.y = m_pPlayer->getTransform()->position.y + 5.0f;
					m_pEnemy[i]->getTransform()->position.y = m_pEnemy[i]->getTransform()->position.y - 5.0f;
				}
			}
		}
	}

	//PlayerBullets Collision
	{
		//Player bullet and enemy Collision
		for (int i = 0; i < m_pBullet.size(); i++)
		{
			for (int t = 0; t < Enemies; t++)
			{
				if (m_pBullet[i]->isEnabled())
				{
					if (m_pEnemy[t]->isEnabled() == true)
					{
						if (CollisionManager::CircleAABBCheck(m_pBullet[i], m_pEnemy[t]))
						{
							m_pBullet[i]->setEnabled(false);
							int h;
							SoundManager::Instance().playSound("Expl", 0, -1);
							//Damage Enemy
							if (t == 0)
							{
								h = m_pEnemy[t]->getCurrentHp();
								Enemy0[h - 1]->setEnabled(false);
								SoundManager::Instance().playSound("dmg", 0, -1);
								m_pEnemy[t]->setCurrentHp(m_pEnemy[t]->getCurrentHp() - 1);
								if (m_pEnemy[t]->getCurrentHp() == 0)
								{
									m_pEnemy[t]->setEnabled(false);
									m_pEnemy[t]->getTransform()->position = glm::vec2{ -100.0f,-100.0f };
									RespawnCD = 0;
									m_pCCStateMachine->setCurrentState(CCPatrolState);
									SoundManager::Instance().playSound("die", 0, -1);
									if (m_pEnemyDebug[t]->isEnabled())
										m_pEnemyDebug[t]->setEnabled(false);
								}
							}
							//Damage Enemy
							else if (t == 1)
							{
								m_pEnemy[t]->isHit=true;
								h = m_pEnemy[t]->getCurrentHp();
								Enemy1[h - 1]->setEnabled(false);
								SoundManager::Instance().playSound("dmg", 0, -1);
								m_pEnemy[t]->setCurrentHp(m_pEnemy[t]->getCurrentHp() - 1);
								if (m_pEnemy[t]->getCurrentHp() == 0)
								{
									m_pEnemy[t]->setEnabled(false);
									m_pEnemy[t]->getTransform()->position = glm::vec2{ -100.0f,-100.0f };
									RespawnCD = 0;
									m_pRCStateMachine->setCurrentState(RCPatrolState);
									SoundManager::Instance().playSound("die", 0, -1);
									if (m_pEnemyDebug[t]->isEnabled())
										m_pEnemyDebug[t]->setEnabled(false);
								}
							}
						}
					}

				}
			}
			for (int y = 0; y < damgobj; y++)
			{
				if (m_pBullet[i]->isEnabled())
				{
					if (m_dDamgobj[y]->isEnabled())
					{
						if (CollisionManager::CircleAABBCheck(m_pBullet[i], m_dDamgobj[y]))
						{
							m_pBullet[i]->setEnabled(false);
							int h = 0;
							SoundManager::Instance().playSound("Expl", 0, -1);
							//Damage Tree right
							if (y == 0)
							{
								h = m_dDamgobj[y]->getCurrentHp();
								Tree1[h - 1]->setEnabled(false);
								m_dDamgobj[y]->setCurrentHp(m_dDamgobj[y]->getCurrentHp() - 1);
								if (m_dDamgobj[y]->getCurrentHp() == 0)
								{
									for (auto node : m_pRightTreeNodes)
									{
										m_pSGrid.push_back(node);
									}
									m_dDamgobj[y]->setEnabled(false);
								}
							}
							//Damage Tree left
							else if (y == 1)
							{
								h = m_dDamgobj[y]->getCurrentHp();
								Tree2[h - 1]->setEnabled(false);
								m_dDamgobj[y]->setCurrentHp(m_dDamgobj[y]->getCurrentHp() - 1);
								if (m_dDamgobj[y]->getCurrentHp() == 0)
								{
									for (auto node : m_pLeftTreeNodes)
									{
										m_pSGrid.push_back(node);
									}
									m_dDamgobj[y]->setEnabled(false);
								}
							}

						}
					}
				}
			}
		}

		//Player bullet and Stage collision
		for (int i = 0; i < m_pBullet.size(); i++)
		{
			for (int y = 0; y < obstacles; y++)
			{
				if (m_pBullet[i]->isEnabled())
				{
					if (CollisionManager::CircleAABBCheck(m_pBullet[i], m_field[y]))
					{
						m_pBullet[i]->setEnabled(false);
						SoundManager::Instance().playSound("Expl", 0, -1);
					}
				}
			}

		}
	}

	//Enemy BulletCollision
	{
		//Enemy Bullet and player Collision
		if (m_pPlayer->isEnabled() == true)
		{
			for (int i = 0; i < m_pEnemyBullet.size(); i++)
			{
				if (m_pEnemyBullet[i]->isEnabled())
				{

					if (CollisionManager::CircleAABBCheck(m_pEnemyBullet[i], m_pPlayer))
					{
						m_pEnemyBullet[i]->setEnabled(false);
						SoundManager::Instance().playSound("Expl", 0, -1);
						m_pEnemy[i]->AttackCd = 0;
						int h = 0;
						//Damage Player
						h = m_pPlayer->getCurrentHp();
						PlayerHp[h - 1]->setEnabled(false);
						m_pPlayer->setCurrentHp(m_pPlayer->getCurrentHp() - 1);
						if (m_pPlayer->getCurrentHp() == 0)
						{
							
							m_pPlayer->setEnabled(false);
							SoundManager::Instance().playSound("die", 0, -1);
						}
					}
				}
			}
		}
		//Enemy Bullet and stage Collision
		if (m_pPlayer->isEnabled())
		{
			for (int i = 0; i < m_pEnemyBullet.size(); i++)
			{
				if (m_pEnemyBullet[i]->isEnabled() == true)
				{
					for (int y = 0; y < obstacles; y++)
					{
						if (m_pEnemyBullet[i]->isEnabled())
						{
							if (CollisionManager::CircleAABBCheck(m_pEnemyBullet[i], m_field[y]))
							{
								m_pEnemyBullet[i]->setEnabled(false);
								SoundManager::Instance().playSound("Expl", 0, -1);
							}
						}
					}
				}
			}
		}
		
	}

	//Respawn
	for (int i = 0; i < Enemies; i++)
	{
		if (m_pEnemy[i]->getTransform()->position.x < 0 || m_pEnemy[i]->getTransform()->position.x > 800 ||
			m_pEnemy[i]->getTransform()->position.y < 0 || m_pEnemy[i]->getTransform()->position.y > 600 &&
			m_pEnemy[i]->getCurrentHp() == 0)
		{
		
			if (m_pEnemy[i]->ENEMY2X == true)
			{
				m_pEnemy[i]->ENEMY2X = false;
				leftEnemyActive = false;
			}
			else if (m_pEnemy[i]->ENEMY1X == true)
			{
				m_pEnemy[i]->ENEMY1X = false;
				rightEnemyActive = false;
			}
		}
	}
	
		////Conditon for change sences
	if (m_pPlayer->isEnabled() == false)
	{
		if (ButtonCD > 3)
		{
			TheGame::Instance()->changeSceneState(LOSE_SCENE);
		}
	}
	else if ((leftEnemyActive == false) && (rightEnemyActive == false))
	{
		if(fleed==false)
		TheGame::Instance()->changeSceneState(WIN_SCENE);
	}
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_3))
	{
		TheGame::Instance()->changeSceneState(WIN_SCENE);
	}

	//Debug
	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_T))
	{

	if(ButtonCD>1)
	{
		if (Test_it)
		{
			Test_it = false;
		}

		else
		{
			Test_it = true;
		}
		for (int i = 0; i < Enemies; i++)
		{
			if (m_pEnemy[i]->isEnabled())
			m_pEnemyDebug[i]->setEnabled(Test_it);

			else
			m_pEnemyDebug[i]->setEnabled(false);
		}

		ButtonCD = 0;
	}
	m_setGridEnabled(Test_it);
	m_toggleGrid(Test_it);
	}


	//Player Attacks
	{
		if (EventManager::Instance().getMouseButton(0) && ShootCD > 1)
		{
			if (m_pPlayer->isEnabled() == true)
			{
				SoundManager::Instance().playSound("at", 0, -1);
				for (int i = 0; i < Enemies; i++)
				{
					if (m_pEnemy[i]->isEnabled())
					{
						m_CheckCloseCombat(m_pPlayer,m_pEnemy[i]);
						if (m_pPlayer->isInCloseCombatDistance())
						{
							if (CollisionManager::lineRectCheck(m_pPlayer->getTransform()->position,
								m_pPlayer->getTransform()->position + m_pPlayer->getOrientation() * m_pPlayer->getCloseCombatDistance(),
								m_pEnemy[i]->getTransform()->position, m_pEnemy[i]->getWidth(), m_pEnemy[i]->getHeight()))
							{
								ShootCD = 0;
								int h = 0;
								//Damage Enemy 0
								if (i == 0)
								{
									h = m_pEnemy[i]->getCurrentHp();
									Enemy0[h - 1]->setEnabled(false);
									SoundManager::Instance().playSound("dmg", 0, -1);
									m_pEnemy[i]->setCurrentHp(m_pEnemy[i]->getCurrentHp() - 1);
									if (m_pEnemy[i]->getCurrentHp() == 0)
									{
										m_pEnemy[i]->setEnabled(false);
										m_pCCStateMachine->setCurrentState(CCPatrolState);
										m_pEnemy[i]->getTransform()->position = glm::vec2{ -100.0f,-100.0f };
										SoundManager::Instance().playSound("die", 0, -1);
										if (m_pEnemyDebug[i]->isEnabled())
											m_pEnemyDebug[i]->setEnabled(false);
									}
								}
								//Damage Enemy 1
								else if (i == 1)
								{
									m_pEnemy[i]->isHit=true;
									h = m_pEnemy[i]->getCurrentHp();
									Enemy1[h - 1]->setEnabled(false);
									SoundManager::Instance().playSound("dmg", 0, -1);
									m_pEnemy[i]->setCurrentHp(m_pEnemy[i]->getCurrentHp() - 1);
									if (m_pEnemy[i]->getCurrentHp() == 0)
									{
										m_pEnemy[i]->setEnabled(false);
										m_pRCStateMachine->setCurrentState(RCPatrolState);
										m_pEnemy[i]->getTransform()->position = glm::vec2{ -100.0f,-100.0f };
										SoundManager::Instance().playSound("die", 0, -1);
										if (m_pEnemyDebug[i]->isEnabled())
											m_pEnemyDebug[i]->setEnabled(false);
									}
								}
								
							}
						}
					}
				}

				for (int i = 0; i < damgobj; i++)
				{
					if (m_dDamgobj[i]->isEnabled())
					{
						m_CheckCloseCombat(m_pPlayer,m_dDamgobj[i]);
						if (m_pPlayer->isInCloseCombatDistance())
						{
							if (CollisionManager::lineRectCheck(m_pPlayer->getTransform()->position,
								m_pPlayer->getTransform()->position + m_pPlayer->getOrientation() * m_pPlayer->getCloseCombatDistance(),
								m_dDamgobj[i]->getTransform()->position, m_dDamgobj[i]->getWidth(), m_dDamgobj[i]->getHeight()))
							{
								ShootCD = 0;
								int h = 0;
								//Damage Tree Right
								if (i == 0)
								{
									h = m_dDamgobj[i]->getCurrentHp();
									Tree1[h - 1]->setEnabled(false);
									m_dDamgobj[i]->setCurrentHp(m_dDamgobj[i]->getCurrentHp() - 1);
									if (m_dDamgobj[i]->getCurrentHp() == 0)
									{
										for (auto node : m_pRightTreeNodes)
										{
											m_pSGrid.push_back(node);
										}
										m_dDamgobj[i]->setEnabled(false);
									}
								}
								//Damage Tree Left
								else if (i == 1)
								{
									h = m_dDamgobj[i]->getCurrentHp();
									Tree2[h - 1]->setEnabled(false);
									m_dDamgobj[i]->setCurrentHp(m_dDamgobj[i]->getCurrentHp() - 1);
									if (m_dDamgobj[i]->getCurrentHp() == 0)
									{
										for (auto node : m_pLeftTreeNodes)
										{
											m_pSGrid.push_back(node);
										}
										m_dDamgobj[i]->setEnabled(false);
									}
								}
							}
						}
					}
				}
				ShootCD = 0;
			}
		}

		//Player BulletShooting
		if (EventManager::Instance().getMouseButton(2) && ShootCD > 1)
		{
			if (m_pPlayer->isEnabled() == true)
			{
				ShootCD = 0;
				m_pBullet.push_back(new Bullet(m_pPlayer->getRotation(), m_pPlayer->getTransform()->position, 
					"../Assets/textures/Tbullet.png", "Tbullet", true));
				addChild(m_pBullet[TotalBullets]);
				TotalBullets++;
				SoundManager::Instance().playSound("sht", 0, -1);
			}
		}
	}
	
}

void PlayScene::start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";


	SoundManager::Instance().load("../Assets/audio/Bgm2.mp3", "Bgm2", SOUND_MUSIC);
	SoundManager::Instance().load("../Assets/audio/CloseCombatAttack.mp3", "at", SOUND_SFX);
	SoundManager::Instance().load("../Assets/audio/Damage.mp3", "dmg", SOUND_SFX);
	SoundManager::Instance().load("../Assets/audio/gunShot.mp3", "sht", SOUND_SFX);
	SoundManager::Instance().load("../Assets/audio/Exp.wav", "Expl", SOUND_SFX);
	SoundManager::Instance().load("../Assets/audio/Die.mp3", "die", SOUND_SFX);
	SoundManager::Instance().playMusic("Bgm2", -1, 0);

	//Offsets
		auto offsetTiles1 = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
		auto offsetTiles2 = glm::vec2(Config::TILE_SIZE * 1.0f, Config::TILE_SIZE * 0.5f);
		auto offsetEnemiesDown = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f + 60.0f);
		auto offsetEnemiesUp = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f - 60.0f);
		auto offsetEnemiesRight = glm::vec2(Config::TILE_SIZE * 0.5f + 60.0f, Config::TILE_SIZE * 0.5f);
		auto offsetEnemiesLeft = glm::vec2(Config::TILE_SIZE * 0.5f - 60.0f, Config::TILE_SIZE * 0.5f);

	//Tiles
	m_buildGrid();
	m_buildGridSight();

	//Background
	Bg = new TileC("../Assets/grid/x.jpg", "Bg");
	Bg->getTransform()->position.x = 800.0f/2;		
	Bg->getTransform()->position.y = 600.0f/2;
	addChild(Bg,0);

	//Obstacles
	{
		m_field[0] = new TileC("../Assets/grid/River120.png", "120");
		m_field[0]->getTransform()->position = m_getTile(1, 5)->getTransform()->position + offsetTiles1;
		addChild(m_field[0], 1);
		m_pMap.push_back(m_field[0]);

		m_field[1] = new TileC("../Assets/grid/River120.png", "120");
		m_field[1]->getTransform()->position = m_getTile(7, 4)->getTransform()->position + offsetTiles1;
		addChild(m_field[1], 1);
		m_pMap.push_back(m_field[1]);

		m_field[2] = new TileC("/Assets/grid/River120.png", "120");
		m_field[2]->getTransform()->position = m_getTile(12, 5)->getTransform()->position + offsetTiles1;
		addChild(m_field[2], 2);
		m_pMap.push_back(m_field[2]);

		m_field[3] = new TileC("../Assets/grid/River120.png", "120");
		m_field[3]->getTransform()->position = m_getTile(18, 6)->getTransform()->position + offsetTiles1;
		addChild(m_field[3], 1);
		m_pMap.push_back(m_field[3]);
	}

	//Destructibles
	{
		m_dDamgobj[0] = new DestructibleObstacle(3, "../Assets/grid/mine.png", "TreeG");
		m_dDamgobj[0]->getTransform()->position = m_getTile(14, 10)->getTransform()->position + offsetTiles1;
		addChild(m_dDamgobj[0], 1);
		m_pMap.push_back(m_dDamgobj[0]);
		//Health Tree1
		Tree1[0] = new Health();
		Tree1[0]->getTransform()->position = { m_dDamgobj[0]->getTransform()->position.x,m_dDamgobj[0]->getTransform()->position.y - 40 };
		addChild(Tree1[0], 3);
		Tree1[1] = new Health();
		Tree1[1]->getTransform()->position = { m_dDamgobj[0]->getTransform()->position.x + 10,m_dDamgobj[0]->getTransform()->position.y - 40 };
		addChild(Tree1[1], 3);
		Tree1[2] = new Health();
		Tree1[2]->getTransform()->position = { m_dDamgobj[0]->getTransform()->position.x - 10,m_dDamgobj[0]->getTransform()->position.y - 40 };
		addChild(Tree1[2], 3);

		m_dDamgobj[1] = new DestructibleObstacle(3, "../Assets/grid/mine.png", "TreeG");
		m_dDamgobj[1]->getTransform()->position = m_getTile(5, 10)->getTransform()->position + offsetTiles1;
		addChild(m_dDamgobj[1], 2);
		m_pMap.push_back(m_dDamgobj[1]);
		//Health Tree2
		Tree2[0] = new Health();
		Tree2[0]->getTransform()->position = { m_dDamgobj[1]->getTransform()->position.x,m_dDamgobj[1]->getTransform()->position.y - 40 };
		addChild(Tree2[0], 3);
		Tree2[1] = new Health();
		Tree2[1]->getTransform()->position = { m_dDamgobj[1]->getTransform()->position.x + 10,m_dDamgobj[1]->getTransform()->position.y - 40 };
		addChild(Tree2[1], 3);
		Tree2[2] = new Health();
		Tree2[2]->getTransform()->position = { m_dDamgobj[1]->getTransform()->position.x - 10,m_dDamgobj[1]->getTransform()->position.y - 40 };
		addChild(Tree2[2], 3);
	}
		
	//ENEMIES
	{
		//Enemy0
		m_pEnemy[0] = new CloseCombatEnemy("../Assets/textures/Landmine.png", "sl");
		m_pEnemy[0]->getTransform()->position = m_getTile(15, 8)->getTransform()->position + offsetTiles1;
		m_pEnemy[0]->active = true;
		m_pEnemy[0]->ENEMY1X = true;
		addChild(m_pEnemy[0], 2);
		//Health
		m_pEnemy[0]->setCurrentHp(4);
		Enemy0[0] = new Health();
		Enemy0[0]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x,m_pEnemy[0]->getTransform()->position.y - 40 };
		addChild(Enemy0[0], 3);
		Enemy0[1] = new Health();
		Enemy0[1]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x + 10,m_pEnemy[0]->getTransform()->position.y - 40 };
		addChild(Enemy0[1], 3);
		Enemy0[2] = new Health();
		Enemy0[2]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x - 10,m_pEnemy[0]->getTransform()->position.y - 40 };
		addChild(Enemy0[2], 3);
		Enemy0[3] = new Health();
		Enemy0[3]->getTransform()->position = { m_pEnemy[0]->getTransform()->position.x - 20,m_pEnemy[0]->getTransform()->position.y - 40 };
		addChild(Enemy0[3], 3);

		//Enemy1
		m_pEnemy[1] = new RangedCombatEnemy("../Assets/textures/Landmine.png", "sl2");
		m_pEnemy[1]->getTransform()->position = m_getTile(4, 8)->getTransform()->position + offsetTiles1;
		m_pEnemy[1]->active = true;
		m_pEnemy[1]->ENEMY2X = true;
		addChild(m_pEnemy[1], 2);
		//Health
		m_pEnemy[1]->setCurrentHp(4);
		Enemy1[0] = new Health();
		Enemy1[0]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x,m_pEnemy[1]->getTransform()->position.y - 40 };
		addChild(Enemy1[0], 3);
		Enemy1[1] = new Health();
		Enemy1[1]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x + 10,m_pEnemy[1]->getTransform()->position.y - 40 };
		addChild(Enemy1[1], 3);
		Enemy1[2] = new Health();
		Enemy1[2]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x - 10,m_pEnemy[1]->getTransform()->position.y - 40 };
		addChild(Enemy1[2], 3);
		Enemy1[3] = new Health();
		Enemy1[3]->getTransform()->position = { m_pEnemy[1]->getTransform()->position.x - 20,m_pEnemy[1]->getTransform()->position.y - 40 };
		addChild(Enemy1[3], 3);


		//Enemy Debug//
		for (int i = 0; i < Enemies; i++)
		{
			m_pEnemyDebug[i] = new EnemyDebugMode(m_pEnemy[i]);
			m_pEnemyDebug[i]->setEnabled(false);
			addChild(m_pEnemyDebug[i], 0);
		}
	}

	//PLAYER:
	{
		//PlayerAgent
		m_pPlayer = new PlayerAgent();
		m_pPlayer->getTransform()->position = m_getTile(10, 0)->getTransform()->position + offsetTiles1;
		m_pPlayer->setEnabled(true);
		addChild(m_pPlayer, 2);
		m_pMap.push_back(m_pPlayer);

		//Player Health
		m_pPlayer->setCurrentHp(5);
		PlayerHp[0] = new Health();
		PlayerHp[0]->getTransform()->position = { m_pPlayer->getTransform()->position.x,m_pPlayer->getTransform()->position.y - 40 };
		addChild(PlayerHp[0], 3);
		PlayerHp[1] = new Health();
		PlayerHp[1]->getTransform()->position = { m_pPlayer->getTransform()->position.x + 10,m_pPlayer->getTransform()->position.y - 40 };
		addChild(PlayerHp[1], 3);
		PlayerHp[2] = new Health();
		PlayerHp[2]->getTransform()->position = { m_pPlayer->getTransform()->position.x - 10,m_pPlayer->getTransform()->position.y - 40 };
		addChild(PlayerHp[2], 3);
		PlayerHp[3] = new Health();
		PlayerHp[3]->getTransform()->position = { m_pPlayer->getTransform()->position.x - 20,m_pPlayer->getTransform()->position.y - 40 };
		addChild(PlayerHp[3], 3);
		PlayerHp[4] = new Health();
		PlayerHp[4]->getTransform()->position = { m_pPlayer->getTransform()->position.x + 20,m_pPlayer->getTransform()->position.y - 40 };
		addChild(PlayerHp[4], 3);
	}

	//StateMachine
	m_buildCloseCombatStateMachine();

	m_buildRangedStateMachine();
	

	
	////DECISION TREES:
	////Enemy 0
	//for (int i = 0; i < Enemies; i++)
	//{
	//	decisionTree[i] = new DecisionTree();
	//	decisionTree[i]->setAgent(m_pEnemy[i]);
	//}
}

void PlayScene::GUI_Function() const
{
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	ImGui::Begin("assiment 4", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove);

	static bool isGridEnabled = false;
	if (ImGui::Checkbox("Grid Enabled", &isGridEnabled))
	{
		//turn the grid on/off
		m_setGridEnabled(isGridEnabled);
	}

	
	ImGui::End();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}

void PlayScene::m_buildGrid()
{

	auto tileSize = Config::TILE_SIZE;
		//Add tiles to the grid
		for (int row = 0; row < Config::ROW_NUM; ++row)
		{
			for (int col = 0; col < Config::COL_NUM; ++col)
			{
				auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

				Tile* tile = new Tile();//Create empty tile
				tile->getTransform()->position = glm::vec2(col * tileSize, row * tileSize);
				tile->setGridPosition(col, row);
				addChild(tile,1);
				tile->addLabels();
				tile->setTileCost(tile->getGridPosition().x);
				tile->setTileStatus(tile->getGridPosition().y);
				tile->setEnabled(false);
				if (tile->getGridPosition() == glm::vec2(8, 3))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(0, 3))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(11, 3))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(19, 3))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(12, 8))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(12, 12))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(16, 12))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(16, 8))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(7, 8))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(7, 12))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(3, 12))
					m_MovementNodes.push_back(tile);
				if (tile->getGridPosition() == glm::vec2(3, 8))
					m_MovementNodes.push_back(tile);
				
				m_pGrid.push_back(tile);
			}
		}
	//Create references for each tile to its neighbours
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			Tile* tile = m_getTile(col, row);

			//Topmost Row
			if (row == 0)
			{
				tile->setNeighbourTile(TOP_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(TOP_TILE, m_getTile(col, row - 1));
			}

			//Rightmost column
			if (col == Config::COL_NUM - 1)
			{
				tile->setNeighbourTile(RIGHT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(RIGHT_TILE, m_getTile(col + 1, row));

			}

			//BottomMost Row
			if (row == Config::ROW_NUM - 1)
			{
				tile->setNeighbourTile(BOTTOM_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(BOTTOM_TILE, m_getTile(col, row + 1));

			}

			//Leftmost Column
			if (col == 0)
			{
				tile->setNeighbourTile(LEFT_TILE, nullptr);
			}
			else
			{
				tile->setNeighbourTile(LEFT_TILE, m_getTile(col - 1, row));
			}
		}

	}

	//m_setGridEnabled(true);//TODO delete to set Tile info to false

	for (auto t : m_MovementNodes)
	{
		t->m_tileColor = glm::vec4(0, 0, 1, 1);
		t->setTileCost(88.8f);
		t->setTileStatus(88.8f);
	}
	
	std::cout << m_pGrid.size() << std::endl;
}

void PlayScene::m_buildGridSight()
{
	auto tileSize = Config::TILE_SIZE;

	// add path_nodes to the Grid
	for (int row = 0; row < Config::ROW_NUM; ++row)
	{
		for (int col = 0; col < Config::COL_NUM; ++col)
		{
			PathNode* path_node = new PathNode();
			path_node->getTransform()->position = glm::vec2(
				(col * tileSize) + tileSize * 0.5f, (row * tileSize) + tileSize * 0.5f);
			path_node->setEnabled(false);
			addChild(path_node, 5); // 04 86
			//Exceptions and creation
			{
				if ((col == 0 && row == 4) || (col == 1 && row == 4) || (col == 2 && row == 4) || (col == 6 && row == 4) || (col == 7 && row == 4) || (col == 8 && row == 4) ||
					(col == 0 && row == 5) || (col == 1 && row == 5) || (col == 2 && row == 5) || (col == 6 && row == 5) || (col == 7 && row == 5) || (col == 8 && row == 5) ||
					(col == 0 && row == 6) || (col == 1 && row == 6) || (col == 2 && row == 6) || (col == 6 && row == 6) || (col == 7 && row == 6) || (col == 8 && row == 6) ||
					(col == 11 && row == 4) || (col == 12 && row == 4) || (col == 13 && row == 4) || (col == 17 && row == 4) || (col == 18 && row == 4) || (col == 19 && row == 4) ||
					(col == 11 && row == 5) || (col == 12 && row == 5) || (col == 13 && row == 5) || (col == 17 && row == 5) || (col == 18 && row == 5) || (col == 19 && row == 5) ||
					(col == 11 && row == 6) || (col == 12 && row == 6) || (col == 13 && row == 6) ||  (col == 17 && row == 6) || (col == 18 && row == 6) || (col == 19 && row == 6))
				{

				}// 49 6 11
				else if ((col == 4 && row == 9) || (col == 4 && row == 10) || (col == 4 && row == 11) || (col == 5 && row == 9) || (col == 5 && row == 10) ||
					(col == 5 && row == 11) || (col == 6 && row == 9) || (col == 6 && row == 10) || (col == 6 && row == 11))
				{
					m_pLeftTreeNodes.push_back(path_node);
				}
				else if ((col == 13 && row == 9) || (col == 13 && row == 10) || (col == 13 && row == 11) || (col == 14 && row == 9) || (col == 14 && row == 10) ||
					(col == 14 && row == 11) || (col == 15 && row == 9) || (col == 15 && row == 10) || (col == 15 && row == 11))
				{
					m_pRightTreeNodes.push_back(path_node);
				}
				else //Create grid with remaining nodes
					m_pSGrid.push_back(path_node);
			}
		}
	}
}

void PlayScene::m_CheckPathNodeLOS()
{
	for (auto path_node : m_pSGrid)
	{
		auto targetDirection = m_pPlayer->getTransform()->position - path_node->getTransform()->position;
		auto normalizedDirection = Util::normalize(targetDirection);
		path_node->setOrientation(normalizedDirection);
		m_CheckAgentLOS(path_node, m_pPlayer);
	}
}

void PlayScene::m_toggleGrid(bool state)
{
	for (auto path_node : m_pSGrid)
	{
		path_node->setEnabled(state);
	}
}

PathNode* PlayScene::m_findClosestPathNodeWithLOS(NavigationAgent* agent)
{
	auto min = INFINITY;
	PathNode* closestPathNode = nullptr;
	for (auto path_node : m_pSGrid)
	{
		if (path_node->hasLOS())
		{
			const auto distance = Util::distance(agent->getTransform()->position, path_node->getTransform()->position);
			if (distance < min)
			{
				min = distance;
				closestPathNode = path_node;
			}
		}
	}

	return closestPathNode;
}

PathNode* PlayScene::m_findClosestPathNodeWithoutLOS(NavigationAgent* agent)
{
	auto min = INFINITY;
	PathNode* closestPathNode = nullptr;
	for (auto path_node : m_pSGrid)
	{
		if (!(path_node->hasLOS()))
		{
			const auto distance = Util::distance(agent->getTransform()->position, path_node->getTransform()->position);
			if (distance < min)
			{
				min = distance;
				closestPathNode = path_node;
			}
		}
	}

	return closestPathNode;
}

void PlayScene::m_setGridEnabled(bool state) const
{
	for (auto tile : m_pGrid)
	{
		tile->setEnabled(state);
		tile->setLabelsEnabled(state);
	}
	if (state == false)
	{
		SDL_RenderClear(Renderer::Instance()->getRenderer());
	}
}

Tile* PlayScene::m_getTile(const int col, const int row) const
{
	return m_pGrid[(row * Config::COL_NUM) + col];
}

Tile* PlayScene::m_getTile(glm::vec2 grid_position) const
{
	const auto col = grid_position.x;
	const auto row = grid_position.y;
	return m_pGrid[(row * Config::COL_NUM) + col];
}

void PlayScene::m_move()
{
	//State Machine stuff

	for (int i = 0; i < Enemies; i++)
	{
	
		if (i == 0)
		{
			if (m_pEnemy[0]->active == true)
			{
				if (m_pPlayer->isEnabled())
				{
					auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

					if (m_pCCStateMachine->getCurrentState()->getAction()->getName() == "Patrol")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}
						if (m_pEnemy[i]->patrol == false)
						{
							m_pEnemy[i]->patrol = true;
						}

						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->search = false;
						/// <summary>
						/// move enemy to loop destination enemy1
						/// </summary>
						float dst2;
						if (m_pEnemy[i]->ENEMY1X == true)
						{
							if (m_pEnemy[i]->location0 == false)
							{
								dst2 = Util::distance(m_pEnemy[0]->getTransform()->position, m_getTile(12, 8)->getTransform()->position + offset);
								m_pEnemy[i]->setDestination(m_getTile(12, 8)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location0 = true;
								}
							}
							if (m_pEnemy[i]->location0 == true)
							{
								m_pEnemy[i]->setDestination(m_getTile(12, 12)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[0]->getTransform()->position, m_getTile(12, 12)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location1 = true;
								}
							}
							if ((m_pEnemy[i]->location0 == true) && (m_pEnemy[i]->location1 == true))
							{
								m_pEnemy[0]->setDestination(m_getTile(16, 12)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(16, 12)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location2 = true;
								}
							}
							if ((m_pEnemy[i]->location1 == true) && (m_pEnemy[i]->location2 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(16, 8)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(16, 8)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location0 = false;
									m_pEnemy[i]->location1 = false;
									m_pEnemy[i]->location2 = false;
								}
							}
						}


						/// <summary>
						/// move enemy to loop destination enemy2
						/// </summary>
						else if (m_pEnemy[i]->ENEMY2X == true)
						{
							m_pEnemy[i]->search = false;
							float dst4;
							if (m_pEnemy[i]->location0 == false)
							{
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(7, 8)->getTransform()->position + offset);
								m_pEnemy[i]->setDestination(m_getTile(7, 8)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location0 = true;
								}
							}
							if (m_pEnemy[i]->location0 == true)
							{
								m_pEnemy[i]->setDestination(m_getTile(7, 12)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(7, 12)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location1 = true;
								}
							}
							if ((m_pEnemy[i]->location0 == true) && (m_pEnemy[i]->location1 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(3, 12)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(3, 12)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location2 = true;
								}
							}
							if ((m_pEnemy[i]->location1 == true) && (m_pEnemy[i]->location2 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(3, 8)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(3, 8)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location0 = false;
									m_pEnemy[i]->location1 = false;
									m_pEnemy[i]->location2 = false;
								}
							}
						}
					}
					else if (m_pCCStateMachine->getCurrentState()->getAction()->getName() == "MoveToLOS")
					{
						if (m_pEnemy[i]->search == false)
						{
							m_pEnemy[i]->move = true;
						}
						m_pEnemy[i]->moveWithAvoidance = true;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->setDestination(m_findClosestPathNodeWithLOS(m_pEnemy[i])->getTransform()->position);
						if (Util::distance(m_pEnemy[i]->getTransform()->position, m_findClosestPathNodeWithLOS(m_pEnemy[i])->getTransform()->position)
							< 2.0f)
						{
							m_pEnemy[i]->search = true;
							m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
							m_pEnemy[i]->move = false;
						}
					}
					else if (m_pCCStateMachine->getCurrentState()->getAction()->getName() == "MoveToPlayer")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}

						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
					}
					else if (m_pCCStateMachine->getCurrentState()->getAction()->getName() == "CloseCombatAttack")
					{
						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->move = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
						if (m_pEnemy[i]->AttackCd >= 3.0f)
						{
							m_CheckCloseCombat(m_pEnemy[i], m_pPlayer);
							if (m_pEnemy[i]->isInCloseCombatDistance())
							{
								if (CollisionManager::lineRectCheck(m_pEnemy[i]->getTransform()->position,
									m_pEnemy[i]->getTransform()->position + m_pEnemy[i]->getOrientation() * m_pEnemy[i]->getCloseCombatDistance(),
									m_pPlayer->getTransform()->position, m_pPlayer->getWidth(), m_pPlayer->getHeight()))
								{
									m_pEnemy[i]->AttackCd = 0;
									int h = 0;
									//Damage Player
									h = m_pPlayer->getCurrentHp();
									PlayerHp[h - 1]->setEnabled(false);
									SoundManager::Instance().playSound("dmg", 0, -1);
									m_pPlayer->setCurrentHp(m_pPlayer->getCurrentHp() - 1);
									if (m_pPlayer->getCurrentHp() == 0)
									{
										m_pPlayer->setEnabled(false);
										SoundManager::Instance().playSound("die", 0, -1);
									}
								}
							}
						}
					}
					else if (m_pCCStateMachine->getCurrentState()->getAction()->getName() == "Flee")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}

						m_pEnemy[i]->moveWithAvoidance = true;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
						m_pEnemy[i]->flee = true;
						if (m_pEnemy[i]->getTransform()->position.x <= 0 || m_pEnemy[i]->getTransform()->position.x >= 800 ||
							m_pEnemy[i]->getTransform()->position.y <= 0 || m_pEnemy[i]->getTransform()->position.y >= 600)
						{
							RespawnCD = 0;
							fleed = true;
						}
					}
				}
			}
		}
		//Ranged 0
		else if (i == 1)
		{
			if (m_pEnemy[1]->active == true)
			{
				if (m_pPlayer->isEnabled())
				{
					auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);

					if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "Patrol")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}
						if (m_pEnemy[i]->patrol == false)
						{
							m_pEnemy[i]->patrol = true;
						}

						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->search = false;
						float dst2;
						if (m_pEnemy[i]->ENEMY1X == true)
						{
							if (m_pEnemy[i]->location0 == false)
							{
								dst2 = Util::distance(m_pEnemy[0]->getTransform()->position, m_getTile(12, 8)->getTransform()->position + offset);
								m_pEnemy[i]->setDestination(m_getTile(12, 8)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location0 = true;
								}
							}
							if (m_pEnemy[i]->location0 == true)
							{
								m_pEnemy[i]->setDestination(m_getTile(12, 12)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[0]->getTransform()->position, m_getTile(12, 12)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location1 = true;
								}
							}
							if ((m_pEnemy[i]->location0 == true) && (m_pEnemy[i]->location1 == true))
							{
								m_pEnemy[0]->setDestination(m_getTile(16, 12)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(16, 12)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location2 = true;
								}
							}
							if ((m_pEnemy[i]->location1 == true) && (m_pEnemy[i]->location2 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(16, 8)->getTransform()->position + offset);
								dst2 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(16, 8)->getTransform()->position + offset);
								if (dst2 < 5.0f)
								{
									m_pEnemy[i]->location0 = false;
									m_pEnemy[i]->location1 = false;
									m_pEnemy[i]->location2 = false;
								}
							}
						}
						else if (m_pEnemy[i]->ENEMY2X == true)
						{
							m_pEnemy[i]->search = false;
							float dst4;
							if (m_pEnemy[i]->location0 == false)
							{
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(7, 8)->getTransform()->position + offset);
								m_pEnemy[i]->setDestination(m_getTile(7, 8)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location0 = true;
								}
							}
							if (m_pEnemy[i]->location0 == true)
							{
								m_pEnemy[i]->setDestination(m_getTile(7, 12)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(7, 12)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location1 = true;
								}
							}
							if ((m_pEnemy[i]->location0 == true) && (m_pEnemy[i]->location1 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(3, 12)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(3, 12)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location2 = true;
								}
							}
							if ((m_pEnemy[i]->location1 == true) && (m_pEnemy[i]->location2 == true))
							{
								m_pEnemy[i]->setDestination(m_getTile(3, 8)->getTransform()->position + offset);
								dst4 = Util::distance(m_pEnemy[i]->getTransform()->position, m_getTile(3, 8)->getTransform()->position + offset);
								if (dst4 < 5.0f)
								{
									m_pEnemy[i]->location0 = false;
									m_pEnemy[i]->location1 = false;
									m_pEnemy[i]->location2 = false;
								}
							}
						}
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "MoveToPlayer")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}

						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "MoveToLOS")
					{
						if (m_pEnemy[i]->search == false)
						{
							m_pEnemy[i]->move = true;
						}

						m_pEnemy[i]->moveWithAvoidance = true;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->setDestination(m_findClosestPathNodeWithLOS(m_pEnemy[i])->getTransform()->position);
						if (Util::distance(m_pEnemy[i]->getTransform()->position, m_findClosestPathNodeWithLOS(m_pEnemy[i])->getTransform()->position)
							< 2.0f)
						{
							m_pEnemy[i]->search = true;
							m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
							m_pEnemy[i]->move = false;
						}
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "RangedAttack")
					{
						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->move = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
						if (m_pPlayer->isEnabled() == true)
						{
							if ((m_pEnemy[i]->getTransform()->position.x > 0.0f) && (m_pEnemy[i]->getTransform()->position.x < 800.0f))
							{
								if ((m_pEnemy[i]->getTransform()->position.y > 0.0f) && (m_pEnemy[i]->getTransform()->position.y < 600.0f))
								{
									if (m_pEnemy[i]->isEnabled() == true)
									{
										if (m_pEnemy[i]->AttackCd >= 3)
										{
											m_CheckRangedCombat(m_pEnemy[i], m_pPlayer);
											if (m_pEnemy[i]->isInRangedCombatDistance())
											{
												m_pEnemy[i]->AttackCd = 0;
												m_pEnemyBullet.push_back(new Bullet(m_pEnemy[i]->getRotation(), m_pEnemy[i]->getTransform()->position,
													"../Assets/textures/Tbullet2.png", "Tbullet2", true));
												addChild(m_pEnemyBullet[TotalEBullets]);
												TotalEBullets++;
											}
										}
									}
								}
							}
						}
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "Flee")
					{
						if (m_pEnemy[i]->move == false)
						{
							m_pEnemy[i]->move = true;
						}

						m_pEnemy[i]->moveWithAvoidance = true;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
						m_pEnemy[i]->flee = true;
						if (m_pEnemy[i]->getTransform()->position.x <= 0 || m_pEnemy[i]->getTransform()->position.x >= 800 ||
							m_pEnemy[i]->getTransform()->position.y <= 0 || m_pEnemy[i]->getTransform()->position.y >= 600)
						{
							RespawnCD = 0;
							fleed = true;
						}
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "MoveBehindCover")
					{
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->setDestination(m_findClosestPathNodeWithoutLOS(m_pEnemy[i])->getTransform()->position);
						if (Util::distance(m_pEnemy[i]->getTransform()->position, m_findClosestPathNodeWithoutLOS(m_pEnemy[i])->getTransform()->position)
							< 2.0f)
						{
							m_pEnemy[i]->moveWithAvoidance = true;
							m_pEnemy[i]->search = false;
							m_pEnemy[i]->setDestination(m_pPlayer->getTransform()->position);
							m_pEnemy[i]->CoveringTime = 0;
							m_pEnemy[i]->isCovering = true;
							m_pEnemy[i]->isHit = false;
							m_pEnemy[i]->move = false;
						}
						else
						{
							m_pEnemy[i]->move = true;
						}
					}
					else if (m_pRCStateMachine->getCurrentState()->getAction()->getName() == "WaitBehindCover")
					{
						m_pEnemy[i]->moveWithAvoidance = false;
						m_pEnemy[i]->patrol = false;
						m_pEnemy[i]->search = false;
						if (m_pEnemy[i]->wait == 0)
						{
							m_pEnemy[i]->wait = rand() % 4 + 2;

						}
						if (m_pEnemy[i]->CoveringTime >= m_pEnemy[i]->wait)
						{
							m_pEnemy[i]->isCovering = false;
							m_pEnemy[i]->wait = 0;
							m_pEnemy[i]->move = true;
						}
					}
				}
			}
		}
	}
}

void PlayScene::m_CheckAgentLOS(NavigationAgent* object,DisplayObject* To)
{
	// if ship to target distance is less than or equal to LOS distance
	auto agent_to_range = Util::distance(object->getTransform()->position, To->getTransform()->position);
	if (agent_to_range <= object->getLOSDistance())
	{
		std::vector<DisplayObject*> contactList;
		for (auto obj : m_pMap)
		{
			if (obj->isEnabled())
			{
				//Check if object is farther than the target
				auto ShipToObjectDistance = Util::distance(object->getTransform()->position, obj->getTransform()->position);
				if (ShipToObjectDistance <= agent_to_range)
				{
					if ((obj->getType() != object->getType()) && (object->getType() != To->getType()))
					{
						contactList.push_back(obj);
					}
				}
			}
		}
		contactList.push_back(To); //add the target at the end of the list
		auto hasLOS = CollisionManager::LOSCheck(object->getTransform()->position,
			object->getTransform()->position + object->getOrientation() * object->getLOSDistance(),
			contactList, To);
		object->setHasLOS(hasLOS);
	}
}

void PlayScene::m_CheckCloseCombat(NavigationAgent* object,DisplayObject* To)
{
	// if ship to target distance is less than or equal to LOS distance
	auto agent_to_range = Util::distance(object->getTransform()->position, To->getTransform()->position);
	if (agent_to_range <= object->getCloseCombatDistance())
	{
		std::vector<DisplayObject*> contactList;
		for (auto obj : m_pMap)
		{
			if (obj->isEnabled())
			{
				//Check if object is farther than the target
				auto ShipToObjectDistance = Util::distance(object->getTransform()->position, obj->getTransform()->position);
				if (ShipToObjectDistance <= agent_to_range)
				{
					if ((obj->getType() != object->getType()) && (object->getType() != To->getType()))
					{
						contactList.push_back(obj);
					}
				}
			}
		}
		contactList.push_back(To); //add the target at the end of the list
		auto hasLOS = CollisionManager::LOSCheck(object->getTransform()->position,
			object->getTransform()->position + object->getOrientation() * object->getCloseCombatDistance(),
			contactList, To);
		object->setIsInCloseCombatDistance(hasLOS);
	}
}

void PlayScene::m_CheckRangedCombat(NavigationAgent* object, DisplayObject* To)
{

	// if ship to target distance is less than or equal to LOS distance
	auto agent_to_range = Util::distance(object->getTransform()->position, To->getTransform()->position);
	if (agent_to_range <= object->getRangedCombatDistance())
	{
		std::vector<DisplayObject*> contactList;
		for (auto obj : m_pMap)
		{
			if (obj->isEnabled())
			{
				//Check if object is farther than the target
				auto ShipToObjectDistance = Util::distance(object->getTransform()->position, obj->getTransform()->position);
				if (ShipToObjectDistance <= agent_to_range)
				{
					if ((obj->getType() != object->getType()) && (object->getType() != To->getType()))
					{
						contactList.push_back(obj);
					}
				}
			}
		}
		contactList.push_back(To); //add the target at the end of the list
		auto hasLOS = CollisionManager::LOSCheck(object->getTransform()->position,
			object->getTransform()->position + object->getOrientation() * object->getRangedCombatDistance(),
			contactList, To);
		object->setIsInRangedCombatDistance(hasLOS);
	}
}

void PlayScene::m_buildCloseCombatStateMachine()
{
	// Define States
	CCPatrolState = new State();
	State* CCMoveToPlayerState = new State();
	State* CCMoveToLOSState = new State();
	State* CCAttackState = new State();
	State* CCFleeState = new State();

	// Define Conditions
	m_pCCHasLOSCondition = new Condition();
	m_pCCLostLOSCondition = new Condition();
	m_pCCIsWithinDetectionRadiusCondition = new Condition();
	m_pCCIsNotWithinDetectionRadiusCondition = new Condition();
	m_pCCIsWithinCombatRangeCondition = new Condition();
	//
	m_pCCLifeIsLow = new Condition();
	m_pCCNotWithinCombatRangeCondition = new Condition();

	// Define Transitions
	Transition* CCmoveToPlayerTransition = new Transition(m_pCCHasLOSCondition, CCMoveToPlayerState);
	Transition* CCmoveToLOSTransition = new Transition(m_pCCIsWithinDetectionRadiusCondition, CCMoveToLOSState);
	Transition* CCattackTransition = new Transition(m_pCCIsWithinCombatRangeCondition, CCAttackState);
	Transition* CCLOSToPatrolTransition = new Transition(m_pCCIsNotWithinDetectionRadiusCondition, CCPatrolState);
	Transition* CCmoveToPlayerToLOSTransition = new Transition(m_pCCLostLOSCondition, CCMoveToLOSState);
	//
	Transition* CCenemyFleesTransition = new Transition(m_pCCLifeIsLow, CCFleeState);
	Transition* CCattackToMoveToPlayerTansition = new Transition(m_pCCNotWithinCombatRangeCondition, CCMoveToPlayerState);
	// Define Actions
	Patrol* CCPatrolAction = new Patrol();
	MoveToLOS* CCMoveToLOSAction = new MoveToLOS();
	MoveToPlayer* CCMoveToPlayerAction = new MoveToPlayer();
	CloseCombatAttack* CCAttackAction = new CloseCombatAttack();
	Flee* CCFleeAction = new Flee();

	// Setup Patrol State
	CCPatrolState->addTransition(CCmoveToPlayerTransition);
	CCPatrolState->addTransition(CCmoveToLOSTransition);
	CCPatrolState->addTransition(CCenemyFleesTransition);
	CCPatrolState->setAction(CCPatrolAction);

	// Setup MoveToPlayer State
	CCMoveToPlayerState->addTransition(CCattackTransition);
	CCMoveToPlayerState->addTransition(CCmoveToPlayerToLOSTransition);
	CCMoveToPlayerState->addTransition(CCenemyFleesTransition);
	CCMoveToPlayerState->setAction(CCMoveToPlayerAction);

	// Setup MoveToLOS State
	CCMoveToLOSState->addTransition(CCmoveToPlayerTransition);
	CCMoveToLOSState->addTransition(CCLOSToPatrolTransition);
	CCMoveToLOSState->addTransition(CCenemyFleesTransition);
	CCMoveToLOSState->setAction(CCMoveToLOSAction);

	// Setup CloseCombatAttack State
	CCAttackState->addTransition(CCattackToMoveToPlayerTansition); // Missing Condition
	CCAttackState->addTransition(CCmoveToPlayerToLOSTransition); // Missing Condition
	CCAttackState->addTransition(CCenemyFleesTransition);
	CCAttackState->setAction(CCAttackAction);

	// Flee State
	CCFleeState->setAction(CCFleeAction);

	m_pCCStateMachine = new StateMachine();
	m_pCCStateMachine->setCurrentState(CCPatrolState);
}



void PlayScene::m_buildRangedStateMachine()
{
	// Define States
	RCPatrolState = new State();
	State* RCMoveToPlayerState = new State();
	State* RCMoveToLOSState = new State();
	State* RCAttackState = new State();
	State* RCFleeState = new State();
	State* RCMoveBehindCoverState = new State();
	State* RCWaitBehindCoverState = new State();

	// Define Conditions
	m_pRCHasLOSCondition = new Condition();
	m_pRCLostLOSCondition = new Condition();
	m_pRCIsWithinDetectionRadiusCondition = new Condition();
	m_pRCIsNotWithinDetectionRadiusCondition = new Condition();
	m_pRCIsWithinCombatRangeCondition = new Condition();
	m_pRCNotWithinCombatRangeCondition = new Condition();
	m_pRCLifeIsLow = new Condition();
	m_pRCIsHit = new Condition();
	m_pRCCoverWait = new Condition();
	m_pRCCoverOut = new Condition();

	// Define Transitions
	Transition* RCMoveToPlayerTransition = new Transition(m_pRCHasLOSCondition, RCMoveToPlayerState);
	Transition* RCMoveToLOSTransition = new Transition(m_pRCIsWithinDetectionRadiusCondition, RCMoveToLOSState);
	Transition* RCAttackTransition = new Transition(m_pRCIsWithinCombatRangeCondition, RCAttackState);
	Transition* RCMLOSToPatrolTransition = new Transition(m_pRCIsNotWithinDetectionRadiusCondition, RCPatrolState);
	Transition* RCMoveToPlayerToLOSTransition = new Transition(m_pRCLostLOSCondition, RCMoveToLOSState);
	//
	Transition* RCEnemyFleesTransition = new Transition(m_pRCLifeIsLow, RCFleeState);
	Transition* RCAttackToMoveToPlayerTansition = new Transition(m_pRCNotWithinCombatRangeCondition, RCMoveToPlayerState);
	Transition* RCMoveBehindCoverTransition = new Transition(m_pRCIsHit, RCMoveBehindCoverState);
	Transition* RCCoverWaitTransition = new Transition(m_pRCCoverWait, RCWaitBehindCoverState);
	Transition* RCOutOfCoverTransition = new Transition(m_pRCCoverOut, RCMoveToLOSState);

	// Define Actions
	Patrol* RCPatrolAction = new Patrol();
	MoveToLOS* RCMoveToLOSAction = new MoveToLOS();
	MoveToPlayer* RCMoveToPlayerAction = new MoveToPlayer();
	RangedAttack* RCAttackAction = new RangedAttack();
	MoveBehindCover* RCMoveBehindCoverAction = new MoveBehindCover();
	WaitBehindCover* RCWaitBehindCoverAction = new WaitBehindCover();
	Flee* RCFleeAction = new Flee();

	// Setup Patrol State
	RCPatrolState->addTransition(RCMoveToPlayerTransition);
	RCPatrolState->addTransition(RCMoveToLOSTransition);
	RCPatrolState->addTransition(RCEnemyFleesTransition);
	RCPatrolState->addTransition(RCMoveBehindCoverTransition);
	RCPatrolState->setAction(RCPatrolAction);

	// Setup MoveToPlayer State
	RCMoveToPlayerState->addTransition(RCAttackTransition);
	RCMoveToPlayerState->addTransition(RCMoveToPlayerToLOSTransition);
	RCMoveToPlayerState->addTransition(RCEnemyFleesTransition);
	RCMoveToPlayerState->addTransition(RCMoveBehindCoverTransition);
	RCMoveToPlayerState->setAction(RCMoveToPlayerAction);

	// Setup MoveToLOS State
	RCMoveToLOSState->addTransition(RCMoveToPlayerTransition);
	RCMoveToLOSState->addTransition(RCMLOSToPatrolTransition);
	RCMoveToLOSState->addTransition(RCEnemyFleesTransition);
	RCMoveToLOSState->addTransition(RCMoveBehindCoverTransition);
	RCMoveToLOSState->setAction(RCMoveToLOSAction);

	// Setup CloseCombatAttack State
	RCAttackState->addTransition(RCAttackToMoveToPlayerTansition); 
	RCAttackState->addTransition(RCMoveToPlayerToLOSTransition); 
	RCAttackState->addTransition(RCEnemyFleesTransition);
	RCAttackState->addTransition(RCMoveBehindCoverTransition);
	RCAttackState->setAction(RCAttackAction);

	// Move Behind Cover State
	RCMoveBehindCoverState->addTransition(RCEnemyFleesTransition);
	RCMoveBehindCoverState->addTransition(RCCoverWaitTransition);
	RCMoveBehindCoverState->setAction(RCMoveBehindCoverAction);

	// Wait Behind Cover State
	RCWaitBehindCoverState->addTransition(RCEnemyFleesTransition);
	RCWaitBehindCoverState->addTransition(RCOutOfCoverTransition);
	RCWaitBehindCoverState->setAction(RCWaitBehindCoverAction);
	
	
	// Flee State
	RCFleeState->setAction(RCFleeAction);

	m_pRCStateMachine = new StateMachine();
	m_pRCStateMachine->setCurrentState(RCPatrolState);
}


