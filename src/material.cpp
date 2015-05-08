#include "material.h"
#include "light.h"
#include "lodepng.h"

GLuint whitehdl::vertex = 0;
GLuint whitehdl::fragment = 0;
GLuint whitehdl::program = 0;

GLuint gouraudhdl::vertex = 0;
GLuint gouraudhdl::fragment = 0;
GLuint gouraudhdl::program = 0;

GLuint phonghdl::vertex = 0;
GLuint phonghdl::fragment = 0;
GLuint phonghdl::program = 0;

GLuint brickhdl::vertex = 0;
GLuint brickhdl::fragment = 0;
GLuint brickhdl::program = 0;

GLuint texturehdl::vertex = 0;
GLuint texturehdl::fragment = 0;
GLuint texturehdl::program = 0;
GLuint texturehdl::texture = 0;

extern string working_directory;

materialhdl::materialhdl()
{
	type = "material";
}

materialhdl::~materialhdl()
{
}

gouraudhdl::gouraudhdl()
{
	type = "gouraud";
	emission = vec3f(0.0, 0.0, 0.0);
	ambient = vec3f(0.1, 0.1, 0.1);
	diffuse = vec3f(1.0, 1.0, 1.0);
	specular = vec3f(1.0, 1.0, 1.0);
	shininess = 1.0;

	if (vertex == 0 && fragment == 0 && program == 0)
	{
		/* TODO Assignment 3: Load and link the shaders. Keep in mind that vertex, fragment, 
		 * and program are static variables meaning they are *shared across all instances of
		 * this class. So you only have to initialize them once when the first instance of
		 * the class is created.
		 */
		glEnable(GL_DEPTH_TEST);
		vertex = load_shader_file(working_directory + "res/gouraud.vx", GL_VERTEX_SHADER);
		fragment = load_shader_file(working_directory + "res/gouraud.ft", GL_FRAGMENT_SHADER);
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		//glBindFragDataLocation(program, 0, "frag_color");
		glLinkProgram(program);
	}
}

gouraudhdl::~gouraudhdl()
{

}

void gouraudhdl::apply(const vector<lighthdl*> &lights)
{
	// TODO Assignment 3: Apply the shader program and pass it the necessary uniform values
	glUseProgram(program);

	int emission_location = glGetUniformLocation(program, "emission");
	int ambient_location = glGetUniformLocation(program, "ambient");
	int diffuse_location = glGetUniformLocation(program, "diffuse");
	int specular_location = glGetUniformLocation(program, "specular");
	int shininess_location = glGetUniformLocation(program, "shininess");

	glUniform3f(emission_location, 0.0, 0.0, 0.0);
	glUniform3f(ambient_location, 0.1, 0.1, 0.1);
	glUniform3f(diffuse_location, 1.0, 1.0, 1.0);
	glUniform3f(specular_location, 1.0, 1.0, 1.0);
	glUniform1f(shininess_location, 1.0);

	int dl = 0, pl = 0, sl = 0;
	for(int i=0; i<lights.size(); i++)
	{
		if(lights[i]->type=="directional")
			lights[i]->apply("dlights[" + to_string(dl++) + "]", program);
		else if(lights[i]->type=="point")
			lights[i]->apply("plights[" + to_string(pl++) + "]", program);
		else if(lights[i]->type=="spot")
			lights[i]->apply("slights[" + to_string(sl++) + "]", program);
	}
	int num_dlights_location = glGetUniformLocation(program, "num_dlights");
	int num_plights_location = glGetUniformLocation(program, "num_plights");
	int num_slights_location = glGetUniformLocation(program, "num_slights");
	glUniform1i(num_dlights_location, dl);
	glUniform1i(num_plights_location, pl);
	glUniform1i(num_slights_location, sl);
}

materialhdl *gouraudhdl::clone() const
{
	gouraudhdl *result = new gouraudhdl();
	result->type = type;
	result->emission = emission;
	result->ambient = ambient;
	result->diffuse = diffuse;
	result->specular = specular;
	result->shininess = shininess;
	return result;
}

