#include <gaml-libsvm.hpp>
#include <cmath>
#include <cstdlib>
#include <array>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <ctime>
#include <limits>

typedef std::pair<double,double> X;
typedef bool                     U;  
typedef std::pair<X,U>           Data;
typedef std::vector<Data>        DataSet;

// This is a ckecker.
U oracle(const X& x) {
  return std::sin(x.first) * std::sin(x.second) > 0;
}

const X&  input_of (const Data& d) {return d.first;}
U        output_of (const Data& d) {return d.second;}

int nb_nodes_of(const X& x) {
  return 3;
}

void fill_nodes(const X& x,struct svm_node* nodes) {
  nodes[0].index = 1;
  nodes[0].value = x.first;  // x 
  nodes[1].index = 2;
  nodes[1].value = x.second; // y
  nodes[2].index = -1;       // end
}

// This defines the discrete values for C and sigma

#define NB_Cs     5
#define NB_SIGMAs 5

// This defines the parameter grid.
std::array<double, NB_Cs    > Cs     = {{.1,  1, 10, 100, 1000}};
std::array<double, NB_SIGMAs> sigmas = {{.1, .5,  1,   3,    5}};


// This fits the gaml::concept::Learner concept.
class GridSearch {
public:
  typedef gaml::libsvm::Predictor<X,U> predictor_type;

  bool verbose;

  GridSearch(void)                    : verbose(false)         {}
  GridSearch(const GridSearch& other) : verbose(other.verbose) {}

  // This does the learning, and returns a predictor from the data.
  template<typename DataIterator, typename InputOf, typename OutputOf>
  predictor_type operator()(const DataIterator& begin, const DataIterator& end,
			    const InputOf& input_of, const OutputOf& output_of) const {
    
    struct svm_parameter params;
    gaml::libsvm::init(params);
    params.eps         = 1e-5;
    params.svm_type    = C_SVC;
    params.kernel_type = RBF;

    auto risk_estimator = gaml::risk::cross_validation(gaml::loss::Classification<U>(),
						       gaml::partition::kfold(5), false);

    double best_risk  = std::numeric_limits<double>::max();
    double best_C     = 0;
    double best_sigma = 0;

    // Grid search here...
    if(verbose) 
      std::cout << "Starting the grid search" << std::endl;
    for(double sigma : sigmas) {
      params.gamma = 1/(2*sigma*sigma);
      for(double C : Cs) {
	params.C = C;
	if(verbose) 
	  std::cout << "  sigma = " << std::setw(5) << sigma 
		    << ", C = "     << std::setw(5) << C 
		    << " -> risk = " << std::flush;
	double risk = risk_estimator(gaml::libsvm::supervized::learner<X,U>(params, nb_nodes_of, fill_nodes),
				     begin, end, input_of, output_of);
	if(verbose) std::cout << risk;
	if(risk < best_risk) {
	  best_risk = risk;
	  best_C = C;
	  best_sigma = sigma;
	  if(verbose) std::cout << " (new minimum)";
	}
	if(verbose) std::cout << std::endl;
      }
    }

    // Let us return a predictor from a SVM with the best parameters.
    params.gamma = 1/(2*best_sigma*best_sigma);
    params.C     = best_C;
    if(verbose) std::cout << "  Learning the predictor with best parameters (sigma = "
			  << best_sigma << ", C = " << best_C 
			  << ", risk = " << best_risk 
			  << ")... " << std::flush;

    auto learner = gaml::libsvm::supervized::learner<X,U>(params, nb_nodes_of, fill_nodes);
    auto pred    = learner(begin, end, input_of, output_of);
    if(verbose) std::cout << "Done." << std::endl;

    return pred;
  }

};
#define PPM_SIDE 500
template<typename DECISION>
void plot_decision(const DECISION& f) {
  std::ofstream ppm("decision.ppm");
  
  ppm << "P5" << std::endl
      << PPM_SIDE << ' ' << PPM_SIDE << std::endl
      << "255" << std::endl;
  
  double coef = 10/(PPM_SIDE-1.0);
  int w,h;
  X p;
  for(h = PPM_SIDE-1; h >= 0; --h) {
    p.second = -5 + h*coef;
    for(w = 0; w < PPM_SIDE; ++w) {
      p.first = -5 + w*coef;
      if(f(p))
	ppm.put((char)255);
      else
	ppm.put((char)0);
    }
  }
  std::cout << "Image \"decision.ppm\" generated" << std::endl;
  ppm.close();
}

int main(int argc, char* argv[]) {

  // Let us make libsvm quiet
  gaml::libsvm::quiet();
  // random seed initialization
  std::srand(std::time(0));

  DataSet basis;

  std::cout << "Building the sample set." << std::endl;

  // Let us fill some databasis.
  basis.resize(1000);
  for(auto& data : basis) {
    X x = {gaml::random::uniform(-5,5),gaml::random::uniform(-5,5)};
    data = {x, oracle(x)};
  }

  try {
    GridSearch learner; // This learner uses an internal cross-validation.
    learner.verbose = true;
    std::cout << "Applying grid search..." << std::endl
	      << std::endl;
    auto pred = learner(basis.begin(), basis.end(), input_of, output_of);
   
    auto emp_eval = gaml::risk::empirical(gaml::loss::Classification<U>());
    std::cout << std::endl
	      << "Empirical risk : " 
	      << emp_eval(pred, basis.begin(), basis.end(), input_of, output_of)
	      << std::endl 
	      << "Drawing decision function..." << std::endl;
    plot_decision(pred);

    // Now, we can estimate the real risk of the whole grid search
    // process... this leads to an intrication of cross-validations.
    
    learner.verbose = false;
    auto real_eval  = gaml::risk::cross_validation(gaml::loss::Classification<U>(), gaml::partition::kfold(5), true);
    double risk     = real_eval(learner, basis.begin(), basis.end(), input_of, output_of);
    std::cout << "The grid search procedure has the following estimated real risk : " << risk << std::endl;
  }
  catch(gaml::exception::Any& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
