#include <gaml.hpp>
#include <cstdlib>
#include <vector>
#include <utility>
#include <numeric>

// This example shows how to select a relevant subset of variables using the wrapper approach
// and various search strategies.

// Read this file.
#include <example-dummy.hpp>


// A useful function for prompting the user.
template <typename VALUE>
VALUE ask(const std::string& prompt, VALUE min, VALUE max) {
  VALUE choice = min;
  do {
    if(std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cout << prompt;
    std::cin >> choice;
  } while(std::cin.fail() || choice < min || choice > max);
  return choice;
}


// The wrapper approach is based on a learning scheme. Let us define a
// family of predictors, a learner and a generic learner for that
// purpose.

// The generic predictor sums the components of the input (that is a
// sequence like an array, a vector or a linked list) and applies a
// multiplicative coefficient. That coefficient differs from one
// predictor to the other, the learning aiming at adjusting it.  This
// fits the gaml::concept::Predictor concept.
template<typename Input>
class Predictor {
  double coef_;
public:
  typedef Input input_type;
  typedef double output_type;

  Predictor(double coef) : coef_(coef) {}

  // This does the prediction.
  output_type operator()(const Input& input) const {
    return coef_ * std::accumulate(input.cbegin(), input.cend(), 0.);
  }
};

// The learner needs to be parameterized by the type of input (i.e the
// set of variables) since we intend to use it in a generic learner
// (see below). The coefficient is the mean. This learner fits the
// gaml::concept::Learner concept.
template<typename Input> 
struct Learner {
  typedef Predictor<Input> predictor_type;

  Learner(void) {}

  // This does the learning, and returns a predictor from the data.
  template<typename DataIterator, typename InputOf, typename OutputOf> 
  Predictor<Input> operator()(const DataIterator& begin, const DataIterator& end,
			      const InputOf& inputOf, const OutputOf& outputOf) const {
    double mean  = 0;
    int nb_terms = 0;

    for(auto it = begin; it != end; ++it) {
      auto& input = inputOf(*it);
      double sum = std::accumulate(input.begin(), input.end(), 0.);
      if(sum != 0) {
	mean += outputOf(*it)/sum;
	++nb_terms;
      }
    }
    if(nb_terms > 0) mean /= nb_terms;
    return Predictor<Input>(mean);
  }
};

// A generic learner produces a learner for a given type of subset of
// variables.  A generic learner is required by the variable subset
// algorithm. It has to fit gaml::concept::GenericLearner.
struct GenericLearner {
  template<typename Input>
  Learner<Input> make() const { return Learner<Input>(); }
};

// Now search for the best subset of variables using the wrapper
// approach and various search strategies.
int main(int argc, char* argv[]) {

  // Make the test verbose
  bool verbose = true;

  // Builds the artificial dataset
  auto dataset = dummy::build_dataset();

  // The evaluator is a function that maps a given subset of variables to a real score.
  // Here one uses the wrapper approach : the evaluation of a subset of variables
  // consists in assessing the empirical risk of a generic learner using cross validation
  // when applied to the dataset reduced to the considered subset of variables.

  // Wrapped generic learner : produces a learner for a given subset of variables
  GenericLearner generic_learner;

  // We need a real risk estimator for evaluating the algorithms when variables are selected.
  auto real_risk_estimator = gaml::risk::cross_validation(gaml::loss::Quadratic<double>(), gaml::partition::kfold(10), false);

  // Wrapping evaluator
  auto evaluator = gaml::varsel::make_wrapper_evaluator(generic_learner, real_risk_estimator, 
							dataset.begin(), dataset.end(),
							dummy::input_of_data, dummy::output_of_data);

  // Make the evaluator verbose.
  evaluator.verbose();

  // Asks for the search strategy
  std::ostringstream prompt;
  prompt << "Choose your search strategy: "                                      << std::endl
	 << "1) Sequential Forward Selection           (SFS or forward greedy)"  << std::endl
	 << "2) Sequential Backward Selection          (SBS or backward greedy)" << std::endl
	 << "3) Sequential Floating Forward Selection  (SFFS)"                   << std::endl
	 << "4) Sequential Floating Backward Selection (SFBS)"                   << std::endl
	 << "5) Sequential Beam Forward Selection"                               << std::endl
	 << "6) Sequential Beam Backward Selection"                              << std::endl
	 << "> ";
  int choice = ask<int>(prompt.str(),1,6);

  // The result of a search is a subset of variables with its risk.
  std::vector<int> variable_subset; // It will store the best found subset of variables.
  double best_risk;                 // It will store the lowest risk of the best found subset of variables.

  switch(choice) {
  case 1 : best_risk = gaml::varsel::SFS (evaluator, variable_subset, verbose); break; // Sequential Forward Selection.
  case 2 : best_risk = gaml::varsel::SBS (evaluator, variable_subset, verbose); break; // Sequential Backward Selection.
  case 3 : best_risk = gaml::varsel::SFFS(evaluator, variable_subset, verbose); break; // Sequential Floating Forward Selection.
  case 4 : best_risk = gaml::varsel::SFBS(evaluator, variable_subset, verbose); break; // Sequential Floating Backward Selection.
  case 5 :
  case 6 :
    // K best first search 
    int k = ask<int>("Enter queue size K (>0) > ", 1, 1000);
    // Asks for the filtering ratio R >= 1.  A subset is inserted into
    // the queue if its risk is lower than the best risk found so
    // far multiplied by R.
    double filtering_ratio = ask<double>("Enter filtering ratio R (0 <= R <= 1) > ", 0, 1);
    switch(choice) {
    case 5 : best_risk = gaml::varsel::SBFS(evaluator, variable_subset, k, filtering_ratio, verbose); break; // Sequential Beam Forward Selection.
    case 6 : best_risk = gaml::varsel::SBBS(evaluator, variable_subset, k, filtering_ratio, verbose); break; // Sequential Beam Backward Selection
    }
    break;
  }

  // Displays the result, that is the risk of the best attribute
  // subset according to the variable selection process.
  std::cout << std::endl << std::endl;
  std::cout << "Best found subset of variables = ";
  for(int elt : variable_subset) std::cout << elt << ' ';
  std::cout << "with risk " << best_risk << std::endl;

  // For the sake of comparison, compute the risk of what is
  // expected to be the best selection, i.e. the
  // RELEVANT_ATTRIBUTE_NUMBER first attributes (the data has been
  // generated accordingly, see example-dummy.hpp).
  variable_subset.clear();
  for(int i = 0; i != dummy::RELEVANT_ATTRIBUTE_NUMBER; ++i) variable_subset.push_back(i);
  best_risk = evaluator(variable_subset.begin(), variable_subset.end());

  std::cout << "Likely the best attribute set = ";
  for(int elt : variable_subset) std::cout << elt << ' ';
  std::cout << "with risk " << best_risk << std::endl;

  //
  // Builds the predictor based on variable_subset, i.e. the best subset of variables found
  //
  // To do this, first build the dataset view restricted to the variable subset variable_subset
  auto projection = gaml::project(dataset.begin(), dataset.end(), variable_subset.begin(), variable_subset.end(), dummy::input_of_data, dummy::output_of_data);

  // Then applies the generic learner to the dataset restricted to the selected variables
  auto predictor = projection.teach(generic_learner);

  // Finally evaluates the empirical risk on the learning examples
  auto predictor_evaluator = gaml::risk::empirical(gaml::loss::Quadratic<double>());
  double risk = predictor_evaluator(predictor,
				    dataset.begin(), dataset.end(), dummy::input_of_data, dummy::output_of_data);
  std::cout << "Empirical risk on the whole dataset = " << risk << std::endl;

  return EXIT_SUCCESS;
}
