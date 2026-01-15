#version 330 core

in vec2 TexCoord;
in vec3 Normal; 
in vec3 Vertex; 

//Texture sampler
uniform sampler2D texture_diffuse1;

//Camera location
uniform vec3 eyePos;

//Light information
uniform vec4		lightPositions[5];
uniform vec4		lightAmbient;
uniform vec4		lightDiffusers[5];
uniform vec3		lightAttenuation; // x=constant, y=linear, z=quadratic (x<0 means light is not active)

//Material iformation
uniform vec4		matAmbient;
uniform vec4		matDiffuse;
uniform vec4        matSpecularColour;
uniform float       matSpecularExponent;

out vec4 FragColour;

vec4 Lights[5];

//Function prototype
vec4 calculateLight(vec4 lightPosition, vec4 lightDiffuse);

void main()
{
	//Calling the light functions
	for (int i = 0; i < 5; i++)
	{
		Lights[i] = calculateLight(lightPositions[i], lightDiffusers[i]);
	}

	FragColour = Lights[0] + Lights[1] + Lights[2] + Lights[3] + Lights[4];
}

vec4 calculateLight(vec4 lightPosition, vec4 lightDiffuse)
{
	//Attenuation/drop-off	
	float d = length(lightPosition.xyz - Vertex);
	float att = 1.0 / (lightAttenuation.x + lightAttenuation.y * d + lightAttenuation.z * (d * d));

	//Ambient light value
	vec4 texColour = texture(texture_diffuse1, TexCoord);
	vec4 ambient = lightAmbient * matAmbient * texColour * att;

	//Diffuse light value
	vec3 N = normalize(Normal);	
	vec3 L = normalize(lightPosition.xyz - Vertex);
	float lambertTerm = clamp(dot(N, L), 0.0, 1.0);
	vec4 diffuse = lightDiffuse * matDiffuse * lambertTerm * texColour * att;

	//Specular light value
	vec3 E = normalize(eyePos - Vertex);
	vec3 R = reflect(-L, N); // reflected light vector about normal N
	float specularIntensity = pow(max(dot(E, R), 0.0), matSpecularExponent);
	vec4 specular = matSpecularColour * specularIntensity * texColour * att;

	return ambient + diffuse + specular;
}