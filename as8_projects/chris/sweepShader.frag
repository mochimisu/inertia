// this is the texture map data we can sample
uniform sampler2D textureMap, heightMap, normalMap;

// these are the texture and bump mapping settings we can toggle
//uniform bool bumpMapEnabled, textureMapEnabled, phongEnabled, toonShadingEnabled;

// These varying values are interpolated over the surface
varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;
varying vec4 v2;

varying vec4 whack;

void main()
{
#ifdef phongEnabled
	// sample from a texture map
	vec4 texcolor;
#ifdef textureMapEnabled
	texcolor = texture2D(textureMap,gl_TexCoord[0].st);
#else
	texcolor = vec4(1,0,0,1);
#endif
	// HINT: you can also sample from the height map for parallax bump mapping (extra credit option)

	// sample from a normal map
	vec3 normal = vec3(0,0,1);
#ifdef bumpMapEnabled
	// @TODO: put in a correct normal here
	normal = texture2D(normalMap, gl_TexCoord[0].st).xyz;
	normal = (normal-vec3(0.5,0.5,0.5))*2.0;
#endif

	mat3 tbn = mat3(normalize(t), normalize(b), normalize(n)); // converts tangent space to object space
	normal = tbn * normal;

	// @TODO: transform normal in to eye space
	// HINT: also use the normalize() function to normalize things
	// You may also want something like "if (phongEnabled) { etc" here-ish
	normal = normalize(gl_NormalMatrix * normal);


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
	vec4 cr = texcolor;
	vec4 cl = gl_LightSource[0].diffuse;
	vec4 ca = vec4(.2,.2,.2,1.0);

	// the below is fine for a lighting equation
	vec4 color;

#ifdef toonShadingEnabled
	p = 150.0;
	color = cr * (floor(4.0*(ca + cl * max(0.0,dot(normal,l)))+0.5)/4.0) + 
		(floor(4.0*cl * pow(max(0.0,dot(r, -e)),p)+0.5)/4.0);
#else
	color = cr * (ca + cl * max(0.0,dot(normal,l))) + 
		cl * pow(max(0.0,dot(r, -e)),p);
#endif

	// set the output color to what we've computed
	gl_FragColor = color;
	//gl_FragColor = vec4(normal.x, normal.y, normal.z, 1);
#else
	gl_FragColor = whack;
#endif
}
