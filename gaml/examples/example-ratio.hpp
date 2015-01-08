#pragma once

// Do not start to read this file. It is included by some of our
// examples. Read the examples, and jump to this file when you are
// invited to do so.

#include <iterator>

namespace ratio {

  // Let us define a regressor, that uses a hypothesis set made y =
  // a*x functions.

  // This fits the gaml::concept::Predictor concept.
  class Predictor {
  public:

    typedef double input_type;
    typedef double output_type;

    double a;
    Predictor(double coef) : a(coef) {}

    Predictor(const Predictor& other) : a(other.a) {}
    Predictor& operator=(const Predictor& other) {
      if(this != &other)
	a = other.a;
      return *this;
    }
  
    // This does the prediction.
    output_type operator()(const input_type& x) const {return a*x;}
  };


  // The learner class for this type of regressor consists in
  // computing the average of all the ratios.

  // This fits the gaml::concept::Learner concept.
  class Learner {

  public:

    typedef Predictor predictor_type;

    Learner(void) {}
    Learner(const Learner& other) {}

    // This does the learning, and returns a predictor from the data.
    template<typename DataIterator, typename InputOf, typename OutputOf> 
    Predictor operator()(const DataIterator& begin, const DataIterator& end,
			 const InputOf& input_of, const OutputOf& label_of) const {
      double mean = 0;
      for(DataIterator it = begin; it != end; ++it) {
	auto x = input_of(*it);
	if(x != 0) {
	  auto y = label_of(*it);
	  mean += y/x; 
	}
      }
      auto nb_elem = std::distance(begin,end);
      if(nb_elem > 0) mean /= nb_elem;
      return Predictor(mean);
    }
  };

  // These are useful type definitions.
  typedef std::pair<double,double> Data;    // We learn from sample/label pairs (int/double here).
  typedef std::vector<Data>        DataSet; // We decide to use a vector for the data set.

  // We need functions to extract input and output from the data.
  double input_of_data (const Data& data) {return data.first;}
  double output_of_data(const Data& data) {return data.second;}

}
