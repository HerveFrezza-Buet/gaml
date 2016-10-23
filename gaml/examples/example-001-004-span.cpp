#include <gaml.hpp>
#include <utility>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <array>
#include <algorithm>
#include <functional>

// Let us define a 2D point from a pair.
typedef std::pair<double,double> Point;

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

// This do not support vectorial operations. Let us implement at least
// the dot product... We can try different ones. Such dot products are
// indeed kernels.
double linear_kernel(const Point& x1, const Point& x2) {
  return x1.first  * x2.first
    +    x1.second * x2.second;
}

double operator*(const Point& x1, const Point& x2) {
  return linear_kernel(x1,x2);
}

double polynomial_kernel(const Point& x1, const Point& x2) {
  return std::pow(x1*x2,3)+1;
}

double gaussian_kernel(const Point& x1, const Point& x2) {
  double dx = x1.first - x2.first;
  double dy = x1.second - x2.second;
  return std::exp(-(dx*dx+dy*dy));;
}

// Let us define a sampling of a donut distribution, with a small disk
// added inside the hole.
#define AREA_RADIUS  5
#define MIN_RADIUS   3
#define MAX_RADIUS   4
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
  double x  = gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  double y  = gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  while(!inside(x,y)) {
    x=gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
    y=gaml::random::uniform(-AREA_RADIUS,AREA_RADIUS);
  }
  return {x,y};
}

#define NU .01
#define NB_SAMPLES 10000


int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // Let us create a dictionary (by default, the * operator is the dot
  // product)
  auto dict = gaml::span::dictionary<Point>(NU, [] (const Point& x1, const Point& x2) -> double { return x1 * x2; });

  std::array<std::string, 3> 
    filenames = {{std::string("span-linear.ppm"), "span-polynomial.ppm", "span-gaussian.ppm"}};

  std::array<std::function<double (const Point&, const Point&)>, 3> 
    kernels   = {{linear_kernel, polynomial_kernel, gaussian_kernel}};

  std::array<std::string, 3>
    names     = {{std::string("linear"), "polynomial", "gaussian"}};

  for(unsigned int mode = 0; mode < 3; ++mode) {

    std::cout << std::endl
	      << std::endl
	      << "###################"<< std::endl
	      << "#                 #"<< std::endl
	      << "# Mode " << std::setw(10) << std::left << names[mode] << " #" << std::endl
	      << "#                 #"<< std::endl
	      << "###################"<< std::endl
	      << std::endl;

    // Let us adjust it so that it spans the distribution.
    dict.clear();
    dict.kernel = kernels[mode];
    for(unsigned int nb = 0; nb < NB_SAMPLES; ++nb) dict.submit(get_sample());
    std::cout << names[mode] << " kernel : " << std::setw(3) << dict.container().size() << " samples added."<< std::endl;
  
    // Let us keep on working with the gaussian kernel. We will
    // implement an on-line version of k-means, in the feature
    // space... at least, in the span of the dictionary elements in the
    // feature space.

    std::cout << "Starting k-means..." << std::endl;

#define K              5
#define NB_SAMPLES 10000
#define ALPHA        .05

    std::array<gaml::span::Vector<Point>,K> prototypes;

    // Let us initialize the prototypes from the distribution.
    for(auto& w : prototypes) w = dict(get_sample());

    // The type gaml::span::Vector<Point> supports all vector operation
    // (including the dot product).

    auto dist2 = [](const gaml::span::Vector<Point>& x,
		    const gaml::span::Vector<Point>& y) -> double {
      auto diff = x-y;
      return diff*diff;
    };
  
    // Now, let us apply the on-line K-means.
    for(unsigned int i=0; i < NB_SAMPLES; ++i) {
      auto xi = dict(get_sample()); 
      // Let us find the prototype closest to xi.
      auto best_proto_it = std::min_element(prototypes.begin(),
					    prototypes.end(),
					    [&xi,dist2](const gaml::span::Vector<Point>& x,
							const gaml::span::Vector<Point>& y) -> bool {return dist2(x,xi) < dist2(y,xi);});
      auto& best_proto = *best_proto_it;
      // Let us move that prototype toward xi.
      best_proto += ALPHA*(xi -  best_proto);
    }

    // Let us plot the result into a ppm image file.
  
#define IMG_RADIUS 200

#define IMG_SIDE       (2*(IMG_RADIUS)+1)
#define STEP_PER_PIXEL (2*AREA_RADIUS/(IMG_SIDE-1.0))

    std::cout << "Generating output image..." << std::endl;
    std::ofstream ppm(filenames[mode]);

    ppm << "P6" << std::endl
	<< IMG_SIDE << ' ' << IMG_SIDE << std::endl
	<< "255" << std::endl;

    double x,y;
    unsigned int w,h;
    for(h = 0, y = -AREA_RADIUS; h < IMG_SIDE; ++h, y += STEP_PER_PIXEL)  {
      std::cout << "line " << std::setw(4) << h+1 << '/' << IMG_SIDE << "      \r" << std::flush;
      for(w = 0, x = -AREA_RADIUS; w < IMG_SIDE; ++w, x += STEP_PER_PIXEL) {
	auto xi = dict(Point(x,y)); 
	// Let us find the prototype closest to xi.
	auto best_proto_it = std::min_element(prototypes.begin(),
					      prototypes.end(),
					      [&xi,dist2](const gaml::span::Vector<Point>& x,
							  const gaml::span::Vector<Point>& y) -> bool {return dist2(x,xi) < dist2(y,xi);});
	auto k = best_proto_it - prototypes.begin();
	unsigned char color_level = (unsigned char)(255*(k+1.0)/K+.5);
	if(inside(x,y)) 
	  ppm << (unsigned char)0 << (unsigned char)0 << color_level;
	else 
	  ppm <<      color_level <<      color_level << color_level;
      }
    }
    ppm.close();
    std::cout << std::endl
	      << "\"" << filenames[mode] << "\" generated." << std::endl;
  }

  return 0;
}
