#include "rayfile.h"
#include "image.h"
#include "camera.h"
#include "group.h"
#include "main.h"
#include "material.h"
#include "light.h"
#include "shape.h"
#include <vector>
#include <complex>


/*
 * method to cast rays through each pixel of image into the scene.
 * uses getColor to find the color of an intersection point for each
 * pixel.  makes pretty pictures.
 */
void RayFile::raytrace (Image* image)
{
    
    // these will be useful
    int imageWidth = image->getWidth();
    int imageHeight = image->getHeight();
    

    // Read camera info and compute distance D to view window and the coordinates of its center and top left
    
    Camera*	camera = getCamera();
    Point3d pos = camera->getPos();
    Vector3d dir = camera->getDir();
    Vector3d up = camera->getUp();
    Vector3d right = camera->getRight();
    double half_angle = camera->getHalfHeightAngle();
    double rad = M_PI / 180 * half_angle;
    double D = imageHeight / (tan(rad)*2);
    
    Rayd DofRay;
    DofRay.setPos(pos);
    DofRay.setDir(dir);
    double focalDist;
    Intersection DofInfo;
    DofInfo.theRay = DofRay;
    focalDist = root -> intersect(DofInfo);
    if(focalDist == -1){
        focalDist = 2 * D;
    }
    
    Point3d center = pos + dir*D;
    Point3d top_left = center + (imageHeight / 2) * up - (imageWidth / 2) * right;
    
    
    // for printing progress to stderr...
    double nextMilestone = 1.0;
    
    //
    // ray trace the scene
    //
    
    for (int j = 0; j < imageHeight; ++j)
    {
        for (int i = 0; i<imageWidth; ++i)
        {
            
            
            // Compute the ray to trace
            Rayd theRay;
            
            // Compute and set the starting poisition and direction of the ray through pixel i,j
            // HINT: be sure to normalize the direction vector
            
            Point3d C_ij = top_left + (i + 0.5) * right - (j + 0.5) * up;
            Vector3d dir_ij = C_ij - pos;
            Vector3d norm_ij = dir_ij.normalize();
            
            theRay.setPos(pos);
            theRay.setDir(norm_ij);
            
            // get the color at the closest intersection point
            Color3d theColor = getColor(theRay, options->recursiveDepth);
            
            int jitter = options->jitter;
            double apertureR = 0.05;
            double AAshaftR = 0.01;
            
            if(jitter > 0){
                Point3d focalPoint = pos + focalDist*norm_ij;
                int numRays = 0;
                
                while(numRays < jitter){
                    double aaDistx = (double)(arc4random_uniform(2 * AAshaftR * 1000 + 1))/1000.0;
                    aaDistx = aaDistx - AAshaftR;
                    double aayBound = sqrt(sqr(AAshaftR) - sqr(aaDistx));
                    double aaDisty = (double)(arc4random_uniform(2 * aayBound * 1000 + 1))/1000.0;
                    aaDisty = aaDisty - aayBound;
                    Rayd aaRay;
                    Point3d aaOffset(aaDistx,aaDisty);
                    Point3d aaRayOrigin = pos + aaOffset;
                    aaRay.setPos(aaRayOrigin);
                    aaRay.setDir(norm_ij);
                    theColor += getColor(aaRay, options->recursiveDepth);
                    numRays++;
                }
                theColor[0] = theColor[0] / (jitter+1);
                theColor[1] = theColor[1] / (jitter+1);
                theColor[2] = theColor[2] / (jitter+1);
                theColor.clampTo(0.0, 1.0);
                numRays = 0;
                
                while(numRays < jitter){
                    double randDistx = (double)(arc4random_uniform(2 * apertureR * 1000 + 1))/1000.0;
                    randDistx = randDistx - apertureR;
                    double yBound = sqrt(sqr(apertureR) - sqr(randDistx));
                    double randDisty = (double)(arc4random_uniform(2 * yBound * 1000 + 1))/1000.0;
                    randDisty = randDisty - yBound;
                    Rayd jitterRay;
                    Point3d jitterRayOffset(randDistx,randDisty,0);
                    Point3d jitterRayOrigin = pos + jitterRayOffset;
                    Vector3d rayDir = focalPoint - jitterRayOrigin;
                    rayDir.normalize();
                    jitterRay.setPos(jitterRayOrigin);
                    jitterRay.setDir(rayDir);
                    theColor += getColor(jitterRay, options->recursiveDepth);
                    numRays++;
                }
                theColor[0] = theColor[0] / (jitter+1);
                theColor[1] = theColor[1] / (jitter+1);
                theColor[2] = theColor[2] / (jitter+1);
                theColor.clampTo(0.0, 1.0);
                
            }
            
            // the image class doesn't know about color3d so we have to convert to pixel
            // update pixel
            Pixel p;
            
            p.r = clamp(theColor[0],0,1);
            p.g = clamp(theColor[1],0,1);
            p.b = clamp(theColor[2],0,1);
            
            
            image->setPixel_(i, j, p);
            
        } // end for-i
        
        // update display
        // you don't need to touch this part!
        
        if (options->progressive)
        {
            
            display();
        }
        else if (!options->quiet)
        {
            if (((double) j / (double) imageHeight) <= nextMilestone)
            {
                cerr << "." << flush;
                nextMilestone -= (1.0 / 79.0);
            }
        }
    } // end for-j
}



