// vim: ts=2:sw=2:softtabstop=2
#include "sweep.h"

#include "global.h"
#include "LoadImage.h"

#include <fstream>

#define EPSILON (.00000000001)

namespace {
  // Used to advance the rotation minimizing frame forward
  // formula from [wang et al. 07]
  inline vec3 advanceFrame(const vec3 &xi, const vec3 &xi1,
			   const vec3 &ti, const vec3 &si, const vec3 &ri,
			   const vec3 &ti1) {
    vec3 v1 = xi1 - xi;
    double c1 = v1 * v1;
    if (c1 == 0)
      return ri;
    vec3 riL = ri - (2/c1)*(v1*ri)*v1;
    vec3 tiL = ti - (2/c1)*(v1*ti)*v1;
    vec3 v2 = ti1 - tiL;
    double c2 = v2 * v2;
    if (c2 == 0)
      return ri;
    return riL - (2/c2)*(v2*riL)*v2;
  }

  // lerps for all types we want to sample as bsplines
  PathPoint lerp(const PathPoint &a, const PathPoint &b, double t) {
    return PathPoint( (1-t)*a.point + t*b.point, (1-t)*a.azimuth + t*b.azimuth, (1-t)*a.scale+t*b.scale );
  }
  vec2 lerp(const vec2 &a, const vec2 &b, double t) {
    return (1-t)*a + t*b;
  }
  double dist2(const PathPoint &a, const PathPoint &b) {
    return (a.point-b.point).length2();
  }
  double dist2(const vec2 &a, const vec2 &b) {
    return (a-b).length2();
  }

  // get a quoted string, unless there's no quote at the start
  // in that case we just get whatever is there.
  string getQuoted(istream &str) {
    string ret;
    char temp;
    str >> temp;
    if (temp != '"') {
      str.putback(temp);
      str >> ret;
    } else {
      getline(str, ret, '"');
    }
    return ret;
  }

  void getValue(istream &str, int &val) {
    int v;
    if (str >> v)
      val = v;
  }
}


template<typename Pt> 
Pt sampleBSpline(const vector<Pt>& cps, double t, bool closed, int degree)
{
  if (cps.empty()) {
    UCBPrint("sampleBSpline", "need at least one control point");
    return Pt();
  }

  // get into 0,1 range
  if (t > 1.0 || t < 0.0)
    t = fmod(t, 1.0);
  if (t < 0.0)
    t += 1.0;
    
  // adjust degree down as needed to get a curve, if too few control points and not on closed loop
  int numCPs = int(closed ? cps.size() + degree : cps.size());
  if (degree >= numCPs)
    degree = numCPs - 1;

  // rescale t to minSupport,maxSupport range
  double minSupport = degree;
  double maxSupport = numCPs;
  t = (1-t)*minSupport + t*maxSupport;

  // 'recursive' form of b-spline is done iteratively here
  Pt *bases = new Pt[degree+1];
  int k = (int)t;

  for (int i = 0; i <= degree; ++i) {
    bases[i] = cps[(k - degree + i) % cps.size()];
  }

  for (int power = 1; power <= degree; ++power) {
    for (int i = 0; i <= degree - power; ++i) {
      int knot = k - degree + power + i;
      double u_i = (double)knot;
      double u_ipr1 = double(knot + degree - power + 1);
      double a = (t - u_i) / (u_ipr1 - u_i);
      bases[i] = lerp(bases[i], bases[i+1], a);
    }
  }

  Pt result = bases[0];

  delete [] bases;

  return result;
}

// create a polyline that samples the curve (helper for the big render function)
template<typename Pt>
void sampleBSpline(const vector<Pt> &pathPts, vector<Pt> &polyline, int totalSamples, bool closed = true, int degree = 3) {
  if (totalSamples == 0)
    return; // ... no samples is easy!

  Pt lastGood;
  for (int i = 0; i < totalSamples + 3; i++) {
    int loc = i % totalSamples;
    double t = loc / double(totalSamples);
    Pt sp = sampleBSpline(pathPts, t);
    if (!polyline.empty() && dist2(sp, lastGood) < EPSILON) {
      continue; // wait for the samples to get a bit further apart ... !
    } else {
      polyline.push_back(sp);
      lastGood = sp;
    }
  }
}

