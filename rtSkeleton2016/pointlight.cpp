#include "pointlight.h"
#include "shapegroup.h"
#include "geometry.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include <stdlib.h>


PointLight::PointLight ()
: Light()
{
}


PointLight::~PointLight ()
{
}


Color3d PointLight::getDiffuse (Intersection& info)
{
  /*
   * Intensity of diffuse lighting is equal to the dot product of
   * the normal with the vector opposite the incident light's direction.
   * Then factor in attenuation.
   */
    
    Color3d colord = Color3d(0,0,0);
    Vector3d intersection = info.iCoordinate - location;
    double intersectionDist = intersection.length();
    
    Vector3d intersectRay = intersection.normalize();
    double aten = quadAtten*sqr(intersectionDist) + linearAtten * intersectionDist + constAtten;
    double maxd = max(0.0, (info.normal).dot(-intersectRay));
    colord = maxd * info.material->getDiffuse(info) / aten;
    colord.clampTo(0, 1);
    
    return colord;
  
}


Color3d PointLight::getSpecular (Intersection& info)
{
  /*
   * Intensity of specular lighting is the dot product of the light's
   * reflected ray and the ray pointing to the camera, raised to
   * some power (in this case, kshine). Then factor in attenuation.
   */
    
    Rayd ray = info.theRay;
    Vector3d dir = (ray.getDir()).normalize();
    Vector3d ld = info.iCoordinate - location;
    ld = ld.normalize();
    Vector3d dr = ld + 2 * ((-ld).dot(info.normal)) * info.normal;
    dr = dr.normalize();
    
    double ldistance = ld.length();
    double aten = quadAtten*sqr(ldistance) + linearAtten * ldistance + constAtten;
    double maxs = max(0.0, (-dir).dot(dr));
    
    Color3d colors = Color3d(0,0,0);
    double kshine = info.material -> getKshine();
    colors = info.material -> getSpecular() * pow(maxs,kshine) / aten;
    colors.clampTo(0, 1);
    
    return colors;

	}


bool PointLight::getShadow (Intersection& iInfo, ShapeGroup* root)
{
  /*
   * To determine if an intersection is in shadow or not with respect
   * to a light, cast a ray from the intersection towards the light
   * and see if it intersects anything. 
   */
    
    Rayd theRay;
    Vector3d intersects = iInfo.iCoordinate - location;
    double intersectionDist = intersects.length();
    Vector3d intersectRay = intersects.normalize();
    if (intersectRay.dot(iInfo.normal)>0){
        return true;
    }
    
    Rayd shadowRay;
    shadowRay.setDir(intersectRay * (-1));
    shadowRay.setPos(iInfo.iCoordinate + iInfo.normal*EPSILON);
    
    Intersection shdwInfo;
    shdwInfo.theRay=shadowRay;
    if (root->intersect(shdwInfo) <= intersectionDist && root->intersect(shdwInfo) >= 0)
        return true;
    return false;

}


void PointLight::glLoad (GLenum light)
{
  /*
   * load the specified OpenGL light as a point light with the 
   * correct position, attenuation and color.
   */
  // unfortunately, OpenGL requires it's input in this format

  GLfloat lightLoc[4] = {  static_cast<GLfloat>(location[0]), static_cast<GLfloat>(location[1]), static_cast<GLfloat>(location[2]), 1.0 };
  GLfloat lightCol[4] = { static_cast<GLfloat>(color[0]),    static_cast<GLfloat>(color[1]),    static_cast<GLfloat>(color[2]),    0.0 };


  glLightfv(light, GL_POSITION,              lightLoc);
  glLightfv(light, GL_DIFFUSE,               lightCol);
  glLightfv(light, GL_SPECULAR,              lightCol);
  glLightf (light, GL_CONSTANT_ATTENUATION,  constAtten);
  glLightf (light, GL_LINEAR_ATTENUATION,    linearAtten);
  glLightf (light, GL_QUADRATIC_ATTENUATION, quadAtten);

  glEnable(light);
}


void PointLight::glDraw ()
{
  /*
   * draw a point light as a small flat shaded sphere
   */

  glDisable(GL_LIGHTING);
  glColor3f(color[0], color[1], color[2]);

  //glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
    // create a small sphere at the light's location
    //glTranslatef(location[0], location[1], location[2]);
    //glutSolidSphere(0.1, 6, 6);
  //glPopMatrix();

  if (options->lighting)
    glEnable(GL_LIGHTING);
}


ostream&  PointLight::write     (ostream&       out)
{
  out << "#light_point" << flush;
  if (name[0] != '\0')
    out << " -n " << name << flush;
  out << " --" << endl;
  
  out << "  " << color << endl;
  out << "  " << location << endl;
  out << "  " << constAtten << " " << linearAtten << " " << quadAtten << endl;
  
  return out;
}
  

istream& PointLight::read (istream& in)
{
  int numFlags = 1;
  Flag flags[1] = { { (char*)"-n", STRING, false,  MAX_NAME, name    }
                  };

  if (parseFlags(in, flags, numFlags) == -1)
  {
    cerr << "ERROR: Point Light: flag parsing failed!" << endl;
    return in;
  }
  
  in >> color >> location;

  constAtten  = nextDouble(in);
  linearAtten = nextDouble(in);
  quadAtten   = nextDouble(in);
    constAtten = (constAtten>0)? constAtten : 0;
  linearAtten = (linearAtten>0)? linearAtten : 0;
  quadAtten = (quadAtten>0)? quadAtten : 0;
  if (constAtten==0 && linearAtten==0 && quadAtten==0)
  {
	  constAtten=1;
  }

  if (in.fail())
  {
    cerr << "ERROR: Point Light: unknown stream failure" << endl;
    return in;
  }

  return in;
}


istream& operator>> (istream& in, PointLight& l)
{
  return l.read(in);
}


ostream& operator<< (ostream& out, PointLight& l)
{
  return l.write(out);
}

