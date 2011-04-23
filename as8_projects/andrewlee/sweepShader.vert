uniform sampler2D heightMap;
uniform bool displacementEnabled;

attribute vec3 tangent;
attribute vec3 bitangent;

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

void main()
{
    // pass down the texture coords
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
    // put everything varying in eye space
    v = gl_Vertex;
    if (displacementEnabled) {
        v += 0.1 * (texture2D(heightMap, gl_TexCoord[0].st)[0] - 0.5) * vec4(gl_Normal, 0);
        //v = vec4(0,0,0,0);
    }
    // @TODO: SET t,b,n appropriately
    // HINT: remember normals transform by the gl_NormalMatrix
    n = gl_Normal;
    t = tangent;
    b = bitangent;
    
    // position by the standard transform
    // as this shader doesn't change geometry
    gl_Position = gl_ModelViewProjectionMatrix * v;
    
    v = gl_ModelViewMatrix * v;
}