#include <gaml.hpp>
#include <cmath>


// Let us define a simple bi-class learning algorithm. Inputs are
// scalars, labelled with two classes. Classes are integer
// values. Learning consists of computing the average value of each
// class. The decision functions returns a score, telling which class
// is detected.

#include <example-scorer.hpp>

// Here are the types
typedef double            X;     
typedef unsigned int      Y;     
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Dataset;

// Data accessors

const X& input_of      (const Data& d) {return d.first; }
const Y& output_of     (const Data& d) {return d.second;}
Y        class_of_label(Y label)       {return label;   } 

// The oracle
#define NB_CLASSES          5
#define NOISE_PROBABILITY   0.01
Y oracle(const X& x) {
  if(gaml::random::proba(NOISE_PROBABILITY)) return gaml::random::uniform(NB_CLASSES);
  else                                       return (Y)(NB_CLASSES*x);
    
}


int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // Making the dataset

  Dataset dataset;
  auto out = std::back_inserter(dataset);
 
  for(unsigned int i=0; i < 1000; ++i) {
    X x        = gaml::random::uniform(0,1);
    *(out++) = {x,oracle(x)};
  }

  // This is an instance of our learning algorithm.
  
  auto score_learner = scorer::Learner<Y>();
    

  std::cout << std::endl
	    << "##########" << std::endl
	    << "#        #" << std::endl
	    << "# 1 vs 1 #" << std::endl
	    << "#        #" << std::endl
	    << "##########" << std::endl
	    << std::endl;

  // 1 vs 1 needs learner and classifiers that produces labels, not
  // scores. Our learning algorithm computes a score. We have to build
  // a learner from it (see the scorer example).
  
  auto biclass_algo = gaml::score2class::learner(score_learner,
						 [](double score) -> bool {return score >=0;},
						 gaml::classification::find_two_classes<Y>());

  // Then, from it, we can compute a multi-class learning algorithm...
  
  auto ovo_algo = gaml::multiclass::one_vs_one::learner(biclass_algo);
  
  // ... and get a predictor from it.
  
  auto ovo_pred = ovo_algo(dataset.begin(), dataset.end(), input_of, output_of);
  
  // We can now compute the confusion matrix of this predictor.
  
  gaml::classification::Confusion<Y> matrix;
  matrix.clear();
  matrix.update(ovo_pred,
		dataset.begin(), dataset.end(),
		input_of, output_of, class_of_label);
  matrix.display(std::cout);
  

  return EXIT_SUCCESS;
}
