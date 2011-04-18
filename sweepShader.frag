
// this is the texture map data we can sample
uniform sampler2D textureMap, heightMap, normalMap; //0,1,2
uniform sampler2D shadowMap; //4
uniform samplerCube skyMap; //3

// these are the texture and bump mapping settings we can toggle
uniform bool bumpMapEnabled, textureMapEnabled, phongEnabled;
uniform bool shadowMapEnabled, ambientOcclusionEnabled, dispAmbientLayer;
uniform bool pcfEnabled, aoPcfEnabled, envEnabled;

uniform float xPixelOffset;
uniform float yPixelOffset;

// These varying values are interpolated over the surface
varying vec4 v;
varying vec3 t;
varying vec3 b;
varying vec3 n;

varying vec4 gouradColorDiffuse;
varying vec4 gouradColorSpecular;

varying vec4 shadowCoord;

float lookupShadow(vec2 offset) {
  vec4 shadowCoordinateWdivide;
  shadowCoordinateWdivide = shadowCoord / shadowCoord.w ;
  vec4 shadowCoordWdivideOffset = shadowCoordinateWdivide+vec4(offset.x*xPixelOffset,offset.y*yPixelOffset, 0.00, 0.0);
  shadowCoordWdivideOffset.z += 0.002;
  //vec4 shadowCoordWdivideOffset = shadowCoordinateWdivide+vec4(offset.x,offset.y, 0.05, 0.0);
  float distanceFromLight = texture2D(shadowMap,shadowCoordWdivideOffset.st).z;
  if(distanceFromLight < shadowCoordWdivideOffset.z)
    return 0.0;
  else
    return 1.0;
}

void main()
{

  float ao = 1.0;
  float shadow = 1.0;

  vec4 shadowCoordinateWdivide;
  float distanceFromLight;

  //compute depth buffer
  if(shadowMapEnabled || ambientOcclusionEnabled) {
    shadowCoordinateWdivide = shadowCoord / shadowCoord.w ;
    // lower self-shadowing and moire effects
    //shadowCoordinateWdivide.z += 0.0005;
    shadowCoordinateWdivide.z += 0.002;
    distanceFromLight = texture2D(shadowMap,shadowCoordinateWdivide.st).z;
  }

  //AMBIENT OCCLUSION

  //random vec inside unit sphere generated with py script
  //need higher version of opengl to use vec3 array... sigh
  const vec3 pr01 = vec3(-0.787673654975, 0.531756376729, 0.311135612019);
  const vec3 pr02 = vec3(0.296058441663, -0.0219710796714, 0.954917101521);
  const vec3 pr03 = vec3(0.51999906092, -0.541746365733, 0.660387652714) ;
  const vec3 pr04 = vec3(-0.167475287849, 0.406544888708, -0.898149921464) ;
  const vec3 pr05 = vec3(0.387117506177, -0.22513657874, -0.894121668077) ;
  const vec3 pr06 = vec3(0.139263648142, 0.323948446159, 0.935768689654) ;
  const vec3 pr07 = vec3(0.239748961548, 0.432392319604, -0.869228000806) ;
  const vec3 pr08 = vec3(-0.00105591095417, 0.775161505594, 0.631762237948) ;
  const vec3 pr09 = vec3(-0.252211990627, -0.104967521218, 0.961962021742) ;
  const vec3 pr10 = vec3(0.878491971318, -0.0898768902419, 0.469227024936) ;
  const vec3 pr11 = vec3(0.228289735059, 0.27180996788, 0.934881349813) ;
  const vec3 pr12 = vec3(0.468951379806, -0.174641181303, 0.865785805596) ;
  const vec3 pr13 = vec3(-0.644938008157, 0.185265480633, -0.741438916783) ;
  const vec3 pr14 = vec3(0.89519310983, -0.17746428039, 0.408822363991) ;
  const vec3 pr15 = vec3(0.306097975713, 0.304030473072, -0.902147161337) ;
  const vec3 pr16 = vec3(0.680206762595, 0.181161725189, -0.710281063697) ;

  float depthdiff;
  const float mindiff = 0.0;
  const float maxdiff = 0.25;
  const float depthscale = 0.4;
  const float vecscale = 0.2;
  if(ambientOcclusionEnabled) {
    depthdiff = texture2D(shadowMap,(vec4(pr01*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr02*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr03*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr04*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr05*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr06*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr07*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr08*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr09*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr10*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr11*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr12*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr13*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr14*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr15*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
    depthdiff = texture2D(shadowMap,(vec4(pr16*vecscale,0.0)+shadowCoordinateWdivide).st).z - distanceFromLight;
    if(depthdiff < -mindiff && depthdiff > -maxdiff)
      ao += depthdiff*depthscale;
  }


  //SHADOWS
  if(shadowMapEnabled) {
    if (shadowCoord.w > 0.0) {
      if(pcfEnabled) {
	float x,y;
	shadow = 0.0;
	/*	 smaller kernel for smaller blur
		for (y = -3.5 ; y <=3.5 ; y+=1.0)
		for (x = -3.5 ; x <=3.5 ; x+=1.0)
	*/
	
	for (y = -15.5 ; y <=15.5 ; y+=5.0)
	  for (x = -15.5 ; x <=15.5 ; x+=5.0)
	    
	    shadow += lookupShadow(vec2(x,y));
	shadow /= 64.0;
	shadow += 0.2;
	
      } else {
	//no need to recompute
	shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;
      }
    }
  }


  // sample from a texture map
  vec4 texcolor;
  if (textureMapEnabled) {
    texcolor = texture2D(textureMap,gl_TexCoord[0].st);
  } else {
    texcolor = vec4(1,1,1,1);
  }
  vec4 cr = texcolor;
  // HINT: you can also sample from the height map for parallax bump mapping (extra credit option)
    
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
    // HINT: also use the normalize() function to normalize things
    // You may also want something like "if (phongEnabled) { etc" here-ish

    
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
    color = cr * (ca + cl * max(0.0,dot(normal,l))) + 
      cl * pow(max(0.0,dot(r,-e)),p);

// Environment mapping: hardcoded to be disabled by default; has warnings about loading the same texture
/*
    if(envEnabled) {
      vec3 reflected = normalize(reflect(l,normal));
      color = mix(color,textureCube(skyMap,reflected),0.5);
    }
*/
  } else {

    color = cr * gouradColorDiffuse + gouradColorSpecular;
  }

  // set the output color to what we've computed
  gl_FragColor = shadow * color * ao;
  if(dispAmbientLayer)
    gl_FragColor = vec4(ao,ao,ao,1);
}
