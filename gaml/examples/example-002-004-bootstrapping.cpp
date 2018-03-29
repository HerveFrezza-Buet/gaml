#include <gaml.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <utility>

// This example shows the usage of bootstrapping methods to evaluate the risk.


// Read this file.
#include <example-ratio.hpp>

#define DATA_SIZE            20
#define NB_BOOTSTRAPPED_SETS 10
#define RATIO                 1
#define NOISE                .2
int main(int argc, char* argv[]) {

  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());


  // Let us build some fake data. The input set is int, and the output
  // set is a double. In our fake data, input is choosen randomly in
  // [0..100[, and output is choosen randomly in [0,1[. Let us store
  // the data set in some vector.
  
  std::uniform_real_distribution<double> uniform(0, 1);
  std::uniform_real_distribution<double> noise  (-NOISE,NOISE);
  
  ratio::DataSet basis(DATA_SIZE);
  for(auto& data : basis) {
    double x = uniform(gen);
    double y = RATIO*x + noise(gen);
    data = {x,y};
  }

  // Let us compute the different bootstrapping risks.
  bool verbosity = true;

  auto leave_one_out_evaluator = gaml::risk::bootstrap::leave_one_out(gaml::loss::Quadratic<double>(), NB_BOOTSTRAPPED_SETS, gen, verbosity);
  auto r632_evaluator          = gaml::risk::bootstrap::r632         (gaml::loss::Quadratic<double>(), NB_BOOTSTRAPPED_SETS, gen, verbosity);
  auto r632plus_evaluator      = gaml::risk::bootstrap::r632plus     (gaml::loss::Quadratic<double>(), NB_BOOTSTRAPPED_SETS, gen, verbosity);

  ratio::Learner learning_algorithm;

  double R    = leave_one_out_evaluator(learning_algorithm, basis.begin(), basis.end(), ratio::input_of_data, ratio::output_of_data);
  std::cout << "#### Bootstrapped leave-one-out risk : " << R        << std::endl << std::endl;
  double R632 = r632_evaluator         (learning_algorithm, basis.begin(), basis.end(), ratio::input_of_data, ratio::output_of_data);
  std::cout << "#### Bootstrapped R.632 risk : "         << R632     << std::endl << std::endl;
  double R632plus = r632plus_evaluator (learning_algorithm, basis.begin(), basis.end(), ratio::input_of_data, ratio::output_of_data);
  std::cout << "#### Bootstrapped R.632+ risk : "        << R632plus << std::endl << std::endl;	

  // Nota : For R632+, the 0-information error rate and the empirical
  // risk are the same here, as displayed, because our regressor is
  // specific and is quite a dummy case.

  return 0;
}
