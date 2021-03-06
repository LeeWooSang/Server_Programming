#include "stdafx.h"
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"
#include "Scene.h"

Scene* g_pScene{ nullptr };
DWORD g_prevTime = 0;

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.f, 0.3f, 0.3f, 1.f);

	DWORD currTime = timeGetTime();
	float elapsedTime = currTime - g_prevTime;
	g_prevTime = currTime;

	if (g_pScene)
	{
		g_pScene->Update(elapsedTime);
		g_pScene->Render();
	}

	glutSwapBuffers();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	RenderScene();
}

void KeyInput(unsigned char key, int x, int y)
{
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();
	else
		RenderScene();
}

void SpecialKeyInput(int key, int x, int y)
{
	RenderScene();
}

int main(int argc, char **argv)
{
	// Initialize GL things
	glutInit(&argc, argv);
	// ���� ���� Ż��
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("GameServer_Programming");

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
		cout << " GLEW Version is 3.0" << endl;
	else
		cout << "GLEW 3.0 not supported" << endl;
	
	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);

	//Initialize Renderer
   g_pScene = new Scene(WIDTH, HEIGHT);
   if (!g_pScene->Initialize())
	   glutLeaveMainLoop();

	g_prevTime = timeGetTime();

	glutMainLoop();

	delete g_pScene;
}

