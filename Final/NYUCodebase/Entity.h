//
//  Entity.hpp
//  NYUCodebase
//
//  Created by Ovais Ellahi on 11/30/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#ifndef Entity_h
#define Entity_h

#include <stdio.h>
#include "SheetSprite.h"
#include "ShaderProgram.h"


enum EntityType {
	ENTITY_PLAYER, ENTITY_ENEMY,
	ENTITY_DIAMOND, ENTITY_KEY
};
enum EnemyType {
	ENEMY_SLUG, ENEMY_SLUG2, ENEMY_FISH, ENEMY_BEE,
	ENEMY_GHOST, ENEMY_SPIDER, ENEMY_BOSS, ENEMY_BAT
};
class Entity {
public:
	Entity();
	Entity(SheetSprite sprite);
	void Update(float elapsed);
	void Render(ShaderProgram *program);
	bool collidesWith(Entity *entity);
	SheetSprite sprite;
	void collidesReset();
	Matrix modelMatrix;
	float x;
	float y;
	float width;
	float height;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	float gravity;
	bool isStatic;
	EntityType entityType;
	EnemyType enemyType;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
};


#endif /* Entity_hpp */
