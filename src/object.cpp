#include "object.h"

rigidhdl::rigidhdl()
{

}

rigidhdl::~rigidhdl()
{

}

/* draw
 *
 * Draw a rigid body.
 */
void rigidhdl::draw()
{
	// TODO Assignment 1: Send the rigid body geometry to the renderer
	
	// Pass in the data for those variables
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*8, geometry.data());
	glNormalPointer(GL_FLOAT, sizeof(GLfloat)*8, (float *)geometry.data()+3);
	glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat)*8, (float *)geometry.data()+6);

	// Draw the triangles
	//cout<<"draw triangles"<<endl;
	glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, indices.data());
	//cout<<"draw end"<<endl;
	// Clean up
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glUseProgram(0);
}

objecthdl::objecthdl()
{
	position = vec3f(0.0, 0.0, 0.0);
	orientation = vec3f(0.0, 0.0, 0.0);
	bound = vec6f(1.0e6, -1.0e6, 1.0e6, -1.0e6, 1.0e6, -1.0e6);
	scale = 1.0;
}

objecthdl::objecthdl(const objecthdl &o)
{
	position = o.position;
	orientation = o.orientation;
	bound = o.bound;
	scale = o.scale;
	rigid = o.rigid;
	for (map<string, materialhdl*>::const_iterator i = o.material.begin(); i != o.material.end(); i++)
		material.insert(pair<string, materialhdl*>(i->first, i->second->clone()));
}

objecthdl::~objecthdl()
{
	for (map<string, materialhdl*>::iterator i = material.begin(); i != material.end(); i++)
		if (i->second != NULL)
		{
			delete i->second;
			i->second = NULL;
		}

	material.clear();
}

/* draw
 *
 * Draw the model. Don't forget to apply the transformations necessary
 * for position, orientation, and scale.
 */
void objecthdl::draw(const vector<lighthdl*> &lights)
{
	// TODO Assignment 1: Send transformations and geometry to the renderer to draw the object
	// TODO Assignment 2: Pass the material as a uniform into the renderer
	for (int i=0; i<rigid.size(); i++){
		material[rigid[i].material]->apply(lights);
		rigid[i].draw();
	}
}

/* draw_bound
 *
 * Create a representation for the bounding box and
 * render it.
 */
void objecthdl::draw_bound()
{
	/* TODO Assignment 1: Generate the geometry for the bounding box and send the necessary
	 * transformations and geometry to the renderer
	 */
	// TODO Assignment 2: clear the material in the uniform list

	vector<vec3f> bound_geometry;
	vector<int> bound_indices;
	bound_geometry.reserve(8);
	bound_geometry.push_back(vec3f(bound[0], bound[2], bound[4]));
	bound_geometry.push_back(vec3f(bound[1], bound[2], bound[4]));
	bound_geometry.push_back(vec8f(bound[1], bound[3], bound[4]));
	bound_geometry.push_back(vec8f(bound[0], bound[3], bound[4]));
	bound_geometry.push_back(vec8f(bound[0], bound[2], bound[5]));
	bound_geometry.push_back(vec8f(bound[1], bound[2], bound[5]));
	bound_geometry.push_back(vec8f(bound[1], bound[3], bound[5]));
	bound_geometry.push_back(vec8f(bound[0], bound[3], bound[5]));	

	bound_indices.reserve(24);
	
	for (int i = 0; i < 4; i++)
	{
		bound_indices.push_back(i);
		bound_indices.push_back((i+1)%4);
		bound_indices.push_back(4+i);
		bound_indices.push_back(4+(i+1)%4);
		bound_indices.push_back(i);
		bound_indices.push_back(4+i);
	}	

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*3, bound_geometry.data());
	// Draw the lines
	glDrawElements(GL_LINES, (int)bound_indices.size(), GL_UNSIGNED_INT, bound_indices.data());
	glDisableClientState(GL_VERTEX_ARRAY);

}

/* draw_normals
 *
 * create a representation of the normals for this object.
 * If face is false, render the vertex normals. Otherwise,
 * calculate the normals for each face and render those.
 */
void objecthdl::draw_normals(bool face)
{
	/* TODO Assignment 1: Generate the geometry to display the normals and send the necessary
	 * transformations and geometry to the renderer
	 */

	float radius = 0.0;
	for (int i = 0; i < 6; i++)
		if (abs(bound[i]) > radius)
			radius = abs(bound[i]);

	vector<vec3f> normal_geometry;
	vector<int> normal_indices;

	for (int i=0; i<rigid.size(); i++){
		
		if (!face){
			for (int j=0; j<rigid[i].geometry.size(); j++){
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(vec3f(rigid[i].geometry[j][0], rigid[i].geometry[j][1], rigid[i].geometry[j][2]));
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(vec3f(rigid[i].geometry[j][0]+radius*0.1f*rigid[i].geometry[j][3],
												rigid[i].geometry[j][1]+radius*0.1f*rigid[i].geometry[j][4],
												rigid[i].geometry[j][2]+radius*0.1f*rigid[i].geometry[j][5]));
			}	
		}else{
			for (int j = 0; j < rigid[i].indices.size(); j+=3){
				vec3f normal = norm((vec3f)rigid[i].geometry[rigid[i].indices[j + 0]](3,6) +
									(vec3f)rigid[i].geometry[rigid[i].indices[j + 1]](3,6) +
									(vec3f)rigid[i].geometry[rigid[i].indices[j + 2]](3,6));
				vec3f center = ((vec3f)rigid[i].geometry[rigid[i].indices[j + 0]](0,3) +
								(vec3f)rigid[i].geometry[rigid[i].indices[j + 1]](0,3) +
								(vec3f)rigid[i].geometry[rigid[i].indices[j + 2]](0,3))/3.0f;
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(center);
				normal_indices.push_back(normal_geometry.size());
				normal_geometry.push_back(center + radius*0.1f*normal);
			}
		}
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(GLfloat)*3, normal_geometry.data());
		glDrawElements(GL_LINES, (int)normal_indices.size(), GL_UNSIGNED_INT, normal_indices.data());
		glDisableClientState(GL_VERTEX_ARRAY);
		normal_geometry.clear();
		normal_indices.clear();
	}
	// TODO Assignment 2: clear the material in the uniform list before rendering
}
