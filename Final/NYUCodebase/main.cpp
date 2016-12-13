//Imran Ahmed Ovais Ellahi
//FINAL PROJECT 
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
#include "SheetSprite.h"
#include "Entity.h"
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

void draw(ShaderProgram *program, int textureID)
{

	float vertexCoords[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

enum GameState { STATE_GAME_TITLE, STATE_GAME_INS, STATE_GAME_LVL1, STATE_GAME_LVL2, STATE_GAME_LVL3, STATE_GAME_WIN, STATE_GAME_LOSE, STATE_GAME_T1, STATE_GAME_T2, STATE_GAME_T3 };
GameState state = STATE_GAME_TITLE;
Mix_Chunk *key;
Mix_Chunk *jump;
Mix_Chunk *diamond;
Mix_Chunk *enemydie;
Mix_Chunk *shoot;
Mix_Chunk *gethit;
Mix_Chunk *fallhit;
Mix_Chunk *winlvl;
Mix_Music *win;
Mix_Music *loser;
Mix_Music *titlee;
Mix_Music *lvl1;
Mix_Music *lvl2;
Mix_Music *lvl3;
Mix_Music *transs;


void switchState(GameState newState){
	state = newState;
	if (newState == STATE_GAME_TITLE){
		Mix_PlayMusic(titlee, -1);

	}
	else if (newState == STATE_GAME_LVL1){
		Mix_PlayMusic(lvl1, -1);
	}
	else if (newState == STATE_GAME_LVL2){
		Mix_PlayMusic(lvl2, -1);
	}
	else if (newState == STATE_GAME_LVL3){
		Mix_PlayMusic(lvl3, -1);
	}
	else if (newState == STATE_GAME_T1){
		Mix_PlayMusic(transs, -1);
	}
	else if (newState == STATE_GAME_T2){
		Mix_PlayMusic(transs, -1);
	}
	else if (newState == STATE_GAME_T3){
		Mix_PlayMusic(transs, -1);
	}
	else if (newState == STATE_GAME_LOSE){
		Mix_PlayMusic(loser, -1);
	}
	else if (newState == STATE_GAME_WIN){
		Mix_PlayMusic(win, -1);
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Hero", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint font = LoadTexture("font2.png");
	GLuint back1 = LoadTexture("back1.png");
	GLuint back2 = LoadTexture("back2.png");
	GLuint wiggle1 = LoadTexture("wiggle1.png");
	GLuint wiggle2 = LoadTexture("wiggle2.png");
	GLuint title = LoadTexture("mainpage.png");
	GLuint arrowkeys = LoadTexture("arrowkeys.png");
	GLuint obj = LoadTexture("obj.png");
	GLuint gems = LoadTexture("gems.png");
	GLuint losep = LoadTexture("lose1.png");
	GLuint lose = LoadTexture("lose2.png");

	GLuint spriteSheetTexture = LoadTexture("spritesheet_rgba.png");
	SheetSprite origChar(spriteSheetTexture, 441.0f / 692.0f, 2.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite lvlUP1(spriteSheetTexture, 441.0f / 692.0f, 48.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite lvlUP3(spriteSheetTexture, 441.0f / 692.0f, 71.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite lvlUP2(spriteSheetTexture, 441.0f / 692.0f, 25.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite red(spriteSheetTexture, 602.0f / 692.0f, 671.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite blue(spriteSheetTexture, 625.0f / 692.0f, 671.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);
	SheetSprite yellow(spriteSheetTexture, 602.0f / 692.0f, 648.0f / 692.0f, 17.0f / 692.0f, 21.0f / 692.0f, 1.0);

	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	float lastFrameTicks = 0.0f;
	float lose1 = 0.0;
	float lose2 = 0.0;
	bool x = true;

	Entity p(origChar);
	p.x = 0.0;
	p.y = 0.0;
	//int pstat = 0;

	Entity e;
	e.x = 0.0;
	e.y = 0.0;

	int counter = 0;
	int spacer = 0;
	float angle = 0.0;
	float scale = 0.0;
	float scalep = 1.0;

	int counter2 = 0;
	int spacer2 = 0;
	float angle2 = 0.0;
	float scale2 = 0.0;
	float scalep2 = 1.0;

	int counter3 = 0;
	int spacer3 = 0;
	float angle3 = 0.0;
	float scale3 = 0.0;
	float scalep3 = 1.0;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	key = Mix_LoadWAV("key.wav");
	jump = Mix_LoadWAV("jump.wav");
	diamond = Mix_LoadWAV("diamond.wav");
	enemydie = Mix_LoadWAV("enemydie.wav");
	shoot = Mix_LoadWAV("shoot.wav");
	gethit = Mix_LoadWAV("gethit.wav");
	fallhit = Mix_LoadWAV("fallhit.wav");
	winlvl = Mix_LoadWAV("winlvl.wav");
	win = Mix_LoadMUS("win.mp3");
	loser = Mix_LoadMUS("lose.mp3");
	titlee = Mix_LoadMUS("title.mp3");
	lvl1 = Mix_LoadMUS("lvl1.mp3");
	lvl2 = Mix_LoadMUS("lvl2.mp3");
	lvl3 = Mix_LoadMUS("lvl3.mp3");
	transs = Mix_LoadMUS("transs.mp3");

	switchState(STATE_GAME_TITLE);

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
		viewMatrix.identity();

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		switch (state){
		case(STATE_GAME_TITLE) :
			if (keys[SDL_SCANCODE_RETURN]) {
				switchState(STATE_GAME_INS);
			}
							   break;
		case(STATE_GAME_INS) :
			if (keys[SDL_SCANCODE_SPACE]){
				//state = STATE_GAME_LVL1;
				switchState(STATE_GAME_LVL1);//for now
			}
							 break;

		case(STATE_GAME_LVL1) : 
			if (keys[SDL_SCANCODE_Y]){
				switchState(STATE_GAME_LVL2);//for now
			}
			break;
		case(STATE_GAME_LVL2) :
			if (keys[SDL_SCANCODE_U]){
				switchState(STATE_GAME_LVL3);//for now
			}
			break;
		case(STATE_GAME_LVL3) :
			if (keys[SDL_SCANCODE_I]){
				switchState(STATE_GAME_T1);//for now
			}
			break;
		case(STATE_GAME_WIN) :
			//Mix_PlayMusic(win, -1);
			if (keys[SDL_SCANCODE_M]){
				switchState(STATE_GAME_TITLE);
			}
							 break;
		case(STATE_GAME_LOSE) :
			if (keys[SDL_SCANCODE_X]){
				switchState(STATE_GAME_WIN);//FOR NOW
			}
							  break;
		case(STATE_GAME_T1):
			if (keys[SDL_SCANCODE_RETURN]){
				switchState(STATE_GAME_T2);//for now
			}
		case(STATE_GAME_T2) :
			if (keys[SDL_SCANCODE_A]){
				switchState(STATE_GAME_T3);//for now
			}
		case(STATE_GAME_T3) :
			if (keys[SDL_SCANCODE_B]){
				switchState(STATE_GAME_LOSE);//for now
			}

		}

		switch (state) {
		case STATE_GAME_TITLE:

			glClearColor((95.0f / 255.0f), (128.0f / 255.0f), (162.0f / 255.0f), 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			modelMatrix.identity();
			modelMatrix.Translate(-1.5, 1.5, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "THE HERO", 0.4f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(2.75, 0.175, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Hit", 0.175f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(2.6, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Enter", 0.175f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(2.85, -0.175, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "To", 0.175f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(2.4, -0.35, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Play...", 0.175f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Scale(4.5, 3.2, 1.0);
			modelMatrix.Translate(0.0, -0.2, 0.0);
			program.setModelMatrix(modelMatrix);

			draw(&program, title);

			break;
		case STATE_GAME_INS:
			modelMatrix.identity();
			modelMatrix.Translate(-1.0, 1.9, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "How to Play", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.0, 1.4, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Use the arrow keys", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.0, 1.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "to move around", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, 0.6, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Kill all the enemies", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, 0.4, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "and collect all keys", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, 0.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "to go to next level", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -0.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "To kill the enemies,", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -0.4, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "you need to get powers", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -1.0, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Collect all gems", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -1.2, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "to get powers", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.8, -1.7, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Hit space to fire power", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-1.5, -1.95, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "Hit SPACE to Start Playing...", 0.15f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Scale(0.8, 0.8, 1.0);
			modelMatrix.Translate(-3.5, 1.6, 0.0);
			program.setModelMatrix(modelMatrix);

			draw(&program, arrowkeys);

			modelMatrix.identity();
			modelMatrix.Scale(1.35, 1.35, 1.0);
			modelMatrix.Translate(-2.0, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);

			draw(&program, obj);

			modelMatrix.identity();
			modelMatrix.Scale(1.0, 1.2, 1.0);
			modelMatrix.Translate(-2.75, -1.2, 0.0);
			program.setModelMatrix(modelMatrix);

			draw(&program, gems);

			break;
			/*case STATE_GAME_LVL1:
			//code here

			break;
			case STATE_GAME_LVL2:
			//code here

			break;
			case STATE_GAME_LVL3:
			//code here

			break;
			*/
		case STATE_GAME_WIN:
			glClearColor((201.0f / 255.0f), (242.0f / 255.0f), (246.0f / 255.0f), 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			modelMatrix.identity();
			modelMatrix.Translate(-3.335, 1.7, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "CONGRATS!! YOU WIN!!", 0.35f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.75, -1.7, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "HIT M TO GO TO MAIN MENU", 0.2f, 0.0f);

			if (x){
				x = false;

				modelMatrix.identity();
				modelMatrix.Scale(1.8, 1.6, 1.0);
				modelMatrix.Translate(0.0, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, back1);

				modelMatrix.identity();
				modelMatrix.Translate(-2.5, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, wiggle1);

				modelMatrix.identity();
				modelMatrix.Translate(2.5, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, wiggle1);

			}
			else{
				x = true;
				modelMatrix.identity();
				modelMatrix.Scale(1.8, 1.6, 1.0);
				modelMatrix.Translate(0.0, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, back2);

				modelMatrix.identity();
				modelMatrix.Translate(-2.5, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, wiggle2);

				modelMatrix.identity();
				modelMatrix.Translate(2.5, 0.0, 0.0);
				program.setModelMatrix(modelMatrix);

				draw(&program, wiggle2);

			}

			break;

		case STATE_GAME_LOSE:
			glClearColor((201.0f / 255.0f), (242.0f / 255.0f), (246.0f / 255.0f), 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			modelMatrix.identity();
			modelMatrix.Translate(-1.6, 1.7, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "YOU LOSE!!", 0.4f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Translate(-2.75, -1.7, 0.0);
			program.setModelMatrix(modelMatrix);
			DrawText(&program, font, "HIT M TO GO TO MAIN MENU", 0.2f, 0.0f);

			modelMatrix.identity();
			modelMatrix.Scale(1.8, 1.6, 1.0);
			modelMatrix.Translate(0.0, 0.0, 0.0);
			program.setModelMatrix(modelMatrix);

			draw(&program, losep);

			modelMatrix.identity();
			modelMatrix.Scale(0.5, 0.5, 1.0);
			lose1 += 5.0 * elapsed;
			modelMatrix.Translate(0.0, 2.25, 0.0);
			modelMatrix.Rotate(lose1);
			program.setModelMatrix(modelMatrix);

			draw(&program, lose);

			modelMatrix.identity();
			modelMatrix.Scale(0.5, 0.5, 1.0);
			lose2 -= 5.0 * elapsed;
			modelMatrix.Translate(0.0, -2.25, 0.0);
			modelMatrix.Rotate(lose2);
			program.setModelMatrix(modelMatrix);

			draw(&program, lose);

			break;
		case STATE_GAME_T1:
			glClearColor(1.0, 1.0, 1.0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			if (counter < 10000){
				p.sprite = origChar;
				e.x = 0.0;
				e.y = 0.0;
			}
			counter++;
			if (counter < 5000){
				modelMatrix.identity();
				modelMatrix.Translate(p.x, p.y, 0.0);
				program.setModelMatrix(modelMatrix);
				p.sprite.Draw(&program);

			}
			else if (counter < 15000){
				modelMatrix.identity();
				modelMatrix.Translate(-3.0, 1.9, 0.0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, font, "Keep Hitting Space to Transform", 0.2f, 0.0f);
				if (keys[SDL_SCANCODE_SPACE]){
					spacer++;
					if (spacer < 11000){
						if (p.x == 0.0){
							p.x = 0.0225;
							p.y = 0.0225;
							viewMatrix.identity();
							viewMatrix.Translate(0.3, 0.0, 0.0);
						}
						else{
							p.x = 0.0;
							p.y = 0.0;
							viewMatrix.identity();
							viewMatrix.Translate(-0.3, 0.0, 0.0);
						}
					}
				}
			}
			else if (counter < 25000 && spacer >=25){
				e.sprite = red;
				scale += 0.001;
			}
			else if (counter > 31000 && counter < 43000 && spacer >=25){
				scale -= 0.001;
				if (scale <= 0.1){
					e.x = 10.0;
					e.y = 10.0;
				}
				p.sprite = lvlUP1;
			}
			else if (counter<49000 && spacer >=25){
				if (p.x == 0.0){
					p.x = 0.0225;
					p.y = 0.0225;
				}
				else{
					p.x = 0.0;
					p.y = 0.0;
				}
				if (counter > 48000 && spacer >=25){
					scalep += 0.005;
				}
			}
			else if (spacer <25 && counter < 15001){
				counter = 0; 
				switchState(STATE_GAME_T1);
			}
			modelMatrix.identity();
			modelMatrix.Scale(scalep, scalep, 1.0);
			modelMatrix.Translate(p.x, p.y, 0.0);
			program.setModelMatrix(modelMatrix);
			p.sprite.Draw(&program);

			modelMatrix.identity();
			angle += 10.0 * elapsed;
			modelMatrix.Scale(scale, scale, 1.0);
			modelMatrix.Translate(e.x, e.y, 0.0);
			modelMatrix.Rotate(angle);
			program.setModelMatrix(modelMatrix);
			e.sprite.Draw(&program);
			break;
		case STATE_GAME_T2:
			glClearColor(1.0, 1.0, 1.0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			if (counter2 < 10000){
				p.sprite = origChar;
				e.x = 0.0;
				e.y = 0.0;
			}
			counter2++;
			if (counter2 < 5000){
				modelMatrix.identity();
				modelMatrix.Translate(p.x, p.y, 0.0);
				program.setModelMatrix(modelMatrix);
				p.sprite.Draw(&program);

			}
			else if (counter2 < 15000){
				modelMatrix.identity();
				modelMatrix.Translate(-3.0, 1.9, 0.0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, font, "Keep Hitting Space to Transform", 0.2f, 0.0f);
				if (keys[SDL_SCANCODE_SPACE]){
					spacer++;
					if (spacer2 < 11000){
						if (p.x == 0.0){
							p.x = 0.0225;
							p.y = 0.0225;
							viewMatrix.identity();
							viewMatrix.Translate(0.3, 0.0, 0.0);
						}
						else{
							p.x = 0.0;
							p.y = 0.0;
							viewMatrix.identity();
							viewMatrix.Translate(-0.3, 0.0, 0.0);
						}
					}
				}
			}
			else if (counter2 < 25000){
				e.sprite = blue;
				scale2 += 0.001;
			}
			else if (counter2 > 31000 && counter2 < 43000){
				scale2 -= 0.001;
				if (scale2 <= 0.1){
					e.x = 10.0;
					e.y = 10.0;
				}
				p.sprite = lvlUP2;
			}
			else if (counter2<49000){
				if (p.x == 0.0){
					p.x = 0.0225;
					p.y = 0.0225;
				}
				else{
					p.x = 0.0;
					p.y = 0.0;
				}
				if (counter2 > 48000){
					scalep2 += 0.005;
				}
			}
			else if (spacer2 <25 && counter2 < 15001){
				counter2 = 0;
				switchState(STATE_GAME_T2);
			}
			modelMatrix.identity();
			modelMatrix.Scale(scalep2, scalep2, 1.0);
			modelMatrix.Translate(p.x, p.y, 0.0);
			program.setModelMatrix(modelMatrix);
			p.sprite.Draw(&program);

			modelMatrix.identity();
			angle2 += 10.0 * elapsed;
			modelMatrix.Scale(scale2, scale2, 1.0);
			modelMatrix.Translate(e.x, e.y, 0.0);
			modelMatrix.Rotate(angle2);
			program.setModelMatrix(modelMatrix);
			e.sprite.Draw(&program);

			break;
		case STATE_GAME_T3:
			glClearColor(1.0, 1.0, 1.0, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			if (counter3 < 10000){
				p.sprite = origChar;
				e.x = 0.0;
				e.y = 0.0;
			}
			counter3++;
			if (counter3 < 5000){
				modelMatrix.identity();
				modelMatrix.Translate(p.x, p.y, 0.0);
				program.setModelMatrix(modelMatrix);
				p.sprite.Draw(&program);

			}
			else if (counter3 < 15000){
				modelMatrix.identity();
				modelMatrix.Translate(-3.0, 1.9, 0.0);
				program.setModelMatrix(modelMatrix);
				DrawText(&program, font, "Keep Hitting Space to Transform", 0.2f, 0.0f);
				if (keys[SDL_SCANCODE_SPACE]){
					spacer++;
					if (spacer3 < 11000){
						if (p.x == 0.0){
							p.x = 0.0225;
							p.y = 0.0225;
							viewMatrix.identity();
							viewMatrix.Translate(0.3, 0.0, 0.0);
						}
						else{
							p.x = 0.0;
							p.y = 0.0;
							viewMatrix.identity();
							viewMatrix.Translate(-0.3, 0.0, 0.0);
						}
					}
				}
			}
			else if (counter3 < 25000){
				e.sprite = yellow;
				scale3 += 0.001;
			}
			else if (counter3 > 31000 && counter3 < 43000){
				scale3 -= 0.001;
				if (scale3 <= 0.1){
					e.x = 10.0;
					e.y = 10.0;
				}
				p.sprite = lvlUP3;
			}
			else if (counter3<49000){
				if (p.x == 0.0){
					p.x = 0.0225;
					p.y = 0.0225;
				}
				else{
					p.x = 0.0;
					p.y = 0.0;
				}
				if (counter3 > 48000){
					scalep3 += 0.005;
				}
			}
			else if (spacer3 <25 && counter3 < 15001){
				counter3 = 0;
				switchState(STATE_GAME_T3);

			}
			modelMatrix.identity();
			modelMatrix.Scale(scalep3, scalep3, 1.0);
			modelMatrix.Translate(p.x, p.y, 0.0);
			program.setModelMatrix(modelMatrix);
			p.sprite.Draw(&program);

			modelMatrix.identity();
			angle3 += 10.0 * elapsed;
			modelMatrix.Scale(scale3, scale3, 1.0);
			modelMatrix.Translate(e.x, e.y, 0.0);
			modelMatrix.Rotate(angle3);
			program.setModelMatrix(modelMatrix);
			e.sprite.Draw(&program);
			break;
		}

		SDL_GL_SwapWindow(displayWindow);
	}

	Mix_FreeChunk(key);
	Mix_FreeChunk(jump);
	Mix_FreeChunk(diamond);
	Mix_FreeChunk(enemydie);
	Mix_FreeChunk(shoot);
	Mix_FreeChunk(gethit);
	Mix_FreeChunk(fallhit);
	Mix_FreeChunk(winlvl);
	Mix_FreeMusic(win);
	Mix_FreeMusic(loser);
	Mix_FreeMusic(titlee);
	Mix_FreeMusic(lvl1);
	Mix_FreeMusic(lvl2);
	Mix_FreeMusic(lvl3);
	Mix_FreeMusic(transs);

	SDL_Quit();
	return 0;
}
