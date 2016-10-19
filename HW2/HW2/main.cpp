//Imran Ahmed
//HW 2 PONG

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

SDL_Window* displayWindow;

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("HW 2 - PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint playerOne = LoadTexture("paddle.png");
	GLuint playerTwo = LoadTexture("paddle.png");
	GLuint ball = LoadTexture("skull.png");
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;
	projectionMatrix.setOrthoProjection(-3.415, 3.415, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	float lastFrameTicks = 0.0f;
	float p1_y = 0.0f;
	float p2_y = 0.0f;
	float ball_x = 0.0f;
	float ball_y = 0.0f;
	float ball_vx = 3.0f;
	float ball_vy = 3.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		const Uint8 *keys = SDL_GetKeyboardState(NULL);

		if (keys[SDL_SCANCODE_UP]){
			if (p1_y > 1.49){
				p1_y += 0;
			}
			else
				p1_y += elapsed * 3;
		}
		else if (keys[SDL_SCANCODE_DOWN]){
			if (p1_y < -1.55){
				p1_y += 0;
			}
			else
				p1_y -= elapsed * 3;
		}


		modelMatrix.identity();
		modelMatrix.Translate(0.0f, p1_y, 0.0f);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glBindTexture(GL_TEXTURE_2D, playerOne);
		float p1[] = { -3.4f, -0.7f, -3.0f, -0.7f, -3.0f, 0.7f, -3.4f, -0.7f, -3.0f, 0.7f, -3.4f, 0.7f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, p1);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);


		if (keys[SDL_SCANCODE_W]){
			if (p2_y > 1.49){
				p2_y += 0;
			}
			else
				p2_y += elapsed * 3;
		}
		else if (keys[SDL_SCANCODE_S]){
			if (p2_y < -1.55){
				p2_y += 0;
			}
			else
				p2_y -= elapsed * 3;
		}

		modelMatrix.identity();
		modelMatrix.Translate(0.0f, p2_y, 0.0f);
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, playerTwo);
		float p2[] = { 3.0f, -0.7f, 3.4f, -0.7f, 3.4f, 0.7f, 3.0f, -0.7f, 3.4f, 0.7f, 3.0f, 0.7f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, p2);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		
		ball_x += ball_vx * elapsed;
		ball_y += ball_vy * elapsed;
		modelMatrix.identity();
		modelMatrix.Translate(ball_x, ball_y, 0.0f);
		program.setModelMatrix(modelMatrix);

		glBindTexture(GL_TEXTURE_2D, ball);
		float ballV[] = { -0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballV);
		glEnableVertexAttribArray(program.positionAttribute);
		float texCoords3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		if ((ball_y + 0.2 >= 1.99 || ball_y - 0.2 <= -2.05))
			ball_vy *= -1.0;

	    else if ((ball_x - 0.2 <= -3.0) && (ball_y - 0.2 <= p1_y + 0.68) && (ball_y + 0.2 >= p1_y - 0.68)){
			ball_vx *= -1.0;
		}

		else if((ball_x + 0.2 >= 3.05) && (ball_y - 0.2 <= p2_y + 0.68) && (ball_y + 0.2 >= p2_y - 0.68)){
			ball_vx *= -1.0;
		}

		else if ((ball_x-0.2 > 3.415 || ball_x+0.2 < -3.415)){
			p1_y = 0.0f;
			p2_y = 0.0f;
		    ball_x = 0.0f;
			ball_y = 0.0f;
			ball_vx = 3.0f;
			ball_vy = 3.0f;
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}


