//
//  SheetSprite.cpp
//  NYUCodebase
//
//  Created by Ovais Ellahi on 12/1/16.
//  Copyright © 2016 Ivan Safrin. All rights reserved.
//

#include "SheetSprite.h"
#include "ShaderProgram.h"



SheetSprite::SheetSprite() :u(0), v(0), width(0), height(0), size(0), textureID(0)
{

}

SheetSprite::SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
	size) : u(u), v(v), width(width), height(height), size(size), textureID(textureID)
{

}

void SheetSprite::Draw(ShaderProgram *program)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
