#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/vec3.hpp>

using namespace std;

enum IlluminationModes
{
	COLOR,								//0 Color on and Ambient off - THIS MEANS AN OBJECT IS A LIGHT EMITTER
	COLOR_AMBIENT,						//1 Color on and Ambient on
	HIGHLIGHT,							//2 Highlight on
	REFLECTION_RAYTRACE,				//3 Reflection on and Ray trace on
	GLASS_RAYTRACE,						//4 Transparency: Glass on, Reflection: Ray trace on
	FRESNEL_RAYTRACE,					//5 Reflection: Fresnel on and Ray trace on
	REFRACTION_RAYTRACE,				//6 Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
	REFRACTION_FRESNEL_RAYTRACE,		//7 Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
	REFLECTION,							//8 Reflection on and Ray trace off
	GLASS,								//9 Transparency: Glass on, Reflection: Ray trace off
	CAST_SHADOW_ON_INVISIBLE_SURFACE	//10 Casts shadows onto invisible surfaces
};

struct Material
{
	string name;

	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;
	float specularColorExponent;
	float alpha;
	float opticalDensity; //optical density / refraction index. The higher it is, the more the light bends inside the material
	IlluminationModes illuminationMode;
};

#endif