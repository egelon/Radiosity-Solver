#version 120

// Input
attribute vec3 lightPosition;

attribute vec3 vertexPosition;
attribute vec3 diffuseColor;
attribute vec3 specularColor;

//Uniforms
uniform mat4 MVP;

//Output
varying vec3 fragmentDiffuseColor;
varying vec3 fragmentSpecularColor;
varying float specularIntensity;
varying float diffuseIntensity;

void main() {
	
	vec4 vertex_position_camera = MVP * vec4(vertexPosition, 1);
	vec3 normal_camera = normalize(gl_NormalMatrix * gl_Normal);
	vec4 light_position_camera = MVP * vec4(lightPosition, 1);
	
	vec3 light_vert = normalize(vec3(light_position_camera - vertex_position_camera));
	vec3 light_refl = normalize(reflect(light_vert, normal_camera));
	
	//diffuse light
	out_diffuseIntensity = max(dot(light_vert, normal_camera), 0.0);
	
	//specular light
	specularIntensity = max(dot(light_refl, normalize(vec3(vertex_position_camera))), 0.0);
	specularIntensity = pow(specularIntensity, 6.0);
	
	fragmentDiffuseColor = diffuseColor;
	fragmentSpecularColor= specularColor;

	//We don't change the vertex position
	gl_Position = vertex_position_camera;
}