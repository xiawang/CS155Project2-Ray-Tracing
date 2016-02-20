#include "sphere.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include "common.h"
#include "math.h"
#include "roots.h"

#define PI 3.14159
// single glu quadric object to do opengl sphere drawing
static GLUquadricObj* sphereQuadric = gluNewQuadric();


Sphere::Sphere ()
: Shape ()
{
}


Sphere::~Sphere ()
{
}


double Sphere::intersect (Intersection& intersectionInfo)
{
	/*
	* This method determines if intersectionInfo.theRay visibly intersects
	* the sphere from the position of the start of intersectionInfo.theRay
    *
    * The first visible intersection point is at alpha along
    * the ray where
	*	alpha is the smallest non-negative real root of
	*	||v||^2 alpha^2 - 2 (u dot v)alpha + ||u||^2 - r^2
	*	where 
	*	v is the unit direction vector of intersectionInfo.theRay
	*   u is the vector for the start of intersectionInfo.theRay to the
	*		center of the sphere
	*	r is the radius of the sphere
	*/

    double alpha=-1;
    
    // RAY_CASTING TODO (sphere intersection)
    // Determine if intersectionInfo.theRay intersects the sphere in front of the camera
    // if so, store intersectionInfo on intersection point in intersectionInfo
    
    Point3d P0 = intersectionInfo.theRay.getPos();
    Vector3d v = intersectionInfo.theRay.getDir();
    Vector3d u = center - P0;
    
    double coef[3];
    coef[0] = sqr(v.length());
    coef[1] = -2 * (u.dot(v));
    coef[2] = sqr(u.length()) - sqr(radius);
    double result[2];
    double insphere = 1;
    int resultNum = solveQuadratic(coef, result);
    if(resultNum == 1 && result[0] > 0){
        alpha = result[0];
        
    }
    else if(resultNum == 2){
        if(result[0] > 0 && result[1] <= 0){
            alpha = result[0];
            insphere = -1;
        }else if(result[0] <= 0 && result[1] > 0){
            alpha = result[1];
            insphere = -1;
        }else if(result[0] > 0 && result[1] > 0){
            alpha = min(result[0],result[1]);
        }
    }
    if (alpha != -1){
        intersectionInfo.iCoordinate = P0 + alpha*v;
        Vector3d normal = insphere*(alpha * v - u);
        intersectionInfo.normal = normal.normalize();
        intersectionInfo.material = material;
    }
    
    // RAY_CASTING TODO (sphere intersection)
    
    
    return alpha;
    

}


void Sphere::glDraw ()
{
	/*
	* draw the sphere with the appropriate material and textured status
	* at the desired position and radius, using a glu quadric (for
	* easy texture coordinate generation)
	*/

	material->glLoad();

	glPushMatrix();
	// move to the origin of the sphere
	glTranslatef(center[0], center[1], center[2]);

	// set up the glu object's parameters - smooth filled polys with normals
	gluQuadricDrawStyle(sphereQuadric, GLU_FILL);
	gluQuadricOrientation(sphereQuadric, GLU_OUTSIDE);
	gluQuadricNormals(sphereQuadric, GLU_SMOOTH);

	// only calculate tex coords if we need to
	if (options->textures && textured && material->textured())
	{
		gluQuadricTexture(sphereQuadric, GLU_TRUE);
		glEnable(GL_TEXTURE_2D);
	}

	// draw the sphere
	gluSphere(sphereQuadric, radius, 50, 50);
	glPopMatrix();

	material->glUnload();

	glDisable(GL_TEXTURE_2D);
}


inline istream& operator>> (istream& in, Sphere& s)
{
	return s.read(in);
}


istream& Sphere::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { { (char*)"-n", STRING, false, MAX_NAME,     name      },
	{ (char*)"-m", STRING, true,  MAX_NAME,     matname   },
	{ (char*)"-t", BOOL,   false, sizeof(bool), &textured },
	{ (char*)"-u", DOUBLE, false, sizeof(double), &bumpScale }
	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Sphere: flag parsing failed!" << endl;
		return in;
	}

	if (bumpScale != 0)
		bumpMapped = true;


	in >> center;

	radius = nextDouble(in);

	if (in.fail())
	{
		cerr << "ERROR: Sphere: unknown stream error" << endl;
		return in;
	}

	return in;
}


inline ostream& operator<< (ostream& out, Sphere& s)
{
	return s.write(out);
}


ostream& Sphere::write (ostream& out)
{
	out << "#sphere -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	out << " --" << endl;

	out << "  " << center << endl;
	out << "  " << radius << endl;

	return out;
}
