#include "box.h"
#include "parse.h"
#include "material.h"
#include "main.h"



Box::Box ()
: Shape ()
{
}


Box::~Box ()
{
}


double Box::planeIntersect (Rayd& ray, Point3d& p0, Vector3d& n)
{
    double alpha = -1;
    Vector3d v = ray.getDir(); // direction of the ray (camera)
    Vector3d u = p0 - ray.getPos(); // vector from camera to the center of a surface
    
    if (v.dot(n) != 0) {
        double local;
        local = (n.dot(u)) / (n.dot(v));
        if (local >= 0) {
            Point3d p = ray.getPos() + local * v; // position of intersection
            Vector3d d = p - p0; // vector from the center of surface to intersection
            
            // check if actually on the plane
            if (d[0]+0.5*size[0] >= 0 && d[0]+0.5*size[0] <= size[0] &&
                d[1]+0.5*size[1] >= 0 && d[1]+0.5*size[1] <= size[1] &&
                d[2]+0.5*size[2] >= 0 && d[2]+0.5*size[2] <= size[2]) {
                alpha = local;
            }
        }
    }
    return alpha;
}


double Box::intersect (Intersection& info)
{
    double alpha = -1;
    
    Rayd ray = info.theRay;
    
    // find all normal vectors for each side
    Vector3d normal[6];
    normal[0] = Vector3d(1,0,0);
    normal[1] = Vector3d(-1,0,0);
    normal[2] = Vector3d(0,1,0);
    normal[3] = Vector3d(0,-1,0);
    normal[4] = Vector3d(0,0,1);
    normal[5] = Vector3d(0,0,-1);
    
    // find all P0
    Point3d P0[6];
    P0[0] = center + 0.5*size[0]*normal[0];
    P0[1] = center + 0.5*size[0]*normal[1];
    P0[2] = center + 0.5*size[1]*normal[2];
    P0[3] = center + 0.5*size[1]*normal[3];
    P0[4] = center + 0.5*size[2]*normal[4];
    P0[5] = center + 0.5*size[2]*normal[5];
    
    double local;
    
    for (int i = 0; i < 6; i++) {
        local = planeIntersect(ray, P0[i], normal[i]);
        if (local != -1) {
            if (alpha == -1 || local < alpha) {
                alpha = local;
                info.normal = normal[i].normalize();
                info.iCoordinate = info.theRay.getPos() + alpha * (info.theRay.getDir());
                info.material = material;
            }
        }
    }
    
    if(alpha != -1){
        if(textured){
            TexCoord2d tex = getTexCoordinates(info.iCoordinate);
            if(tex[0]>=0){
                tex[0] = tex[0] - floor(tex[0]);
            }else{
                tex[0] = ceil(-tex[0]) + tex[0];
            }
            if(tex[1]>=0){
                tex[1] = tex[1] - floor(tex[1]);
            }else{
                tex[1] = ceil(-tex[1]) + tex[1];
            }
            info.texCoordinate = tex;
        }
        
        if(bumpMapped){
            TexCoord2d tex = getTexCoordinates(info.iCoordinate);
            if(tex[0]>=0){
                tex[0] = tex[0] - floor(tex[0]);
            }else{
                tex[0] = ceil(-tex[0]) + tex[0];
            }
            if(tex[1]>=0){
                tex[1] = tex[1] - floor(tex[1]);
            }else{
                tex[1] = ceil(-tex[1]) + tex[1];
            }
            info.texCoordinate = tex;
            Vector3d zoz(0,1,0);
            Vector3d normal = info.normal.normalize();
            if(normal[0] != 0 || normal[1] != 1 || normal[2] != 0){
                Vector3d up = zoz - normal.dot(zoz)*normal;
                up.normalize();
                Vector3d right = normal.cross(up);
                right.normalize();
                info.material->bumpNormal(normal, up, right, info, bumpScale);
            }
        }
    }
    
	return alpha;
}


TexCoord2d Box::getTexCoordinates (Point3d i)
{

    TexCoord2d tCoord;
    if(i[1] - center[1] == size[1]/2){
        tCoord = TexCoord2d(i[0], i[2]);
    }else if(i[1] - center[1] == -size[1]/2){
        tCoord = TexCoord2d(i[0],-i[2]+size[1]+2*size[2]);
    }else if(i[2] - center[2] == size[2]/2){
        tCoord = TexCoord2d(i[0], i[1]+size[2]);
    }else if(i[2] - center[2] == -size[2]/2){
        tCoord = TexCoord2d(i[0]+size[0]+size[2], i[1]+size[2]);
    }else if(i[0] - center[0] == size[0]/2){
        tCoord = TexCoord2d(i[2]+size[0],i[1]+size[2]);
    }else{
        tCoord = TexCoord2d(i[2]+size[2]+2*size[0],i[1]+size[2]);
    }
    tCoord = 0.2 * tCoord;
    
    return tCoord;
}



void Box::glutTexturedCube (GLfloat size)
{
	static GLfloat n[6][3] =
	{ 
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
		{0, 1, 2, 3},
		{3, 2, 6, 7},
		{7, 6, 5, 4},
		{4, 5, 1, 0},
		{5, 6, 2, 1},
		{7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	for (i = 5; i >= 0; i--) {
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		glTexCoord2f(0.0,0.0);
		glVertex3fv(&v[faces[i][0]][0]);
		glTexCoord2f(1.0,0.0);
		glVertex3fv(&v[faces[i][1]][0]);
		glTexCoord2f(1.0,1.0);
		glVertex3fv(&v[faces[i][2]][0]);
		glTexCoord2f(0.0,1.0);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}


void Box::glDraw ()
{
	material->glLoad();

	glPushMatrix();
	// move to the origin of the Box
	glTranslatef(center[0], center[1], center[2]);

	// scale to the appropriate size
	glScalef(size[0], size[1], size[2]);

	if (options->textures && textured && material->textured())
	{
		glEnable(GL_TEXTURE_2D);
	}

	// draw a unit cube
	glutTexturedCube(1);
	glPopMatrix();

	material->glUnload();
	glDisable(GL_TEXTURE_2D);
}


inline istream& operator>> (istream& in, Box& s)
{
	return s.read(in);
}


istream& Box::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { {(char*) "-n", STRING, false, MAX_NAME,       name      },
	{ (char*) "-m", STRING, true,  MAX_NAME,       matname   },
	{ (char*) "-t", BOOL,   false, sizeof(bool),   &textured },
	{ (char*) "-u", DOUBLE, false, sizeof(double), &bumpScale }
	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Box: flag parsing failed!" << endl;
		return in;
	}

	if (bumpScale != 0)
		bumpMapped = true;

	in >> center;
	in >> size;

	size[0] = fabs(size[0]);
	size[1] = fabs(size[1]);
	size[2] = fabs(size[2]);

	if (in.fail())
	{
		cerr << "ERROR: Box: unknown stream error" << endl;
		return in;
	}

	return in;
}


inline ostream& operator<< (ostream& out, Box& s)
{
	return s.write(out);
}


ostream& Box::write (ostream& out)
{
	out << "#box -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	if (bumpMapped)
		out << " -u" << flush;
	out << " --" << endl;

	out << "  " << center << endl;
	out << "  " << size << endl;

	return out;
}
