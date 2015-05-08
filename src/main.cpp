#include "opengl.h"
#include "standard.h"
#include "scene.h"
#include "camera.h"
#include "model.h"
#include "primitive.h"
#include "tinyfiledialogs.h"
#include "light.h"

int window_id;

scenehdl scene;

int mousex = 0, mousey = 0;
bool bound = false;
bool menu = false;

int width = 750;
int height = 750;

string working_directory = "";

int Menu_normal, Menu_object, Menu_camera, Menu_light;

namespace manipulate
{
	enum type
	{
		none = -1,
		translate = 0,
		rotate = 1,
		scale = 2,
		fovy = 3,
		aspect = 4,
		width = 5,
		height = 6,
		front = 7,
		back = 8
	};
}

manipulate::type manipulator;

bool keys[256];

void init()
{
	for (int i = 0; i < 256; i++)
		keys[i] = false;

	// TODO Assignment 1: Initialize the Scene as necessary.
	//scene.cameras.push_back(new frustumhdl());
	scene.cameras.push_back(new perspectivehdl());
	scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
	for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
		for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
		{
			swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
			scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
			swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
			scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
		}
	swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
	swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

	scene.cameras.back()->model = scene.objects.back();
	if (!scene.active_camera_valid())
	{
		scene.active_camera = scene.cameras.size()-1;
		scene.cameras[scene.active_camera]->project();
	}
	scene.cameras[scene.active_camera]->position[2] = 10.0;
}

void displayfunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene.draw();

	glutSwapBuffers();
}

void reshapefunc(int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	glutPostRedisplay();
}

