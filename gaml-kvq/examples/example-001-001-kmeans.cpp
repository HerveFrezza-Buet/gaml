
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
#include <iostream>
#include <fstream>
#include <array>
#include <map>

#include <gaml-kvq.hpp>

// Let us define a 2D point from a pair.
typedef std::pair<double,double> Point;

// Our prototypes will not be points, but vectors in the feature space, related to some dictionary.
typedef gaml::span::Vector<Point> Feature;

typedef vq2::algo::kmeans::Unit<Feature> Unit; // This will be stored as graph vertices.

// The feature class supports all vector operations, so it is easy to
// set up the VectorOp class required by vq2 manipulations.
class VectorOp {
public:
  void raz(Feature& v)                    {v.raz();}
  void add(Feature& v, const Feature& w)  {v += w;}
  void div(Feature& v, double coef)       {v /= coef;}
  void mul(Feature& v, double coef)       {v *= coef;}
};

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

// LBG adds prototypes by splitting existing once. Splitting a
// prototype consists of creating a new one, that is very close to the
// original.
#define SPLIT_AMPLITUDE 1e-5
class Split {
public:
  typedef Feature value_type;
  typedef Feature sample_type;
  void operator()(value_type& destination,
		  const sample_type& source) {
    destination = source;
    for(auto& coef : destination.coefficients()) coef += gaml::random::uniform(-SPLIT_AMPLITUDE,SPLIT_AMPLITUDE);
  }
};

#define NU             .01
#define SAMPLE_SIZE   3000
#define NB_SAMPLES   10000
#define K               10

// Read this file
#include "disk.hpp"

class VertexId {
private:
  unsigned int k;
public:
  VertexId() : k(0) {}
  
  bool operator()(Vertex& n) {
    n.stuff.label = k++;
    return false; // the element should not be removed.
  }
};

int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // Let us create a dictionary...
  auto dict = gaml::span::dictionary<Point>(NU,gaussian_kernel);

  // ... and adjust it so that it spans the distribution.
  for(unsigned int nb = 0; nb < NB_SAMPLES; ++nb) dict.submit(get_sample());
  std::cout << dict.container().size() << " samples added."<< std::endl;

  Graph          prototypes;
  Similarity     distance;
  UnitSimilarity unit_distance(distance);
  Split          split;
  VectorOp       op;
  
  std::cout << "Processing K-means..." << std::endl;

  std::array<Feature,SAMPLE_SIZE> data_set;
  for(auto& sample : data_set) sample = dict(get_sample());
  
  // We apply k-means
  vq2::algo::kmeans::process(prototypes,K,
			     [&dict](void) -> Feature {
			       Feature zero = dict(Point(0,0));
			       zero.raz();
			       return zero;}, // Prototype initializer function (lambda here).
			     unit_distance,op,split,
			     data_set.begin(),data_set.end());

  // Now, let us plot the boundary function into a ppm image file.

  // We first tag the nodes so that they have 0,1,...,K-1 labels.
  VertexId vertex_id;
  prototypes.for_each_vertex(vertex_id);

  plot(prototypes,unit_distance,dict,"kmeans.ppm");

  return 0;
}
