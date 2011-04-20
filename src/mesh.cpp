
#include "mesh.h"
#include "LoadImage.h"

void Mesh::centerAndScale(double scale) {
    if (verts.empty())
	    return;

    vec3 maxp = verts[0].p, minp = verts[0].p;
    for (vector<Vert>::iterator it = verts.begin(); it != verts.end(); ++it) {
	    maxp = max(it->p, maxp); // max and min def'd in algebra3.h take the max or min componentwise from each vector
	    minp = min(it->p, minp);
    }
    vec3 center = (maxp+minp)*.5;
    vec3 size = maxp-minp;
    double maxSizeInv = MAX(size[0],MAX(size[1],size[2]));
    if (maxSizeInv == 0) // mesh is just one point
	    return;
    maxSizeInv = 1.0/maxSizeInv;
    for (vector<Vert>::iterator it = verts.begin(); it != verts.end(); ++it) {
	    it->p = (it->p-center)*maxSizeInv*scale;
    }
}


// just use the first two edges
vec3 Mesh::getNormal(int f) {
	vec3 e1 = v(f,0).p - v(f,1).p;
	vec3 e2 = v(f,2).p - v(f,1).p;
    vec3 n = -e1 ^ e2;
    return n.normalize();
}

void Mesh::draw(GeometryShader &shade) {
    shade.set();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heighttex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normaltex);

	for (size_t i = 0; i < faces.size(); ++i) {
		glBegin(GL_POLYGON);
        bool hasn = !faces[i].ni.empty() && faces[i].ni[0] > 0;
        bool hast = !faces[i].ti.empty() && faces[i].ti[0] > 0;
        vec3 n;
        if (!hasn) {
            n = getNormal((int)i);
            glNormal3dv(&n[0]);
        }
        int count = (int)faces[i].edges.size();
		for (int ind = 0; ind < count; ind++) {
            if (hasn) {
                n = normals[faces[i].ni[ind]];
                glNormal3dv(&n[0]);
            }
            if (hast) {
                glTexCoord2dv(&uvs[faces[i].ti[ind]][0]);
                int ii[3] = { (ind-1+count)%count, ind, (ind+1)%count };
                double c3c1y = uvs[faces[i].ti[ii[0]]][1] - uvs[faces[i].ti[ii[1]]][1];
                double c2c1y = uvs[faces[i].ti[ii[2]]][1] - uvs[faces[i].ti[ii[1]]][1];
                double c3c1x = uvs[faces[i].ti[ii[0]]][0] - uvs[faces[i].ti[ii[1]]][0];
                double c2c1x = uvs[faces[i].ti[ii[2]]][0] - uvs[faces[i].ti[ii[1]]][0];                
                vec3 v1 = verts[faces[i].vi[ii[1]]].p;
                vec3 T = c3c1y*(verts[faces[i].vi[ii[2]]].p - v1) 
                        - c2c1y*(verts[faces[i].vi[ii[0]]].p - v1);
                T.normalize();
                vec3 B = c3c1x*(verts[faces[i].vi[ii[2]]].p - v1) 
                        - c2c1x*(verts[faces[i].vi[ii[0]]].p - v1);
                B.normalize();
                glVertexAttrib3dvARB(shade.getBitangentAttrib(), &B[0]);
                glVertexAttrib3dvARB(shade.getTangentAttrib(), &T[0]);
            }
			glVertex3dv(&v(i,ind).p[0]);
		}
		glEnd();
	}
}

// just naively sums all nearby triangle normals; no area weighting
void Mesh::computeVertexNormals()
{
    normals.clear();
    normals.resize(verts.size(), vec3(0));

    for (unsigned int f = 0; f < faces.size(); ++f) {
        vec3 n = getNormal(f);
        faces[f].ni.clear();
        for (unsigned int i = 0; i < faces[f].vi.size(); ++i) {
			normals[faces[f].vi[i]] += n;
            faces[f].ni.push_back(faces[f].vi[i]);
        }
    }

    for (unsigned int i = 0; i < normals.size(); ++i) {
		normals[i].normalize();
    }
}

void Mesh::connectFace(Face &face) {
	int find = (int)faces.size();
	for (int i = 0; i < (int)face.vi.size(); ++i) {
        int v1 = face.vi[ i ];
        int v2 = face.vi[(i+1)%face.vi.size()];
		int ei = edge(v1, v2);
        if (ei == -1) { // create the edge and set the first face on it
            ei = (int)edges.size();
            edges.push_back(Edge(find, v1, v2, 0));
            verts[v1].edges.push_back(ei); // add the edge to each vert's one ring
            verts[v2].edges.push_back(ei);
        } else {
            edges[ei].f[1] = find; // set the second on the edge
        }
        face.edges.push_back(ei);
	}
    faces.push_back(face);
}

namespace { 
    int getNValues(stringstream &ss, vector<int> &values, char delim = '/') {
	    values.clear();
	    string sblock;
	    if (ss >> sblock) {
		    stringstream block(sblock);
		    string s;
		    int value;
		    while (getline(block, s, delim)) {
			    stringstream valuestream(s);
			    if (valuestream >> value)
				    values.push_back(value);
                else
                    values.push_back(-1);
		    }
	    }
	    return (int)values.size();
    }
}

