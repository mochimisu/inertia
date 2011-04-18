
void main()
{
  //dont really care about displacement because we're blurring here
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
  gl_TexCoord[0] =  gl_MultiTexCoord0;
}
