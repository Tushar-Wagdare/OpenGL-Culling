#include <windows.h>
#include<windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#include <gl/GL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "WindowManager.h"
#include "Logger.h"
#include "Timer.h"
#include "camera.h"
#include "Shader.h"



//*** Globle Function Declarations ***
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


//*** Global Variable Declaration ***
WindowManager* pWindow = NULL;
Camera* camera = NULL;
float lastX = WindowManager::SCR_WIDTH / 2.0f;
float lastY = WindowManager::SCR_HEIGHT / 2.0f;
bool firstMouse = true;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


GLuint vaoPiramid = 0;
GLuint vbo_position_piramid = 0;
GLuint vbo_color_piramid = 0;
GLuint mvpMatrixUniform = 0;
glm::mat4 perspectiveProjectionMatrix;
GLfloat anglePiramid = 0.0f;




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    BOOL LoadGLTexture(GLuint * texture, GLsizei width, GLsizei height, void* data);
	pWindow = new WindowManager();
	MSG msg = { 0 };
	camera = new Camera();



	Logger::Init();

	TIMER_INIT("Window");
	pWindow->initialize();
	TIMER_END(); 
	LOG_INFO("Window Initialized in %.6f seconds", TIMER_GET("Window"));



	///======================== OpenGL ==============================///
	Shader ourShader("shaders/camera.vs", "shaders/camera.fs");

	//Declare Position And Color Arrays
	const GLfloat piramid_position[] =
	{
		0.0f,0.5f,0.0f,
		-0.5f,-0.5f,0.5f,
		0.5f,-0.5f,0.5f,

		0.0f,0.5f,0.0f,
		0.5f,-0.5f,0.5f,
		0.5f,-0.5f,-0.5f,

		0.0f,0.5f,0.0f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,

		0.0f,0.5f,0.0f,
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,0.5f
	};

	const GLfloat piramid_color[] =
	{
				1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f

	};

	/// Pyramid
	//VAO
	glGenVertexArrays(1, &vaoPiramid);
	glBindVertexArray(vaoPiramid);
	//VBO for Position 
	glGenBuffers(1, &vbo_position_piramid);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_position_piramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(piramid_position), piramid_position, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//VBO for color
	glGenBuffers(1, &vbo_color_piramid);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_color_piramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(piramid_color), piramid_color, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	

	//07 - 2nd step Enabling Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	//08 - Set the Clear Color of Window To Blue
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	ourShader.use();
	glm::mat4 perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)WindowManager::SCR_WIDTH / (float)WindowManager::SCR_HEIGHT, 0.1f, 100.0f);
	ourShader.setMat4("uMVPMatrix", perspectiveProjectionMatrix);

	// world space positions of our cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};




	//*** Game LOOP ***
	while (pWindow->isRunning == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				pWindow->isRunning = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// per-frame time logic
			// --------------------
			float currentFrame = static_cast<float>(Timer::getAppRunTime());
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;


			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ourShader.use();

			
			glm::mat4 modelMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);
			glm::mat4 translationMatrix = glm::mat4(1.0f);
			glm::mat4 rotationMatrix = glm::mat4(1.0f);
			glm::mat4 modelViewProjectionMatrix = glm::mat4(1.0f);

			
			// camera/view transformation
			float radius = 10.0f;
			float camX = static_cast<float>(sin(anglePiramid*0.05f) * radius);
			float camZ = static_cast<float>(cos(anglePiramid*0.05f) * radius);
			//viewMatrix = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			viewMatrix = camera->GetViewMatrix();
			

			// render boxes
			glBindVertexArray(vaoPiramid);
			for (unsigned int i = 0; i < 10; i++)
			{
				// calculate the model matrix for each object and pass it to shader before drawing
				modelMatrix = glm::translate(modelMatrix, cubePositions[i]);
				float angle = 20.0f * i;
				modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
				ourShader.setMat4("model", modelMatrix);

				modelViewProjectionMatrix = perspectiveProjectionMatrix * viewMatrix * modelMatrix;
				ourShader.setMat4("uMVPMatrix", modelViewProjectionMatrix);

				glDrawArrays(GL_TRIANGLES, 0, 12);
			}

			
			
			glBindVertexArray(0);
			glUseProgram(0);
            
			
            pWindow->swapDisplayBuffer();

			///================== UPDATE =======================//
			anglePiramid = anglePiramid + 0.01f;
			
		}
	}
	//Delete VBO Color
	if (vbo_color_piramid)
	{
		glDeleteBuffers(1, &vbo_color_piramid);
		vbo_color_piramid = 0;
	}


	//Delete VBO Position
	if (vbo_position_piramid)
	{
		glDeleteBuffers(1, &vbo_position_piramid);
		vbo_position_piramid = 0;
	}


	//Delete VAO
	if (vaoPiramid)
	{
		glDeleteVertexArrays(1, &vaoPiramid);
		vaoPiramid = 0;
	}


	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//*** Function Declaration ***
	void ToggleFullscreen(void);
	void resize(int, int);


	//*** Code ***
	switch (iMsg)
	{
	case WM_SETFOCUS:
		break;

	case WM_KILLFOCUS:
		break;

	case WM_SIZE:
        glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_MOUSEMOVE:
		{
			float xpos = static_cast<float>(GET_X_LPARAM(lParam));
			float ypos = static_cast<float>(GET_Y_LPARAM(lParam));

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

			lastX = xpos;
			lastY = ypos;
			camera->ProcessMouseMovement(xoffset, yoffset);
		}
		
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_CHAR:
		switch (LOWORD(wParam))
		{
		case 'F':
		case 'f':
			
			break;

		case 'W':
		case 'w':
			camera->ProcessKeyboard(FORWARD, deltaTime * 0.1f);
			break;

		case 'A':
		case 'a':
			camera->ProcessKeyboard(LEFT, deltaTime * 0.1f);
			break;

		case 'S':
		case 's':
			camera->ProcessKeyboard(BACKWARD, deltaTime * 0.1f);
			break;

		case 'D':
		case 'd':
			camera->ProcessKeyboard(RIGHT, deltaTime * 0.1f);
			break;

		
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}


	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}