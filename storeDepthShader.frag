varying vec4 v_position;

void main()
{
  float depth = v_position.z / v_position.w ;
  depth = depth * 0.5 + 0.5;			//Don't forget to move away from unit cube ([-1,1]) to [0,1] coordinate system

  float moment1 = depth;
  float moment2 = depth * depth;

  // Adjusting moments (this is sort of bias per pixel) using derivative
  float dx = dFdx(depth);
  float dy = dFdy(depth);
  moment2 += 0.25*(dx*dx+dy*dy) ;

  //[bmw] todo: double precision floats by storing in all rgba channels (2 channels per double)	

  gl_FragColor = vec4( moment1,moment2, 0.0, 0.0 );
}
