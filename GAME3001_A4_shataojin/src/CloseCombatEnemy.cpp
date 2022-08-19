#include "CloseCombatEnemy.h"
#include "FleeAction.h"
#include "AttackAction.h"
#include "EventManager.h"
#include "TextureManager.h"
#include "Util.h"
#include "Game.h"
#include "MoveToLOSAction.h"
#include "MoveToPlayerAction.h"
#include "PatrolAction.h"
CloseCombatEnemy::CloseCombatEnemy(Scene* scene) : m_maxSpeed(20.0f),
                                       m_turnRate(5.0f), m_accelerationRate(3.0f), m_pScene(scene)
{
	TextureManager::Instance().Load("../Assets/textures/d7_small.png","close_enemy");

	const auto size = TextureManager::Instance().GetTextureSize("close_enemy");
	SetWidth(static_cast<int>(size.x));
	SetHeight(static_cast<int>(size.y));

	GetRigidBody()->velocity = glm::vec2(0, 0);
	GetRigidBody()->acceleration = glm::vec2(0, 0);
	GetRigidBody()->isColliding = false;

	SetCurrentHeading(0.0f); // current facing angle
	SetCurrentDirection(glm::vec2(1.0f, 0.0f)); // facing right

	SetLOSDistance(400.0f);
	SetLOSColour(glm::vec4(1, 0, 0, 1)); // default LOS colour is Red
	
	SetType(GameObjectType::AGENT);

	// New for Lab 7
	SetActionState(ActionState::NO_ACTION);
	BuildPatrolPath();

	// New for Lab 7 part 2
	m_tree = new DecisionTree(this); // Create a new Tree
	m_buildTree();
	m_tree->Display(); // optional
}

CloseCombatEnemy::~CloseCombatEnemy()
= default;

void CloseCombatEnemy::Draw()
{
	// draw the target
	TextureManager::Instance().Draw("close_enemy", GetTransform()->position, GetCurrentHeading(), 255, true);

	// draw LOS

	if(EventManager::Instance().IsIMGUIActive())
	{
		Util::DrawLine(GetTransform()->position + GetCurrentDirection() * 0.5f * static_cast<float>(GetWidth()),
			GetMiddleLOSEndPoint(), GetLOSColour());
	}
	
}

void CloseCombatEnemy::Update()
{
	// Determine which action to perform
	m_tree->MakeDecision();
}

void CloseCombatEnemy::Clean()
{

}

float CloseCombatEnemy::GetMaxSpeed() const
{
	return m_maxSpeed;
}

float CloseCombatEnemy::GetTurnRate() const
{
	return m_turnRate;
}

glm::vec2 CloseCombatEnemy::GetDesiredVelocity() const
{
	return m_desiredVelocity;
}

float CloseCombatEnemy::GetAccelerationRate() const
{
	return m_accelerationRate;
}

void CloseCombatEnemy::SetMaxSpeed(const float speed)
{
	m_maxSpeed = speed;
}

void CloseCombatEnemy::SetTurnRate(const float angle)
{
	m_turnRate = angle;
}

void CloseCombatEnemy::SetDesiredVelocity(const glm::vec2 target_position)
{
	m_desiredVelocity = Util::Normalize(target_position - GetTransform()->position) * m_maxSpeed;
}

void CloseCombatEnemy::SetAccelerationRate(const float rate)
{
	m_accelerationRate = rate;
}

