#include <gaml.hpp>
#include <vector>
#include <utility>
#include <numeric>
#include <iostream>
#include <sstream>
#include <string>
#include <random>

// This example shows how to select a relevant subset of variables
// using the filter approach and various search strategies.

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

// Let's assume one has a dataset and an evaluator
// (see the main function at the bottom of this file to see examples for both numeric and nominal datasets)

// The evaluator is a function that maps a given subset of variables
// to a real score.  Here one uses the filter approach based on
// correlation: the evaluation of a subset of variables consists in
// computing the ratio of the average correlation between each
// variable and the target divided by the average correlations
// between every pair of variables in the subset.


template<typename DataSet, typename Evaluator>
void test(DataSet& dataset, Evaluator& evaluator) {
  // Make the test verbose.
  bool verbose = true;
  
// Now search for the best subset of variables using the filter
// approach and various search strategies.

  // First asks for the search strategy
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

  // The result of a search is a subset of variables with its score.
  std::vector<int> variable_subset; // It will store the best found subset of variables.
  double best_score;                // It will store the highest score of the best found subset of variables.

  // Apply the selected search strategy
  switch(choice) {
  case 1 : best_score = gaml::varsel::SFS (evaluator, variable_subset, verbose); break; // Sequential Forward Selection.
  case 2 : best_score = gaml::varsel::SBS (evaluator, variable_subset, verbose); break; // Sequential Backward Selection.
  case 3 : best_score = gaml::varsel::SFFS(evaluator, variable_subset, verbose); break; // Sequential Floating Forward Selection.
  case 4 : best_score = gaml::varsel::SFBS(evaluator, variable_subset, verbose); break; // Sequential Floating Backward Selection.
  case 5 :
  case 6 :
    // K best first search 
    int k = ask<int>("Enter queue size K (>0) > ", 1, 1000);
    // Asks for the filtering ratio R >= 1.  A subset is inserted into
    // the queue if its score is greater than the best score found so
    // far multiplied by R.
    double filtering_ratio = ask<double>("Enter filtering ratio R (0 <= R <= 1) > ", 0, 1);
    switch(choice) {
    case 5 : best_score = gaml::varsel::SBFS(evaluator, variable_subset, k, filtering_ratio, verbose); break; // Sequential Beam Forward Selection.
    case 6 : best_score = gaml::varsel::SBBS(evaluator, variable_subset, k, filtering_ratio, verbose); break; // Sequential Beam Backward Selection
    }
    break;
  }

  // Displays the result, that is the score of the best attribute
  // subset according to the variable selection process.
  std::cout << std::endl << std::endl;
  std::cout << "Best found subset of variables = ";
  for(int elt : variable_subset) std::cout << elt << ' ';
  std::cout << "with score " << best_score << std::endl;

  // For the sake of comparison, compute the score of what is
  // expected to be the best selection, i.e. the
  // RELEVANT_ATTRIBUTE_NUMBER first attributes (the data has been
  // generated accordingly, see example-dummy.hpp).
  variable_subset.clear();
  for(int i = 0; i != dummy::RELEVANT_ATTRIBUTE_NUMBER; ++i) variable_subset.push_back(i);
  best_score = evaluator(variable_subset.begin(), variable_subset.end());

  std::cout << "Likely the best attribute set = ";
  for(int elt : variable_subset) std::cout << elt << ' ';
  std::cout << "with score " << best_score << std::endl;
}

int main(int argc, char* argv[]) {

  std::random_device rd;
  std::mt19937 gen(rd());
  
  // Tests first the correlation filter with a numeric dataset
  {
    std::cout << "Let's first test a numeric dataset with the correlation filter\n" << std::endl;

    // Builds an artificial numeric dataset.
    auto dataset = dummy::numeric::build_dataset(rd);

    // Correlation based evaluator
    auto evaluator = gaml::varsel::make_correlation_filter(dataset.begin(), dataset.end(), dummy::numeric::input_of_data, dummy::numeric::output_of_data);

    test(dataset, evaluator);
  }
  
  // Tests then the mutual information filter with a nominal dataset
  {
    std::cout << "\n\nLet's then test a nominal dataset with the mutual information filter\n" << std::endl;

    // Builds an artificial nominal dataset.
    auto dataset = dummy::nominal::build_dataset(rd);

    // Information based evaluator
    auto evaluator = gaml::varsel::make_information_filter(dataset.begin(), dataset.end(), dummy::nominal::input_of_data, dummy::nominal::output_of_data);

    test(dataset, evaluator);
  }

  return EXIT_SUCCESS;
}
