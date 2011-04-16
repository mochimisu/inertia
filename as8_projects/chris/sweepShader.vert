attribute vec3 tangent;
attribute vec3 bitangent;
uniform sampler2D heightMap;

// NOTE:
// varying variables are set by the vertex shader
// then interpolated by the harder 
// and the interpolated values are passed to the fragment shader
// so the corresponding varying vec4 v in the fragment shader
// will be the linearly interpolated position of the spot you're shading.
varying vec4 v;

// NOTE:
// we'll pass the TBN matrix down
// in order to do the lighting in eye space
// many people do it in tangent space instead, for efficiency
// but for the sake of exercise we'll do it all in eye space
varying vec3 t;
varying vec3 b;
varying vec3 n;
varying vec4 v2;

//uniform bool phongEnabled, displacementEnabled;

varying vec4 whack;

void main()
{
	// put everything varying in eye space
	v = gl_ModelViewMatrix * gl_Vertex;
	v2 = gl_Vertex;
	// @TODO: SET t,b,n appropriately
	// HINT: remember normals transform by the gl_NormalMatrix
	t = normalize(tangent);
	b = normalize(bitangent);
	n = cross(t, b);

	n = normalize(n); // shouldn't be necessary

	// pass down the texture coords
	gl_TexCoord[0] = gl_MultiTexCoord0;

	// position by the standard transform
	// as this shader doesn't change geometry
	gl_Position = ftransform();

#ifdef displacementEnabled
		// gl_Vertex is in object space. Let's add n * height map shiz to it.

		float height = texture2D(heightMap, gl_TexCoord[0].st).x;

		height = 2.0*height - 1.0;

		vec3 displacement = gl_NormalMatrix * height * n * 0.1;
		gl_Position = vec4(displacement.x, displacement.y, displacement.z, 0.0) + gl_Position;
		vec3 displacement2 = height * n * 0.2;
		v = gl_ModelViewMatrix * (gl_Vertex + vec4(displacement2.xyz, 0));
#endif

	// gouraud shading stuff in here
#ifndef phongEnabled
		vec3 normal = gl_NormalMatrix * n;
		// normalize the eye, light, and reflection vectors
		vec3 e = normalize(v).xyz; // in eye space, eye is at origin
		// light sources are in eye space; it is okay to assume directional light
		vec3 l = normalize(gl_LightSource[0].position.xyz);
		vec3 r = reflect(-l,normal); // reflect function assumes vectors are normalized

		// setting up your light parameters in the shader is fine
		// don't worry about matching the opengl lighting settings
		// the fixed function uses the blinn-phong half-angle formula anyway
		// while we've asked you to use the standard phong model
		float p = 20.0;
		vec4 cr = vec4(1,0,0,1);
		vec4 cl = gl_LightSource[0].diffuse;
		vec4 ca = vec4(.2,.2,.2,1.0);

		// the below is fine for a lighting equation
		vec4 color;
		color = cr * (ca + cl * max(0.0,dot(normal,l))) + 
			cl * pow(max(0.0,dot(r,normal)),p);
		whack = color;
#endif
}
