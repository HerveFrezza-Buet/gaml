#include <utility>
#include <vector>
#include <iterator>
#include <cmath>
#include <iomanip>

#include <gaml.hpp>


// here, let us address the case where the bi-class learning algorithm
// provides a scorer rather than a predictor. A scorer gives a scalar
// score to the input. From the value of this score (usually the
// sign), a class can be deduced. Predictors, which are central in
// gaml, compute labels rather than scores. The link between scorer
// and predictors is easy in gaml.


// Read this file
#include <example-scorer.hpp>

// Here are the types
typedef double            X;        // Inputs
typedef char              Y;        // Outputs : 'A' or 'B'
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Dataset;

// Data accessors

const X& input_of(const Data& d) {return d.first;}
const Y& output_of(const Data& d) {return d.second;}


int main(int argc, char* argv[]) {
  
  // random seed initialization
  std::srand(std::time(0));

  // Making the dataset

  Dataset dataset;
  auto out = std::back_inserter(dataset);

  for(unsigned int i=0; i < 1000; ++i) {
    X x        = gaml::random::uniform(0,1);
    bool noise = gaml::random::proba(.05);
    if(x < .5)
      if(noise) *(out++) = {x,'B'};
      else      *(out++) = {x,'A'};
    else
      if(noise) *(out++) = {x,'A'};
      else      *(out++) = {x,'B'};
  }

  // Using our scorer

  auto scoring_algo = scorer::Learner<Y>();
  auto scoring_fct  = scoring_algo(dataset.begin(), dataset.end(), input_of, output_of);

  std::cout << std::endl
	    << "Scoring function : " << scoring_fct << std::endl;

  // Now, let us build actual predictors rather than scorers.
  
  auto classification_algo
    = gaml::score2class::learner(scoring_algo,
				 [](double the_score) -> bool {return the_score >= 0;}, // This tells how to decide from the scorer.
				 gaml::classification::find_two_classes<Y>());          // This nows how to retrieve the two class values in the data.
  
  auto predictor = classification_algo(dataset.begin(), dataset.end(), input_of, output_of);

  std::cout << std::endl
	    << "Scoring vs predicting" << std::endl;
  
  for(unsigned int i=0; i < 10; ++i) {
    X x        = gaml::random::uniform(0,1);
    std::cout << "  " << x << " -> " << predictor(x) << '(' << scoring_fct(x) << ')' << std::endl;
  }
  std::cout << std::endl;

  // Now that we have a predictor, we can compute its real risk, as usually.
  
  auto algo_evaluator = gaml::risk::cross_validation(gaml::loss::Classification<Y>(),
                                                     gaml::partition::kfold(5),
                                                     true);
  double real = algo_evaluator(classification_algo,
                               dataset.begin(),dataset.end(),
                               input_of,output_of);
  std::cout << "Real risk  = " << real << std::endl;
  

  return EXIT_SUCCESS;
  
}
