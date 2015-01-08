// In this example, we make use of LARS in order to approximate a sawtooth wave function
// making use of sine harmonics at different frequencies.

// We here make use of a target active set size stopping condition

#include <gaml.hpp>
#include <gaml-linear.hpp>

#include "example-003.h"

int main(int argc, char* argv[]) {

  std::srand(std::time(0));
  
  if(argc != 2) {
    std::cerr << "Usage : " << argv[0] << " active_set_size" << std::endl;
    ::exit(0);
  }

  int active_set_size = atoi(argv[1]);

  // Create the basis
  Basis b;
  fill_basis(b);

  // Learn a predictor
  auto learner = gaml::linear::lars::target_active_set_size_learner<X>(phi, NB_FEATURES, active_set_size, true, false);
  auto pred = learner(b.begin(), b.end(), input_of, label_of);

  auto predictor_evaluator = gaml::risk::empirical(gaml::loss::Quadratic<double>());
  // The evaluator can then evaluate out predictor on the data basis.
  double risk = predictor_evaluator(pred,
				    b.begin(),b.end(),
				    input_of, label_of);
  std::cout << "Empirical risk : " << risk << std::endl;
  std::cout << "Number of active dimensions : " << pred.w.size() << std::endl;

  generate_plot("sawtooth_lars_active_set", b, pred);
}
