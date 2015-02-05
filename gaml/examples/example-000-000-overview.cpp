#include <gaml.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <utility>

// This examples shows the basics of gaml : setting up a learner and
// test it on some data basis, and estimate its real risk.

// Read this file.
#include <example-silly.hpp>

#define DATA_SIZE 100
int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // Let us build some fake data. The input set is int, and the output
  // set is a double. In our fake data, input is choosen randomly in
  // [0..100[, and output is choosen randomly in [0,1[. Let us store
  // the data set in some vector.

  silly::DataSet basis(DATA_SIZE);
  for(auto& data : basis)
    data = {gaml::random::uniform(0,100),gaml::random::uniform(0,1)};

  // Let us display the data, using the provided stl-like output iterators.
  auto parser           = gaml::make_JSON_parser<silly::Data>();
  auto outputDataStream = gaml::make_output_data_stream(std::cout, parser);
  auto out              = gaml::make_output_iterator(outputDataStream);
  std::copy(basis.begin(),basis.end(),out);
  std::cout << std::endl << std::endl;

  // Now, let us train our algorithm on this database.

  // First create a learner
  silly::Learner learner;

  // Second get a predictor from the learner and the example dataset
  auto predictor = learner(basis.begin(),basis.end(),
			   silly::input_of_data,silly::output_of_data);  
  // lambda functions could have been used instead of input_of_data and output_of_data.

  // The predictor built by the learning process behaves as a function.
  std::cout << "Learnt constant = " << predictor(0) << std::endl;

  // let us now measure the empirical risk of the learnt predictor on
  // the basis. 

  // We use the quadratic loss to set up an predictor evaluator. It
  // fits the concept gaml::concept::PredictorEvaluator.
  auto predictor_evaluator = gaml::risk::empirical(gaml::loss::Quadratic<double>());
  
  // The evaluator can then evaluate our predictor on the data basis.
  double risk = predictor_evaluator(predictor,
				    basis.begin(),basis.end(),
				    silly::input_of_data,silly::output_of_data);
  std::cout << "Empirical risk  = " << risk << std::endl;

  // The library also provides tools for evaluating learning
  // algorithms, instead of predictors. This is typically what cross
  // validation does.
  auto algo_evaluator = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(),
						     gaml::partition::kfold(6),
						     true);
  double real = algo_evaluator(learner,
			       basis.begin(),basis.end(),
			       silly::input_of_data,silly::output_of_data);
  std::cout << "Real risk  = " << real << std::endl;
  

  return EXIT_SUCCESS;
}
