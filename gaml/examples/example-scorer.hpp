#pragma once


#include <iostream>
#include <functional>
#include <gaml.hpp>


// This learning algorithm expects data which are labelled with two
// classes. The input is scalar. The scorer owns two positions, one
// for the positive class, and the other of the negative class. The
// score given to an input x depends on the distances to these two
// positions. It is positive if x is closer to the positive class
// position.

// The learning consists in finding the two class-dependant positions.

namespace scorer {

  /**
   * This fits concept::score::Scorer
   */
  class Scorer {
  private:
    double pos_avg;
    double neg_avg;

    friend std::ostream& operator<<(std::ostream& os, const Scorer& sc) {
      os << "{[+] = " << sc.pos_avg << ", [-] = " << sc.neg_avg << '}';
      return os;
    }
    
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


    // As we fit concept::score::Learner, we can assume that output_of
    // returns a boolean telling which are positive class samples.
    template<typename DataIterator, typename InputOf, typename OutputOf> 
    scorer_type operator()(const DataIterator& begin, const DataIterator& end,
			   const InputOf& input_of, const OutputOf& output_of) const {
	
      // if output_of is a function, it cannot be captured in the
      // lambda capture block. We use std::bind to solve this.
      auto get_output = std::bind(output_of,std::placeholders::_1);

      // We split data according to the class label and we compute
      // averages of each parts.
      auto split = gaml::split(begin, end,
			       [get_output](const decltype(*begin)& d) -> bool {
				 return get_output(d); 
			       });
      double pos_avg = gaml::average(split.true_values.begin(),  split.true_values.end(),  input_of);
      double neg_avg = gaml::average(split.false_values.begin(), split.false_values.end(), input_of);

      return Scorer(pos_avg, neg_avg);
    }
  };
}