void pmotionfunc(int x, int y)
{
	if (bound)
	{
		int deltax = x - mousex;
		int deltay = y - mousey;

		mousex = x;
		mousey = y;

		bool warp = false;
		if (mousex > 3*width/4 || mousex < width/4)
		{
			mousex = width/2;
			warp = true;
		}

		if (mousey > 3*height/4 || mousey < height/4)
		{
			mousey = height/2;
			warp = true;
		}

		if (warp)
			glutWarpPointer(mousex, mousey);

		// TODO Assignment 1: Use the mouse delta to change the orientation of the active camera
		if (scene.active_camera_valid())
		{
			scene.cameras[scene.active_camera]->orientation[1] -= (float)deltax/500.0;
			scene.cameras[scene.active_camera]->orientation[0] -= (float)deltay/500.0;
		}

		glutPostRedisplay();
	}
	else if (scene.active_camera_valid())
	{
		/* TODO Assignment 1: Figure out which object the mouse pointer is hovering over and make
		 * that the active object.
		 */
		
		vec3f direction;
		vec3f position;

		if (scene.active_camera_valid())
		{
		    GLdouble modelview[16];
		    GLdouble projection[16];
		    glGetDoublev( GL_TRANSPOSE_MODELVIEW_MATRIX, modelview );
		    glGetDoublev( GL_TRANSPOSE_PROJECTION_MATRIX, projection );
		    mat4f proj;
		    mat4f mdvl;
		    for(int i = 0; i < 16; i++){
		    	int row = i / 4;
		    	int col = i % 4;
		    	proj[row][col] = projection[i];
		    	mdvl[row][col] = modelview[i];
		    }

		    vec2i pixel = vec2i(x, y);
		    vec3f window = vec3f(2.0*(float)pixel[0]/(float)(width-1) - 1.0,
				 2.0*(float)(height - 1 - pixel[1])/(float)(height-1) - 1.0,
				 1.04);
		    vec4f v1 = inverse(proj * mdvl) * homogenize(window);//mouse point on canvas
 
			if (scene.cameras[scene.active_camera]->type == "ortho")
			{
				position = v1;
				direction = ror3(vec3f(0.0f, 0.0f, 1.0f), scene.cameras[scene.active_camera]->orientation);
			}
			else
			{
				position = scene.cameras[scene.active_camera]->position;
				direction = norm(v1);
			}
		}

		int old_active_object = scene.active_object;
		scene.active_object = -1;
		for (int i = 0; i < scene.objects.size(); i++)
		{
			if (scene.objects[i] != NULL && scene.cameras[scene.active_camera]->model != scene.objects[i])
			{
				bool is_camera = false;
				bool is_light = false;

				for (int j = 0; j < scene.cameras.size() && !is_camera; j++)
					if (scene.cameras[j] != NULL && scene.cameras[j]->model == scene.objects[i])
						is_camera = true;
				for (int j = 0; j < scene.lights.size() && !is_light; j++)
					if (scene.lights[j] != NULL && scene.lights[j]->model == scene.objects[i])
						is_light = true;

				if ((!is_camera && !is_light) || (is_camera && scene.render_cameras) || (is_light && scene.render_lights))
				{
					vec3f invdir = 1.0f/direction;
					vec3i sign((int)(invdir[0] < 0), (int)(invdir[1] < 0), (int)(invdir[2] < 0));
					vec3f origin = position - scene.objects[i]->position;
					float tmin, tmax, tymin, tymax, tzmin, tzmax;
					tmin = (scene.objects[i]->bound[0 + sign[0]]*scene.objects[i]->scale - origin[0])*invdir[0];
					tmax = (scene.objects[i]->bound[0 + 1-sign[0]]*scene.objects[i]->scale - origin[0])*invdir[0];
					tymin = (scene.objects[i]->bound[2 + sign[1]]*scene.objects[i]->scale - origin[1])*invdir[1];
					tymax = (scene.objects[i]->bound[2 + 1-sign[1]]*scene.objects[i]->scale - origin[1])*invdir[1];
					if ((tmin <= tymax) && (tymin <= tmax))
					{
						if (tymin > tmin)
							tmin = tymin;
						if (tymax < tmax)
							tmax = tymax;

						tzmin = (scene.objects[i]->bound[4 + sign[2]]*scene.objects[i]->scale - origin[2])*invdir[2];
						tzmax = (scene.objects[i]->bound[4 + 1-sign[2]]*scene.objects[i]->scale - origin[2])*invdir[2];

						if ((tmin <= tzmax) && (tzmin <= tmax))
						{
							scene.active_object = i;
							i = scene.objects.size();
						}
					}
				}
			}
		}

		if (scene.active_object != old_active_object)
		{
			bool is_camera = false;
			bool is_light = false;

			for (int i = 0; i < scene.cameras.size() && !is_camera; i++)
				if (scene.cameras[i] != NULL && scene.active_object_valid() && scene.cameras[i]->model == scene.objects[scene.active_object])
					is_camera = true;
			for (int i = 0; i < scene.lights.size() && !is_light; i++)
				if (scene.lights[i] != NULL && scene.active_object_valid() && scene.lights[i]->model == scene.objects[scene.active_object])
					is_light = true;

			glutDetachMenu(GLUT_RIGHT_BUTTON);
			if (scene.active_object == -1)
				glutSetMenu(Menu_normal);
			else if (is_camera)
				glutSetMenu(Menu_camera);
			else if (is_light)
				glutSetMenu(Menu_light);
			else
				glutSetMenu(Menu_object);
			glutAttachMenu(GLUT_RIGHT_BUTTON);
			glutPostRedisplay();
		}
	}
}

void mousefunc(int button, int state, int x, int y)
{
	mousex = x;
	mousey = y;
}

