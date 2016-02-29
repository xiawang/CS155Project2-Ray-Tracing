#include "material.h"
#include "parse.h"
#include "image.h"
#include "main.h"
#include "rayfile.h"
#include "perlin.h"




   
const Color3d Material::getDiffuse(Intersection& info)
  {
	  if (!textured() && !procTextured())
		  return diffuse;
	  if (textured())
		  return getTexture(info.texCoordinate[0], info.texCoordinate[1]);
	  return getProceduralTexture(info.iCoordinate);
  }

Material::Material ()
{
  memset(name,    0, MAX_NAME);
  memset(texname, 0, MAX_PATH);
  texture=NULL;
  bumpMap=NULL;
  pNoise[0]=pNoise[1]=pNoise[2]=NULL;
  pOctaves=3;
  pPersistence=.5;
  glTexID = 0;
}


Material::~Material ()
{
  if (texture) 
  {
    delete texture;
    glDeleteTextures(1, &glTexID);
  }
  if (bumpMap)
	  delete bumpMap;
}


bool Material::textured ()
{
  return (texture != NULL);
}

bool Material::bumpMapped ()
{
  return (bumpMap != NULL);
}

bool Material::procTextured()
{ 
	if (pNoise[0]==NULL)
		return false;

	return true;
}

/* LIGHTINGMATERIALS TODO
 * compute texture value
 */

const Color3d Material::getTexture(double u, double v)
{

    Image* src = texture;
    u = u * (src->getWidth() - 1);
    v = v * (src->getHeight() - 1);
    
    int lowerU = floor(u);
    int lowerV = floor(v);
    int upperU = ceil(u);
    int upperV = ceil(v);
    double upperWeightx = (u - lowerU)/(upperU - lowerU);
    double upperWeighty = (v - lowerV)/(upperV - lowerV);
    
    double ULR,ULG,ULB,URR,URG,URB,LLR,LLG,LLB,LRR,LRG,LRB;
    ULR = src -> getPixel(lowerU, lowerV, 0);
    ULG = src -> getPixel(lowerU, lowerV, 1);
    ULB = src -> getPixel(lowerU, lowerV, 2);
    URR = src -> getPixel(upperU, lowerV, 0);
    URG = src -> getPixel(upperU, lowerV, 1);
    URB = src -> getPixel(upperU, lowerV, 2);
    LLR = src -> getPixel(lowerU, upperV, 0);
    LLG = src -> getPixel(lowerU, upperV, 1);
    LLB = src -> getPixel(lowerU, upperV, 2);
    LRR = src -> getPixel(upperU, upperV, 0);
    LRG = src -> getPixel(upperU, upperV, 1);
    LRB = src -> getPixel(upperU, upperV, 2);
    
    double RmiddleU, GmiddleU, BmiddleU, RmiddleB, GmiddleB, BmiddleB;
    RmiddleU = ULR*(1 - upperWeightx) + URR * upperWeightx;
    GmiddleU = ULG*(1 - upperWeightx) + URG * upperWeightx;
    BmiddleU = ULB*(1 - upperWeightx) + URB * upperWeightx;
    RmiddleB = LLR*(1 - upperWeightx) + LRR * upperWeightx;
    GmiddleB = LLG*(1 - upperWeightx) + LRG * upperWeightx;
    BmiddleB = LLB*(1 - upperWeightx) + LRB * upperWeightx;
    
    double fR, fG, fB;
    fR = RmiddleU * (1 - upperWeighty) + RmiddleB * upperWeighty;
    fG = GmiddleU * (1 - upperWeighty) + GmiddleB * upperWeighty;
    fB = BmiddleU * (1 - upperWeighty) + BmiddleB * upperWeighty;
    
    Color3d myColor(fR,fG,fB);
    myColor.clampTo(0.0, 1.0);
    return myColor;

}
const Color3d Material::getProceduralTexture(Point3d point)
{
	
	return Color3d(0,0,0);

}

/* BELLS WHISTLES TODO
 * comput bump map values
 */
