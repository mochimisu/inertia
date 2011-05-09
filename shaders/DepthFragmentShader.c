varying vec4 v_position;

void main()
{
  //normalizing depth from -1 -> 1 to 0->1

  float depth = (v_position.z / v_position.w) * 0.5 + 0.5 ;
  
  float moment1 = depth;
  float moment2 = depth * depth;

  // adjusting moments with finite derivative
  float dx = dFdx(depth);
  float dy = dFdy(depth);
  moment2 += 0.25*(dx*dx+dy*dy);
	
  gl_FragColor = vec4(moment1, moment2, 0.0, 0.0);
}
