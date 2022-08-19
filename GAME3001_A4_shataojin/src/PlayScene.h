#pragma once
#ifndef __PLAY_SCENE__
#define __PLAY_SCENE__

#include "Bullet.h"
#include "Scene.h"
#include "Button.h"
#include "Label.h"
#include "Enemy.h"
#include "Target.h"
#include "PlayerAgent.h"
#include "EnemyDebugMode.h"
#include <vector>
#include "CloseCombatEnemy.h"
#include "Condition.h"
#include "DecisionTree.h"
#include "Health.h"
#include "PathNode.h"
#include "RangedCombatEnemy.h"
#include "StateMachine.h"
#include "Tile.h"
#include "TiledLevel.h"


class EnemyDebugMode;

class PlayScene : public Scene
{
public:
	PlayScene();
	~PlayScene();

	// Scene LifeCycle Functions
	virtual void draw() override;
	virtual void update() override;
	virtual void clean() override;
	virtual void handleEvents() override;
	virtual void start() override;

	float GameTimer = 0;
	float ShootCD = 0;
	float ButtonCD = 0;
	float CheckCD = 0;
	float RespawnCD = 0;

	bool rightEnemyActive = true;
	bool leftEnemyActive = true;

	bool fleed = false;
	
	int EnemiesDestroyed = 0;
	int TotalBullets = 0;
	int TotalEBullets = 0;
private:
	// IMGUI Function
	void GUI_Function() const;
	std::string m_guiTitle;

	glm::vec2 m_Position;
	SDL_Point m_mousePosition;


	//Tiles
	void m_setGridEnabled(bool state) const;
	std::vector<Tile*> m_pGrid;
	std::vector<Tile*> m_MovementNodes;
	std::vector<DisplayObject*>m_pMap;
	void m_buildGrid();

	//Grid LOS
	std::vector<PathNode*> m_pSGrid;
	std::vector<PathNode*> m_pLeftTreeNodes;
	std::vector<PathNode*> m_pRightTreeNodes;
	void m_buildGridSight();
	void m_CheckPathNodeLOS();
	void m_toggleGrid(bool state);
	PathNode* m_findClosestPathNodeWithLOS(NavigationAgent* agent);
	PathNode* m_findClosestPathNodeWithoutLOS(NavigationAgent* agent);
	
	//Map
	TileC* Bg;
	int obstacles = 4;
	TileC* m_field[6];
	int damgobj = 2;
	DestructibleObstacle* m_dDamgobj[2];

	//convenience functions
	Tile* m_getTile(int col, int row) const;
	Tile* m_getTile(glm::vec2 grid_position) const;


	//Labels
	Label* m_Inst[7];

	//Enemy
	int Enemies = 2;
	Enemy* m_pEnemy[2];
	EnemyDebugMode* m_pEnemyDebug[4];
	void m_move();

	//Player
	PlayerAgent* m_pPlayer;

	//Bullets
	std::vector<Bullet*>m_pBullet;
	std::vector<Bullet*>m_pEnemyBullet;
	
	void m_CheckAgentLOS(NavigationAgent* from, DisplayObject* to);
	void m_CheckCloseCombat(NavigationAgent* from,DisplayObject* to);
	void m_CheckRangedCombat(NavigationAgent* from, DisplayObject* to);

	int m_wayPoint;

	//StateMachine
	//CloseCombat0
	State* CCPatrolState;
	Condition* m_pCCHasLOSCondition;
	Condition* m_pCCLostLOSCondition;
	Condition* m_pCCIsWithinDetectionRadiusCondition;
	Condition* m_pCCIsNotWithinDetectionRadiusCondition;
	Condition* m_pCCIsWithinCombatRangeCondition;
	Condition* m_pCCLifeIsLow;
	Condition* m_pCCNotWithinCombatRangeCondition;
	
	//Ranged0
	State* RCPatrolState;
	Condition* m_pRCHasLOSCondition;
	Condition* m_pRCLostLOSCondition;
	Condition* m_pRCIsWithinDetectionRadiusCondition;
	Condition* m_pRCIsNotWithinDetectionRadiusCondition;
	Condition* m_pRCIsWithinCombatRangeCondition;
	Condition* m_pRCLifeIsLow;
	Condition* m_pRCNotWithinCombatRangeCondition;
	Condition* m_pRCIsHit;
	Condition* m_pRCCoverWait;
	Condition* m_pRCCoverOut;
	


	void m_buildCloseCombatStateMachine();
	StateMachine* m_pCCStateMachine;
	
	
	void m_buildRangedStateMachine();
	StateMachine* m_pRCStateMachine;
	
	
	////Decision tree
	//DecisionTree* decisionTree[6];
	
	// Others
	bool Test_it = false;
	//Hp

	Health* PlayerHp[5];
	Health* Enemy0[4];
	Health* Enemy1[4];
	Health* Tree1[3];
	Health* Tree2[3];

};

#endif /* defined (__PLAY_SCENE__) */