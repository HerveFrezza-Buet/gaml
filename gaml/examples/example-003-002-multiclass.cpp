#include <gaml.hpp>
#include <cmath>


// Let us define a simple bi-class learning algorithm. Inputs are
// scalars, labelled with two classes. Classes are integer
// values. Learning consists of computing the average value of each
// class. The decision functions returns a score, telling which class
// is detected.

namespace my_algo {

  /**
   * This fits concept::score::Scorer
   */
  class Scorer {
  private:
    double pos_avg;
    double neg_avg;
    
    
  public:
    Scorer(double pos_avg, double neg_avg)
      : pos_avg(pos_avg), neg_avg(neg_avg) {}
    
    Scorer()                         = default;
    Scorer(const Scorer&)            = default;
    Scorer& operator=(const Scorer&) = default;

    typedef double input_type;
    
    double operator()(const input_type& x) const {
      return std::fabs(x-neg_avg)-std::fabs(x-pos_avg);
    }
  };

  /**
   * This fits concept::score::Learner
   */
  class Learner {
  public:

    typedef Scorer scorer_type;
      
    Learner()                                = default;
    Learner(const Learner& other)            = default;
    Learner& operator=(const Learner& other) = default;

    template<typename DataIterator, typename InputOf, typename OutputOf> 
    scorer_type operator()(const DataIterator& begin, const DataIterator& end,
			   const InputOf& input_of, const OutputOf& output_of) const {
      
      int pos_class = output_of(*begin);
      int neg_class = pos_class;

      // This codes works only if there are two classes in the data...
      auto it = begin;
      for(++it; it != end; ++it)
	if((neg_class = output_of(*it)) != pos_class)
	  break;
      
      auto split = gaml::split(begin, end,
			       [pos_class,output_of](const decltype(*begin)& d) -> bool {
				 return output_of(d) == pos_class;
			       });
      double pos_avg = gaml::average(split.true_values.begin(),  split.true_values.end(),  input_of);
      double neg_avg = gaml::average(split.false_values.begin(), split.false_values.end(), input_of);

      return Scorer(pos_class, neg_class, pos_avg, neg_avg);
    }
  };
}


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
						 [](double score) -> 
  
  

  return EXIT_SUCCESS;
}