Sweep::Sweep(string filename) : globalTwist(0), globalAzimuth(0) {
  // Load the track file
  ifstream f(filename.c_str());
  if (!f) {
    UCBPrint("Sweep", "Couldn't load file " << filename);
    return;
  }
  string line;
  while (getline(f,line)) {
    stringstream linestream(line);
    string op;
    linestream >> op;
    if (op.empty() || op[0] == '#') // comments are marked by # at the start of a line
      continue;
    if (op == "p") { // p marks profile points (2d cross section vertices)
      vec2 v(0);
      linestream >> v;
      profilePts.push_back(v);
    } else if (op == "v") { // v marks bspline control points with optional azimuth info
      vec3 v(0);
      linestream >> v;
      double az;
      if (linestream >> az) {
	double s;
	if (linestream >> s) {
	  pathPts.push_back(PathPoint(v, az, s));
	} else {
	  pathPts.push_back(PathPoint(v, az));
	}
      } else {
	pathPts.push_back(PathPoint(v));
      }
    } else if (op == "twist") {
      linestream >> globalTwist;
    } else if (op == "azimuth") {
      linestream >> globalAzimuth;
    } else if (op == "texture") {
      string textureFile = getQuoted(linestream);
      loadTexture(textureFile, texture);
      getValue(linestream, lengthRepeats);
      getValue(linestream, widthRepeats);
    } else if (op == "bump") {
      string bumpFile = getQuoted(linestream);
      loadHeightAndNormalMaps(bumpFile, heightMap, normalMap, .2);
    }
  }

  
  vector<vec2>::iterator fwd = profilePts.begin();
  vector<vec2>::iterator bkwd = profilePts.end();
  bkwd--;
  while(fwd < bkwd) {
    vec2 tmpfw = *fwd;
    *fwd = *bkwd;
    *bkwd = tmpfw;
    fwd++;
    bkwd--;
  }


}


// sample the curve at a point
PathPoint Sweep::sample(double t) {
  return sampleBSpline(pathPts, t);
}
// get the forward direction
vec3 Sweep::sampleForward(double t, double step) {
  return sample(t+step).point - sample(t).point;
}
// search for a non-zero forward direction
vec3 Sweep::sampleForward_nonzero(double t, double step, int searchdist) {
  int k = 1;
  vec3 dir;
  do {
    dir = sampleForward(t,step*k);
  } while (dir.length2() < EPSILON && k++ < searchdist);
  return dir;
}
vec3 Sweep::sampleUp(double t, double step) {
  t = fmod(t, 1.0);
  if (t < 0.0) t+=1.0;
  vec3 up = getFirstUp();
  vec3 lastpos = sample(0).point;
  vec3 lastdir = sampleForward_nonzero(0);
  lastdir.normalize();
  vec3 dir = lastdir;
  vec3 pos = lastpos;
  for (double st = step; st <= t + step*.5; st+=step) {
    double tt = min(st, t);
    dir = sampleForward(tt);
    if (dir.length2() < EPSILON)
      continue;
    dir.normalize();
    pos = sample(tt).point;
    vec3 right = lastdir ^ up;
    right.normalize();
    up = advanceFrame(lastpos, pos, lastdir, right, up, dir);
    right = dir ^ up;
    up = right ^ dir;
    up.normalize();
    lastpos = pos; lastdir = dir;
  }

        
  // orthonormalize the frame
  dir = sampleForward(t);
  if (dir.length2() < EPSILON)
    dir = lastdir;
  dir.normalize();
  vec3 right = dir^up;
  up = right^dir;
  up.normalize();
        
  orientVectorInFrame(-dir, fmod(t,1.0), sample(t).azimuth, up);

        
  return up;
}

//TODO: efficient-ize this
mat4 Sweep::tbnBasis(double t, vec3 worldLoc, double step) {
  vec3 up = this->sampleUp(t,step);
  vec3 forward = this->sampleForward(t,step);

  mat4 basis = mat4(vec4(up ^ forward, 0),
		    vec4(up, 0),
		    vec4(-forward,0),
		    vec4(worldLoc,1)).transpose().inverse();
  return basis;
}


// rotates a vector according to the global azimuth, local azimuth, twist, direction, and location on curve
void Sweep::orientVectorInFrame(const vec3 &dir, double percent, double localAz, vec3 &inFrame) {
  double rot = globalAzimuth + globalTwist * percent + localAz;
  inFrame = rotation3D(dir, rot) * inFrame;
}