void CloseCombatEnemy::Seek()
{
	// Find Next Waypoint if within 10px of the current waypoint
	if (Util::Distance(m_patrolPath[m_wayPoint], GetTransform()->position) < 10)
	{
		// check to see if you are at the last point in the path
		if (++m_wayPoint == m_patrolPath.size())
		{
			// if so...reset
			m_wayPoint = 0;
		}
		SetTargetPosition(m_patrolPath[m_wayPoint]);
	}


	SetDesiredVelocity(GetTargetPosition());

	const glm::vec2 steering_direction = GetDesiredVelocity() - GetCurrentDirection();

	LookWhereIAmGoing(steering_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * GetAccelerationRate();
}

void CloseCombatEnemy::LookWhereIAmGoing(const glm::vec2 target_direction)
{
	const auto target_rotation = Util::SignedAngle(GetCurrentDirection(), target_direction);
	if (GetCollisionWhiskers()[0])
	{
		SetCurrentHeading(GetCurrentHeading() + GetTurnRate());
	}
	else if (GetCollisionWhiskers()[2])
	{
		SetCurrentHeading(GetCurrentHeading() - GetTurnRate());
	}
	else if (abs(target_rotation) > m_turnRate)
	{
		SetCurrentHeading(GetCurrentHeading() + GetTurnRate() * Util::Sign(target_rotation));
	}
	UpdateWhiskers(GetWhiskerAngle()); // New for Lab 3.
}

void CloseCombatEnemy::BuildPatrolPath()
{
	m_patrolPath.push_back(glm::vec2(760, 40)); // Top Right corner node
	m_patrolPath.push_back(glm::vec2(760, 560)); // Bottom Right corner node
	m_patrolPath.push_back(glm::vec2(40, 560)); // Bottom Left corner node
	m_patrolPath.push_back(glm::vec2(40, 40)); // Top Left corner node
	m_wayPoint = 0;

	SetTargetPosition(m_patrolPath[m_wayPoint]); // Top Right to Start
}

void CloseCombatEnemy::Flee()
{
	if (GetActionState() != ActionState::FLEE)
	{
		SetActionState(ActionState::FLEE);
	}
	//FleeAction Algorithm
}

void CloseCombatEnemy::Move()
{
	Seek(); // Get our target for this frame.

	// Kinematic equation: final_position = position + velocity * time + 0.5*acceleration * time*time
	const auto delta_time = Game::Instance().GetDeltaTime();

	// compute the position term
	const glm::vec2 initial_position = GetTransform()->position;

	// compute the velocity term
	const glm::vec2 velocity_term = GetRigidBody()->velocity * delta_time;

	// compute the acceleration term
	const glm::vec2 acceleration_term = GetRigidBody()->acceleration * 0.5f;// *dt;


	// compute the new position
	glm::vec2 final_position = initial_position + velocity_term + acceleration_term;

	GetTransform()->position = final_position;

	// add our acceleration to velocity
	GetRigidBody()->velocity += GetRigidBody()->acceleration;

	// clamp our velocity at max speed
	GetRigidBody()->velocity = Util::Clamp(GetRigidBody()->velocity, GetMaxSpeed());
}

void CloseCombatEnemy::CheckBounds(){}

void CloseCombatEnemy::Reset()
{
	GetTransform()->position = m_startPos;
}
//done patrol
void CloseCombatEnemy::Patrol()
{
	if(GetActionState() != ActionState::PATROL)
	{
		// Initialize
		SetActionState(ActionState::PATROL);
	}
	Move();
}
//done move to player
void CloseCombatEnemy::MoveToPlayer()
{
	auto scene = dynamic_cast<PlayScene*>(m_pScene);
	if(GetActionState() != ActionState::MOVE_TO_PLAYER)
	{
		SetActionState(ActionState::MOVE_TO_PLAYER);
	}

	glm::vec2 target_direction = Util::Normalize(scene->GetTarget()->GetTransform()->position - GetTransform()->position);
	
	if (Util::Distance(target_direction, GetTransform()->position) < 10)
	{
		SetTargetPosition(target_direction);
	}


	SetDesiredVelocity(GetTargetPosition());

	LookWhereIAmGoing(target_direction);

	GetRigidBody()->acceleration = GetCurrentDirection() * GetAccelerationRate();

	// Kinematic equation: final_position = position + velocity * time + 0.5*acceleration * time*time
	const auto delta_time = Game::Instance().GetDeltaTime();

	// compute the position term
	const glm::vec2 initial_position = GetTransform()->position;

	// compute the velocity term
	const glm::vec2 velocity_term = GetRigidBody()->velocity * delta_time;

	// compute the acceleration term
	const glm::vec2 acceleration_term = GetRigidBody()->acceleration * 0.5f;// *dt;


	// compute the new position
	glm::vec2 final_position = initial_position + velocity_term + acceleration_term;

	GetTransform()->position = final_position;

	// add our acceleration to velocity
	GetRigidBody()->velocity += GetRigidBody()->acceleration;

	// clamp our velocity at max speed
	GetRigidBody()->velocity = Util::Clamp(GetRigidBody()->velocity, GetMaxSpeed());


}
void CloseCombatEnemy::MoveToLOS()
{
	if (GetActionState() != ActionState::MOVE_TO_LOS)
	{
		SetActionState(ActionState::MOVE_TO_LOS);
	}

	// MoveToLOS Algorithm
}


void CloseCombatEnemy::Attack()
{
	auto scene = dynamic_cast<PlayScene*>(m_pScene); // alias

	if (GetActionState() != ActionState::ATTACK)
	{
		SetActionState(ActionState::ATTACK);

		// initialize
		m_meleeCounter = 0;
	}
	// Attack Action Algorithm

	// New for Lab 8
	// Need to get the target object from Play Scene
	glm::vec2 target_direction = Util::Normalize(scene->GetTarget()->GetTransform()->position - GetTransform()->position);
	LookWhereIAmGoing(target_direction);

	// wait for a number of frames before firing = frame delay
	/*if (m_meleeCounter++ % m_meleeCounterMax == 0)
	{
		std::cout << "melee!!//change me!!" << std::endl;
	}*/
}

DecisionTree* CloseCombatEnemy::GetTree() const
{
	return m_tree;
}

void CloseCombatEnemy::m_buildTree()
{

	m_tree->SetEnemyHealthNode(new EnemyHealthCondition(this));
	m_tree->GetTree().push_back(m_tree->GetEnemyHealthNode());
	// 1 1
	TreeNode* fleeNode = m_tree->AddNode(m_tree->GetEnemyHealthNode(), new FleeAction(this), TreeNodeType::LEFT_TREE_NODE);
	m_tree->GetTree().push_back(fleeNode);
	//1 2
	m_tree->SetEnemyHitNode(new EnemyHitCondition(this));
	m_tree->AddNode(m_tree->GetEnemyHealthNode(), m_tree->GetEnemyHitNode(), TreeNodeType::RIGHT_TREE_NODE);
	m_tree->GetTree().push_back(m_tree->GetEnemyHitNode());
	//2 1
	m_tree->SetPlayerDetectedNode(new PlayerDetectedCondition(this));
	m_tree->AddNode(m_tree->GetEnemyHitNode(), m_tree->GetPlayerDetectedNode(), TreeNodeType::LEFT_TREE_NODE);
	m_tree->GetTree().push_back(m_tree->GetPlayerDetectedNode());
	// 2 2
	LOSCondition* LOSNodeuseless = new LOSCondition(this);
	m_tree->AddNode(m_tree->GetEnemyHitNode(), LOSNodeuseless, TreeNodeType::RIGHT_TREE_NODE);
	// 3 1
	TreeNode* patrolNode = m_tree->AddNode(m_tree->GetPlayerDetectedNode(), new PatrolAction(this), TreeNodeType::LEFT_TREE_NODE);
	patrolNode->SetAgent(this);
	m_tree->GetTree().push_back(patrolNode);
	// 3 2
	LOSCondition* LOSNode = new LOSCondition(this);
	m_tree->AddNode(m_tree->GetPlayerDetectedNode(), LOSNode, TreeNodeType::RIGHT_TREE_NODE);
	//LOSNodeLeft->SetAgent(this);
	m_tree->GetTree().push_back(LOSNode);
	// 4 1
	TreeNode* moveToLOSNodes = m_tree->AddNode(LOSNode, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	moveToLOSNodes->SetAgent(this);
	m_tree->GetTree().push_back(moveToLOSNodes);
	//4  2
	m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	m_tree->AddNode(LOSNode, m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());
	//5 1
	TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	moveToPlayerNode->SetAgent(this);
	m_tree->GetTree().push_back(moveToPlayerNode);
	//5 2
	TreeNode* attackNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	attackNode->SetAgent(this);
	m_tree->GetTree().push_back(attackNode);




	////level   2  ×ó  1 
	//m_tree->SetPlayerDetectedNode(new PlayerDetectedCondition(this));
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), m_tree->GetPlayerDetectedNode(), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetPlayerDetectedNode());
	////level   2  ÓÒ  2
	//LOSCondition* LOSNodeRight = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), LOSNodeRight, TreeNodeType::RIGHT_TREE_NODE);
	////LOSNodeRight->SetAgent(this);
	//m_tree->GetTree().push_back(LOSNodeRight);

	////level   3 ×ó  1 
	//TreeNode* patrolNode = m_tree->AddNode(m_tree->GetPlayerDetectedNode(), new PatrolAction(this), TreeNodeType::LEFT_TREE_NODE);
	////patrolNode->SetAgent(this);
	//m_tree->GetTree().push_back(patrolNode);
	////level   3 ÓÒ  2
	//LOSCondition* LOSNodeLeft = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetPlayerDetectedNode(), LOSNodeLeft, TreeNodeType::RIGHT_TREE_NODE);
	////LOSNodeLeft->SetAgent(this);
	//m_tree->GetTree().push_back(LOSNodeLeft);

	////level   3 ×ó  3
	//TreeNode* moveToLOSNode = m_tree->AddNode(LOSNodeRight, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToLOSNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);

	////level   3 ÓÒ  4
	//m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	//m_tree->AddNode(LOSNodeRight, m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());


	////level   4 ×ó

	//TreeNode* moveToLOSNode = m_tree->AddNode(LOSNodeLeft, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToLOSNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);

	////level   4 ÓÒ

	//m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	//m_tree->AddNode(LOSNodeLeft, m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());

	////level   4 ×ó

	//TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToPlayerNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToPlayerNode);


	////level   4 ÓÒ
	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	////attackNode->SetAgent(this);
	//m_tree->GetTree().push_back(attackNode);





	//TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToPlayerNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToPlayerNode);

	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	////attackNode->SetAgent(this);
	//m_tree->GetTree().push_back(attackNode);


	////level   3 Ö±½ÓÒ»Ì×

	//TreeNode* moveToLOSNode = m_tree->AddNode(LOSNodeLeft, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToLOSNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);

	//m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	//m_tree->AddNode(LOSNodeLeft, m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());

	//TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToPlayerNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToPlayerNode);

	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	////attackNode->SetAgent(this);
	//m_tree->GetTree().push_back(attackNode);




	//TreeNode* moveToLOSNode = m_tree->AddNode(LOSNodeLeft, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToLOSNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);




	//TreeNode* patrolNode = m_tree->AddNode(m_tree->GetPlayerDetectedNode(), new PatrolAction(this), TreeNodeType::LEFT_TREE_NODE);
	////patrolNode->SetAgent(this);
	//m_tree->GetTree().push_back(patrolNode);

	//LOSCondition* losNode = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetPlayerDetectedNode(), losNode, TreeNodeType::RIGHT_TREE_NODE);
	////losNode->SetAgent(this);
	//m_tree->GetTree().push_back(losNode);

	//TreeNode* moveToLOSNode = m_tree->AddNode(losNode, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToLOSNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);

	//m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	//m_tree->AddNode(losNode, m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());

	//TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	////moveToPlayerNode->SetAgent(this);
	//m_tree->GetTree().push_back(moveToPlayerNode);

	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	////attackNode->SetAgent(this);
	//m_tree->GetTree().push_back(attackNode);





	//// Conditions
	//// Create and add the root node - Health Condition
	//m_tree->SetEnemyHealthNode(new EnemyHealthCondition(this));
	//m_tree->GetTree().push_back(m_tree->GetEnemyHealthNode());
	//// Create and add the root node
	////m_tree->SetLOSNode(new LOSCondition(this));
	////m_tree->GetTree().push_back(m_tree->GetLOSNode());

	////m_tree->SetRadiusNode(new RadiusCondition(this));
	////m_tree->AddNode(m_tree->GetLOSNode(), m_tree->GetRadiusNode(), TreeNodeType::LEFT_TREE_NODE);
	////m_tree->GetTree().push_back(m_tree->GetRadiusNode());

	////m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	////m_tree->AddNode(m_tree->GetLOSNode(), m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	////m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());

	//// Actions
	////0
	//m_tree->SetRadiusNode(new RadiusCondition(this));
	//m_tree->AddNode(m_tree->GetLOSNode(), m_tree->GetRadiusNode(), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetRadiusNode());

	//LOSCondition* LOSNodeRight = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), LOSNodeRight, TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(LOSNodeRight);

	////1
	//TreeNode* patrolNode = m_tree->AddNode(m_tree->GetRadiusNode(), new PatrolAction(this), TreeNodeType::LEFT_TREE_NODE);
	////dynamic_cast<ActionNode*>(patrolNode)->SetAgent(this);
	//m_tree->GetTree().push_back(patrolNode);

	//LOSCondition* LOSNodeLeft = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetPlayerDetectedNode(), LOSNodeLeft, TreeNodeType::RIGHT_TREE_NODE);
	////dynamic_cast<ActionNode*>(LOSNodeLeft)->SetAgent(this);
	//m_tree->GetTree().push_back(LOSNodeLeft);


	////2
	//TreeNode* moveToLOSNode = m_tree->AddNode(m_tree->GetRadiusNode(), new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	////dynamic_cast<ActionNode*>(moveToLOSNode)->SetAgent(this);
	//m_tree->GetTree().push_back(moveToLOSNode);

	//m_tree->SetCloseCombatNode(new CloseCombatCondition(this));
	//m_tree->AddNode(LOSNodeLeft,  m_tree->GetCloseCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetCloseCombatNode());

	////3
	//TreeNode* moveToPlayerNode = m_tree->AddNode(m_tree->GetCloseCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	////dynamic_cast<ActionNode*>(moveToPlayerNode)->SetAgent(this);
	//m_tree->GetTree().push_back(moveToPlayerNode);

	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetRangedCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(attackNode);



	//// Create and add the root node - Health Condition
	//m_tree->SetEnemyHealthNode(new EnemyHealthCondition(this));
	//m_tree->GetTree().push_back(m_tree->GetEnemyHealthNode());

	//// Left SubTree Level 1 - Player Detected Condition
	//m_tree->SetPlayerDetectedNode(new PlayerDetectedCondition(this));
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), m_tree->GetPlayerDetectedNode(), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetPlayerDetectedNode());
	//// Right SubTree Level 1 - Enemy Hit Condition
	//m_tree->SetEnemyHitNode(new EnemyHitCondition(this));
	//m_tree->AddNode(m_tree->GetEnemyHealthNode(), m_tree->GetEnemyHitNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetEnemyHitNode());

	//// Left SubTree Level 2 - Player Detected Condition
	//m_tree->SetPlayerDetectedNode(new PlayerDetectedCondition(this));
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), m_tree->GetPlayerDetectedNode(), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetPlayerDetectedNode());

	//// Right SubTree Level 2 - Right LOS Condition
	//LOSCondition* LOSNodeRight = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetEnemyHitNode(), LOSNodeRight, TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(LOSNodeRight);

	//// Left Left Sub-Tree Level 3 - Patrol Action
	//TreeNode* patrolNode = m_tree->AddNode(m_tree->GetPlayerDetectedNode(), new PatrolAction(this), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(patrolNode);

	//// Left Right Sub-Tree Level 3 - Left LOS Condition
	//LOSCondition* LOSNodeLeft = new LOSCondition(this);
	//m_tree->AddNode(m_tree->GetPlayerDetectedNode(), LOSNodeLeft, TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(LOSNodeLeft);
	//// Left Sub-Tree Level 4 - Move to LOS Action
	//TreeNode* moveToLOSNode = m_tree->AddNode(LOSNodeLeft, new MoveToLOSAction(this), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(moveToLOSNode);

	//// Right Sub-Tree Level 4 - Ranged Combat Condition
	//m_tree->SetRangedCombatNode(new RangedCombatCondition(this));
	//m_tree->AddNode(LOSNodeLeft, m_tree->GetRangedCombatNode(), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(m_tree->GetRangedCombatNode());

	//// Left Sub-Tree Level 5 - MoveToRange Action
	//TreeNode* moveToRangeNode = m_tree->AddNode(m_tree->GetRangedCombatNode(), new MoveToPlayerAction(this), TreeNodeType::LEFT_TREE_NODE);
	//m_tree->GetTree().push_back(moveToRangeNode);

	//// Right Sub-Tree Level 5 - Attack Action
	//TreeNode* attackNode = m_tree->AddNode(m_tree->GetRangedCombatNode(), new AttackAction(this), TreeNodeType::RIGHT_TREE_NODE);
	//m_tree->GetTree().push_back(attackNode);

}
