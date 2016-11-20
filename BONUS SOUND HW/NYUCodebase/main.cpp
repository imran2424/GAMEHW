//Imran Ahmed
//HW 3 SPACE INVADERS BONUS SOUND HW
//Some differences from traditional space invaders include:
// - way in which the enemies move
// - enemies have to hit your shield 5 times for it to break, but if you hit it once, you will break it and go through

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
#include <SDL_mixer.h>

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



string convertInt(int a){
	stringstream ss;
	ss << a;
	string str = ss.str();
	return str;
}

class SheetSprite {
public:
	SheetSprite();
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

class Entity {
public:
	Entity(float x, float y, float z, SheetSprite sprite) : x(x), y(y), z(z), sprite(sprite) {}
	float x;
	float y;
	float z;
	float velocity;
	SheetSprite sprite;
	float size = sprite.size;
	int rlife;
	bool alive = true;
};

vector<Entity> enemies;
vector<Entity> rockV;
vector<Entity> pBullets;
vector<Entity> eBullets;

void reset(ShaderProgram program, Matrix modelMatrix, SheetSprite enemy, SheetSprite rock){
	float stx = -2.3;
	float sty = 1.0;
	for (int i = 0; i < 32; ++i){
		if (i < 8){
			Entity e(stx + (0.66*i), sty, 0.0, enemy);
			modelMatrix.identity();
			modelMatrix.Translate(e.x, e.y, 0.0);
			program.setModelMatrix(modelMatrix);
			enemies.push_back(e);
			e.velocity = 0.75;
		}
		else if (i < 16){
			Entity e(stx + (0.66*(i - 8)), sty - 0.4, 0.0, enemy);
			modelMatrix.identity();
			modelMatrix.Translate(e.x, e.y, 0.0);
			program.setModelMatrix(modelMatrix);
			enemies.push_back(e);
			e.velocity = 0.75;
		}
		else if (i < 24){
			Entity e(stx + (0.66*(i - 16)), sty - 0.8, 0.0, enemy);
			modelMatrix.identity();
			modelMatrix.Translate(e.x, e.y, 0.0);
			program.setModelMatrix(modelMatrix);
			enemies.push_back(e);
			e.velocity = 0.75;
		}
		else {
			Entity e(stx + (0.66*(i - 24)), sty - 1.2, 0.0, enemy);
			modelMatrix.identity();
			modelMatrix.Translate(e.x, e.y, 0.0);
			program.setModelMatrix(modelMatrix);
			enemies.push_back(e);
			e.velocity = 0.75;
		}
	}

	for (int i = 0; i < 5; ++i){
		Entity e(-2.6 + (i*1.25), -0.9, 0.0, rock);
		modelMatrix.identity();
		modelMatrix.Translate(e.x, e.y, 0.0);
		program.setModelMatrix(modelMatrix);
		rockV.push_back(e);
		e.rlife = 5;
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW 3 - SPACE INVADERS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint font = LoadTexture("font1.png");
	GLuint spriteSheetTexture = LoadTexture("sheet.png");
	SheetSprite mySprite(spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f / 1024.0f, 1.0);
	SheetSprite enemy(spriteSheetTexture, 423.0f / 1024.0f, 644.0f / 1024.0f, 97.0f / 1024.0f, 84.0f / 1024.0f, 0.3);
	SheetSprite player(spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f / 1024.0f, 0.4);
	SheetSprite pBullet(spriteSheetTexture, 856.0f / 1024.0f, 94.0f / 1024.0f, 9.0f / 1024.0f, 37.0f / 1024.0f, 0.3);
	SheetSprite eBullet(spriteSheetTexture, 858.0f / 1024.0f, 475.0f / 1024.0f, 9.0f / 1024.0f, 37.0f / 1024.0f, 0.2);
	SheetSprite rock(spriteSheetTexture, 224.0f / 1024.0f, 748.0f / 1024.0f, 101.0f / 1024.0f, 84.0f / 1024.0f, 0.55);

	enum GameState { STATE_GAME_TITLE, STATE_GAME_PLAY, STATE_GAME_WIN, STATE_GAME_LOSE };
	GameState state = STATE_GAME_TITLE;

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);
	float lastFrameTicks = 0.0f;

	reset(program, modelMatrix, enemy, rock);

	Entity p(-0.1, -1.7, 0.0, player);
	modelMatrix.identity();
	modelMatrix.Translate(p.x, p.y, 0.0);
	program.setModelMatrix(modelMatrix);
	p.sprite.Draw(&program);
	p.velocity = 1.75;

	int score = 0;
	int lives = 5;
	int counter = 28;
	float shoot = 0;

	Entity ebullet(0.0, 0.0, 0.0, eBullet);
	ebullet.velocity = 4.0;
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk *playershoot;
	playershoot = Mix_LoadWAV("player_shoot.wav");
	Mix_Chunk *enemyshoot;
	enemyshoot = Mix_LoadWAV("enemy_shoot.wav");
	Mix_Chunk *players;
	players = Mix_LoadWAV("player_shoot.wav");
	Mix_Chunk *playerdie;
	playerdie = Mix_LoadWAV("player_die.wav");
	Mix_Chunk *enemydie;
	enemydie = Mix_LoadWAV("enemy_die.wav");
	Mix_Chunk *rockE;
	rockE = Mix_LoadWAV("rock explode.wav");
	Mix_Music *mainmen;
	mainmen = Mix_LoadMUS("Guile_Theme.mp3");
	Mix_Music *winlose;
	winlose = Mix_LoadMUS("Norwegian_Blue_Records.mp3");

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

		if (state == STATE_GAME_TITLE){
			if (keys[SDL_SCANCODE_RETURN]) {
				state = STATE_GAME_PLAY;
			}
		}
		else if (state == STATE_GAME_PLAY){
			if (keys[SDL_SCANCODE_RIGHT]){
				if (p.x > 3.25){
					p.x += 0;
				}
				else{
					p.x += elapsed * p.velocity;
					modelMatrix.identity();
					modelMatrix.Translate(p.x, p.y, 0.0);
					program.setModelMatrix(modelMatrix);
				}
			}
			else if (keys[SDL_SCANCODE_LEFT]){
				if (p.x < -3.25){
					p.x += 0;
				}
				else{
					p.x -= elapsed * p.velocity;
					modelMatrix.identity();
					modelMatrix.Translate(p.x, p.y, 0.0);
					program.setModelMatrix(modelMatrix);
				}
			}
			else if (keys[SDL_SCANCODE_SPACE]){
				if (pBullets.size() == 0){
					Entity fire(p.x, p.y + (p.size / 2) + (pBullet.size / 2), 0.0, pBullet);
					fire.velocity = 5.2;
					pBullets.push_back(fire);
					Mix_PlayChannel(-1, playershoot, 0);
				}
			}
		}
		else if (state == STATE_GAME_WIN){
			if (keys[SDL_SCANCODE_BACKSPACE]){
				counter = 28;
				enemies.clear();
				reset(program, modelMatrix, enemy, rock);

				modelMatrix.identity();
				modelMatrix.Translate(p.x = -0.1, p.y = -1.7, 0.0);
				program.setModelMatrix(modelMatrix);
				p.sprite.Draw(&program);
				state = STATE_GAME_TITLE;
			}
		}
		else if (state == STATE_GAME_LOSE){
			if (keys[SDL_SCANCODE_BACKSPACE]){
				counter = 28;
				enemies.clear();
				reset(program, modelMatrix, enemy, rock);

				modelMatrix.identity();
				modelMatrix.Translate(p.x = -0.1, p.y = -1.7, 0.0);
				program.setModelMatrix(modelMatrix);
				p.sprite.Draw(&program);
				state = STATE_GAME_TITLE;
			}
		}

		switch (state) {
		case STATE_GAME_TITLE:
			Mix_PlayMusic(mainmen, -1);
			modelMatrix.identity();
			modelMatrix.Translate(-1.5, 1.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "UNIVERSAL", 0.4f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.25, 1.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "INVADERS", 0.4f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.5, 0.3, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "HIT ENTER TO PLAY", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(0.0, -1.0, 0.0);
			program.setModelMatrix(modelMatrix);
			mySprite.Draw(&program);

			break;

		case STATE_GAME_PLAY:
			modelMatrix.identity();
			modelMatrix.Translate(-3.0, 1.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "SCORE: ", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, 1.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, convertInt(score), 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(1.8, 1.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "LIVES: ", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(3.0, 1.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, convertInt(lives), 0.2f, 0.0f);

			for (int i = 0; i < enemies.size(); ++i) {
				if (enemies[i].alive != false){
					enemies[i].x += elapsed * enemies[i].velocity;
					modelMatrix.identity();
					modelMatrix.Translate(enemies[i].x, enemies[i].y, 0);
					program.setModelMatrix(modelMatrix);
					enemies[i].sprite.Draw(&program);
				}
			}

			for (int i = 0; i < 32; ++i){
				if (enemies[i].x > 3.4 || enemies[i].x < -3.4){
					enemies[i].velocity *= -1;
				}
			}

			for (int i = 0; i < rockV.size(); ++i){
				if (rockV[i].rlife > 0){
					modelMatrix.identity();
					modelMatrix.Translate(rockV[i].x, rockV[i].y, 0.0);
					program.setModelMatrix(modelMatrix);
					rockV[i].sprite.Draw(&program);
				}
			}

			modelMatrix.identity();
			modelMatrix.Translate(p.x, p.y, 0.0);
			program.setModelMatrix(modelMatrix);
			p.sprite.Draw(&program);

			for (int i = 0; i < pBullets.size(); ++i){
				modelMatrix.identity();
				modelMatrix.Translate(pBullets[i].x, pBullets[i].y += elapsed*pBullets[i].velocity, 0.0);
				program.setModelMatrix(modelMatrix);
				pBullets[i].sprite.Draw(&program);
			}

			if (pBullets.size() == 1){
				if (pBullets[0].y >= 1.9){
					pBullets[0].x = 10.0;
					pBullets.erase(pBullets.begin() + 0);
					break;
				}
			}

			if (pBullets.size() == 1){
				for (int j = 0; j < rockV.size(); ++j){
					if (!(pBullets[0].x + (pBullets[0].size / 2) <= rockV[j].x - (rockV[j].size / 2) || pBullets[0].x - (pBullets[0].size / 2) >= rockV[j].x + (rockV[j].size / 2) ||
						pBullets[0].y + (pBullets[0].size / 2) <= rockV[j].y - (rockV[j].size / 2) || pBullets[0].y - (pBullets[0].size / 2) >= rockV[j].y + (rockV[j].size / 2))){
						pBullets[0].x = 10.0;
						pBullets[0].y = 10.0;
						pBullets.erase(pBullets.begin() + 0);
						rockV[j].rlife = rockV[j].rlife - 1;
						if (rockV[j].rlife == 0){
							rockV[j].x = 10.0;
						}
						Mix_PlayChannel(-1, rockE, 0);
						break;
					}
				}
			}

			if (pBullets.size() == 1){
				for (int j = 0; j < enemies.size(); ++j){
					if (!(pBullets[0].x + (pBullets[0].size / 2) <= enemies[j].x - (enemies[j].size / 2) || pBullets[0].x - (pBullets[0].size / 2) >= enemies[j].x + (enemies[j].size / 2) ||
						pBullets[0].y + (pBullets[0].size / 2) <= enemies[j].y - (enemies[j].size / 2) || pBullets[0].y - (pBullets[0].size / 2) >= enemies[j].y + (enemies[j].size / 2))){
						pBullets[0].x = 10.0;
						pBullets.erase(pBullets.begin() + 0);
						enemies[j].x = 10.0;
						enemies[j].alive = false;
						score += 5;
						counter = counter - 1;;
						Mix_PlayChannel(-1, enemydie, 0);
						break;
					}
				}
			}

			shoot += elapsed;
			if (shoot > 2){
				int i = rand() % 28;
				if (enemies[i].alive){
					ebullet.x = enemies[i].x;
					ebullet.y = enemies[i].y - (ebullet.size / 2) - (enemies[i].size / 2);
					shoot = 0;
					Mix_PlayChannel(-1, enemyshoot, 0);
				}
			}

			modelMatrix.identity();
			modelMatrix.Translate(ebullet.x, ebullet.y -= elapsed * ebullet.velocity, 0.0);
			program.setModelMatrix(modelMatrix);
			ebullet.sprite.Draw(&program);

			for (int j = 0; j < rockV.size(); ++j){
				if (!(ebullet.x + (ebullet.size / 2) <= rockV[j].x - (rockV[j].size / 2) || ebullet.x - (ebullet.size / 2) >= rockV[j].x + (rockV[j].size / 2) ||
					ebullet.y + (ebullet.size / 2) <= rockV[j].y - (rockV[j].size / 2) || ebullet.y - (ebullet.size / 2) >= rockV[j].y + (rockV[j].size / 2))){
					ebullet.y = -10.0;
					rockV[j].rlife = rockV[j].rlife - 1;
					if (rockV[j].rlife == 0){
						rockV[j].x = 10.0;
					}
					Mix_PlayChannel(-1, rockE, 0);
					break;
				}
			}

			if (!(ebullet.x + (ebullet.size / 2) <= p.x - (p.size / 2) || ebullet.x - (ebullet.size / 2) >= p.x + (p.size / 2) ||
				ebullet.y + (ebullet.size / 2) <= p.y - (p.size / 2) || ebullet.y - (ebullet.size / 2) >= p.y + (p.size / 2))){
				ebullet.y = -10.0;
				lives--;
				Mix_PlayChannel(-1, playerdie, 0);
			}

			if (counter == 0){
				state = STATE_GAME_WIN;
			}

			if (lives == 0){
				state = STATE_GAME_LOSE;
			}

			break;

		case STATE_GAME_WIN:
			Mix_PlayMusic(winlose, -1);
			score = 0;
			lives = 5;
			counter = 28;
			shoot = 0;
			modelMatrix.identity();
			modelMatrix.Translate(-2.2, 0.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU WIN", 0.8f, 0.0f);
			modelMatrix.identity();
			modelMatrix.Translate(-3.25, -0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "BACKSPACE for main menu", 0.3f, 0.0f);

			break;

		case STATE_GAME_LOSE:
			Mix_PlayMusic(winlose, -1);
			score = 0;
			lives = 5;
			counter = 28;
			shoot = 0;
			modelMatrix.identity();
			modelMatrix.Translate(-2.5, 0.8, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU LOSE", 0.8f, 0.0f);
			modelMatrix.identity();
			modelMatrix.Translate(-3.25, -0.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "BACKSPACE for main menu", 0.3f, 0.0f);

			break;
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		SDL_GL_SwapWindow(displayWindow);
	}

	Mix_FreeChunk(playershoot);
	Mix_FreeChunk(enemyshoot);
	Mix_FreeChunk(playerdie);
	Mix_FreeChunk(enemydie);
	Mix_FreeChunk(rockE);
	Mix_FreeMusic(mainmen);
	Mix_FreeMusic(winlose);

	SDL_Quit();
	return 0;
}