vec3 Sweep::getFirstUp() {
  vec3 leg1 = sampleForward_nonzero(0).normalize();
  vec3 leg2 = sampleForward_nonzero(0,-.001).normalize();
  vec3 up = leg1+leg2; // start with the frenet frame
  if (up.length2() < .0001) { // if that doesn't work, try something else
    up = leg1 ^ vec3(0,1,0);
    if (up.length2() < .0001) {
      up = leg1 ^ vec3(.1,1,0);
    }
  }
  up.normalize();
  return up;
}



// sweep the cross section along the curve (helper for the big render function)
void Sweep::renderSweep(GeometryShader &shader, vector<PathPoint> &polyline, vector<vec2> &profile, double crossSectionScale) {
  PathPoint pts[3]; // pts[1] is us, pts[0] and pts[3] surround us
  vector<vec2> & crossSection = profile;
  int size = (int) polyline.size();
  vec3 * newSlice = new vec3[crossSection.size()];
  vec3 * oldSlice = new vec3[crossSection.size()];
  vec3 oldDir(0), right(0), up(0);
  bool firstDir = true;
  for (int i = 1; i < size-1; i++) {
    double percent = double(i % size) / (double(size-3));
    double deltaPercent = 1.0 / (double(size-3));
    for (int c = -1; c <= 1; c++) { // populate local pts
      pts[c+1] = polyline[ (i + size + c) %  size ];
    }

    vec3 leg1 = (pts[0].point - pts[1].point).normalize();
    vec3 leg2 = (pts[2].point - pts[1].point).normalize();
    vec3 dir = (leg2 - leg1);
    if (dir.length2() < .0001)
      dir = pts[2].point - pts[1].point;
    dir.normalize();
        
        
    if (firstDir) { // first time around use a special routine to find the up dir
      up = getFirstUp();
      firstDir = false;
    }
    else { // after the first frame, advance with the rotation minimizing frame
      up = advanceFrame(pts[0].point, pts[1].point,
                        oldDir, right, up, dir);
    }
    right = dir ^ up;
    up = right ^ dir;
    up.normalize(); right.normalize();


    double rot = globalAzimuth + globalTwist * percent + pts[1].azimuth;

    vec3 bisect = leg1 + leg2;
    double len = bisect.length();
    bool scaleSect = false;
    double scaleTrans = 0;
    if (len > .0001) { // only scale if not going straight already
      scaleSect = true;
      bisect = bisect/len;
      double dot = -leg1*leg2;
      double angle = acos(CLAMP(dot,-1.0,1.0));
      double scale = 1.0 / MAX(cos(.5*angle ),.1);
      scaleTrans = scale - 1.0;
    }

    double s = crossSectionScale;
    int ind = 0;
    for (vector<vec2>::iterator it = crossSection.begin(); it != crossSection.end(); ++it, ++ind) {
      vec2 pos2d = rotation2D(vec2(0,0),rot) * (*it);
      vec3 pt = right * pos2d[0] * s + up * pos2d[1] * s;
      if (scaleSect) {
	pt = pt + scaleTrans * (pt * bisect) * bisect;
      }
      pt *= pts[1].scale;
      newSlice[ind] = pts[1].point + pt;
    }

    if (i > 1) {
      glBegin(GL_QUAD_STRIP);
      int csize = (int) crossSection.size();
      for (int v = 0; v <= csize; v++) {

	GLint tangentAttrib = shader.getTangentAttrib();
	GLint bitangentAttrib = shader.getBitangentAttrib();

	int vn = v % csize;
	vec3 tan0 = oldSlice[(vn+1)%csize]-oldSlice[vn]; tan0.normalize();
	vec3 tan1 = newSlice[(vn+1)%csize]-newSlice[vn]; tan1.normalize();

	//glColor3f(v%4!=0,v%4!=1,v%4!=2);
	double percentAround = v / double(csize); // the percent around the cross section

	//vec3 n = (tan0^oldDir);
	vec3 n = -(tan0^oldDir);
	n.normalize();
	glNormal3dv(&n[0]);
	/*
	cout << "dp: " << deltaPercent << endl;
	cout << percent << "," << percentAround << endl;
	cout << lengthRepeats << ";" << widthRepeats << endl;
	*/

	// @TODO: SET TEXTURE COORDINATE
	// HINT: use percent, deltaPercent, percentAround to determine where on the curves you are
	// HINT: use lengthRepeats and widthRepeats to determine how much to repeat
	double vcoord = (percent-deltaPercent)*lengthRepeats;
	while(vcoord > 1)
	  vcoord--;
	double ucoord = percentAround*widthRepeats;
	while(ucoord > 1)
	  ucoord--;
	//cout << percentAround << "," << ucoord << endl;
	//glTexCoord2d(ucoord,vcoord);
	glTexCoord2d(vcoord,1-ucoord);
		       	
	// @TODO: SET TANGENT AND BITANGENT
	// HINT: Use the glVertexAttrib3fARB, like:
	// glVertexAttrib3dARBv(tangentAttrib, x=?, y=?, z=?);
	// glVertexAttrib3dARBv(bitangentAttrib, x=?, y=?, z=?);
	glVertexAttrib3fARB(tangentAttrib,tan0[0],tan0[1],tan0[2]);
	vec3 bit0 = tan0 ^ n;
	glVertexAttrib3fARB(bitangentAttrib,bit0[0],bit0[1],bit0[2]);

	glVertex3dv(&oldSlice[vn][0]);
                
	//vec3 n2 = (tan1^dir);
	vec3 n2 = -(tan1^dir);
	n2.normalize();
	glNormal3dv(&n2[0]);
                
	// @TODO: SET TEXTURE COORDINATE
	// HINT: use percent, deltaPercent, percentAround to determine where on the curves you are
	// HINT: use lengthRepeats and widthRepeats to determine how much to repeat
	//glTexCoord2d(ucoord,vcoord + lengthRepeats*deltaPercent);
	  glTexCoord2d((vcoord + lengthRepeats*deltaPercent),1-ucoord);
				
	// @TODO: SET TANGENT AND BITANGENT
	// HINT: Use the glVertexAttrib3fARB, like:
	// glVertexAttrib3dARBv(tangentAttrib, x=?, y=?, z=?);
	// glVertexAttrib3dARBv(bitangentAttrib, x=?, y=?, z=?);
	glVertexAttrib3fARB(tangentAttrib,tan1[0],tan1[1],tan1[2]);
	vec3 bit1 = tan1 ^ n2;
	glVertexAttrib3fARB(bitangentAttrib,bit1[0],bit1[1],bit1[2]);
                
	glVertex3dv(&newSlice[vn][0]);
                
      }
      glEnd();
    }

    // swap new and old lists
    vec3 *temp = newSlice;
    newSlice = oldSlice;
    oldSlice = temp;

    oldDir = dir;
  }
  delete [] newSlice;
  delete [] oldSlice;
}

