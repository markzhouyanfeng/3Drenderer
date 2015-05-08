#include "scene.h"
#include "camera.h"
#include "object.h"
#include "light.h"

#include "primitive.h"
#include "model.h"

scenehdl::scenehdl()
{
	active_camera = -1;
	active_object = -1;
	render_normals = none;
	render_lights = false;
	render_cameras = false;
}

scenehdl::~scenehdl()
{

}

/* draw
 *
 * Update the locations of all of the lights, draw all of the objects, and
 * if enabled, draw the normals, the lights, the cameras, etc.
 */
void scenehdl::draw()
{
	/* TODO Assignment 1: Draw all of the objects, and
	 * if enabled, draw the normals and the cameras.
	 */

	/* TODO Assignment 2: Clear the uniform variables and pass the vector of
	 * lights into the renderer as a uniform variable.
	 * TODO Assignment 2: Update the light positions and directions
	 * TODO Assignment 2: Render the lights
	 */

	
	if (active_camera_valid())
	{
		//set projection matrix and first part of modelview matrix: camera position and orientation
		//cameras[active_camera]->project();
		cameras[active_camera]->view();
		for(int l=0; l<lights.size(); l++)
			lights[l]->update();
		for(int i=0; i<objects.size(); i++){
			bool iscamera = false;
			bool islight = false;
			for(int j=0; j<cameras.size(); j++){
		 		if(cameras[j]->model==objects[i]){
		 			iscamera = true;
		 			break;
		 		}
		 	}
		 	for(int j=0; j<lights.size(); j++){
		 		if(lights[j]->model==objects[i]){
		 			islight = true;
		 			break;
		 		}
		 	}
	 		if((!iscamera && !islight) || (iscamera && render_cameras) || (islight && render_lights))
	 		{
				//set the second part of modelview matrix: object
				glTranslatef(objects[i]->position[0], objects[i]->position[1], objects[i]->position[2]);
				glScalef(objects[i]->scale, objects[i]->scale, objects[i]->scale);
				glRotatef(radtodeg(objects[i]->orientation[0]), 1.0, 0.0, 0.0);
				glRotatef(radtodeg(objects[i]->orientation[1]), 0.0, 1.0, 0.0);
				glRotatef(radtodeg(objects[i]->orientation[2]), 0.0, 0.0, 1.0);

				objects[i]->draw(lights);
				if(render_normals==face)
			 		objects[i]->draw_normals(true);				
			 	else if(render_normals==vertex)
			 		objects[i]->draw_normals(false);
			 	if(active_object==i)
			 		objects[i]->draw_bound();

			 	//rip of the second part of modelview matrix to get ready for the next object
				glRotatef(-radtodeg(objects[i]->orientation[2]), 0.0, 0.0, 1.0);
				glRotatef(-radtodeg(objects[i]->orientation[1]), 0.0, 1.0, 0.0);
				glRotatef(-radtodeg(objects[i]->orientation[0]), 1.0, 0.0, 0.0);
				glScalef(1/objects[i]->scale, 1/objects[i]->scale, 1/objects[i]->scale);		 	
			 	glTranslatef(-objects[i]->position[0], -objects[i]->position[1], -objects[i]->position[2]);
			}
		}
	}

}

bool scenehdl::active_camera_valid()
{
	return (active_camera >= 0 && active_camera < cameras.size() && cameras[active_camera] != NULL);
}

bool scenehdl::active_object_valid()
{
	return (active_object >= 0 && active_object < objects.size() && objects[active_object] != NULL);
}
