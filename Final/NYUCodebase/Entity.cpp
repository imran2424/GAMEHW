//
//  Entity.cpp
//  NYUCodebase
//
//  Created by Ovais Ellahi on 11/30/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#include "Entity.h"
#include "ShaderProgram.h"


float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}


Entity::Entity() :width(0), height(0), velocity_x(0), velocity_y(0), acceleration_x(0), acceleration_y(0), gravity(-5.0)
{
	collidesReset();
}

Entity::Entity(SheetSprite sprite) : sprite(sprite), velocity_x(0), velocity_y(0), acceleration_x(0), acceleration_y(0), gravity(-5.0)

{
	width = sprite.width / sprite.height*sprite.size;
	height = sprite.size;
	collidesReset();
}


void Entity::Update(float elapsed)
{
	collidesReset();
	velocity_x = lerp(velocity_x, 0.0f, elapsed * 5.0);
	velocity_y = lerp(velocity_y, 0.0f, elapsed * 1.0);
	velocity_x += acceleration_x * elapsed;
	velocity_y += acceleration_y * elapsed;
	y += velocity_y * elapsed;
	x += velocity_x * elapsed;
}

void Entity::Render(ShaderProgram* program)
{
	modelMatrix.identity();
	modelMatrix.Translate(x, y, 0);
	program->setModelMatrix(modelMatrix);
	sprite.Draw(program);
}
void Entity::collidesReset()
{
	collidedLeft = false;
	collidedRight = false;
	collidedTop = false;
	collidedBottom = false;

}
bool Entity::collidesWith(Entity * entity)
{
	float top = y + height / 2.0f;
	float bot = y - height / 2.0f;
	float left = x - width / 2.0f;
	float right = x + width / 2.0f;
	float entTop = entity->y + entity->height / 2.0f;
	float entBot = entity->y - entity->height / 2.0f;
	float entLeft = entity->x - entity->width / 2.0f;
	float entRight = entity->x + entity->width / 2.0f;
	float pen;
	if (top>entBot && bot< entTop)
	{
		if (bot<entBot)
		{
			collidedTop = true;

		}
		else if (bot > entBot)
		{
			collidedBottom = true;
		}

	}
	if (right>entLeft && left< entRight)
	{
		if (left<entLeft)
		{
			collidedRight = true;
		}
		else if (left>entLeft)
		{
			collidedLeft = true;
		}
	}

	return (top>entBot && bot< entTop) && (right>entLeft && left< entRight);

}