phonghdl::phonghdl()
{
	type = "phong";
	emission = vec3f(0.0, 0.0, 0.0);
	ambient = vec3f(0.1, 0.1, 0.1);
	diffuse = vec3f(1.0, 1.0, 1.0);
	specular = vec3f(1.0, 1.0, 1.0);
	shininess = 1.0;

	if (vertex == 0 && fragment == 0 && program == 0)
	{
		/* TODO Assignment 3: Load and link the shaders. Keep in mind that vertex, fragment, 
		 * and program are static variables meaning they are *shared across all instances of
		 * this class. So you only have to initialize them once when the first instance of
		 * the class is created.
		 */
		glEnable(GL_DEPTH_TEST);
		vertex = load_shader_file(working_directory + "res/phong.vx", GL_VERTEX_SHADER);
		fragment = load_shader_file(working_directory + "res/phong.ft", GL_FRAGMENT_SHADER);
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		//glBindFragDataLocation(program, 0, "frag_color");
		glLinkProgram(program);
	}
}

phonghdl::~phonghdl()
{

}

void phonghdl::apply(const vector<lighthdl*> &lights)
{
	// TODO Assignment 3: Apply the shader program and pass it the necessary uniform values
	glUseProgram(program);

	int emission_location = glGetUniformLocation(program, "emission");
	int ambient_location = glGetUniformLocation(program, "ambient");
	int diffuse_location = glGetUniformLocation(program, "diffuse");
	int specular_location = glGetUniformLocation(program, "specular");
	int shininess_location = glGetUniformLocation(program, "shininess");

	glUniform3f(emission_location, 0.0, 0.0, 0.0);
	glUniform3f(ambient_location, 0.1, 0.1, 0.1);
	glUniform3f(diffuse_location, 1.0, 1.0, 1.0);
	glUniform3f(specular_location, 1.0, 1.0, 1.0);
	glUniform1f(shininess_location, 1.0);

	int dl = 0, pl = 0, sl = 0;
	for(int i=0; i<lights.size(); i++)
	{
		if(lights[i]->type=="directional")
			lights[i]->apply("dlights[" + to_string(dl++) + "]", program);
		else if(lights[i]->type=="point")
			lights[i]->apply("plights[" + to_string(pl++) + "]", program);
		else if(lights[i]->type=="spot")
			lights[i]->apply("slights[" + to_string(sl++) + "]", program);
	}
	int num_dlights_location = glGetUniformLocation(program, "num_dlights");
	int num_plights_location = glGetUniformLocation(program, "num_plights");
	int num_slights_location = glGetUniformLocation(program, "num_slights");
	glUniform1i(num_dlights_location, dl);
	glUniform1i(num_plights_location, pl);
	glUniform1i(num_slights_location, sl);
}

materialhdl *phonghdl::clone() const
{
	phonghdl *result = new phonghdl();
	result->type = type;
	result->emission = emission;
	result->ambient = ambient;
	result->diffuse = diffuse;
	result->specular = specular;
	result->shininess = shininess;
	return result;
}

whitehdl::whitehdl()
{
	type = "white";

	if (vertex == 0 && fragment == 0 && program == 0)
	{
		/* TODO Assignment 3: Load and link the shaders. Keep in mind that vertex, fragment, 
		 * and program are static variables meaning they are *shared across all instances of
		 * this class. So you only have to initialize them once when the first instance of
		 * the class is created.
		 */
		glEnable(GL_DEPTH_TEST);
		vertex = load_shader_file(working_directory + "res/white.vx", GL_VERTEX_SHADER);
		fragment = load_shader_file(working_directory + "res/white.ft", GL_FRAGMENT_SHADER);
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		//glBindFragDataLocation(program, 0, "frag_color");
		glLinkProgram(program);
	}
}

whitehdl::~whitehdl()
{

}

void whitehdl::apply(const vector<lighthdl*> &lights)
{
	glUseProgram(program);
}

materialhdl *whitehdl::clone() const
{
	whitehdl *result = new whitehdl();
	result->type = type;
	return result;
}

