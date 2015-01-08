#pragma once



// These are required for dictionary serialization (operators << and
// >> are allowed for the dictionary).
std::ostream& operator<<(std::ostream& os, const Point& p) {
  os << p.first << ' ' << p.second;
  return os;
}
std::istream& operator>>(std::istream& is, Point& p) {
  is >> p.first >> p.second;
  return is;
}

double gaussian_kernel(const Point& x1, const Point& x2) {
  double dx = x1.first - x2.first;
  double dy = x1.second - x2.second;
  return std::exp(-(dx*dx+dy*dy));;
}

// Let us define a sampling of a donut distribution, with a small disk
// added inside the hole.
#define AREA_RADIUS 5
#define MIN_RADIUS 3
#define MAX_RADIUS 4
#define SMALL_RADIUS 2

bool inside(double x, double y) {
  double d2 = x*x+y*y;
  return
    (d2 <= MAX_RADIUS*MAX_RADIUS
     && d2 >= MIN_RADIUS*MIN_RADIUS)
    ||
    d2 <= SMALL_RADIUS*SMALL_RADIUS;
}

Point get_sample() {
  double x = gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  double y = gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  while(!inside(x,y)) {
    x=gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
    y=gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  }
  return {x,y};
}


#define IMG_RADIUS 200
#define IMG_SIDE (2*(IMG_RADIUS)+1)
#define STEP_PER_PIXEL (2*AREA_RADIUS/(IMG_SIDE-1.0))

void plot(Graph& prototypes, UnitSimilarity& unit_distance, gaml::span::Dictionary<Point>& dict,
	  const std::string& image_name) {

  auto nb_protos = prototypes.nbVertices();

  std::cout << "Generating output image..." << std::endl;

  std::ofstream ppm(image_name.c_str());
  ppm << "P6" << std::endl
      << IMG_SIDE << ' ' << IMG_SIDE << std::endl
      << "255" << std::endl;

  double x,y;
  unsigned int w,h;
  for(h = 0, y = -AREA_RADIUS; h < IMG_SIDE; ++h, y += STEP_PER_PIXEL) {
    std::cout << "line " << std::setw(4) << h+1 << '/' << IMG_SIDE << " \r" << std::flush;
    for(w = 0, x = -AREA_RADIUS; w < IMG_SIDE; ++w, x += STEP_PER_PIXEL) {
      auto xi = dict(Point(x,y));
      double closest_dist;
      auto best = vq2::algo::closest(prototypes,unit_distance,xi,closest_dist);
      unsigned char color_level = (unsigned char)(255*((*best).stuff.label)/(nb_protos-1.0)+.5);
      if(inside(x,y))
	ppm << (unsigned char)0 << (unsigned char)0 << color_level;
      else
	ppm << color_level << color_level << color_level;
    }
  }
  ppm.close();
  std::cout << std::endl
	    << "\"" << image_name << "\" generated." << std::endl;
}
  
