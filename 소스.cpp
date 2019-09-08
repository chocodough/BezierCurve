#include <stdlib.h>
#include <stdio.h>
#include <glut.h>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>

/*
* Description : 지정한 차수의 베지어 곡선을 그린다
* Author : 황녹운
* Since : 2016.12.04
*/

using namespace std;

int Width = 600, Height = 600;
int k = 5; //차수
bool click = false;
bool rclick = false;

double zoom = 1;
double cx = 0;
double cy = 0;

struct point
{
	double x, y;
};  // 정점 구조체 정의

point ts; // T의 시작점

int max(int x, int y);
void Render();
void Reshape(int w, int h);                         // 변경된 창 크기에 화면을 맞춤
int Combination(int a, int b);                      // 조합 aCb를 계산하는 함수
double bezier_term(double t, int i, int x, int n);  // 베지어 곡선 계산식의 각 항을 계산
point GetBezierPoint(double t);       
void DrawBezierLine(int k);                         // n번째 점까지의 베지어 곡선을 그림
void DrawControlPoligon(int n);                     // ControlPoligon을 그림
void MouseFunc(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void KeyBoard(unsigned char key, int x, int y);

vector<point> P;   //정점들을 저장할 벡터
vector<point> L;   //그린 베지어곡선의 좌표들
vector<point> T;   //현재 그리는 베지어곡선의 정점들

void SetupViewTranform()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(cx, cy, -10.0);
	glScalef(zoom, zoom, 0);
}
void Reshape(int w, int h)  // 변경된 창 크기에 화면을 맞춤
{
	glViewport(0, 0, w, h);
	Width = w;
	Height = h;
}

void SetupViewVolume()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0, 600.0, 0.0, 600.0, -10, 10);
	glOrtho(-300.0, 300.0, -300.0, 300.0, -10, 10);
	//gluPerspective(45, 1, zoom, 2*zoom);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(Width, Height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutCreateWindow("Bezier Curve");

	
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	
	glutMouseFunc(MouseFunc);
	glutPassiveMotionFunc(MouseMotion);

	glutKeyboardFunc(KeyBoard);

	glutMainLoop();

	return 0;
}



void Render()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	SetupViewTranform();
	SetupViewVolume();
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(5.0, 5.0, 0.0);

	glColor3f(1, 0, 0);
	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	vector<point>::iterator iter;
	for (iter = P.begin(); iter != P.end(); iter++)
	{
		glVertex3f(iter->x, iter->y, 0.0);
	}         // 점 P0 부터 Pn까지 이어진 직선을 그림
	glEnd();

	DrawBezierLine(k);        // 베지어 곡선을 그림

	bool jp = false;
	glColor3f(0, 0, 0);
	glLineWidth(3);
	glBegin(GL_LINE_STRIP);
	vector<point>::iterator iter2;
	for (iter2 = L.begin(); iter2 != L.end(); iter2++)
	{
		if (iter2->x == 10000)
		{
			jp = true;
			glEnd();
		}
		else if (jp)
		{
			glBegin(GL_LINE_STRIP);
			glVertex3f(iter2->x, iter2->y, 0.0);
			jp = false;
		}
		else
		glVertex3f(iter2->x, iter2->y, 0.0);
	}         // 점 P0 부터 Pn까지 이어진 직선을 그림
	if(!jp)
		glEnd();

	DrawControlPoligon(k);    // ControlPoligon을 그림

	glutSwapBuffers();
}

/*
 * 조합 aCb를 계산한다 
 * 참고) aCb = a! / ((a-b)! * b!)
 */
int Combination(int a, int b) {  
	int temp = 1;
	for (int i = a; i > b; i--)
		temp *= i;
	for (int i = (a - b); i > 0; i--)
		temp /= i;
	if (b = 0)
		temp = 1;
	return temp;
}  

/*
 * 베지어 곡선 계산식 각 항의 계수를 구한다
 */
double bezier_term(double t, int i, int x, int n)  // 베지어 곡선 계산식의 각 항을 계산
{
	return Combination(n - 1, i) * pow(t, i) * pow(1 - t,n - 1 - i) * x;
}