void Material::bumpNormal (Vector3d& normal, 
						   Vector3d& up,
						   Vector3d& right,
						   Intersection& info,
						   double bumpScale)
{

	if (bumpMap != NULL)
	{
        double x = info.texCoordinate[0];
        double y = info.texCoordinate[1];
        Image* src = bumpMap;
        x = x * (src->getWidth() - 1);
        y = y * (src->getHeight() - 1);
        Pixel a = bumpMap->getPixel(floor(x), y);
        int b1 = (int)ceil(x) % (int)(src->getWidth());
        Pixel b = bumpMap->getPixel(b1, y);
        Pixel c = bumpMap->getPixel(x,floor(y));
        int d2 = (int)ceil(y) % (int)(src->getHeight());
        Pixel d = bumpMap->getPixel(x,d2);
        double dx = a.r - b.r;
        double dy = c.r - d.r;
        Vector3d perturbation = dx * right + dy * up;
        perturbation = perturbation * bumpScale + normal;
        perturbation.normalize();
        info.normal = perturbation;
	}
	return;
}



const Color3d Material::sample(Image* theImage, double x, double y) {
    
    int i = floor(x);
    int j = floor(y);
    if (i<0 || j<0 || i+1>=theImage->getWidth() || j+1>=theImage->getHeight()) {
        return Color3d(0,0,0);
    }
    Pixel pix00 = theImage->getPixel_(i,j);
    Pixel pix01 = theImage->getPixel_(i,j+1);
    Pixel pix10 = theImage->getPixel_(i+1,j);
    Pixel pix11 = theImage->getPixel_(i+1,j+1);
    
    double alphaX = floor(x+1)-x;
    double alphaY = floor(y+1)-y;
    
    double pix0R = pix00.r * alphaX + pix10.r * (1-alphaX);
    double pix0G = pix00.g * alphaX + pix10.g * (1-alphaX);
    double pix0B = pix00.b * alphaX + pix10.b * (1-alphaX);
    
    double pix1R = pix01.r * alphaX + pix11.r * (1-alphaX);
    double pix1G = pix01.g * alphaX + pix11.g * (1-alphaX);
    double pix1B = pix01.b * alphaX + pix11.b * (1-alphaX);
    
    Color3d pix0(pix0R, pix0G, pix0B);
    Color3d pix1(pix1R, pix1G, pix1B);
    
    Color3d color = pix0 * (alphaY) + pix1 * (1-alphaY);
    
    return color;
    
}

void Material::glLoad ()
{
  /*
   * load the material's properties into OpenGL...turn on
   * texturing if necessary.
   */

  // if lighting is on, set material properties
  if (options->lighting)
  {
    // unfortunately, opengl needs the args in this format
    GLfloat vals[4][4] = { { static_cast<GLfloat>(ambient[0]),  static_cast<GLfloat>(ambient[1]),  static_cast<GLfloat>(ambient[2]),  static_cast<GLfloat>(ktrans) },
                           { static_cast<GLfloat>(diffuse[0]),  static_cast<GLfloat>(diffuse[1]),  static_cast<GLfloat>(diffuse[2]),  static_cast<GLfloat>(ktrans) },
                           { static_cast<GLfloat>(specular[0]), static_cast<GLfloat>(specular[1]), static_cast<GLfloat>(specular[2]), static_cast<GLfloat>(ktrans) },
                           { static_cast<GLfloat>(emissive[0]), static_cast<GLfloat>(emissive[1]), static_cast<GLfloat>(emissive[2]), static_cast<GLfloat>(ktrans) }
                         };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   vals[0]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   vals[1]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  vals[2]);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  vals[3]);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, kshine );
  }
  // if lighting is off, just set the color
  else
  {
    (ambient * scene->getAmbient() + emissive).glLoad();
  }

  // separate the specular light out for texture calculations
  if (texture && options->textures)
  {
#ifdef GL_VERSION_1_2
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif

    glBindTexture(GL_TEXTURE_2D, glTexID);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  }
}


void Material::glUnload ()
{
  // go back to normal single color mode
  if (texture && options->textures)
  {
#ifdef GL_VERSION_1_2
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }
}


