//
//  SheetSprite.hpp
//  NYUCodebase
//
//  Created by Ovais Ellahi on 12/1/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#ifndef SheetSprite_h
#define SheetSprite_h

#include <stdio.h>
#include "ShaderProgram.h"

class SheetSprite {
public:
	SheetSprite();
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
		size);
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;


};


#endif /* SheetSprite_hpp */
