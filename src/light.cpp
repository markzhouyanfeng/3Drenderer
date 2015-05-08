#include "light.h"
#include "object.h"
#include "opengl.h"

lighthdl::lighthdl()
{
	model = NULL;
	type = "light";
}

lighthdl::lighthdl(const vec3f &ambient, const vec3f &diffuse, const vec3f &specular)
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
	model = NULL;
	type = "light";
}

lighthdl::~lighthdl()
{

}

directionalhdl::directionalhdl() : lighthdl(white*0.1f, white*0.5f, white)
{
	type = "directional";
}

directionalhdl::directionalhdl(const vec3f &direction, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular) : lighthdl(ambient, diffuse, specular)
{
	type = "directional";
}

directionalhdl::~directionalhdl()
{

}

void directionalhdl::update()
{
	/* TODO Assignment 2: Update the direction of the light using the orientation of the attached model.
	 * The easiest thing is to do translations and rotations like you were going to render the object, and
	 * then just multiply some initial direction vector by the normal matrix.
	 */
	if(model)
	{
		glTranslatef(model->position[0], model->position[1], model->position[2]);
		glRotatef(radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);
		glRotatef(radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);

		GLdouble modelview[16];
		glGetDoublev( GL_TRANSPOSE_MODELVIEW_MATRIX, modelview );
		mat4f mdvl;
		for(int i = 0; i < 16; i++){
			int row = i / 4;
			int col = i % 4;
			mdvl[row][col] = modelview[i];
		}

		mat4f normal = transpose(inverse(mdvl));
		direction = normal * vec4f(0.0, 0.0, -1.0, 0.0);

		glRotatef(-radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);
		glRotatef(-radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(-radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);		 	
	 	glTranslatef(-model->position[0], -model->position[1], -model->position[2]);
	}
}

void directionalhdl::apply(string name, GLuint program)
{
	/* TODO Assignment 3: Pass all necessary uniforms to the shaders for the directional light.
	 */
	
	int ambient_location = glGetUniformLocation(program, (name + ".ambient").c_str());
	int diffuse_location = glGetUniformLocation(program, (name + ".diffuse").c_str());
	int specular_location = glGetUniformLocation(program, (name + ".specular").c_str());
	int direction_location = glGetUniformLocation(program, (name + ".direction").c_str());

	glUniform3f(ambient_location, ambient[0], ambient[0], ambient[0]);
	glUniform3f(diffuse_location, diffuse[0], diffuse[1], diffuse[2]);
	glUniform3f(specular_location, specular[0], specular[1], specular[2]);
	glUniform3f(direction_location, direction[0], direction[1], direction[2]);
}

pointhdl::pointhdl() : lighthdl(white*0.1f, white*0.5f, white)
{
	this->attenuation = vec3f(1.0, 0.05, 0.25);
	type = "point";
}

pointhdl::pointhdl(const vec3f &position, const vec3f &attenuation, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular) : lighthdl(ambient, diffuse, specular)
{
	this->attenuation = attenuation;
	type = "point";
}

pointhdl::~pointhdl()
{

}

void pointhdl::update()
{
	/* TODO Assignment 2: Update the position of the light using the position of the attached model.
	 * The easiest thing is to do translations and rotations like you were going to render the object, and
	 * then just multiply the origin by the modelview matrix.
	 */
	if(model)
	{
		glTranslatef(model->position[0], model->position[1], model->position[2]);
		glRotatef(radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);
		glRotatef(radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);

		GLdouble modelview[16];
		glGetDoublev( GL_TRANSPOSE_MODELVIEW_MATRIX, modelview );
		mat4f mdvl;
		for(int i = 0; i < 16; i++){
			int row = i / 4;
			int col = i % 4;
			mdvl[row][col] = modelview[i];
		}

		vec4f p = mdvl * vec4f(0.0, 0.0, 0.0, 1.0);
		position = p(0,3)/p[3];

		glRotatef(-radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);
		glRotatef(-radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(-radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);		 	
	 	glTranslatef(-model->position[0], -model->position[1], -model->position[2]);
	}
}

void pointhdl::apply(string name, GLuint program)
{
	/* TODO Assignment 3: Pass all necessary uniforms to the shaders for point lights.
	 */

	int ambient_location = glGetUniformLocation(program, (name + ".ambient").c_str());
	int diffuse_location = glGetUniformLocation(program, (name + ".diffuse").c_str());
	int specular_location = glGetUniformLocation(program, (name + ".specular").c_str());
	int position_location = glGetUniformLocation(program, (name + ".position").c_str());
	int attenuation_location = glGetUniformLocation(program, (name + ".attenuation").c_str());

	glUniform3f(ambient_location, ambient[0], ambient[0], ambient[0]);
	glUniform3f(diffuse_location, diffuse[0], diffuse[1], diffuse[2]);
	glUniform3f(specular_location, specular[0], specular[1], specular[2]);
	glUniform3f(position_location, position[0], position[1], position[2]);
	glUniform3f(attenuation_location, attenuation[0], attenuation[1], attenuation[2]);
}

spothdl::spothdl() : lighthdl(white*0.1f, white*0.5f, white)
{
	this->attenuation = vec3f(1.0, 0.05, 0.25);
	this->cutoff = 0.5;
	this->exponent = 1.0;
	type = "spot";
}

spothdl::spothdl(const vec3f &attenuation, const float &cutoff, const float &exponent, const vec3f &ambient, const vec3f &diffuse, const vec3f &specular) : lighthdl(ambient, diffuse, specular)
{
	this->attenuation = attenuation;
	this->cutoff = cutoff;
	this->exponent = exponent;
	type = "spot";
}

spothdl::~spothdl()
{

}

void spothdl::update()
{
	/* TODO Assignment 2: Update both the direction and position of the light using the position and orientation
	 * of the attached model. See above.
	 */
	if(model)
	{
		glTranslatef(model->position[0], model->position[1], model->position[2]);
		glRotatef(radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);
		glRotatef(radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);

		GLdouble modelview[16];
		glGetDoublev( GL_TRANSPOSE_MODELVIEW_MATRIX, modelview );
		mat4f mdvl;
		for(int i = 0; i < 16; i++){
			int row = i / 4;
			int col = i % 4;
			mdvl[row][col] = modelview[i];
		}

		mat4f normal = transpose(inverse(mdvl));
		direction = normal * vec4f(0.0, 0.0, -1.0, 0.0);

		vec4f p = mdvl * vec4f(0.0, 0.0, 0.0, 1.0);
		position = p(0,3)/p[3];

		glRotatef(-radtodeg(model->orientation[2]), 0.0, 0.0, 1.0);
		glRotatef(-radtodeg(model->orientation[1]), 0.0, 1.0, 0.0);
		glRotatef(-radtodeg(model->orientation[0]), 1.0, 0.0, 0.0);		 	
	 	glTranslatef(-model->position[0], -model->position[1], -model->position[2]);
	}
}

void spothdl::apply(string name, GLuint program)
{
	/* TODO Assignment 3: Pass all necessary uniforms to the shaders for spot lights.
	 */

	int ambient_location = glGetUniformLocation(program, (name + ".ambient").c_str());
	int diffuse_location = glGetUniformLocation(program, (name + ".diffuse").c_str());
	int specular_location = glGetUniformLocation(program, (name + ".specular").c_str());
	int direction_location = glGetUniformLocation(program, (name + ".direction").c_str());
	int position_location = glGetUniformLocation(program, (name + ".position").c_str());
	int attenuation_location = glGetUniformLocation(program, (name + ".attenuation").c_str());
	int cutoff_location = glGetUniformLocation(program, (name + ".cutoff").c_str());
	int exponent_location = glGetUniformLocation(program, (name + ".exponent").c_str());

	glUniform3f(ambient_location, ambient[0], ambient[0], ambient[0]);
	glUniform3f(diffuse_location, diffuse[0], diffuse[1], diffuse[2]);
	glUniform3f(specular_location, specular[0], specular[1], specular[2]);
	glUniform3f(direction_location, direction[0], direction[1], direction[2]);
	glUniform3f(position_location, position[0], position[1], position[2]);
	glUniform3f(attenuation_location, attenuation[0], attenuation[1], attenuation[2]);
	glUniform1f(cutoff_location, cutoff);
	glUniform1f(exponent_location, exponent);
}
