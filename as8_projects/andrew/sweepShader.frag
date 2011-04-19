// this is the texture map data we can sample
uniform sampler2D textureMap, heightMap, normalMap;
uniform sampler2D skyboxNorth, skyboxSouth, skyboxEast, skyboxWest, skyboxUp, skyboxDown;

// these are the texture and bump mapping settings we can toggle
uniform bool bumpMapEnabled, textureMapEnabled, phongEnabled;
uniform bool displacementEnabled, environmentEnabled;
uniform mat4 inverseOrientation;

// These varying values are interpolated over the surface
varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;

void main()
{
    
    
    
    
    // normalize the eye, light, and reflection vectors
    vec3 e = normalize(v).xyz; // in eye space, eye is at origin
    
    
    // sample from a texture map
    vec4 texcolor;
    vec2 diff = vec2(0.0, 0.0);
    if (textureMapEnabled) {
        if (bumpMapEnabled) {
            if (!displacementEnabled) {
                float depth = (1.0 - texture2D(heightMap, gl_TexCoord[0].st)[0]) * 0.05;
                float diffX = dot(e, normalize(gl_NormalMatrix*t));
                float diffY = dot(e, normalize(gl_NormalMatrix*b));
                diff = depth * vec2(diffX, diffY);
            }
            texcolor = texture2D(textureMap, gl_TexCoord[0].st + diff);
        }
        else {
            texcolor = texture2D(textureMap, gl_TexCoord[0].st);
        }
    } else {
        texcolor = vec4(1,1,1,1);
    }
    // HINT: you can also sample from the height map for parallax bump mapping (extra credit option)
    
    
    
    
    // sample from a normal map
    vec3 normal = n;
    if (bumpMapEnabled) {
        // @TODO: put in a correct normal here
        if (phongEnabled) {
            vec3 tempNormal = normalize(n);
            vec3 tangent = normalize(t);
            vec3 bitangent = normalize(b);
            vec4 bumpNormal = texture2D(normalMap, gl_TexCoord[0].st + diff);
            //normal = normalize(tangent*bumpNormal[0] + bitangent*bumpNormal[1] + tempNormal*bumpNormal[2]);
            normal = normalize(tangent*2*(bumpNormal[0]-0.5) + bitangent*2*(bumpNormal[1]-0.5) + tempNormal*2*(bumpNormal[2]-0.5));
            //normal = normalize(tangent*2*(bumpNormal[1]-0.5) + bitangent*2*(bumpNormal[0]-0.5) + tempNormal*2*(bumpNormal[2]-0.5));
            //normal = bitangent;
        }
    } else {
        if (phongEnabled) {
            normal = normalize(normal);
        }
    }
    
    normal = gl_NormalMatrix * normal;
    
    // light sources are in eye space; it is okay to assume directional light
    vec3 l = normalize(gl_LightSource[0].position.xyz);
    vec3 r = reflect(-l,normal); // reflect function assumes vectors are normalized
    
    
    
    // Environment mapping
    if (environmentEnabled) {
        //texcolor = vec4(r.x,r.y,r.z,1); <-- trippy colors
        vec4 rr = inverseOrientation * vec4(r, 0);
        rr.x = -rr.x;
        rr.z = -rr.z;
        float xx = abs(rr.x);
        float yy = abs(rr.y);
        float zz = abs(rr.z);
        float s, t;
        if (xx > yy && xx > zz) {
            s = rr.z / rr.x / 2.0 + 0.5;
            t = rr.y / xx / 2.0 + 0.5;
            if (rr.x > 0) {
                texcolor = texture2D(skyboxWest, vec2(s,t));
            }
            else {
                texcolor = texture2D(skyboxEast, vec2(s,t));
            }
        }
        else if (yy > xx && yy > zz) {
            s = rr.x / yy / 2.0 + 0.5;
            t = rr.z / rr.y / 2.0 + 0.5;
            if (rr.y > 0) {
                texcolor = texture2D(skyboxUp, vec2(s,t));
            }
            else {
                texcolor = texture2D(skyboxDown, vec2(s,t));
            }
        }
        else {
            s = -rr.x / rr.z / 2.0 + 0.5;
            t = rr.y / zz / 2.0 + 0.5;
            if (rr.z > 0) {
                texcolor = texture2D(skyboxNorth, vec2(s,t));
            }
            else {
                texcolor = texture2D(skyboxSouth, vec2(s,t));
            }
        }
    }
    
    
    
    
    // @TODO: transform normal in to eye space
    // HINT: also use the normalize() function to normalize things
    // You may also want something like "if (phongEnabled) { etc" here-ish
    
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
    color = cr * (ca + cl * max(0.0,dot(normal,l))) + 
                    cl * pow(max(0.0,dot(r,-e)),p);
                    //0.0;
                    
    // set the output color to what we've computed
    gl_FragColor = color;
}
