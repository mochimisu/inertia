attribute vec3 tangent;
attribute vec3 bitangent;
varying vec4 shadowCoord;

uniform sampler2D textureMap, heightMap, normalMap;

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

uniform bool phongEnabled;
varying vec4 gouradColorDiffuse;
varying vec4 gouradColorSpecular;

uniform bool displacementEnabled;

varying vec2 uv;

void main()
{
  // put everything varying in eye space
  v = gl_ModelViewMatrix * gl_Vertex;

  // @TODO: SET t,b,n appropriately
  // HINT: remember normals transform by the gl_NormalMatrix

  n=normalize(gl_NormalMatrix * gl_Normal);
  t=normalize(gl_NormalMatrix * tangent);
  b=normalize(gl_NormalMatrix * bitangent);

  // pass down the texture coords
  gl_TexCoord[0] = gl_MultiTexCoord0;

  if(!phongEnabled) {
    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
    // light sources are in eye space; it is okay to assume directional light
    vec3 l = normalize(gl_LightSource[0].position.xyz);
    vec3 r = reflect(-l,normal); // reflect function assumes vectors are normalized

    // normalize the eye, light, and reflection vectors
    vec3 e = normalize(v).xyz; // in eye space, eye is at origin
    
    // setting up your light parameters in the shader is fine
    // don't worry about matching the opengl lighting settings
    // the fixed function uses the blinn-phong half-angle formula anyway
    // while we've asked you to use the standard phong model
    float p = 20.0;
    vec4 cl = gl_LightSource[0].diffuse;
    vec4 ca = vec4(.2,.2,.2,1.0);

    
    // the below is fine for a lighting equation
    gouradColorDiffuse = (ca + cl * max(0.0,dot(normal,l)));
    gouradColorSpecular = cl * pow(max(0.0,dot(r,normal)),p);
  }


  
  if(displacementEnabled) {
    vec4 newVertexPos;
    vec4 dv;
    dv = texture2D( heightMap,  gl_TexCoord[0].st ) - vec4(0.5);
    newVertexPos = vec4(gl_Normal * dv.x * 0.25, 0.0) + gl_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix * newVertexPos;
  } else {
    // position by the standard transform
    // as this shader doesn't change geometry
    gl_Position =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
  }

  shadowCoord= gl_TextureMatrix[4] * gl_Vertex;

}