void motionfunc(int x, int y)
{
	if (!bound && !menu)
	{
		int deltax = x - mousex;
		int deltay = mousey - y;

		mousex = x;
		mousey = y;

		/* TODO Assignment 1: Implement the functionality for the following operations here:
		 * translation, rotation, and scaling of an object
		 * changing the fovy, aspect, width, height, near, or far values of the active camera
		 *
		 * This uses the mouse, so you'll have to determine the world coordinate position of the
		 * mouse pointer.
		 */

		vec3f direction;
		vec3f position;

		if (scene.active_camera_valid())
		{
		    GLdouble modelview[16];
		    GLdouble projection[16];
		    glGetDoublev( GL_TRANSPOSE_MODELVIEW_MATRIX, modelview );
		    glGetDoublev( GL_TRANSPOSE_PROJECTION_MATRIX, projection );
		    mat4f proj;
		    mat4f mdvl;
		    for(int i = 0; i < 16; i++){
		    	int row = i / 4;
		    	int col = i % 4;
		    	proj[row][col] = projection[i];
		    	mdvl[row][col] = modelview[i];
		    }

		    vec2i pixel = vec2i(x, y);
		    vec3f window = vec3f(2.0*(float)pixel[0]/(float)(width-1) - 1.0,
				 2.0*(float)(height - 1 - pixel[1])/(float)(height-1) - 1.0,
				 1.04);
		    vec4f v1 = inverse(proj * mdvl) * homogenize(window);//mouse point on canvas

		    if (scene.cameras[scene.active_camera]->type == "ortho")
			{
				position = v1;
				direction = ror3(vec3f(0.0f, 0.0f, 1.0f), scene.cameras[scene.active_camera]->orientation);
			}
			else
			{
				position = scene.cameras[scene.active_camera]->position;
				direction = norm(v1);
			}
		}

		if (scene.active_object_valid() && scene.active_camera_valid())
		{
			if (manipulator == manipulate::translate)
			{
				float d = mag(scene.objects[scene.active_object]->position - position);
				scene.objects[scene.active_object]->position = d*direction + position;
			}
			else if (manipulator == manipulate::rotate)
				scene.objects[scene.active_object]->orientation += vec3f(-(float)deltay/100.0, (float)deltax/100.0, 0.0);
			else if (manipulator == manipulate::scale)
				scene.objects[scene.active_object]->scale += (float)deltay/100.0;

			for (int i = 0; i < scene.cameras.size(); i++)
				if (scene.cameras[i]->model == scene.objects[scene.active_object])
				{
					scene.cameras[i]->position = scene.objects[scene.active_object]->position;
					scene.cameras[i]->orientation = scene.objects[scene.active_object]->orientation;
				}
		}
		if (scene.active_camera_valid())
		{
			if (manipulator == manipulate::fovy && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->fovy += (float)deltay/100.0;
			else if (manipulator == manipulate::aspect && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->aspect += (float)deltay/100.0;
			else if (manipulator == manipulate::width && scene.cameras[scene.active_camera]->type == "ortho")
			{
				((orthohdl*)scene.cameras[scene.active_camera])->right += (float)deltay/200.0;
				((orthohdl*)scene.cameras[scene.active_camera])->left -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::width && scene.cameras[scene.active_camera]->type == "frustum")
			{
				((frustumhdl*)scene.cameras[scene.active_camera])->right += (float)deltay/200.0;
				((frustumhdl*)scene.cameras[scene.active_camera])->left -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::height && scene.cameras[scene.active_camera]->type == "ortho")
			{
				((orthohdl*)scene.cameras[scene.active_camera])->top += (float)deltay/200.0;
				((orthohdl*)scene.cameras[scene.active_camera])->bottom -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::height && scene.cameras[scene.active_camera]->type == "frustum")
			{
				((frustumhdl*)scene.cameras[scene.active_camera])->top += (float)deltay/200.0;
				((frustumhdl*)scene.cameras[scene.active_camera])->bottom -= (float)deltay/200.0;
			}
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "ortho")
				((orthohdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "frustum")
				((frustumhdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::front && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->front += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "ortho")
				((orthohdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "frustum")
				((frustumhdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;
			else if (manipulator == manipulate::back && scene.cameras[scene.active_camera]->type == "perspective")
				((perspectivehdl*)scene.cameras[scene.active_camera])->back += (float)deltay/100.0;
			if (manipulator == manipulate::fovy ||
				manipulator == manipulate::aspect ||
				manipulator == manipulate::width ||
				manipulator == manipulate::height ||
				manipulator == manipulate::front ||
				manipulator == manipulate::back)
				scene.cameras[scene.active_camera]->project();
		}
		glutPostRedisplay();
	}
	else if (!bound)
	{
		menu = false;
		pmotionfunc(x, y);
	}
}

void keydownfunc(unsigned char key, int x, int y)
{
	keys[key] = true;

	if (key == 27) // Escape Key Pressed
	{
		glutDestroyWindow(window_id);
		exit(0);
	}
	else if (key == 'm' && bound)
	{
		bound = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}
	else if (key == 'm' && !bound)
	{
		bound = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		glutDetachMenu(GLUT_RIGHT_BUTTON);
		mousex = x;
		mousey = y;
	}
}

void keyupfunc(unsigned char key, int x, int y)
{
	keys[key] = false;
}

void idlefunc()
{
	bool change = false;

	// TODO Assignment 1: handle continuous keyboard inputs
	if (scene.active_camera_valid() && scene.cameras[scene.active_camera]->focus == NULL)
	{
		if (keys['w'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(0.0, 0.0, 1.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['s'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(0.0, 0.0, 1.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['a'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(1.0, 0.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['d'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(1.0, 0.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['q'])
		{
			scene.cameras[scene.active_camera]->position += -0.25f*ror3(vec3f(0.0, 1.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
		if (keys['e'])
		{
			scene.cameras[scene.active_camera]->position += 0.25f*ror3(vec3f(0.0, 1.0, 0.0), scene.cameras[scene.active_camera]->orientation);
			change = true;
		}
	}
	else if (scene.active_camera_valid() && scene.cameras[scene.active_camera]->focus != NULL)
	{
		if (keys['w'])
		{
			scene.cameras[scene.active_camera]->radius -= 0.25;
			change = true;
		}
		if (keys['s'])
		{
			scene.cameras[scene.active_camera]->radius += 0.25;
			change = true;
		}
	}
	if (change)
		glutPostRedisplay();
}

void menustatusfunc(int status, int x, int y)
{
	if (status == GLUT_MENU_IN_USE)
		menu = true;
}

void processMenuEvents(int id){
	switch(id){
		case 7:
		exit(1);
	}
};

void processMenuEvents2(int id){
	switch(id){
		case 0://set active
		{
			for (int i = 0; i < scene.cameras.size(); i++)
				if (scene.cameras[i] != NULL && scene.active_object_valid() && scene.cameras[i]->model == scene.objects[scene.active_object])
					scene.active_camera = i;
			if (scene.active_camera_valid())
				scene.cameras[scene.active_camera]->project();
			break;
		}
		case 1://set focus
		{
			if(scene.active_object_valid() && scene.active_camera_valid())
			{
				scene.cameras[scene.active_camera]->focus = scene.objects[scene.active_object];
				scene.cameras[scene.active_camera]->radius = dist(scene.objects[scene.active_object]->position, scene.cameras[scene.active_camera]->position);
			}
			break;
		}
		case 2: manipulator = manipulate::translate; break;
		case 3: manipulator = manipulate::rotate; break;
		case 4:	manipulator = manipulate::scale; break;
		case 5://delete
		{
			if (scene.active_object >= 0 && scene.active_object < scene.objects.size())
			{
				if (scene.objects[scene.active_object] != NULL)
				{
					/* TODO Assignment 2: clean up the lights as well when the associated object
					 * is deleted.
					 */

					for (int i = 0; i < scene.cameras.size(); )
					{
						if (scene.cameras[i] != NULL && scene.cameras[i]->model == scene.objects[scene.active_object])
						{
							delete scene.cameras[i];
							scene.cameras.erase(scene.cameras.begin() + i);
						}
						else
							i++;
					}

					for (int i = 0; i < scene.lights.size(); )
					{
						if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object])
						{
							delete scene.lights[i];
							scene.lights.erase(scene.lights.begin() + i);
						}
						else
							i++;
					}

					delete scene.objects[scene.active_object];
				}
				scene.objects.erase(scene.objects.begin() + scene.active_object);
			}
			break;
		}

	}
	glutPostRedisplay();
}

void processObjectEvents(int id){
	switch(id){
		case 0:	scene.objects.push_back(new boxhdl(1.0,1.0,1.0)); break;
		case 1:	scene.objects.push_back(new cylinderhdl(1.0, 1.0, 20));	break;
		case 2:	scene.objects.push_back(new spherehdl(1.0, 20, 20)); break;
		case 3:	scene.objects.push_back(new pyramidhdl(1.0, 1.0, 20)); break;
		case 4:{
			const char* filters[1];
			filters[0] = "*.obj";
			const char *path = tinyfd_openFileDialog("Load a Model", "", 1, filters, 0);
			if (path != NULL && strlen(path) > 0)
				scene.objects.push_back(new modelhdl(path));
				break;
			}
	}
	glutPostRedisplay();
}

void processLightEvents(int id){
	vec3f orient = scene.cameras[scene.active_camera]->orientation;
	vec3f at = ror3(vec3f(0.0, 0.0, -1.0), orient);
	at=norm(at);
	vec3f up = ror3(vec3f(0.0, 1.0, 0.0), orient);
	up=norm(up);
	switch(id){
		case 0://toggle draw
		{
			scene.render_lights = !scene.render_lights;
			break;
		}
		case 1://directional
		{
			
			scene.lights.push_back(new directionalhdl());
			scene.objects.push_back(new cylinderhdl(0.25, 1.0, 8));
			//((uniformhdl*)scene.objects.back()->material["default"])->emission = vec3f(1.0, 1.0, 1.0);
			for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
				for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
				{
					swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
					scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
					swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
					scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
				}
			swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
			swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);
			scene.lights.back()->model = scene.objects.back();
			break;
		}
		case 2://point
		{
			scene.lights.push_back(new pointhdl());
			scene.objects.push_back(new spherehdl(0.25, 4, 8));
			//((uniformhdl*)scene.objects.back()->material["default"])->emission = vec3f(1.0, 1.0, 1.0);
			scene.lights.back()->model = scene.objects.back();
			break;
		}
		case 3://spot
		{
			scene.lights.push_back(new spothdl());
			scene.objects.push_back(new pyramidhdl(0.25, 1.0, 8));
			//((uniformhdl*)scene.objects.back()->material["default"])->emission = vec3f(1.0, 1.0, 1.0);
			for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
				for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
				{
					swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
					scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
					swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
					scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
				}
			swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
			swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);
			scene.lights.back()->model = scene.objects.back();
			break;
		}
	}
	glutPostRedisplay();
}

void processCameraEvents(int id){
	switch(id){
		case 0:	manipulator = manipulate::fovy; break;
		case 1:	manipulator = manipulate::aspect; break;
		case 2: manipulator = manipulate::width; break;
		case 3:	manipulator = manipulate::height; break;
		case 4:	manipulator = manipulate::front; break;
		case 5:	manipulator = manipulate::back;	break;
		case 6:	scene.render_cameras = !scene.render_cameras; break;
		case 7:	scene.cameras[scene.active_camera]->focus = NULL; break;
		case 8://ortho
		{
			scene.cameras.push_back(new orthohdl());
			scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
			for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
				for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
				{
					swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
					scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
					swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
					scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
				}
			swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
			swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

			scene.cameras.back()->model = scene.objects.back();
			if (!scene.active_camera_valid())
			{
				scene.active_camera = scene.cameras.size()-1;
				scene.cameras[scene.active_camera]->project();
			}
			break;
		}
		case 9://frustum
		{
			scene.cameras.push_back(new frustumhdl());
			scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
			for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
				for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
				{
					swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
					scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
					swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
					scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
				}
			swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
			swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

			scene.cameras.back()->model = scene.objects.back();
			if (!scene.active_camera_valid())
			{
				scene.active_camera = scene.cameras.size()-1;
				scene.cameras[scene.active_camera]->project();
			}
			break;
		}
		case 10://perspective
		{
			scene.cameras.push_back(new perspectivehdl());
			scene.objects.push_back(new pyramidhdl(1.0, 1.0, 8));
			for (int k = 0; k < scene.objects.back()->rigid.size(); k++)
				for (int i = 0; i < scene.objects.back()->rigid[k].geometry.size(); i++)
				{
					swap(scene.objects.back()->rigid[k].geometry[i][1], scene.objects.back()->rigid[k].geometry[i][2]);
					scene.objects.back()->rigid[k].geometry[i][1] *= -1.0;
					swap(scene.objects.back()->rigid[k].geometry[i][4], scene.objects.back()->rigid[k].geometry[i][5]);
					scene.objects.back()->rigid[k].geometry[i][4] *= -1.0;
				}
			swap(scene.objects.back()->bound[2], scene.objects.back()->bound[4]);
			swap(scene.objects.back()->bound[3], scene.objects.back()->bound[5]);

			scene.cameras.back()->model = scene.objects.back();
			if (!scene.active_camera_valid())
			{
				scene.active_camera = scene.cameras.size()-1;
				scene.cameras[scene.active_camera]->project();
			}
			break;
		}
	
	}
	glutPostRedisplay();
}

void processNormalEvents(int id){
	switch(id){
		case 0:	scene.render_normals = scene.none; break;
		case 1:	scene.render_normals = scene.face; break;
		case 2:	scene.render_normals = scene.vertex; break;
	}
	glutPostRedisplay();
}

void processPolygonEvents(int id){
	switch(id){
		case 0:	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
		case 1:	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
		case 2:	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
	}
	glutPostRedisplay();
}

void processCullingEvents(int id){
	glEnable(GL_CULL_FACE);
	switch(id){
		case 0: glDisable(GL_CULL_FACE); break;
		case 1:	glCullFace(GL_FRONT); break;
		case 2:	glCullFace(GL_BACK); break;
	}
	glutPostRedisplay();
}

void processColorEvents(int id)
{
	for (int i = 0; i < scene.lights.size(); i++)
		if (scene.lights[i] != NULL && scene.lights[i]->model == scene.objects[scene.active_object])
		{
			switch (id)
			{
			case 0: scene.lights[i]->ambient = red*0.2f; break;
			case 1: scene.lights[i]->ambient = orange*0.2f; break;
			case 2: scene.lights[i]->ambient = yellow*0.2f; break;
			case 3: scene.lights[i]->ambient = green*0.2f; break;
			case 4: scene.lights[i]->ambient = blue*0.2f; break;
			case 5: scene.lights[i]->ambient = indigo*0.2f; break;
			case 6: scene.lights[i]->ambient = violet*0.2f; break;
			case 7: scene.lights[i]->ambient = black*0.2f; break;
			case 8: scene.lights[i]->ambient = white*0.2f; break;
			case 9: scene.lights[i]->ambient = brown*0.2f; break;
			case 10: scene.lights[i]->diffuse = red; break;
			case 11: scene.lights[i]->diffuse = orange; break;
			case 12: scene.lights[i]->diffuse = yellow; break;
			case 13: scene.lights[i]->diffuse = green; break;
			case 14: scene.lights[i]->diffuse = blue; break;
			case 15: scene.lights[i]->diffuse = indigo; break;
			case 16: scene.lights[i]->diffuse = violet; break;
			case 17: scene.lights[i]->diffuse = black; break;
			case 18: scene.lights[i]->diffuse = white; break;
			case 19: scene.lights[i]->diffuse = brown; break;
			case 20: scene.lights[i]->specular = red; break;
			case 21: scene.lights[i]->specular = orange; break;
			case 22: scene.lights[i]->specular = yellow; break;
			case 23: scene.lights[i]->specular = green; break;
			case 24: scene.lights[i]->specular = blue; break;
			case 25: scene.lights[i]->specular = indigo; break;
			case 26: scene.lights[i]->specular = violet; break;
			case 27: scene.lights[i]->specular = black; break;
			case 28: scene.lights[i]->specular = white; break;
			case 29: scene.lights[i]->specular = brown; break;
			}
		}
}

void processAttenuationEvents(int id){
	int i;
	for(i=0; i<scene.lights.size(); i++){
		if(scene.lights[i]->model==scene.objects[scene.active_object])
			break;
	}
	switch(id){//range
		case 0:{
			if(scene.lights[i]->type=="point"){
				pointhdl* pnt = (pointhdl*)scene.lights[i];
				pnt->attenuation = vec3f(1.0, 0.14, 0.7);
			}
			else if(scene.lights[i]->type=="spot"){
				spothdl* spt = (spothdl*)scene.lights[i];
				spt->attenuation = vec3f(1.0, 0.14, 0.7);
			}
			break;
		}
		case 1:{
			if(scene.lights[i]->type=="point"){
				pointhdl* pnt = (pointhdl*)scene.lights[i];
				pnt->attenuation = vec3f(1.0, 0.05, 0.25);
			}
			else if(scene.lights[i]->type=="spot"){
				spothdl* spt = (spothdl*)scene.lights[i];
				spt->attenuation = vec3f(1.0, 0.05, 0.25);
			}
			break;
		}
		case 2:{
			if(scene.lights[i]->type=="point"){
				pointhdl* pnt = (pointhdl*)scene.lights[i];
				pnt->attenuation = vec3f(1.0, 0.0, 0.0);
			}
			else if(scene.lights[i]->type=="spot"){
				spothdl* spt = (spothdl*)scene.lights[i];
				spt->attenuation = vec3f(1.0, 0.0, 0.0);
			}
			break;
		}
	}
	glutPostRedisplay();
}


void processMaterialEvents(int id){
	switch(id){
		case 0:{
			whitehdl * wht = new whitehdl();
			for (map<string, materialhdl*>::iterator it = scene.objects[scene.active_object]->material.begin(); 
				it != scene.objects[scene.active_object]->material.end();
				++it){
				it->second = wht;
			}
			break;
		}
		case 1:{
			gouraudhdl * grd = new gouraudhdl();
			for (map<string, materialhdl*>::iterator it = scene.objects[scene.active_object]->material.begin(); 
				it != scene.objects[scene.active_object]->material.end();
				++it){
				it->second = grd;
			}
			break;
		}
		case 2:{
			phonghdl * phg= new phonghdl();
			for (map<string, materialhdl*>::iterator it = scene.objects[scene.active_object]->material.begin(); 
				it != scene.objects[scene.active_object]->material.end();
				++it){
				it->second = phg;
			}
			break;
		}
		case 3:{
			brickhdl * brk = new brickhdl();
			for (map<string, materialhdl*>::iterator it = scene.objects[scene.active_object]->material.begin(); 
				it != scene.objects[scene.active_object]->material.end();
				++it){
				it->second = brk;
			}
			break;
		}
		case 4:{
			texturehdl * txt = new texturehdl();
			for (map<string, materialhdl*>::iterator it = scene.objects[scene.active_object]->material.begin(); 
				it != scene.objects[scene.active_object]->material.end();
				++it){
				it->second = txt;
			}
			break;
		}
	}
	glutPostRedisplay();
}

void create_menu()
{
	/* TODO Assignment 1: Implement a menu that allows the following operations:
	 * - create a box, cylinder, sphere, pyramid, or load a model
	 * - change the fovy, aspect, width, height, near or far values of the active camera
	 * - enable/disable drawing of cameras
	 * - create an orthographic, frustum, or perspective camera
	 * - set the polygon mode to point or line
	 * - enable culling for front or back face or disable culling
	 * - enable rendering of vertex or face normals, or disabe both
	 * - Set an object or camera as the focus of the active camera (using canvashdl::look_at)
	 * - Unset the focus of the active camera
	 * - Translate, rotate, or scale an object or camera
	 * - delete an object or camera
	 * - Set the active camera
	 * - quit
	 */

	/* TODO Assignment 2: Add menus for handling the lights. You should
	 * be able to
	 * - enable/disable the drawing of the lights
	 * - create directional, point, or spot lights sources
	 * - change the emissive, ambient, diffuse, and specular colors, and the attenuation of the lights
	 * - rotate and translate the lights just like you would an object and have it affect the lighting of the scene
	 * - set the polygon mode to fill
	 * - set the shading mode to none, flat, gouraud, or phong
	 */

	int Objects, Lights, Cameras, Polygon, Shading, Culling, Normals, Ambient, Diffuse, Specular, Attenuation;
	int Quit = 7;
	int Active=0, Focus=1, Translate=2, Rotate=3, Scale=4, Delete=5, Material=6;

	
	Objects = glutCreateMenu(processObjectEvents);
	glutAddMenuEntry("Box", 0);
	glutAddMenuEntry("Cylinder", 1);
	glutAddMenuEntry("Sphere", 2);
	glutAddMenuEntry("Pyramid", 3);
	glutAddMenuEntry("Model", 4);

	Lights = glutCreateMenu(processLightEvents);
	glutAddMenuEntry("Toggle Draw", 0);
	glutAddMenuEntry("Directional", 1);
	glutAddMenuEntry("Point", 2);
	glutAddMenuEntry("Spot", 3);

	Cameras = glutCreateMenu(processCameraEvents);
	glutAddMenuEntry("Fovy", 0);
	glutAddMenuEntry("Aspect", 1);
	glutAddMenuEntry("Width", 2);
	glutAddMenuEntry("Height", 3);
	glutAddMenuEntry("Near", 4);
	glutAddMenuEntry("Far", 5);
	glutAddMenuEntry("Toggle Draw", 6);
	glutAddMenuEntry("Clear Focus", 7);
	glutAddMenuEntry("Ortho", 8);
	glutAddMenuEntry("Frustum", 9);
	glutAddMenuEntry("Perspective", 10);

	Normals = glutCreateMenu(processNormalEvents);
	glutAddMenuEntry("None", 0);
	glutAddMenuEntry("Face", 1);
	glutAddMenuEntry("Vertex", 2);

	Polygon = glutCreateMenu(processPolygonEvents);
	glutAddMenuEntry("Point", 0);
	glutAddMenuEntry("Line", 1);
	glutAddMenuEntry("Fill", 2);

	Culling = glutCreateMenu(processCullingEvents);
	glutAddMenuEntry("None", 0);
	glutAddMenuEntry("Front", 1);
	glutAddMenuEntry("Back", 2);

	Ambient = glutCreateMenu(processColorEvents);
	glutAddMenuEntry("Red", 0);
	glutAddMenuEntry("Orange", 1);
	glutAddMenuEntry("Yellow", 2);
	glutAddMenuEntry("Green", 3);
	glutAddMenuEntry("Blue", 4);
	glutAddMenuEntry("Indigo", 5);
	glutAddMenuEntry("Violet", 6);
	glutAddMenuEntry("Black", 7);
	glutAddMenuEntry("White", 8);
	glutAddMenuEntry("Brown", 9);

	Diffuse = glutCreateMenu(processColorEvents);
	glutAddMenuEntry("Red", 10);
	glutAddMenuEntry("Orange", 11);
	glutAddMenuEntry("Yellow", 12);
	glutAddMenuEntry("Green", 13);
	glutAddMenuEntry("Blue", 14);
	glutAddMenuEntry("Indigo", 15);
	glutAddMenuEntry("Violet", 16);
	glutAddMenuEntry("Black", 17);
	glutAddMenuEntry("White", 18);
	glutAddMenuEntry("Brown", 19);

	Specular = glutCreateMenu(processColorEvents);
	glutAddMenuEntry("Red", 20);
	glutAddMenuEntry("Orange", 21);
	glutAddMenuEntry("Yellow", 22);
	glutAddMenuEntry("Green", 23);
	glutAddMenuEntry("Blue", 24);
	glutAddMenuEntry("Indigo", 25);
	glutAddMenuEntry("Violet", 26);
	glutAddMenuEntry("Black", 27);
	glutAddMenuEntry("White", 28);
	glutAddMenuEntry("Brown", 29);

	Attenuation = glutCreateMenu(processAttenuationEvents);
	glutAddMenuEntry("Short", 0);
	glutAddMenuEntry("Long", 1);
	glutAddMenuEntry("Full", 2);

	Material = glutCreateMenu(processMaterialEvents);
	glutAddMenuEntry("White", 0);
	glutAddMenuEntry("Gouraud", 1);
	glutAddMenuEntry("Phong", 2);
	glutAddMenuEntry("Brick", 3);
	glutAddMenuEntry("Texture", 4);

	Menu_normal = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Objects", Objects);
	glutAddSubMenu("Lights", Lights);
	glutAddSubMenu("Cameras", Cameras);
	glutAddSubMenu("Polygon", Polygon);
	glutAddSubMenu("Culling", Culling);
	glutAddSubMenu("Normals", Normals);
	glutAddMenuEntry("Quit", Quit);

	Menu_object = glutCreateMenu(processMenuEvents2);
	glutAddSubMenu("Material", Material);
	glutAddMenuEntry("Set Focus", Focus);
	glutAddMenuEntry("Translate", Translate);
	glutAddMenuEntry("Rotate", Rotate);
	glutAddMenuEntry("Scale", Scale);
	glutAddMenuEntry("Delete", Delete);


	Menu_camera = glutCreateMenu(processMenuEvents2);
	glutAddMenuEntry("Set Active", Active);
	glutAddMenuEntry("Set Focus", Focus);
	glutAddMenuEntry("Translate", Translate);
	glutAddMenuEntry("Rotate", Rotate);
	glutAddMenuEntry("Scale", Scale);
	glutAddMenuEntry("Delete", Delete);

	Menu_light = glutCreateMenu(processMenuEvents2);
	glutAddSubMenu("Ambient", Ambient);
	glutAddSubMenu("Diffuse", Diffuse);
	glutAddSubMenu("Specular", Specular);
	glutAddSubMenu("Attenuation", Attenuation);
	glutAddMenuEntry("Set Focus", Focus);
	glutAddMenuEntry("Translate", Translate);
	glutAddMenuEntry("Rotate", Rotate);
	glutAddMenuEntry("Delete", Delete);

	glutSetMenu(Menu_normal);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMenuStatusFunc(menustatusfunc);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	int display_mode = GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE;
#ifdef OSX_CORE3
	display_mode |= GLUT_3_2_CORE_PROFILE;
#endif
	glutInitDisplayMode(display_mode);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	window_id = glutCreateWindow("Assignment");

#ifdef __GLEW_H__
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		exit(1);
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
#endif

	cout << "Status: Using OpenGL " << glGetString(GL_VERSION) << endl;
	cout << "Status: Using GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	working_directory = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\")) + "/";

	init();
	create_menu();

	glutReshapeFunc(reshapefunc);
	glutDisplayFunc(displayfunc);
	glutIdleFunc(idlefunc);

	glutPassiveMotionFunc(pmotionfunc);
	glutMotionFunc(motionfunc);
	glutMouseFunc(mousefunc);

	glutKeyboardFunc(keydownfunc);
	glutKeyboardUpFunc(keyupfunc);

	glutMainLoop();
}