ostream& operator<< (ostream& out, Material& m)
{
  return m.write(out);
}


ostream& Material::write (ostream& out)
{
  out << "#material -n " << name << flush;
  if (texture)
    out << " -t " << texname << flush;
  out << " --" << endl;

  out << "  " << ambient << endl;
  out << "  " << diffuse << endl;
  out << "  " << specular << endl;
  out << "  " << emissive << endl;
  out << "  " << kshine << " " << ktrans << " " << refind << endl;

  return out;
}


istream& operator>> (istream& in, Material& m)
{
  return m.read(in);
}


istream& Material::read (istream& in)
{
  char texfile[MAX_PATH];
  texfile[0] = '\0';

  char bumpfile[MAX_PATH];
  bumpfile[0] = '\0';

  bool pNoiseTrue=false;

  int numFlags = 6;
  Flag flags[6] = { { (char*)"-n", STRING, true,  MAX_NAME, name    },
                    { (char*)"-t", STRING, false, MAX_PATH, texfile },
                    { (char*)"-u", STRING, false, MAX_PATH, bumpfile },
					{ (char*)"-p", BOOL,   false, sizeof(bool), &pNoiseTrue },
					{ (char*)"-po", INT, false, sizeof(int), &pOctaves},
					{ (char*)"-pp", DOUBLE, false, sizeof(double), &pPersistence}
                  };

  if (parseFlags(in, flags, numFlags) == -1)
  {
    cerr << "ERROR: Material: flag parsing failed!" << endl;
    return in;
  }

  in >> ambient >> diffuse >> specular >> emissive;

  kshine  = nextDouble(in);
  if (kshine < 0 || kshine>1) 
  {
	  cerr << "WARNING: Invalid kshine value clamped to [0,1]." << endl;
  }
  kshine = clamp(kshine,0,1);
  // now we scale kshine to range [0,128]
  kshine *= 128.0;

  ktrans = nextDouble(in);
  refind = nextDouble(in);
  if (refind<1)
  {
	  cerr << "WARNING: Invalid refractive index (refind) reset to 1." << endl;
	  refind=1;
  }

  if (in.fail())
  {
    cerr << "ERROR: Material: unknown stream failure" << endl;
    return in;
  }

  if (texfile[0] != '\0')
  {
    if (texture)
      delete texture;

    strncat(texname, "/", 2);
    strncat(texname, texfile, MAX_PATH);
    texture = new Image(texname);

    if(texture->good())
    {
      glGenTextures(1, &glTexID);
      glBindTexture(GL_TEXTURE_2D, glTexID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      texture->glTexImage2DWrapper();
    }
    else
    {
      cerr << "WARNING: Unable to load texture for material: " << name;
      cerr << endl;
      delete texture;
      texture = NULL;
    }
  }
  if (bumpfile[0] != '\0')
  {
    if (bumpMap)
      delete bumpMap;

    strncat(bumpname, "/", 2);
    strncat(bumpname, bumpfile, MAX_PATH);
    bumpMap = new Image(bumpname);

    if(!bumpMap->good())
    {
      cerr << "WARNING: Unable to load bump map for material: " 
	   << name << endl;
      delete bumpMap;
      bumpMap = NULL;
    }
  }
  
	if (pNoiseTrue) {
		pNoise[0] = new PerlinNoise(137777893); // for now hard code seed
		pNoise[1] = new PerlinNoise(9973281);
		pNoise[2] = new PerlinNoise(399990001);
	}
	
  return in;
  }


void Material::setMatDirectory (const char* dir)
{
  memset(texname, 0, MAX_PATH);
  if (dir == NULL || dir[0] == '\0')
	  strncpy(texname, ".", 2);
  else
      strncpy(texname, dir, MAX_PATH);


  memset(bumpname, 0, MAX_PATH);
  if (dir == NULL || dir[0] == '\0')
  	  strncpy(bumpname, ".", 2);
  else
    strncpy(bumpname, dir, MAX_PATH);

}