Color3d RayFile::getColor(Rayd theRay, int rDepth)
{
    // some useful constants
    Color3d white(1,1,1);
    Color3d black(0,0,0);

    // check for intersections
    Intersection intersectionInfo;
    intersectionInfo.theRay=theRay;
    double dist = root->intersect(intersectionInfo);
    
    if (dist <=EPSILON)
        return background;
    
    // intersection found so compute color
    
    Color3d color = Color3d(0,0,0);
    
    // LIGHTING TODO
    // add ambient term
    color += intersectionInfo.material -> getAmbient() * ambient;

    // add emissive term
    color += intersectionInfo.material -> getEmissive();

    // END LIGHTING TODO
    
    // LIGHTING TO DO
    // implement getShadow, getDiffuse, and getSpecular for each new type of light
    // add contribution from each light
    for (VECTOR(Light*)::iterator theLight = lights.begin(); theLight != lights.end(); ++theLight)
    {
        if (!((*theLight)->getShadow(intersectionInfo, root))) {
            

            color += (*theLight)->getDiffuse(intersectionInfo);
            color += (*theLight)->getSpecular(intersectionInfo);
        }
    }
    
    color.clampTo(0,1);

    // stop if no more recursion required
    if (rDepth == 0)
        return color; // done
    
    
    // stop if we are already at white
    if (color==white) // can't add any more
        return color;
    
    // RECURSIVE TO DO
    rDepth--;
    
    Vector3d normal = intersectionInfo.normal;
    Point3d intersecLoc = intersectionInfo.iCoordinate;
    Vector3d inDir = theRay.getDir();
    
    Vector3d reflectRay = (inDir + 2 * ((-inDir).dot(normal)) * normal).normalize();
    
    // reflection
    Vector3d trans = inDir;
    Rayd reflect;
    reflect.setDir(reflectRay);
    reflect.setPos(intersecLoc + normal * EPSILON);
    
    Color3d reflectColor = getColor(reflect, rDepth);
    color += intersectionInfo.material -> getSpecular() * reflectColor;
    color.clampTo(0,1);
    
    // transmission with snell's law
    if(intersectionInfo.material -> getKtrans() > 0){
        double beta = intersectionInfo.material -> getRefind();
        double thetaIn = acos(inDir.dot(- intersectionInfo.normal));
        
        if(beta * sin(thetaIn) > 0 && beta*sin(thetaIn) <= 1){
            Vector3d vs = (inDir - (cos(thetaIn))*(-1 * intersectionInfo.normal)) / (sin(thetaIn));
            double thetaOut = asin(beta * sin(thetaIn));
            trans = (cos(thetaOut) * (-1 * intersectionInfo.normal) + (beta * sin(thetaIn)) * (vs.normalize())).normalize();
            
            // set trans ray
            Rayd transRay;
            transRay.setDir(trans);
            transRay.setPos(intersecLoc - normal*EPSILON);
            
            // set trans color
            Color3d transColor = getColor(transRay, rDepth);
            color += intersectionInfo.material -> getKtrans() * transColor;
            color.clampTo(0, 1);
        }
    }
    
    // END RECURSIVE TODO
    
    color.clampTo(0,1);
    
    return color;
    
}

