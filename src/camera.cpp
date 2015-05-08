#include "camera.h"
#include "object.h"

camerahdl::camerahdl()
{
	position = vec3f(0.0, 0.0, 0.0);
	orientation = vec3f(0.0, 0.0, 0.0);
	model = NULL;
	type = "camera";
	focus = NULL;
	radius = 10.0f;
}

camerahdl::~camerahdl()
{

}

void camerahdl::view()
{
	/* TODO Assignment 1: Do the necessary modelview transformations to move
	 * the camera into place.
	 */
	//first part of modelview matrix
	glLoadIdentity();
	if (focus == NULL)
	{
		glRotatef(-radtodeg(orientation[0]), 1.0, 0.0, 0.0);
		glRotatef(-radtodeg(orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(-radtodeg(orientation[2]), 0.0, 0.0, 1.0);
		glTranslatef(-position[0], -position[1], -position[2]);
	}
	else
	{
		position = focus->position + ror3(vec3f(0.0, 0.0, radius), orientation);
		vec3f center = focus->position;
		vec3f up = ror3(vec3f(0.0, 1.0, 0.0), orientation);
		gluLookAt(position[0], position[1], position[2], center[0], center[1], center[2], up[0], up[1], up[2]);
		//canvas->look_at(position, focus->position, ror3(vec3f(0.0, 1.0, 0.0), orientation));
	}

	if (model != NULL)
	{
		model->position = position;
		model->orientation = orientation;
		model->orientation[0] *= -1;
	}
}

orthohdl::orthohdl()
{
	left = -10.0;
	right = 10.0;
	bottom = -10.0;
	top = 10.0;
	front = 2.0;
	back = 101.0;
	type = "ortho";
}

orthohdl::~orthohdl()
{
}

void orthohdl::project()
{
	// TODO Assignment 1: Use the canvashdl::ortho function to set up an orthographic projection
	//set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, front, back);
	glMatrixMode(GL_MODELVIEW);
}

frustumhdl::frustumhdl()
{
	left = -1.0;
	right = 1.0;
	bottom = -1.0;
	top = 1.0;
	front = 2.0;
	back = 101.0;
	type = "frustum";
}

frustumhdl::~frustumhdl()
{

}

void frustumhdl::project()
{
	// TODO Assignment 1: Use the canvashdl::frustum function to set up a perspective projection
	//set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left, right, bottom, top, front, back);
	glMatrixMode(GL_MODELVIEW);
}

perspectivehdl::perspectivehdl()
{
	fovy = 45.0;
	aspect = 1.0;
	front = 2.0;
	back = 101.0;
	type = "perspective";
}

perspectivehdl::~perspectivehdl()
{

}

void perspectivehdl::project()
{
	// TODO Assignment 1: Use the canvashdl::perspective function to set up a perspective projection
	//set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect, front, back);
	glMatrixMode(GL_MODELVIEW);
}
