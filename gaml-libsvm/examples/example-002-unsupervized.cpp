
#include <gaml-libsvm.hpp>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <utility>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>



typedef std::pair<double,double> XY;
typedef XY                       Data;
typedef std::vector<Data>        DataSet;


// We need a function that builds an array of svm_nodes for
// representing some input. When the input is a collection of values
// that can provide iterators, libsvm::input_of can help. Here, we
// have to write it by ourselves.


int nb_nodes_of(const XY& xy) {
  return 3;
}

void fill_nodes(const XY& xy,struct svm_node* nodes) {
  nodes[0].index = 1;
  nodes[0].value = xy.first;  // x 
  nodes[1].index = 2;
  nodes[1].value = xy.second; // y
  nodes[2].index = -1;        // end
}

#define PPM_SIDE 500
template<typename DECISION>
void plot_decision(const DECISION& f) {
  std::ofstream ppm("decision.ppm");
  
  ppm << "P5" << std::endl
      << PPM_SIDE << ' ' << PPM_SIDE << std::endl
      << "255" << std::endl;
  
  double coef = 2/(PPM_SIDE-1.0);
  int w,h;
  XY p;
  for(h = PPM_SIDE-1; h >= 0; --h) {
    p.second = -1 + h*coef;
    for(w = 0; w < PPM_SIDE; ++w) {
      p.first = -1 + w*coef;
      if(f(p) || w == PPM_SIDE/2 || h == PPM_SIDE/2)
	ppm.put((char)255);
      else
	ppm.put((char)0);
    }
  }
  std::cout << "Image \"decision.ppm\" generated" << std::endl;
  ppm.close();
}

const XY& input_of(const Data& data) {return data;}

int main(int argc, char* argv[]) {

  // Let us make libsvm quiet
  gaml::libsvm::quiet();
  // random seed initialization
  std::srand(std::time(0));


  if(argc != 2) {
    std::cerr << "Usage : " << argv[0] << " <nb-samples>" << std::endl;
    return 1;
  }

  int nb_samples = atoi(argv[1]);
  if(nb_samples < 50) {
    std::cout << "I am using at least 50 samples." << std::endl;
    nb_samples = 50;
  }
  
  try {

    // Let us collect samples.
    
    DataSet basis;

    basis.resize(nb_samples);
    for(auto& data : basis) {
      double x = gaml::random::uniform(-1,1);
      data = XY(x,                                           // x
		.5*(3*x*x-1)+gaml::random::uniform(-.2,.2)); // y
    }

    // Let us set configure a svm

    struct svm_parameter params;
    gaml::libsvm::init(params);
    params.kernel_type = RBF;          // RBF kernel
    params.gamma       = 10;           // k(u,v) = exp(-gamma*(u-v)^2)
    params.svm_type    = ONE_CLASS;
    params.C           = 10;
    params.eps         = 1e-5;         // numerical tolerence

    // This sets up a svm learning algorithm. The libsvm decision for
    // one class is a double, but we want it to be boolean since we
    // dont care about the exact value, so we provide the conversion
    // function.
    auto learner = gaml::libsvm::unsupervized::learner<XY>(params, nb_nodes_of, fill_nodes,
							   [](double decision) -> bool {return decision > 0;});

    // Let us train it and get some decision function f. 
    std::cout << "Learning..." << std::endl;
    auto f = learner(basis.begin(), basis.end(), input_of);

    // Let us see the decision function.
    plot_decision(f);

  }
  catch(gaml::exception::Any& e) {
    std::cout << e.what() << std::endl;
  }
  
  return 0;
}
