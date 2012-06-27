#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include <GLUT/glut.h>
#include "BMPLoader.h"

// for all
int windowWidth		= 800;
int windowHeight	= 600;

// for OpenCV
const char*		CV_WINDOW_NAME		= "Vision";
CvMemStorage*	storage				= NULL;
CvCapture *		camera				= NULL;
float			leftHandRectX		= 0;
float			leftHandRectY		= 0;
float			rightHandRectX		= 0;
float			rightHandRectY		= 0;
const int		SKIN_AREA_THRESHOLD	= 10000;


// for OpenGL
#define NUM_PICTURE 1
const char* GL_WINDOW_NAME	= "Graphics";
GLuint	texture[NUM_PICTURE];
GLfloat lightAmbient[]		= {1.0, 1.0, 1.0, 1.0};		// 环境光
GLfloat lightDiffuse[]		= {1.0, 1.0, 1.0, 1.0};		// 散射光
GLfloat lightPosition[]		= {0.0, 0.0, 0.0, 1.0};		// 光源坐标
int glWindow = 0;

void init(int argc, const char * argv[]);
void loadGLTexture(std::string fileName, int planet);
void drawObject(GLfloat x, GLfloat y, GLfloat z, GLdouble size);
void display(void);
void reshape(int w, int h);
void timer(int value);
void done();
void keyboard(unsigned char key, int x, int y);
void detectPosition();

int main(int argc, const char * argv[]) {
	init(argc, argv);
	glutMainLoop();
	done();
	
    return 0;
}

void init(int argc, const char * argv[]) {
	// init for CV
	cvNamedWindow(CV_WINDOW_NAME, CV_WINDOW_AUTOSIZE);
	camera	= cvCreateCameraCapture(CV_CAP_ANY);
	storage	= cvCreateMemStorage(0);
	
	// init for GL
	glutInit(& argc, (char **) argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(windowWidth, windowHeight); 
	glutInitWindowPosition(0, 0);
	glWindow = glutCreateWindow(GL_WINDOW_NAME);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(1, timer, 0);
	
	// 光照设置
	glEnable(GL_LIGHTING);								// 打开光照
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);		// 设置光照0的环境光
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);		// 设置光照0的散射光
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);	// 放置光照0
	glEnable(GL_LIGHT0);								// 打开光照0
	
	glEnable(GL_CULL_FACE);								// 打开背面裁剪
	glEnable(GL_DEPTH_TEST);							// 打开深度检测
	glEnable(GL_TEXTURE_2D);							// 打开2D贴图
	
	// 载入各种材质
	glGenTextures(NUM_PICTURE, texture);
	loadGLTexture("picture.bmp", 0);
}

// 加载bmp材质
void loadGLTexture(std::string fileName, int index) {
	BMPClass bmp;
	BMPLoad(fileName, bmp);
	
	glBindTexture(GL_TEXTURE_2D, texture[index]);
	// Generate The Texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);					// 线性过滤
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, bmp.width, bmp.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bmp.bytes);
}

void drawObject(GLfloat x, GLfloat y, GLfloat z, int textureIndex) {
	glPushMatrix();
	
	x = x - (leftHandRectX + rightHandRectX) / 2.0 * windowWidth;
	y = y - (leftHandRectY + rightHandRectY) / 2.0 * windowHeight;
	float d = sqrt((leftHandRectX - rightHandRectX) * (leftHandRectX - rightHandRectX) + (leftHandRectY - rightHandRectY) * (leftHandRectY - rightHandRectY));
	z = (1.0 - d) * z;
	GLfloat rx = cvFastArctan(y, - z);
	GLfloat ry = - cvFastArctan(x, - z);
	GLfloat rz = 0;
	if (rightHandRectX - leftHandRectX != 0) {
		rz = cvFastArctan(rightHandRectY - leftHandRectY, rightHandRectX - leftHandRectX);
	}
	
	glRotatef(rx, 1, 0, 0);
	glRotatef(ry, 0, 1, 0);
	glRotatef(rz, 0, 0, 1);
	glTranslatef(x, y, z);
	
	glBindTexture(GL_TEXTURE_2D, texture[textureIndex]);
	
	float a = 400.0;
	float b = a * 0.75;
	glBegin(GL_QUADS);						// Draw A Quad
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-a, -b, 0.0f);	// Bottom Left Of The Texture and Quad
		glTexCoord2f(1.0f, 0.0f); glVertex3f( a, -b, 0.0f);	// Bottom Right Of The Texture and Quad
		glTexCoord2f(1.0f, 1.0f); glVertex3f( a,  b, 0.0f);	// Top Right Of The Texture and Quad
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-a,  b, 0.0f);	// Top Left Of The Texture and Quad
	glEnd();
	
	glPopMatrix();
}