/*
 * 베지어곡선에서 t시점의 좌표를 계산한다
 */
point GetBezierPoint(double t)
{	
	point tempPoint = { 0,0 };

	for (int i = 0; i < T.size(); i++)
		tempPoint.x += bezier_term(t, i,T.at(i).x, T.size());  // x좌표 계산	
	for (int i = 0; i < T.size(); i++)
		tempPoint.y += bezier_term(t, i, T.at(i).y, T.size());  // y좌표 계산	
	return tempPoint;
}

/*
 * 베지어 곡선을 그린다
 */
void DrawBezierLine(int k)  
{
	point temp;
	glColor3f(0, 1, 0);
	glBegin(GL_LINE_STRIP);
	for (double t = 0.001; t < 1; t = t + 0.001)
	{
		temp = GetBezierPoint(t);
		glVertex3f(temp.x,temp.y, 0);
		if (rclick)
			L.push_back(temp);
		else if (click && (t < 1.0/(double)max(1,T.size()-1) ))
		{
			printf("!!");
			if (T.size() >= k + 1)  //차수가 같아질때부턴 저장
			{
				L.push_back(temp);
			}
		}
	}
	if (rclick)
	{
		point jump = { 10000,10000 };
		L.push_back(jump);
	}
	glEnd();
	if (click)
		if(L.size() != 0)
			ts = L.back();

	click = false;
	if (rclick)
		T.clear();
	rclick = false;
}

/*
 * ControlPoligon을 그린다
 */
void DrawControlPoligon(int n)  
{
	// 1/2 지점까지의 베지어 곡선을 그림
	glColor3f(0, 0, 0);
	glPointSize(2);
	for (double t = 0.001; t < 0.5; t = t + 0.001)
	{
		glBegin(GL_POINTS);
		glVertex3f(GetBezierPoint(t, n).x, GetBezierPoint(t, n).y, 0);
		glEnd();
	}         

	// 각 점 (P0, P1... Pn)까지 계산한 베지어 곡선의 중점(t = 1/2)들을 연결함
	glLineWidth(2);
	for (int i = 1; i < n; i++)
	{
		glBegin(GL_LINES);
		glVertex3f(GetBezierPoint(0.5, i).x, GetBezierPoint(0.5, i).y,0);
		glVertex3f(GetBezierPoint(0.5, i + 1).x, GetBezierPoint(0.5, i + 1).y, 0);
		glEnd();
	}     
	return;
}

void MouseFunc(int button, int state, int x, int y)
{
	x = x - 300;
	y = Height - y - 300; 
	
	point Mouse;
	Mouse.x = x;
	Mouse.y = y;
	if(button == 0 && state == 0)
	{
		click = true;
		P.push_back(Mouse);
		T.push_back(Mouse);
		if (T.size() - 1 > k)   // T가 차수보다 커지면 
		{
			T.erase(T.begin());
			T.at(0) = ts;
		}
		Render();
	}	

	if (button == 2 && state == 0)
	{
		rclick = true;
		P.push_back(Mouse);
		T.push_back(Mouse);
		if (T.size() - 1 > k)   // T가 차수보다 커지면 
		{
			T.erase(T.begin());
			T.at(0) = ts;
		}
		Render();
	}

}

void MouseMotion(int x, int y)  //마우스가 움직이는 동안
{
	x = x - 300;
	y = Height - y - 300;

	point Mouse;

	Mouse.x = x;
	Mouse.y = y;
	P.push_back(Mouse);
	T.push_back(Mouse);
	
	Render();
	

	P.pop_back();
	T.pop_back();
	//printf("%d %d\n", x, y);
}

int max(int x, int y)
{
	if (x >= y)
		return x;
	else
		return y;
}

/*
 * 키보드 동작을 감지한다
 */
void KeyBoard(unsigned char key, int x, int y)
{
	if (key == 'q')
		zoom += 0.1;
	else if (key == 'e')
		zoom -= 0.1;
	else if (key == 'd')
		cx -= 20;
	else if (key == 'a')
		cx += 20;
	else if (key == 'w')
		cy -= 20;
	else if (key == 's')
		cy += 20;
	Render();
}