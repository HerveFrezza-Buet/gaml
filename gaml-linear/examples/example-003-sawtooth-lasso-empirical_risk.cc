// In this example, we make use of LARS in order to approximate a sawtooth wave function
// making use of sine harmonics at different frequencies.

// We here make use of a target empirical risk stopping condition

#include <gaml.hpp>
#include <gaml-linear.hpp>

#include "example-003.h"

int main(int argc, char* argv[]) {

  std::srand(std::time(0));
  
  if(argc != 2) {
    std::cerr << "Usage : " << argv[0] << " empirical_risk" << std::endl;
    ::exit(0);
  }

  double empirical_risk = atof(argv[1]);

  // Create the basis
  Basis b;
  fill_basis(b);

  // Learn a predictor
  auto learner = gaml::linear::lasso::target_empirical_risk_learner<X>(phi, NB_FEATURES, empirical_risk, true, false);
  auto pred = learner(b.begin(), b.end(), input_of, label_of);

  auto predictor_evaluator = gaml::risk::empirical(gaml::loss::Quadratic<double>());
  // The evaluator can then evaluate out predictor on the data basis.
  double risk = predictor_evaluator(pred,
				    b.begin(),b.end(),
				    input_of, label_of);
  std::cout << "Empirical risk : " << risk << std::endl;
  std::cout << "Number of active dimensions : " << pred.w.size() << std::endl;

  generate_plot("sawtooth_lasso_empirical_risk", b, pred);
}