void display(void) {	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	detectPosition();
	
	drawObject(0, 0, -1000, 0);
	
	glutSwapBuffers();									// 交换双缓冲区
}

void reshape(int width, int height) {
	glViewport(0, 0, (GLsizei) width, (GLsizei) height);				// 视口大小
	glMatrixMode(GL_PROJECTION);										// 透视投影
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat) width / (GLfloat) height, 1, 10240);	// 投影参数
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void timer(int value) {
	display();
	glutTimerFunc(20, timer, 0);
}

void done() {
	cvReleaseCapture(& camera);
	cvReleaseMemStorage(& storage);
	cvDestroyWindow(CV_WINDOW_NAME);
	glutDestroyWindow(glWindow);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
			exit(0);
			break;
		default:
			break;
	}
}

// detect hands position
void detectPosition() {
	CvSeq*			contour			= NULL;
	IplImage*		image			= cvQueryFrame(camera);
	IplImage*		filterMask		= cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, 1);
	CvRect			leftHandRect	= cvRect(image->width / 2.0, image->height / 2.0, 0, 0);
	CvRect			rightHandRect	= cvRect(image->width / 2.0, image->height / 2.0, 0, 0);
	
	CvAdaptiveSkinDetector filter(1, CvAdaptiveSkinDetector::MORPHING_METHOD_ERODE_DILATE);
	filter.process(image, filterMask);
	cvFindContours(filterMask, storage, & contour, sizeof(CvContour), CV_RETR_EXTERNAL);
	for (; contour != NULL; contour = contour->h_next) {
		CvContour* cc = (CvContour*) contour;
		if (cc->rect.width * cc->rect.height > SKIN_AREA_THRESHOLD) {
			if (cc->rect.width * cc->rect.height > rightHandRect.width * rightHandRect.height) {
				rightHandRect = cc->rect;
				cvRectangle(image, 
							cvPoint(cc->rect.x, cc->rect.y), 
							cvPoint(cc->rect.x + cc->rect.width, cc->rect.y + cc->rect.height), 
							CV_RGB(255, 0, 0),
							2
							);
				// 左手为最大区域，右手次之
				if (rightHandRect.width * rightHandRect.height > leftHandRect.width + leftHandRect.height) {
					CvRect tmp = rightHandRect;
					rightHandRect = leftHandRect;
					leftHandRect = tmp;
				}
			}
		}
	}
	
	if (rightHandRect.width * rightHandRect.height == 0) {
		rightHandRect = leftHandRect;
	}
	
	// 如果左手在右手右边，则对调
	if (leftHandRect.x > rightHandRect.x) {
		CvRect tmp = rightHandRect;
		rightHandRect = leftHandRect;
		leftHandRect = tmp;
	}
	
	leftHandRectX = (float) (leftHandRect.x + leftHandRect.width / 2) / (float) image->width - 0.5;
	leftHandRectY = (float) (leftHandRect.y + leftHandRect.height / 2) / (float) image->height - 0.5;
	rightHandRectX = (float) (rightHandRect.x + rightHandRect.width / 2) / (float) image->width - 0.5;
	rightHandRectY = (float) (rightHandRect.y + rightHandRect.height / 2) / (float) image->height - 0.5;
	
	cvFlip(image, NULL, 1);
	cvShowImage(CV_WINDOW_NAME, image);
	cvReleaseImage(& filterMask);
	cvClearMemStorage(storage);
}
