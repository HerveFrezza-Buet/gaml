#include <gaml.hpp>
#include <random>
#include <vector>
#include <utility>


// This examples shows how real risk estimation can be achieved, by
// cross-validation.


// Read this file.
#include <example-silly.hpp>

#define DATA_SIZE 100
int main(int argc, char* argv[]) {

  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());

  // Let us build some fake data. The input set is int, and the output
  // set is a double. In our fake data, input is choosen randomly in
  // [0..100[, and output is choosen randomly in [0,1[. Let us store
  // the data set in some vector.

  std::uniform_real_distribution<double> big_uniform(0, 100);
  std::uniform_real_distribution<double> small_uniform(0, 1);
  silly::DataSet basis(DATA_SIZE);
  for(auto& data : basis)
    data = {big_uniform(gen), small_uniform(gen)};


  // The cross validation techniques are based on a partition of the
  // data set. Let us here cross-validate with different kinds of
  // partitions. To do so, we set up an evaluator for each kind of
  // partition. Such evaluators are learner evaluators (they evaluate
  // a learning algorithm, as opposed to predictor evaluators that
  // evaluate some predicting function). They fit
  // gaml::concepts::LearnerEvaluator.

  bool verbosity = true;

  auto leave_one_out_evaluator = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(), gaml::partition::leave_one_out(), verbosity);
  auto chunk_evaluator         = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(), gaml::partition::chunk(30),       verbosity);
  auto kfold_evaluator         = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(), gaml::partition::kfold(6),        verbosity);

  // Let us now set up our learning algorithm....
  silly::Learner learning_algorithm;

  // ... and evaluate it with the previously defined evaluators.
  double l1o_risk   = leave_one_out_evaluator(learning_algorithm, basis.begin(), basis.end(), silly::input_of_data, silly::output_of_data);
  double chunk_risk = chunk_evaluator        (learning_algorithm, basis.begin(), basis.end(), silly::input_of_data, silly::output_of_data);
  double kfold_risk = kfold_evaluator        (learning_algorithm, basis.begin(), basis.end(), silly::input_of_data, silly::output_of_data);
  
  std::cout << std::endl 
	    << "Estimation of the real risk (leave one out): "  << l1o_risk   << std::endl
	    << "Estimation of the real risk         (chunk): "  << chunk_risk << std::endl
	    << "Estimation of the real risk        (k-fold): "  << kfold_risk << std::endl;

  return EXIT_SUCCESS;
}
