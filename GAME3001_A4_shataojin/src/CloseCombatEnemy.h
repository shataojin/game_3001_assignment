#pragma once
#ifndef __CLOSE_COMBAT_ENEMY__
#define __CLOSE_COMBAT_ENEMY__
#include "Enemy.h"

class CloseCombatEnemy : public Enemy
{
public:
	CloseCombatEnemy (std::string texture,std::string key);
	~CloseCombatEnemy() = default;


private:

};


#endif // !__CLOSE_COMBAT_ENEMY__
