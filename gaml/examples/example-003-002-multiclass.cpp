#include <gaml.hpp>
#include <cmath>


// Let us define a simple bi-class learning algorithm. Inputs are
// scalars, labelled with two classes. Classes are integer
// values. Learning consists of computing the average value of each
// class. The decision functions returns a score, telling which class
// is detected.

#include <example-scorer.hpp>


int main(int argc, char* argv[]) {

  // random seed initialization
  std::srand(std::time(0));

  // This is an instance of our learning algorithm.
  auto score_learner = my_algo::Learner();
    

  std::cout << std::endl
	    << "##########" << std::endl
	    << "#        #" << std::endl
	    << "# 1 vs 1 #" << std::endl
	    << "#        #" << std::endl
	    << "##########" << std::endl
	    << std::endl;

  /* 1 vs 1 needs learner and classifiers that produces labels, not
     scores. Our learning algorithm computes a score. We have to build
     a learner from it. */
  auto biclass_algo = gaml::score2class::learner(score_learner,
						 [](double score) -> bool {return score >=0;},
						 gaml::classification::find_two_class<int>());
  
  

  return EXIT_SUCCESS;
}