// the big render function
void Sweep::render(GeometryShader &shader, int pathSamplesPerPt, double crossSectionScale, int xsectSamplesPerPt) {
  shader.set();

  // load textures
  //glActiveTexture(GL_TEXTURE3);
  //glBindTexture(GL_TEXTURE_CUBE_MAP, skyMap);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, heightMap);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, normalMap);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);


  int totalSamples = (int) pathPts.size() * pathSamplesPerPt;

  vector<PathPoint> polyline;
  sampleBSpline(pathPts, polyline, totalSamples);
  //createPolyline(polyline, totalSamples);

  vector<vec2> profile;
  sampleBSpline(profilePts, profile, xsectSamplesPerPt * int(profilePts.size()));

  int size = (int) polyline.size();
  if (size <= 1) { // a polyline with only one point is pretty lame!
    cout << "not enough curve to sweep ..." << endl;
    return;
  }

  renderSweep(shader, polyline, profile, crossSectionScale);
}

void Sweep::renderWithDisplayList(GeometryShader &shader, int pathSamplesPerPt, double crossSectionScale, int xsectSamplesPerPt) {
  
  int shadeId = shader.getId();

  if (shaderDL.count(shadeId) == 0) {
    GLuint DLid = glGenLists(1);
    glNewList(DLid, GL_COMPILE);
    render(shader, pathSamplesPerPt, crossSectionScale, xsectSamplesPerPt);
    glEndList();
    shaderDL[shadeId] = DLid;
  }
  glCallList(shaderDL[shadeId]);
  
}

void Sweep::clearDisplayList() {
  for(map<int, GLuint>::iterator it=shaderDL.begin(); it != shaderDL.end(); it++) {
    glDeleteLists(it->second, 1);
  }
  shaderDL.clear();
}