bool Mesh::loadTextures(string texture, string bumpmap) {
    bool texloaded = loadTexture(texture, tex);
    bool bumploaded = loadHeightAndNormalMaps(bumpmap, heighttex, normaltex, .2);
    return texloaded && bumploaded;
}


bool Mesh::loadFile(string file)
{
	faces.clear(); verts.clear();
	ifstream f(file.c_str());
	if (!f) {
		cerr << "Couldn't open file: " << file << endl;
		return false;
	}
	string line;
	while (getline(f,line)) {
		if (line.empty())
			continue;
		stringstream ss(line);
		string op;
		ss >> op;
		if (op.empty() || op[0] == '#')
			continue;
		if (op == "v") {
			vec3 v;
			ss >> v;
			verts.push_back(Vert(v));
		}
		if (op == "vn") {
			vec3 n;
			ss >> n;
			normals.push_back(n);
		}
        if (op == "vt") {
            vec2 uv;
            ss >> uv;
            uvs.push_back(uv);
        }
		if (op == "f")
		{
			vector<int> first, second;
			vector<int> orig;
			Face f;			
			if (!getNValues(ss, first))
				continue;
			orig = first;
			while (getNValues(ss, second)) {
                f.addVertex(first, -1);
				first = second;
			}
            f.addVertex(first, -1);
            if (f.vi.size() >= 3) {
				connectFace(f);
			}
		}
	}
	return true;
}

vec3 Mesh::getCentroid(int f) {
    vec3 centroid(0,0,0);
    if (f > -1) {
        for (int ii = 0; ii < (int)faces[f].vi.size(); ++ii) {
            centroid += v(f,ii).p;
        }
        centroid *= (1.0 / (double)faces[f].vi.size());
    }
    return centroid;
}

vec3 Mesh::getCCEdgeStencil(int ei) {
    Edge &e = edges[ei];
	vec3 toret = verts[e.v[0]].p + verts[e.v[1]].p + getCentroid(e.f[0]) + getCentroid(e.f[1]);
    double denom = 2 + int(e.f[0]>-1) + int(e.f[1]>-1);
	toret *= (1.0/denom);
	return toret;
}

vec3 Mesh::getCCVertexStencil(int vi) {
	vec3 esum(0,0,0), fsum(0,0,0);
    double n = (double)verts[vi].edges.size();
    double nf = 0;
    for (int i = 0; i < (int)verts[vi].edges.size(); i++) {
        Edge &e = edges[verts[vi].edges[i]];
        int ii = 0;
        if (e.v[0] == vi)
            ii = 1;
        esum += verts[e.v[ii]].p;
        fsum += getCentroid(e.f[ii]);
        if (e.f[ii] != -1) // if we actually added a centroid, count it.
            nf++;
    }
	double nsqinv = 1.0/(n*n);
	return verts[vi].p * ((n-2.0)/n) + esum * nsqinv + fsum * (1.0/(nf*n));
}

// put a CC subdivided version of this mesh into 'output' mesh
void Mesh::subdivide(Mesh &output) {
	output.clearMesh();

	// copy all original vertices
	for (int v = 0; v < (int)verts.size(); ++v) {
		output.verts.push_back(Vert(getCCVertexStencil(v)));
	}

	// add edge midpoints for faces
	for (int eo = 0; eo < (int)edges.size(); ++eo) {
		edges[eo].temp = -1;
	}
    for (int eo = 0; eo < (int)edges.size(); ++eo) {
		int midp = (int)output.verts.size();
		
		output.verts.push_back(Vert(getCCEdgeStencil(eo)));
		edges[eo].temp = midp;
	}

	// add face midpoints and faces
	for (int fo = 0; fo < (int)faces.size(); ++fo) {
		int fmid = (int)output.verts.size();
		output.verts.push_back(Vert(getCentroid(fo)));
		int n = (int)faces[fo].edges.size();
        bool hast = !faces[fo].ti.empty()&& faces[fo].ti[0]>-1;
        vec2 avgt(0);
        int miduv = -1;
        if (hast) {
            for (int ii = 0; ii < n; ii++) {
                avgt += uvs[faces[fo].ti[ii]];
            }
            avgt *= (1.0/double(n));
            miduv = (int)output.uvs.size();
            output.uvs.push_back(avgt);
        }
        for (int ii = 0, i_prev = n-1; ii < n; i_prev = ii, ii++) {
			Face f;
            f.vi.push_back(fmid);
            if (hast) f.ti.push_back(miduv);
			f.vi.push_back(edges[faces[fo].edges[i_prev]].temp);
            if (hast) {
                vec2 uv = .5*(uvs[faces[fo].ti[i_prev]] + uvs[faces[fo].ti[ii]]);
                int uvi = (int)output.uvs.size();
                output.uvs.push_back(uv);
                f.ti.push_back(uvi);
            }
			f.vi.push_back(vind(fo,ii)); // vinds from *this are equivalent in output
            if (hast) {
                f.ti.push_back((int)output.uvs.size());
                output.uvs.push_back( uvs[faces[fo].ti[ii]] );
            }
			f.vi.push_back(edges[faces[fo].edges[ii]].temp);
            if (hast) {
                vec2 uv = .5*(uvs[faces[fo].ti[ii]] + uvs[faces[fo].ti[(ii+1)%n]]);
                int uvi = (int)output.uvs.size();
                output.uvs.push_back(uv);
                f.ti.push_back(uvi);
            }
			output.connectFace(f);
		}
	}
}
