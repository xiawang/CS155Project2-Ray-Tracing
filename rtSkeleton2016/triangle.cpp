#include "triangle.h"
#include "parse.h"
#include "material.h"
#include "main.h"


Triangle::Triangle ()
: Shape ()
{
}


Triangle::~Triangle ()
{
}



double Triangle::intersect (Intersection& intersectionInfo)
{
	/*
	* This method solves for inersection between intersectionInfo.theRay and
	* the triangle. 
	*   If there is no intersection OR if the ray lies in the
	*	plane containing the triangle we return -1;
	*	Otherwise we return alpha the distance from the
	*	starting point of intersectionInfo.theRay to the intersection point
	
	*
	* Then, decompose the vector p - v0 into a linear combination of
	*/
    
    // RAY CASTING TO DO
    double alpha=-1;
    Point3d	v0 = v[0];
    Point3d	v1 = v[1];
    Point3d	v2 = v[2];
    Vector3d w1 = v1 - v0;
    Vector3d w2 = v2 - v0;
    Vector3d v = intersectionInfo.theRay.getDir();
    Vector3d u = v0 - intersectionInfo.theRay.getPos();
    
    if (n.dot(v) != 0) {
        alpha = (n.dot(u)) / (n.dot(v));
        
        if (alpha != -1) {
            double a = w1.dot(w1);
            double b = w1.dot(w2);
            double c = w1.dot(w2);
            double d = w2.dot(w2);
            Vector3d e = alpha*v - u;
            
            double beta = (d*e.dot(w1) - b*e.dot(w2)) / (a*d - c*b);
            double gamma = (c*e.dot(w1) - a*e.dot(w2)) / (c*b - a*d);
            
            if (beta >= 0 && gamma >= 0 && beta + gamma <= 1) {
                intersectionInfo.normal = n.normalize();
                if (n.dot(intersectionInfo.theRay.getDir()) > 0) {
                    intersectionInfo.normal = -(n.normalize());
                }
                
                intersectionInfo.material = material;
                Point3d q = intersectionInfo.theRay.getPos() + alpha*v;
                intersectionInfo.iCoordinate = q;
                
            } else {
                alpha = -1;
            }
        }
    }

    return alpha;
}


void Triangle::glDraw ()
{
	/*
	* draw the sphere with the appropriate material and textured status
	* at the desired position and radius
	*/

	material->glLoad();

	if (options->textures && textured && material->textured())
	{
		glEnable(GL_TEXTURE_2D);
		// draw the triangle with normal and tex coords
		glBegin(GL_TRIANGLES);
		n.glLoad();
		t[0].glLoad();
		v[0].glLoad();
		t[1].glLoad();
		v[1].glLoad();
		t[2].glLoad();
		v[2].glLoad();
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		// draw the triangle with normal but no tex coords
		glBegin(GL_TRIANGLES);
		n.glLoad();
		v[0].glLoad();
		v[1].glLoad();
		v[2].glLoad();
		glEnd();
	}

	material->glUnload();
}


inline istream& operator>> (istream& in, Triangle& t)
{
	return t.read(in);
}


istream& Triangle::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { { (char*)"-n", STRING, false, MAX_NAME,     name      },
	{ (char*)"-m", STRING, true,  MAX_NAME,     matname   },
	{ (char*)"-t", BOOL,   false, sizeof(bool), &textured },
	{ (char*)"-u", DOUBLE, false, sizeof(double), &bumpScale }

	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Triangle: flag parsing failed!" << endl;
		return in;
	}
	if (bumpScale != 0)
		bumpMapped = true;

	if (textured || bumpMapped)
		in >> v[0] >> t[0] >> v[1] >> t[1] >> v[2] >> t[2];
	else
		in >> v[0] >> v[1] >> v[2];

	if (in.fail())
	{
		cerr << "ERROR: Triangle: unknown stream error" << endl;
		return in;
	}

	// Vectors from vertex 0 to the other vertices
	Vector3d v1 = v[1]-v[0];
	Vector3d v2 = v[2]-v[0];  

	// Calculate this triangle's normal
	n = (v1).cross(v2);
	n.normalize();

	return in;
}


inline ostream& operator<< (ostream& out, Triangle& t)
{
	return t.write(out);
}


ostream& Triangle::write (ostream& out)
{
	out << "#triangle -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	out << " --" << endl;

	if (textured)
	{
		out << "  " << v[0] << "   " << t[0] << endl
			<< "  " << v[1] << "   " << t[1] << endl
			<< "  " << v[2] << "   " << t[2] << endl;
	}
	else
	{
		out << "  " << v[0] << endl
			<< "  " << v[1] << endl
			<< "  " << v[2] << endl;
	}

	return out;
}
