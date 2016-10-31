#include <gaml.hpp>
#include <cmath>


// Let us use a scorer from this file (read it).
#include <example-scorer.hpp>

// Here are the types
typedef double            X;     
typedef unsigned int      Y;     
typedef std::pair<X,Y>    Data;
typedef std::vector<Data> Dataset;

// Data accessors

const X& input_of      (const Data& d) {return d.first; }
const Y& output_of     (const Data& d) {return d.second;}
Y        class_of_label(Y label)       {return label;   } // for the confusion matrix

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
    *(out++)   = {x,oracle(x)};
  }

  // This is an instance of our learning algorithm. It learns a
  // scorer, i.e. a function that associates a scalar to the
  // input. From the value of this scalar, one can guess the class
  // associated to the input. We are in a bi-class context.
  
  auto score_learner = scorer::Learner();
    
    

  std::cout << std::endl
	    << "############" << std::endl
	    << "#          #" << std::endl
	    << "# 1 vs all #" << std::endl
	    << "#          #" << std::endl
	    << "############" << std::endl
	    << std::endl;
  
  // We can compute a multi-class learning algorithm from a score learner...
  
  auto ova_algo = gaml::multiclass::one_vs_all::learner<Y>(score_learner);
  
  
  // ... and get a predictor from it.
  
  auto ova_pred = ova_algo(dataset.begin(), dataset.end(), input_of, output_of);
  
  // We can now compute the confusion matrix of this predictor.
  
  gaml::classification::Confusion<Y> ova_matrix;
  ova_matrix.clear();
  ova_matrix.update(ova_pred,
		    dataset.begin(), dataset.end(),
		    input_of, output_of, class_of_label);
  ova_matrix.display(std::cout);

  

  std::cout << std::endl
	    << "##########" << std::endl
	    << "#        #" << std::endl
	    << "# 1 vs 1 #" << std::endl
	    << "#        #" << std::endl
	    << "##########" << std::endl
	    << std::endl;

  // 1 vs 1 needs learner and classifiers that produce labels, not
  // scores. Our learning algorithm computes a score. We have to build
  // a learner from it (see the scorer example).
  
  auto biclass_algo = gaml::score2class::learner<Y>(score_learner,
						    [](double score) -> bool {return score >=0;});

  // Then, from it, we can compute a multi-class learning algorithm...
  
  auto ovo_algo = gaml::multiclass::one_vs_one::learner(biclass_algo);
  
  // ... and get a predictor from it.
  
  auto ovo_pred = ovo_algo(dataset.begin(), dataset.end(), input_of, output_of);
  
  // We can now compute the confusion matrix of this predictor.
  
  gaml::classification::Confusion<Y> ovo_matrix;
  ovo_matrix.clear();
  ovo_matrix.update(ovo_pred,
		    dataset.begin(), dataset.end(),
		    input_of, output_of, class_of_label);
  ovo_matrix.display(std::cout);
  

  return EXIT_SUCCESS;
}
