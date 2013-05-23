/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

 * File Name : Main.cpp

 * Purpose :

 * Creation Date : 07-05-2013

 * Last Modified : Thu 23 May 2013 02:17:50 PM CST

 * Created By : Philip Zhang 

 _._._._._._._._._._._._._._._._._._._._._.*/

#include <GLUtils.h>
#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <stdlib.h>

#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <dirent.h>
#ifndef GLUT_WHEEL_UP
#define GLUT_WHEEL_UP 3
#define GLUT_WHEEL_DOWN 4
#endif
#include "TreeSkeleton.h"

#define MOUSE_DRAG_MAX 0x7FFFFFFF
#define MOUSE_DRAG_MIN 0xFFFFFFFF

const int gb_max_skl_count = 8;
GLFrustum           gb_viewFrustum;				// view frustum
GLMatrixStack       gb_modelViewMatrix;			// modelview matrix stack
GLMatrixStack       gb_projectionMatrix;		// projection matrix stack
GLGeometryTransform gb_transformPipeline;		// transform pipeline object
GLShaderManager     gb_shaderManager;			// shader manager
CTreeSkeleton		gb_treeskl;					// the tree skeleton model
float				gb_max_angle = 5.0;			// max angle to simplify the model
float				gb_eye_radius = 5.0;		// the distance to view the model
float				gb_eye_theta = 0.0;			// theta angle of eye
float				gb_eye_phi = 0.0;			// phi angle of eye
float				gb_eye_height = 0.0;		// height of eye
float				gb_linear_ratio = 0.6;
bool				gb_bTexture = false;		// whether render with texture
bool				gb_bBlack = true;			// background is black or white
bool				gb_bCoord = true;			// whether to draw the axis
bool				gb_bBothDirection = true;	// whether the axis is both directional
bool				gb_bPoints = false;			// whether render in point mode
bool				gb_bVoxel = false;
bool				gb_bOnlyVoxel = false;
GLTriangleBatch     gb_sphereBatch;			// batch of sphere
GLBatch				gb_cubeBatch;			// batch of voxel cube
GLBatch				gb_axisBatches[6];		// batch of axis
char				gb_sklList[gb_max_skl_count][256];		// names of skeleton files
int					gb_sklCount = 0;
int					gb_winWidth, gb_winHeight;

GLuint	normalMapShader;	// The textured diffuse light shader
GLuint  adsPhongShader;		// The ADSPhong shader
// the locations of uniforms, note the *1 indicates it only applies to
// ADSPhong, *2 indicates it only applies to normal map, and ** both.
GLint	locAmbient;			// ** The location of the ambient color
GLint   locDiffuse;			// ** The location of the diffuse color
GLint   locSpecular;		// *1 The location of the specular color
GLint	locLight;			// ** The location of the Light in eye coordinates
GLint	locMVP;				// ** The location of the ModelViewProjection matrix uniform
GLint	locMV;				// ** The location of the ModelView matrix uniform
GLint	locNM;				// ** The location of the Normal matrix uniform
GLint   locColorMap;        // *2 The location of the color map sampler
GLint   locNormalMap;       // *2 The location of the normal map sampler
GLuint  texture[2];         // Two textures, color map and normal map

