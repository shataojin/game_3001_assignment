#pragma once
#ifndef _HP_
#define _HP_
#include "DisplayObject.h"

class Health :public DisplayObject
{
public:
	Health();

	~Health();

	void draw() override;
	void update() override;
	void clean() override {};

	void destroy();
private:

};
#endif