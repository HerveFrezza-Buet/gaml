
/*
 *   Copyright (C) 2014,  Supelec
 *
 *   Author : Hervé Frezza-Buet
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : herve.frezza-buet@supelec.fr
 *
 */

#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <array>
#include <map>

#include <gaml-kvq.hpp>

// Let us define a 2D point from a pair.
typedef std::pair<double,double> Point;

// Our prototypes will not be points, but vectors in the feature space, related to some dictionary.
typedef gaml::span::Vector<Point> Feature;

typedef vq2::algo::som::Unit<Feature> Unit; // This will be stored as graph vertices.
typedef vq2::Graph<Unit,char,Unit::copy_constructor> Graph; // char is dummy....
typedef Graph::vertex_type Vertex;


// For finding the winner in the LBG algorithm, we need some
// similarity measure. Here, let us use the squared euclidian
// distance.
class Similarity {
public:
  typedef Feature value_type;
  typedef Feature sample_type;
  double operator()(const value_type& arg1,
		    const sample_type& arg2) {
    value_type diff = arg2 - arg1;
    return diff * diff;
  }
};

// The similarity is instrumented to handle units.
typedef vq2::unit::Similarity<Unit,Similarity> UnitSimilarity;

class WinnerTakeMost {
public:
  double coef;
  double operator()(double topo_dist) {
    return std::max(1-topo_dist*coef,0.0);
  }
};

// The learning process.
class Learn {
public:
  typedef Feature sample_type;
  typedef Feature weight_type;
  void operator()(double coef,
		  weight_type& prototype,
		  const sample_type& target) {
    prototype += coef*(target - prototype);
  }
};
typedef vq2::unit::Learn<Unit,Learn> UnitLearn;


#define NU             .01
#define NB_SAMPLES   10000
#define K               10
#define LEARNING_RATE  .005

#define EPOCH_SIZE  1000
#define NB_EPOCHS     50

#define START_NARROW_EPOCH   20
#define WIDE_COEF           .3
#define NARROW_COEF         .6


// Read this file
#include "disk.hpp"

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " <movie | pic>" << std::endl;
    return 0;
  }

  bool movie = std::string(argv[1])=="movie";

  // random seed initialization
  std::srand(std::time(0));

  // Let us create a dictionary...
  auto dict = gaml::span::dictionary<Point>(NU,gaussian_kernel);

  // ... and adjust it so that it spans the distribution.
  for(unsigned int nb = 0; nb < NB_SAMPLES; ++nb) dict.submit(get_sample());
  std::cout << dict.container().size() << " samples added."<< std::endl;

  Graph          som;
  Similarity     distance;
  UnitSimilarity unit_distance(distance);
  Learn          learning_rule;
  UnitLearn      unit_learning_rule(learning_rule);
  WinnerTakeMost competition;
  
  std::cout << "Processing SOM..." << std::endl;

  auto line = vq2::algo::make::line(som,K,
				    [&dict](unsigned int w) -> Feature {return dict(Point(0,0));},
				    [](unsigned int w, unsigned int ww) -> char {return ' ';});
  unsigned int k = 0;
  for(auto& ref_vertex : line) (*ref_vertex).stuff.label = k++;
  
  competition.coef = WIDE_COEF;
  for(unsigned int nb = 0; nb < NB_EPOCHS; ++nb) {
    if(nb == START_NARROW_EPOCH) competition.coef = NARROW_COEF;
    for(unsigned int e = 0; e < EPOCH_SIZE; ++e) 
      vq2::algo::som::step(som,unit_distance,competition,
			   unit_learning_rule,
			   LEARNING_RATE,
			   dict(get_sample()));
    if(movie) {
      std::ostringstream filename;
      filename << "som-" << std::setw(6) << std::setfill('0') << nb << ".ppm";
      plot(som,unit_distance,dict,filename.str());
    }
  }
   
  if(!movie)
    plot(som,unit_distance,dict,"som.ppm");
  
  return 0;
}