// Load a TGA as a 2D Texture. Completely initialize the state
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;

	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if(pBits == NULL) 
		return false;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
				 eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if(minFilter == GL_LINEAR_MIPMAP_LINEAR || 
	   minFilter == GL_LINEAR_MIPMAP_NEAREST ||
	   minFilter == GL_NEAREST_MIPMAP_LINEAR ||
	   minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

void FillSklList()
{
	DIR *d;
	struct dirent *dir;
	d = opendir("./skls");
	if(d)
	{
		while((dir = readdir(d)) != NULL)
		{
			if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
			   continue;
			strcpy(gb_sklList[gb_sklCount], "./skls/");
			strcat(gb_sklList[gb_sklCount], dir->d_name);
			gb_sklCount++;
			if(gb_sklCount == gb_max_skl_count)
				return;
		}
	}
}

void SortSklList()
{
	for(int i = 0; i < gb_sklCount - 1; i++)
	{
		for(int j = i + 1; j < gb_sklCount; j++)
		{
			if(strcmp(gb_sklList[j - 1], gb_sklList[j]) > 0)
			{
				// swap the two strings
				char tmp[256];
				strcpy(tmp, gb_sklList[j - 1]);
				strcpy(gb_sklList[j - 1], gb_sklList[j]);
				strcpy(gb_sklList[j], tmp);
			}
		}
	}
	printf("%s\n", "skeletons in path ./skls/ are listed below:");
	for(int i = 0; i < gb_sklCount; i++)
		printf("%s\n", gb_sklList[i]);
}

// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
{
	// Background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gb_shaderManager.InitializeStockShaders();
	// find skeleton files in ./skls/ and fill their names into gb_sklList
	FillSklList();
	SortSklList();
	if(gb_sklCount > 0)
		gb_treeskl.Load(gb_sklList[0]);
	else
		exit(0);

	// Make the sphere
	gltMakeSphere(gb_sphereBatch, 1.0f, 52, 26);
	//gltMakeLineCube(gb_cubeBatch, 1.0f);
	gltMakeCube(gb_cubeBatch, 1.0f);

	// make axis
	GLfloat vVerts[12][3] = { {0.0, 0.0, 0.0}, {100.0, 0.0, 0.0},
							 {0.0, 0.0, 0.0}, {0.0, 100.0, 0.0},
							 {0.0, 0.0, 0.0}, {0.0, 0.0, 100.0},
							 {0.0, 0.0, 0.0}, {-100.0, 0.0, 0.0},
							 {0.0, 0.0, 0.0}, {0.0, -100.0, 0.0},
							 {0.0, 0.0, 0.0}, {0.0, 0.0, -100.0} };
	for(int i = 0; i < 6; i++)
	{
		gb_axisBatches[i].Begin(GL_LINES, 2);
		gb_axisBatches[i].CopyVertexData3f(&vVerts[2 * i]);
		gb_axisBatches[i].End();
	}

	// load normal map shader, this shader will be used when the gb_bTexture is true.
	normalMapShader = gltLoadShaderPairWithAttributes("./shaders/NormalMapped.vp", "./shaders/NormalMapped.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex",
													  GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexture0");
	if(gb_bTexture)
	{
		locAmbient = glGetUniformLocation(normalMapShader, "ambientColor");
		locDiffuse = glGetUniformLocation(normalMapShader, "diffuseColor");
		locLight = glGetUniformLocation(normalMapShader, "vLightPosition");
		locMVP = glGetUniformLocation(normalMapShader, "mvpMatrix");
		locMV  = glGetUniformLocation(normalMapShader, "mvMatrix");
		locNM  = glGetUniformLocation(normalMapShader, "normalMatrix");
		locColorMap = glGetUniformLocation(normalMapShader, "colorMap");
		locNormalMap = glGetUniformLocation(normalMapShader, "normalMap");
	}

	// load ads phong shader and load its location, this is the default shader.
	adsPhongShader = gltLoadShaderPairWithAttributes("./shaders/ADSPhong.vp", "./shaders/ADSPhong.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
	if(!gb_bTexture)
	{
		locLight = glGetUniformLocation(adsPhongShader, "vLightPosition");
		locAmbient = glGetUniformLocation(adsPhongShader, "ambientColor");
		locDiffuse = glGetUniformLocation(adsPhongShader, "diffuseColor");
		locSpecular = glGetUniformLocation(adsPhongShader, "specularColor");
		locMVP = glGetUniformLocation(adsPhongShader, "mvpMatrix");
		locMV  = glGetUniformLocation(adsPhongShader, "mvMatrix");
		locNM  = glGetUniformLocation(adsPhongShader, "normalMatrix");
	}

	// load textures
	glGenTextures(2, texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	LoadTGATexture("./textures/bark.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	LoadTGATexture("./textures/barkBump.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

}

// Cleanup
void ShutdownRC(void)
{
	glDeleteTextures(2, texture);
}

void SetGeneralColor()
{
	if(!gb_bTexture)
	{
		GLfloat vAmbientColor[] = { 0.2f, 0.2f, 0.2f, 0.9f };
		GLfloat vDiffuseColor[] = { 0.3f, 0.3f, 0.3f, 0.9f };
		GLfloat vSpecularColor[] = { 0.3f, 0.3f, 0.3f, 0.9f };
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform4fv(locSpecular, 1, vSpecularColor);
	}
}

void SetSelectedColor()
{
	if(!gb_bTexture)
	{
		// brown
		GLfloat vAmbientColor[] = { 0.2f, 0.0f, 0.0f, 0.9f };
		GLfloat vDiffuseColor[] = { 0.1f, 0.0f, 0.0f, 0.9f };
		GLfloat vSpecularColor[] = { 0.1f, 0.0f, 0.0f, 0.9f };
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform4fv(locSpecular, 1, vSpecularColor);
	}
}

void SetVoxelColor()
{
	GLfloat vAmbientColor[] = { 0.5f, 0.5f, 0.0f, 0.1f };
	GLfloat vDiffuseColor[] = { 0.1f, 0.0f, 0.0f, 0.1f };
	GLfloat vSpecularColor[] = { 0.3f, 0.3f, 0.0f, 0.1f };
	glUniform4fv(locAmbient, 1, vAmbientColor);
	glUniform4fv(locDiffuse, 1, vDiffuseColor);
	glUniform4fv(locSpecular, 1, vSpecularColor);
}

void SetUsedVoxelColor()
{
	GLfloat vAmbientColor[] = { 0.5f, 0.0f, 0.2f, 0.2f };
	GLfloat vDiffuseColor[] = { 0.3f, 0.0f, 0.1f, 0.2f };
	GLfloat vSpecularColor[] = { 0.3f, 0.2f, 0.2f, 0.2f };
	glUniform4fv(locAmbient, 1, vAmbientColor);
	glUniform4fv(locDiffuse, 1, vDiffuseColor);
	glUniform4fv(locSpecular, 1, vSpecularColor);
}

void DrawCoordinateAxis()
{
	GLfloat vColors[6][4] = { {1.0, 0.0, 0.0, 1.0},   // +x
							  {0.0, 1.0, 0.0, 1.0},   // +y
							  {0.0, 0.5, 1.0, 1.0},   // +z
							  {0.4, 0.0, 0.0, 1.0},   // -x
							  {0.0, 0.4, 0.0, 1.0},   // -y
							  {0.0, 0.2, 0.4, 1.0} }; // -z
	int nAxis = (gb_bBothDirection) ? 6 : 3;
	
	for(int i = 0; i < nAxis; i++)
	{
		gb_shaderManager.UseStockShader(GLT_SHADER_FLAT, gb_transformPipeline.GetModelViewProjectionMatrix(), vColors[i]);
		gb_axisBatches[i].Draw();
	}

}

void UpdateLocations()
{
	if(gb_bTexture)
	{
		locAmbient = glGetUniformLocation(normalMapShader, "ambientColor");
		locDiffuse = glGetUniformLocation(normalMapShader, "diffuseColor");
		locLight = glGetUniformLocation(normalMapShader, "vLightPosition");
		locMVP = glGetUniformLocation(normalMapShader, "mvpMatrix");
		locMV  = glGetUniformLocation(normalMapShader, "mvMatrix");
		locNM  = glGetUniformLocation(normalMapShader, "normalMatrix");
		locColorMap = glGetUniformLocation(normalMapShader, "colorMap");
		locNormalMap = glGetUniformLocation(normalMapShader, "normalMap");

	}
	else
	{
		locLight = glGetUniformLocation(adsPhongShader, "vLightPosition");
		locAmbient = glGetUniformLocation(adsPhongShader, "ambientColor");
		locDiffuse = glGetUniformLocation(adsPhongShader, "diffuseColor");
		locSpecular = glGetUniformLocation(adsPhongShader, "specularColor");
		locMVP = glGetUniformLocation(adsPhongShader, "mvpMatrix");
		locMV  = glGetUniformLocation(adsPhongShader, "mvMatrix");
		locNM  = glGetUniformLocation(adsPhongShader, "normalMatrix");

	}
}

void GetWorldDirFromView(M3DVector3f wol/*out*/, M3DVector3f view/*in*/)
{
	GLFrame eyeFrame;
	eyeFrame.RotateWorld(gb_eye_theta * 3.1415926 / 180.0, 1.0, 0.0, 0.0);
	eyeFrame.RotateWorld(gb_eye_phi * 3.1415926 / 180.0, 0.0, 1.0, 0.0);
	M3DMatrix44f mat, inv;
	eyeFrame.GetCameraMatrix(mat);
	m3dInvertMatrix44(inv, mat);
	m3dTransformVector3(wol, view, inv);
}

void MoveCurNode(M3DVector3f vector)
{
	M3DVector3f wol;
	GetWorldDirFromView(wol, vector);
	gb_treeskl.MoveCurNode(wol);
}

// Called to draw scene
void onDisplay(void)
{
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// calculate the view matrix.
	GLFrame eyeFrame;
	eyeFrame.MoveUp(gb_eye_height);
	eyeFrame.RotateWorld(gb_eye_theta * 3.1415926 / 180.0, 1.0, 0.0, 0.0);
	eyeFrame.RotateWorld(gb_eye_phi * 3.1415926 / 180.0, 0.0, 1.0, 0.0);
	eyeFrame.MoveForward(-gb_eye_radius);
	M3DMatrix44f mat;
	eyeFrame.GetCameraMatrix(mat);
	gb_modelViewMatrix.PushMatrix(mat);

	// draw coordinate system
	if(gb_bCoord)
	{
		DrawCoordinateAxis();
	}

	if(gb_bTexture)
	{

		GLfloat vEyeLight[] = { -100.0f, 100.0f, 150.0f };
		GLfloat vAmbientColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat vDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 1.0f};

		glUseProgram(normalMapShader);
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform3fv(locLight, 1, vEyeLight);
		glUniform1i(locColorMap, 0);
		glUniform1i(locNormalMap, 1);
		gb_treeskl.Display(SetGeneralColor, SetSelectedColor, 0);
	}
	else
	{
		if(!gb_bOnlyVoxel)
		{
			if(gb_bPoints)
			{
				//GLfloat vPointColor[] = { 1.0, 1.0, 0.0, 0.6 };
				GLfloat vPointColor[] = { 0.2, 0.0, 0.0, 0.9 };
				gb_shaderManager.UseStockShader(GLT_SHADER_FLAT, gb_transformPipeline.GetModelViewProjectionMatrix(), vPointColor);
				gb_treeskl.Display(NULL, NULL, 1);
			}
			else
			{
				GLfloat vEyeLight[] = { -100.0f, 100.0f, 150.0f };
				glUseProgram(adsPhongShader);
				glUniform3fv(locLight, 1, vEyeLight);
				gb_treeskl.Display(SetGeneralColor, SetSelectedColor, 0);
			}
		}
		if(gb_bVoxel)
		{
			GLfloat vEyeLight[] = { -100.0f, 100.0f, 150.0f };
			glUseProgram(adsPhongShader);
			glUniform3fv(locLight, 1, vEyeLight);
			SetVoxelColor();
			glPolygonMode(GL_FRONT, GL_LINE);
			glLineWidth(2.0f);
			gb_treeskl.DisplayVoxel();
			glPolygonMode(GL_FRONT, GL_FILL);
		}
	}
	//glUniformMatrix4fv(locMVP, 1, GL_FALSE, gb_transformPipeline.GetModelViewProjectionMatrix());
	//glUniformMatrix4fv(locMV, 1, GL_FALSE, gb_transformPipeline.GetModelViewMatrix());
	//glUniformMatrix3fv(locNM, 1, GL_FALSE, gb_transformPipeline.GetNormalMatrix());
	//gb_sphereBatch.Draw();
	gb_modelViewMatrix.PopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();
}



void onReshape(int w, int h)
{
	// Prevent a divide by zero
	if(h == 0)
		h = 1;
	gb_winHeight = h;
	gb_winWidth = w;

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	gb_viewFrustum.SetPerspective(35.0f, float(w)/float(h), 0.001f, 100.0f);

	gb_projectionMatrix.LoadMatrix(gb_viewFrustum.GetProjectionMatrix());
	gb_transformPipeline.SetMatrixStacks(gb_modelViewMatrix, gb_projectionMatrix);
}

void onMouseDrag(int x, int y)
{
	static int prev_x = 0, prev_y = 0;
	static bool bDown = false;
	static bool bLeft = false;
	if(x == MOUSE_DRAG_MAX)
	{
		bDown = false;
		return;
	}
	if(x == MOUSE_DRAG_MIN)
	{
		bDown = true;
		prev_y = y;
		if(y == MOUSE_DRAG_MAX)
		{
			bLeft = false;
		}
		if(y == MOUSE_DRAG_MIN)
		{
			bLeft = true;
		}
		return;
	}
	if(bDown == true)  
	{
		if(prev_y == MOUSE_DRAG_MIN || prev_y == MOUSE_DRAG_MAX) // if it's the first drag, record the origin
		{
			prev_x = x;
			prev_y = y;
			return;
		}
		// calculate the change of mouse position
		float xChange = 0.0, yChange = 0.0;
		if(abs(y - prev_y) > abs(x - prev_x))
			yChange = GLfloat(y - prev_y);
		else
			xChange = GLfloat(x - prev_x);
		if(bLeft)
		{
			// drag left button, modify current node on xy plane in eye coordinate system
			float step;
			step = 1.0 / ((gb_winWidth > gb_winHeight) ? gb_winWidth : gb_winHeight);
			M3DVector3f vector = { 0.0, 0.0, 0.0 };
			vector[0] = step * xChange;
			vector[1] = -step * yChange;
			M3DVector3f wol;
			GetWorldDirFromView(wol, vector);
			gb_treeskl.MoveCurNode(wol);
		}
		else
		{
			// drag right button, rotate eye
			gb_eye_theta -= yChange / 4;
			gb_eye_phi -= xChange / 4;
			if(gb_eye_theta < -90.0) gb_eye_theta = -90.0;
			else if(gb_eye_theta > 90.0) gb_eye_theta = 90.0;
			if(gb_eye_phi < 0) gb_eye_phi += 360.0;
			else if(gb_eye_phi > 360.0) gb_eye_phi -= 360.0;
		}
		glutPostRedisplay();
		prev_x = x;
		prev_y = y;
	}
}

void onMouseKey(int button, int state, int x, int y)
{
	switch(button)
	{
	case GLUT_RIGHT_BUTTON:
		if(state == GLUT_DOWN) onMouseDrag(MOUSE_DRAG_MIN, MOUSE_DRAG_MAX);
		if(state == GLUT_UP) onMouseDrag(MOUSE_DRAG_MAX, MOUSE_DRAG_MAX);
		break;
	case GLUT_LEFT_BUTTON:
		if(gb_bPoints)
			return;
		if(state == GLUT_DOWN) onMouseDrag(MOUSE_DRAG_MIN, MOUSE_DRAG_MIN);
		if(state == GLUT_UP) onMouseDrag(MOUSE_DRAG_MAX, MOUSE_DRAG_MIN);
		break;
	case GLUT_WHEEL_DOWN:
		gb_eye_radius *= 1.01f;
		if(gb_eye_radius > 40.0) gb_eye_radius = 40.0;
		glutPostRedisplay();
		break;
	case GLUT_WHEEL_UP:
		gb_eye_radius /= 1.01f;
		if(gb_eye_radius < 0.1) gb_eye_radius = 0.1;
		glutPostRedisplay();
		break;
	case GLUT_MIDDLE_BUTTON:
		{
			if(gb_bPoints)
				return;
			GLFrame eyeFrame;
			eyeFrame.MoveUp(gb_eye_height);
			eyeFrame.RotateWorld(gb_eye_theta * 3.1415926 / 180.0, 1.0, 0.0, 0.0);
			eyeFrame.RotateWorld(gb_eye_phi * 3.1415926 / 180.0, 0.0, 1.0, 0.0);
			eyeFrame.MoveForward(-gb_eye_radius);
			M3DMatrix44f mat;
			eyeFrame.GetCameraMatrix(mat);
			M3DVector3f pt;
			bool bSuccess = HitTest(x, y, pt, mat, (float*)gb_viewFrustum.GetProjectionMatrix());
			if(bSuccess)
				gb_treeskl.Select(pt);
		}
		break;
	}
}

void onKeyboard(unsigned char key, int x, int y)
{
	M3DVector3f vector = {0.0, 0.0, 0.0};
	switch(key)
	{
	case '=':
		gb_eye_height += 0.2;
		break;
	case '-':
		gb_eye_height -= 0.2;
		break;
	case 's':	// simply the model
		if(gb_bPoints)
			return;
		gb_treeskl.Simplify(gb_max_angle * 3.1415926 / 180.0);
		gb_max_angle += 5.0;
		break;
	case 'S':
		if(gb_bPoints)
			return;
		gb_treeskl.Simplify(gb_max_angle * 3.1415926 / 180.0, false);
		gb_max_angle += 5.0;
		break;
	case 'j':	// select the parent
		if(gb_bPoints)
			return;
		gb_treeskl.Ascent();
		break;
	case 'k':	// select the first child
		if(gb_bPoints)
			return;
		gb_treeskl.Descent();
		break;
	case 'h':	// select previous brother
		if(gb_bPoints)
			return;
		gb_treeskl.Previous();
		break;
	case 'l':	// select next brother
		if(gb_bPoints)
			return;
		gb_treeskl.Next();
		break;
	case 'L':	// for point cloud test
		gb_treeskl.LoadPointCloud("Tree.ply");
		gb_treeskl.LoadVoxelModel(20);
		break;
	case 'e':	// extract skeleton of current node
		gb_treeskl.ExtractSkeleton(1);
		break;
	case 'E':	// extract the whole skeleton
		gb_treeskl.ExtractSkeleton(0);
		break;
		// change the radius of current node
	case 'i':
		if(gb_bPoints)
			return;
		gb_treeskl.ChangeRadius(0.0001);
		printf("%s\n", "radius plus 0.0001");
		break;
	case 'I':
		if(gb_bPoints)
			return;
		gb_treeskl.ChangeRadius(0.001);
		printf("%s\n", "radius plus 0.001");
		break;
	case 'd':
		if(gb_bPoints)
			return;
		gb_treeskl.ChangeRadius(-0.0001);
		printf("%s\n", "radius minus 0.0001");
		break;
	case 'D':
		if(gb_bPoints)
			return;
		gb_treeskl.ChangeRadius(-0.001);
		printf("%s\n", "radius minus 0.001");
		break;
	case 'n':		// move the node nearer(in eye coordinate system)
		{
			if(gb_bPoints)
				return;
			vector[2] = 0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 nearer to observer");
		}
		break;
	case 'f':		// move the node farther(in eye coordinate system)
		{
			if(gb_bPoints)
				return;
			vector[2] = -0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 farther to observer");
		}
		break;
	case 'g':		// generate a new child
		{
			if(gb_bPoints)
				return;
			vector[0] = 1;
			M3DVector3f wol;
			GetWorldDirFromView(wol, vector);
			gb_treeskl.Insert(wol[0], wol[1], wol[2], -1);	// the last parameter -1 indicates it equals to parent'radius
		}
		break;
	case 'p':		// pull out the current select subtree(exclude current node)
		if(gb_bPoints)
			return;
		gb_treeskl.Delete(0);
		break;
	case 'P':		// pull out the current select subtree(include current node)
		if(gb_bPoints)
			return;
		gb_treeskl.Delete(1);
		break;
	case 'w':		// save the skeleton to file
		gb_treeskl.Save("Tree.skl", 0);
		printf("Model saved to %s\n", "Tree.skl");
		return;
	case 'W':
		gb_treeskl.Save("Tree.ply", 1);
		printf("Model saved to %s\n", "Tree.ply");
		return;
	case 'b':		// reverse the background color
		gb_bBlack = !gb_bBlack;
		if(gb_bBlack)	glClearColor(0.0, 0.0, 0.0, 1.0);
		else	glClearColor(1.0, 1.0, 1.0, 1.0);
		break;
	case 't':		// load or unload texture
		gb_bTexture = !gb_bTexture;
		UpdateLocations();
		break;
	case 'x':
		gb_bCoord = !gb_bCoord;
		break;
	case 'X':
		gb_bBothDirection = !gb_bBothDirection;
		break;
	case 'm':		// change display mode
		{
			gb_bPoints = !gb_bPoints;
			char prompt[100];
			gb_bPoints ? (strcpy(prompt, "change to point mode")) : (strcpy(prompt, "change to skeleton mode"));
			printf("%s\n", prompt);
		}
		break;
	case 'v':
		{
			gb_bVoxel = !gb_bVoxel;
			char prompt[100];
			gb_bVoxel ? (strcpy(prompt, "display voxels")) : (strcpy(prompt, "undisplay voxels"));
			printf("%s\n", prompt);
		}
		break;
	case 'V':
		{
			gb_bOnlyVoxel = !gb_bOnlyVoxel;
		}
		break;
	case '1':
		gb_treeskl.LinearRadius(gb_linear_ratio);
		break;
	case '2':
		gb_treeskl.SquareRadius();
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void onSpecial(int key, int x, int y)
{
	M3DVector3f vector = { 0.0, 0.0, 0.0 };
	switch(key)
	{
	case GLUT_KEY_LEFT:
		{
			if(gb_bPoints)
				return;
			vector[0] = -0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 left to observer");
		}
		break;
	case GLUT_KEY_RIGHT:
		{
			if(gb_bPoints)
				return;
			vector[0] = 0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 right to observer");
		}
		break;
	case GLUT_KEY_UP:
		{
			if(gb_bPoints)
				return;
			vector[1] = 0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 up to observer");
		}
		break;
	case GLUT_KEY_DOWN:
		{
			if(gb_bPoints)
				return;
			vector[1] = -0.003;
			MoveCurNode(vector);
			printf("%s\n", "current node move 0.003 down to observer");
		}
		break;
	case GLUT_KEY_F1:
	case GLUT_KEY_F2:
	case GLUT_KEY_F3:
	case GLUT_KEY_F4:
	case GLUT_KEY_F5:
	case GLUT_KEY_F6:
	case GLUT_KEY_F7:
	case GLUT_KEY_F8:
		if(key - GLUT_KEY_F1 >= gb_sklCount)
		{
			printf("%s\n", "No skeleton file exist in ./skls/");
			return;
		}
		printf("Operating on skeleton %s\n", gb_sklList[key - GLUT_KEY_F1]);
		gb_treeskl.Load(gb_sklList[key - GLUT_KEY_F1]);
		break;
	}
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("tree skeleton manipulation");
	glutReshapeFunc(onReshape);
	glutDisplayFunc(onDisplay);
	glutMouseFunc(onMouseKey);
	glutMotionFunc(onMouseDrag);
	glutKeyboardFunc(onKeyboard);
	glutSpecialFunc(onSpecial);

	glewExperimental = GL_TRUE;
	glewInit();

	SetupRC();    
	glutMainLoop();
	ShutdownRC();
	return 0;
}

