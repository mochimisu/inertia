varying vec4 v_position;
uniform bool displacementEnabled;

uniform sampler2D heightMap;

void main()
{

  //obey displacement (if enabled)
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
    gl_Position =  gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

  v_position = gl_Position;

}		
