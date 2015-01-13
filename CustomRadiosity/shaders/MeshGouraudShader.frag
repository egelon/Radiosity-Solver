#version 120

//Input
varying vec3 fragmentDiffuseColor;
varying vec3 fragmentSpecularColor;
varying float specularIntensity;
varying float diffuseIntensity;

void main() {
	gl_FragColor = fragmentDiffuseColor * diffuseIntensity + fragmentSpecularColor * specularIntensity;
}