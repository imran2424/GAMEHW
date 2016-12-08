//Imran Ahmed
//HW 5	
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <algorithm>

using namespace std;

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
}

class SheetSprite {
public:
	SheetSprite(){}
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float
		size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

void SheetSprite::Draw(ShaderProgram *program) {
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

class Vector{
public:
	Vector(){};
	Vector(float x, float y, float z) :x(0), y(0), z(0){}
	float x;
	float y;
	float z;
	void normalize();
};

void Vector::normalize(){
	float len = sqrt(x*x + y*y);
	x /= len;
	y /= len;
}

class Entity {
public:
	Entity(SheetSprite sprite) : sprite(sprite) {}
	void Render(ShaderProgram *program, Matrix modelMatrix);
	SheetSprite sprite;
	Vector pos;
	Vector velocity;
	float width;
	float height;
	bool isStatic;
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
	float rotation;
};


bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, const std::vector<Vector> &points2) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];
	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p < 0) {
		return true;
	}
	return false;
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}

		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
		if (!result) {
			return false;
		}
	}
	return true;
}



int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW 5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");


	GLuint font = LoadTexture("font2.png");
	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	SheetSprite enemy(spriteSheetTexture, 423.0f / 1024.0f, 644.0f / 1024.0f, 97.0f / 1024.0f, 84.0f / 1024.0f, 0.3);
	SheetSprite player(spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f / 1024.0f, 0.4);
	SheetSprite rock1(spriteSheetTexture, 224.0f / 1024.0f, 748.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f, 0.55);
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);


	float lastFrameTicks = 0.0f;

	Entity rock(rock1);
	rock.pos.x = -2.0;
	rock.pos.y = 0.5;
	rock.pos.z = 0.0;
	rock.rotation = 0.0;

	Entity rockV(rock1);
	rockV.pos.x = 2.0;
	rockV.pos.y = 0.5;
	rockV.pos.z = 0.0;
	rockV.rotation = 0.0;


	Entity enem(enemy);
	enem.pos.x = -2.0;
	enem.pos.y = -1.5;
	enem.pos.z = 0.0;
	enem.rotation = 0.0;

	Entity p(player);
	p.pos.x = 2.0;
	p.pos.y = -1.5;
	p.pos.z = 0.0;
	p.rotation = 0.0;

	bool started = false;
	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		modelMatrix.identity();
		modelMatrix.Translate(-2.5, 1.5, 0.0);
		program.setModelMatrix(modelMatrix);
		DrawText(&program, font, "Click SPACE to Start DEMO", 0.3f, 0.0f);
		
		if (keys[SDL_SCANCODE_SPACE]){
			started = true;
		}
		if (started)
		{
			rock.pos.x += elapsed * 0.5;
			rock.rotation += 5.5*elapsed;

			rockV.pos.x += elapsed*-0.5;
			rockV.rotation += -7.5*elapsed;

			enem.pos.x += elapsed*0.5;
			enem.rotation += -3.5*elapsed;

			p.pos.x += elapsed*-0.5;
			p.rotation += 4.0*elapsed;
		}
		modelMatrix.identity();
		modelMatrix.Translate(rock.pos.x, rock.pos.y, 0.0);
		modelMatrix.Rotate(rock.rotation);
		program.setModelMatrix(modelMatrix);
		rock.sprite.Draw(&program);

		modelMatrix.identity();
		modelMatrix.Translate(rockV.pos.x, rockV.pos.y, 0.0);
		modelMatrix.Rotate(rockV.rotation);
		program.setModelMatrix(modelMatrix);
		rockV.sprite.Draw(&program);

		modelMatrix.identity();
		modelMatrix.Translate(enem.pos.x, enem.pos.y, 0.0);
		modelMatrix.Rotate(enem.rotation);
		program.setModelMatrix(modelMatrix);
		enem.sprite.Draw(&program);

		modelMatrix.identity();
		modelMatrix.Translate(p.pos.x, p.pos.y, 0.0);
		modelMatrix.Rotate(p.rotation);
		program.setModelMatrix(modelMatrix);
		p.sprite.Draw(&program);	

		Vector rock1(rock.pos.x + (rock.width) / 2, rock.pos.y + (rock.height) / 2, 0.0);
		Vector rock2(rock.pos.x - (rock.width) / 2, rock.pos.y + (rock.height) / 2, 0.0);
		Vector rock3(rock.pos.x + (rock.width) / 2, rock.pos.y - (rock.height) / 2, 0.0);
		Vector rock4(rock.pos.x - (rock.width) / 2, rock.pos.y - (rock.height) / 2, 0.0);
		vector <Vector> entity1;
		entity1.push_back(rock1);
		entity1.push_back(rock2);
		entity1.push_back(rock3);
		entity1.push_back(rock4);

		Vector rockV1(rockV.pos.x + (rockV.width) / 2, rockV.pos.y + (rockV.height) / 2, 0.0);
		Vector rockV2(rockV.pos.x - (rockV.width) / 2, rockV.pos.y + (rockV.height) / 2, 0.0);
		Vector rockV3(rockV.pos.x + (rockV.width) / 2, rockV.pos.y - (rockV.height) / 2, 0.0);
		Vector rockV4(rockV.pos.x - (rockV.width) / 2, rockV.pos.y - (rockV.height) / 2, 0.0);
		vector <Vector> entity2;
		entity2.push_back(rockV1);
		entity2.push_back(rockV2);
		entity2.push_back(rockV3);
		entity2.push_back(rockV4);

		Vector enem1(enem.pos.x + (enem.width) / 2, enem.pos.y + (enem.height) / 2, 0.0);
		Vector enem2(enem.pos.x - (enem.width) / 2, enem.pos.y + (enem.height) / 2, 0.0);
		Vector enem3(enem.pos.x + (enem.width) / 2, enem.pos.y - (enem.height) / 2, 0.0);
		Vector enem4(enem.pos.x - (enem.width) / 2, enem.pos.y - (enem.height) / 2, 0.0);
		vector <Vector> entity3;
		entity3.push_back(enem1);
		entity3.push_back(enem2);
		entity3.push_back(enem3);
		entity3.push_back(enem4);

		Vector p1(p.pos.x + (p.width) / 2, p.pos.y + (p.height) / 2, 0.0);
		Vector p2(p.pos.x - (p.width) / 2, p.pos.y + (p.height) / 2, 0.0);
		Vector p3(p.pos.x + (p.width) / 2, p.pos.y - (p.height) / 2, 0.0);
		Vector p4(p.pos.x - (p.width) / 2, p.pos.y - (p.height) / 2, 0.0);
		vector <Vector> entity4;
		entity4.push_back(p1);
		entity4.push_back(p2);
		entity4.push_back(p3);
		entity4.push_back(p4);

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity1, entity2) && maxChecks > 0) {
				Vector responseVector = Vector(entity1[i].x - entity2[i].x, entity1[i].y - entity2[i].y, 0.0);
				responseVector.normalize();
				rock.pos.x -= responseVector.x * 0.002;
				rock.pos.y -= responseVector.y * 0.002;

				rockV.pos.x += responseVector.x * 0.002;
				rockV.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity1, entity3) && maxChecks > 0) {
				Vector responseVector = Vector(entity1[i].x - entity3[i].x, entity1[i].y - entity3[i].y, 0.0);
				responseVector.normalize();
				rock.pos.x -= responseVector.x * 0.002;
				rock.pos.y -= responseVector.y * 0.002;

				enem.pos.x += responseVector.x * 0.002;
				enem.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity1, entity4) && maxChecks > 0) {
				Vector responseVector = Vector(entity1[i].x - entity4[i].x, entity1[i].y - entity4[i].y, 0.0);
				responseVector.normalize();
				rock.pos.x -= responseVector.x * 0.002;
				rock.pos.y -= responseVector.y * 0.002;

				p.pos.x += responseVector.x * 0.002;
				p.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity2, entity3) && maxChecks > 0) {
				Vector responseVector = Vector(entity2[i].x - entity3[i].x, entity2[i].y - entity3[i].y, 0.0);
				responseVector.normalize();
				rockV.pos.x -= responseVector.x * 0.002;
				rockV.pos.y -= responseVector.y * 0.002;

				enem.pos.x += responseVector.x * 0.002;
				enem.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity2, entity4) && maxChecks > 0) {
				Vector responseVector = Vector(entity2[i].x - entity4[i].x, entity2[i].y - entity4[i].y, 0.0);
				responseVector.normalize();
				rockV.pos.x -= responseVector.x * 0.002;
				rockV.pos.y -= responseVector.y * 0.002;

				p.pos.x += responseVector.x * 0.002;
				p.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		for (int i = 0; i < 4; ++i){
			int maxChecks = 10;
			while (checkSATCollision(entity3, entity4) && maxChecks > 0) {
				Vector responseVector = Vector(entity3[i].x - entity4[i].x, entity3[i].y - entity4[i].y, 0.0);
				responseVector.normalize();
				enem.pos.x -= responseVector.x * 0.002;
				enem.pos.y -= responseVector.y * 0.002;

				p.pos.x += responseVector.x * 0.002;
				p.pos.x += responseVector.x * 0.002;

				maxChecks -= 1;
			}
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
