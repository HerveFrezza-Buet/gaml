#include <gaml.hpp>
#include <gaml-xtree.hpp>

#include <fstream>
#include <vector>
#include <array>
#include <utility>
#include <cmath>
#include <string>
#include <random>

#define DIM             2
#define XMIN            0
#define XMAX           10
#define PLOT_SIDE     200
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

template<typename RANDOM_DEVICE>
Y noisify(Y y, RANDOM_DEVICE& rd) {
  Y res = y;
  std::uniform_int_distribution<Y> uniform(0,3);
  if(std::bernoulli_distribution(NOISE_PROBA)(rd))
    while(res == y)
      res = uniform(rd);
  return res;
}

template<typename RANDOM_DEVICE>
Data sample(RANDOM_DEVICE& rd) {
  std::uniform_real_distribution<double> uniform(XMIN,XMAX);
  X x = {{uniform(rd), uniform(rd)}};
  return {x, noisify(oracle(x), rd)};
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

  if(argc != 4) {
    std::cout << "Usage : " << argv[0]
	      << " <forest-size> <nb-samples> <max-leaf-size>" << std::endl
	      << "  e.g : " << argv[0] << " 100 1000 50" << std::endl;
      return 0;
  }
  
  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());
  
  unsigned int forest_size   = (unsigned int)(atoi(argv[1]));
  unsigned int nb_samples    = (unsigned int)(atoi(argv[2]));
  unsigned int max_leaf_size = (unsigned int)(atoi(argv[3]));

  Basis basis(nb_samples);
  for(auto& xy : basis) xy = sample(gen);

  auto learner = gaml::xtree::classification::learner<X, Y, gaml::score::NormalizedInformationGain>(max_leaf_size, DIM, gen);
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
  auto forest_learner = gaml::bag::learner(learner,
					   gaml::functor::highest_cumulated_frequency<Y>(),
					   gaml::bag::functor::identity(),
					   forest_size,true);
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