brickhdl::brickhdl()
{
	type = "brick";

	if (vertex == 0 && fragment == 0 && program == 0)
	{
		/* TODO Assignment 3: Load and link the shaders. Keep in mind that vertex, fragment, 
		 * and program are static variables meaning they are *shared across all instances of
		 * this class. So you only have to initialize them once when the first instance of
		 * the class is created.
		 */
		glEnable(GL_DEPTH_TEST);
		vertex = load_shader_file(working_directory + "res/brick.vx", GL_VERTEX_SHADER);
		fragment = load_shader_file(working_directory + "res/brick.ft", GL_FRAGMENT_SHADER);
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		//glBindFragDataLocation(program, 0, "frag_color");
		glLinkProgram(program);
	}
}

brickhdl::~brickhdl()
{

}

void brickhdl::apply(const vector<lighthdl*> &lights)
{
	// TODO Assignment 3: Apply the shader program and pass it the necessary uniform values
	glUseProgram(program);

	int dl = 0, pl = 0, sl = 0;
	for(int i=0; i<lights.size(); i++)
	{
		if(lights[i]->type=="directional")
			lights[i]->apply("dlights[" + to_string(dl++) + "]", program);
		else if(lights[i]->type=="point")
			lights[i]->apply("plights[" + to_string(pl++) + "]", program);
		else if(lights[i]->type=="spot")
			lights[i]->apply("slights[" + to_string(sl++) + "]", program);
	}
	int num_dlights_location = glGetUniformLocation(program, "num_dlights");
	int num_plights_location = glGetUniformLocation(program, "num_plights");
	int num_slights_location = glGetUniformLocation(program, "num_slights");
	glUniform1i(num_dlights_location, dl);
	glUniform1i(num_plights_location, pl);
	glUniform1i(num_slights_location, sl);
}

materialhdl *brickhdl::clone() const
{
	brickhdl *result = new brickhdl();
	result->type = type;
	return result;
}

texturehdl::texturehdl()
{
	type = "texture";

	shininess = 1.0;

	if (vertex == 0 && fragment == 0 && program == 0)
	{
		/* TODO Assignment 3: Load and link the shaders and load the texture Keep in mind that vertex, fragment,
		 * and program are static variables meaning they are *shared across all instances of
		 * this class. So you only have to initialize them once when the first instance of
		 * the class is created.
		 */
		glEnable(GL_DEPTH_TEST);
		vertex = load_shader_file(working_directory + "res/texture.vx", GL_VERTEX_SHADER);
		fragment = load_shader_file(working_directory + "res/texture.ft", GL_FRAGMENT_SHADER);
		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		//glBindFragDataLocation(program, 0, "frag_color");
		glLinkProgram(program);

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		unsigned error;
		unsigned char* image;
		size_t width, height;
		const char* filename = "res/texture.png";

		error = lodepng_decode32_file(&image, &width, &height, filename);

		if(error) printf("decoder error %u: %s\n", error, lodepng_error_text(error));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		free(image);
	}
}

texturehdl::~texturehdl()
{

}

void texturehdl::apply(const vector<lighthdl*> &lights)
{
	// TODO Assignment 3: Apply the shader program and pass it the necessary uniform values
	glUseProgram(program);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	int tex_location = glGetUniformLocation(program, "tex");
	glUniform1i(tex_location, 0);
	
	//cout<<"texture: "<<texture<<endl;
	int shininess_location = glGetUniformLocation(program, "shininess");
	glUniform1f(shininess_location, 1.0);

	int dl = 0, pl = 0, sl = 0;
	for (int i=0; i<lights.size(); i++)
	{
		if (lights[i]->type=="directional")
			lights[i]->apply("dlights[" + to_string(dl++) + "]", program);
		else if (lights[i]->type=="point")
			lights[i]->apply("plights[" + to_string(pl++) + "]", program);
		else if (lights[i]->type=="spot")
			lights[i]->apply("slights[" + to_string(sl++) + "]", program);
	}
	int num_dlights_location = glGetUniformLocation(program, "num_dlights");
	int num_plights_location = glGetUniformLocation(program, "num_plights");
	int num_slights_location = glGetUniformLocation(program, "num_slights");
	glUniform1i(num_dlights_location, dl);
	glUniform1i(num_plights_location, pl);
	glUniform1i(num_slights_location, sl);
}

materialhdl *texturehdl::clone() const
{
	texturehdl *result = new texturehdl();
	result->type = type;
	result->shininess = shininess;
	return result;
}
