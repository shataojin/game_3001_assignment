#include "Health.h"
#include "TextureManager.h"

Health::Health()
{
	TextureManager::Instance()->load("../Assets/textures/Health.png", "Health");

	auto size = TextureManager::Instance()->getTextureSize("Health");
	setWidth(size.x);
	setHeight(size.y);

	getTransform()->position = glm::vec2(400.0f, 300.0f);
	setEnabled(true);
}

Health::~Health()
= default;


void Health::draw()
{
	TextureManager::Instance()->draw("Health",
		getTransform()->position.x, getTransform()->position.y,
		0, 255, true);
}

void Health::update()
{

}

void Health::destroy()
{
	setEnabled(false);
}
