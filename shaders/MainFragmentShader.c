//texture maps
uniform sampler2D textureMap, heightMap, normalMap; //0,1,2
uniform sampler2D shadowMap; //7

//toggled settings
uniform bool bumpMapEnabled, textureMapEnabled, phongEnabled;
uniform bool shadowMapEnabled, ambientOcclusionEnabled;


// These varying values are interpolated over the surface
varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;

varying vec4 gouradColorDiffuse;
varying vec4 gouradColorSpecular;

varying vec4 shadowCoord;

float chebyshevUpperBound( vec4 shadowCoordPostW)
{
	vec2 moments = texture2D(shadowMap,shadowCoordPostW.xy).rg;
	
	// Surface is fully lit. as the current fragment is before the light occluder
	if (shadowCoordPostW.z <= moments.x)
		return 1.0 ;

	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x*moments.x);
	variance = max(variance,0.02);

	float d = shadowCoordPostW.z - moments.x;
	float p_max = variance / (variance + d*d);

	return max(p_max, 0.0);
}


void main()
{	
  vec4 shadowCoordPostW;
  shadowCoordPostW = shadowCoord / shadowCoord.w;
  shadowCoordPostW.z += 0.002;

  float ao = 1.0;
  float shadow = 1.0;

  vec4 shadowCoordinateWdivide;
  float distanceFromLight;

  // shadows
  if(shadowMapEnabled)
      shadow = chebyshevUpperBound(shadowCoordPostW);

  // sample from a texture map
  vec4 texcolor;
  if (textureMapEnabled) {
    texcolor = texture2D(textureMap,gl_TexCoord[0].st);
  } else {
    texcolor = vec4(1,1,1,1);
  }
  vec4 cr = texcolor;
    

  vec4 color;
  if(phongEnabled) {

    // sample from a normal map

    vec3 normal;
  
    if (bumpMapEnabled) {
      normal = normalize ( (texture2D(normalMap, gl_TexCoord[0].st).xyz) - vec3(0.5));
    } else {
      normal = vec3(0,0,1);
    }
    normal = mat3(normalize(t),normalize(b),normalize(n)) * normal;
    normal = normalize(normal);

    // light sources are in eye space; it is okay to assume directional light
    vec3 l = normalize(gl_LightSource[0].position.xyz);
    vec3 r = reflect(-l,normal); // reflect function assumes vectors are normalized

    // normalize the eye, light, and reflection vectors
    vec3 e = normalize(v).xyz; // in eye space, eye is at origin
    
    float p = 20.0;
    vec4 cl = gl_LightSource[0].diffuse;
    vec4 ca = vec4(.2,.2,.2,1.0);

    
    // the below is fine for a lighting equation
    color = cr * (ca + cl * max(0.0,dot(normal,l))) + 
      cl * pow(max(0.0,dot(r,-e)),p);

  } else {

    color = cr * gouradColorDiffuse + gouradColorSpecular;
  }
  // set the output color to what we've computed
  gl_FragColor = shadow * color * ao;
  
  //gl_FragColor = (texture2D(normalMap, gl_TexCoord[0].st));
}
