// Used for shadow lookup
varying vec4 shadowCoord;
attribute vec3 tangent;
attribute vec3 bitangent;

uniform sampler2D textureMap, heightMap, normalMap;

varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;

uniform bool phongEnabled;
uniform bool displacementEnabled;

varying vec2 uv;

void main()
{

 // put everything varying in eye space
  v = gl_ModelViewMatrix * gl_Vertex;

  n=normalize(gl_NormalMatrix * gl_Normal);
  t=normalize(gl_NormalMatrix * tangent);
  b=normalize(gl_NormalMatrix * bitangent);

  // pass down the texture coords
  gl_TexCoord[0] = gl_MultiTexCoord0;

  
    gl_Position =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

  shadowCoord= gl_TextureMatrix[7] * gl_Vertex;
}
