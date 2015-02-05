#include <gaml.hpp>
#include <gaml-xtree.hpp>

#include <fstream>
#include <vector>
#include <array>
#include <utility>
#include <cmath>
#include <string>

#define N_MIN          50
#define DIM             2
#define XMIN            0
#define XMAX           10
#define PLOT_SIDE     200
#define NB_SAMPLES   1000
#define FOREST_SIZE    50
#define NOISE_PROBA    .2

typedef std::array<double,DIM> X;
typedef int                    Y; // The class is an integer.
typedef std::pair<X,Y>         Data;
typedef std::vector<Data>      Basis;

const X& input_of (const Data& d) {return d.first;}
const Y  output_of(const Data& d) {return d.second;}

// The class is in {0,1,2,3}.
Y oracle(const X& x) {
  return (Y)(1.99999*(1+std::sin(x[0])*std::sin(x[1])));
}

Y noisify(Y y) {
  Y res = y;
  if(gaml::random::proba(NOISE_PROBA))
    while(res == y)
      res = gaml::random::uniform(4);
  return res;
}

Data sample() {
  X x = {{gaml::random::uniform(XMIN,XMAX),
	  gaml::random::uniform(XMIN,XMAX)}};
  return {x, noisify(oracle(x))};
}

template<typename Fun, typename ClassOf>
void plot(const std::string& name,
	  const Fun& f,
	  const ClassOf& class_of) {
  std::ofstream file;
  std::string filename = name+".ppm";
  file.open(filename.c_str());
  file << "P6" << std::endl
       << PLOT_SIDE << ' ' << PLOT_SIDE << std::endl
       << "255" << std::endl;
  
  X x;
  for(int h = 0; h < PLOT_SIDE; ++h) {
    x[1] = (1-h/(PLOT_SIDE-1.0))*(XMAX-XMIN)+XMIN;
    for(int w = 0; w < PLOT_SIDE; ++w) {
      x[0] = (w/(PLOT_SIDE-1.0))*(XMAX-XMIN)+XMIN;
      switch(class_of(f(x))) {
      case  0: file << (char)255 << (char)000 << (char)000; break;
      case  1: file << (char)255 << (char)255 << (char)000; break;
      case  2: file << (char)000 << (char)000 << (char)255; break;
      case  3: file << (char)255 << (char)000 << (char)255; break;
      default: file << (char)000 << (char)000 << (char)000; break;
      }
    }
  }
  file.close();
  std::cout << "Image \"" << filename << "\" generated." << std::endl;
}

int main(int argc, char* argv[]) {

  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " <forest-size>" << std::endl;
    return 0;
  }
  
  unsigned int forest_size = (unsigned int)(atoi(argv[1]));

  Basis basis(NB_SAMPLES);
  for(auto& xy : basis) xy = sample();

  auto learner = gaml::xtree::classification::learner<X,Y,gaml::score::NormalizedInformationGain>(N_MIN,DIM);
  std::cout << "Learning a single tree... " << std::flush;
  auto predictor = learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "done." << std::endl;

  std::ofstream ofile("xtree.pred");
  ofile << predictor;
  ofile.close();

 
  gaml::xtree::classification::Predictor<X,Y> tree;
  std::ifstream ifile("xtree.pred");
  ifile >> tree;
  ifile.close();

  // A classification predictor outputs a map, that stores frequencies
  // of the labels (i.e (label,freq) pairs).


  // Now, let us set up a forest rather than a single tree.
  auto forest_learner = gaml::bag::learner(learner,gaml::HighestCumulatedFrequency<Y>(),gaml::bag::set::Identity(),forest_size,true);
  std::cout << "Learning a forest... " << std::flush;
  auto forest = forest_learner(basis.begin(), basis.end(), input_of, output_of);
  std::cout << "done." << std::endl;
  
  // The forest predictor outputs a label.


  // This does the plotting

  std::cout << std::endl
  	    << "Plotting..." << std::endl
  	    << std::endl;

  plot("oracle",oracle, 
       [](Y y) -> Y {return y;});
  plot("tree",tree,
       [](const gaml::xtree::classification::Predictor<X,Y>::output_type& frequencies) -> Y {return gaml::most_frequent(frequencies);});
  plot("forest",forest,
       [](Y y) -> Y {return y;});
  
  return 0;
}
