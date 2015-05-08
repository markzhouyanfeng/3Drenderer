struct directional
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	vec3 direction;
};

void shade_directional(directional light, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular, vec3 vertex, vec3 normal, float shininess)
{
	vec3 half_vector = normalize(light.direction - vertex);

	float nDotVP = max(0.0, dot(normal, light.direction));
	float nDotHV = max(0.0, dot(normal, half_vector));

	float power_factor = 0.0;
	if (nDotVP > 0.0)
		power_factor = pow(nDotHV, shininess);

	ambient += light.ambient;
	diffuse += light.diffuse*nDotVP;
	specular += light.specular*power_factor;
}

struct point
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	
	vec3 attenuation;
	
	vec3 position;
};

void shade_point(point light, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular, vec3 vertex, vec3 normal, float shininess)
{
	vec3 light_direction = light.position - vertex;
	float light_distance = length(light_direction);
	light_direction /= light_distance;

	float attenuation = 1.0/(light.attenuation[0] + light.attenuation[1]*light_distance + light.attenuation[2]*light_distance*light_distance);

	vec3 half_vector = normalize(light_direction - vertex);

	float nDotVP = max(0.0, dot(normal, light_direction));
	float nDotHV = max(0.0, dot(normal, half_vector));

	float power_factor = 0.0;
	if (nDotVP > 0.0)
		power_factor = pow(nDotHV, shininess);
    
	ambient += light.ambient*attenuation;
	diffuse += light.diffuse*nDotVP*attenuation;
	specular += light.specular*power_factor*attenuation;
}

struct spot
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	vec3 attenuation;
	float cutoff;
	float exponent;
	
	vec3 position;
	vec3 direction;
};

void shade_spot(spot light, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular, vec3 vertex, vec3 normal, float shininess)
{
	vec3 light_direction = light.position - vertex;
	float light_distance = length(light_direction);
	light_direction /= light_distance;

	float attenuation = 1.0/(light.attenuation[0] + light.attenuation[1]*light_distance + light.attenuation[2]*light_distance*light_distance);
	float spotdot = dot(-light_direction, light.direction);

	float spotAttenuation = 0.0;
	if (spotdot >= light.cutoff)
		spotAttenuation = pow(spotdot, light.exponent);

	attenuation *= spotAttenuation;

	vec3 half_vector = normalize(light_direction -vertex);
	float nDotVP = max(0.0, dot(normal, light_direction));
	float nDotHV = max(0.0, dot(normal, half_vector));

	float power_factor = 0.0;
	if (nDotVP > 0.0)
		power_factor = pow(nDotHV, shininess);

	ambient += light.ambient*attenuation;
	diffuse += light.diffuse*nDotVP*attenuation;
	specular += light.specular*power_factor*attenuation;
}

uniform directional dlights[6];
uniform int num_dlights;

uniform point plights[6];
uniform int num_plights;

uniform spot slights[6];
uniform int num_slights;

vec3 lighting(vec3 emission, vec3 ambient, vec3 diffuse, vec3 specular, float shininess, vec3 vertex, vec3 normal)
{
	vec3 light_ambient = vec3(0.0, 0.0, 0.0);
	vec3 light_diffuse = vec3(0.0, 0.0, 0.0);
	vec3 light_specular = vec3(0.0, 0.0, 0.0);

	for (int j = 0; j < num_dlights && j < 6; j++)
		shade_directional(dlights[j], light_ambient, light_diffuse, light_specular, vertex, normal, shininess);
		
	for (int j = 0; j < num_plights && j < 6; j++)
		shade_point(plights[j], light_ambient, light_diffuse, light_specular, vertex, normal, shininess);
	
	for (int j = 0; j < num_slights && j < 6; j++)
		shade_spot(slights[j], light_ambient, light_diffuse, light_specular, vertex, normal, shininess);
	
	return clamp(emission + ambient*light_ambient + diffuse*light_diffuse + specular*light_specular, 0.0, 1.0);
}
