#pragma once

// Do not start to read this file. It is included by some of our
// examples. Read the examples, and jump to this file when you are
// invited to do so.

#include <iterator>

namespace silly {

  // Let us define a silly regressor, that uses a hypothesis set made of
  // constant functions. The predicted label is thus constant, whatever
  // the input. 

  // This fits the gaml::concepts::Predictor concept.
  class Predictor {
  private:
  
    double constant_label;
  
  public:

    typedef int    input_type;
    typedef double output_type;

    Predictor(double label) : constant_label(label) {}

    Predictor(const Predictor&) = default;
    Predictor& operator=(const Predictor&) = default;
  
    // This does the prediction.
    output_type operator()(const input_type& x) const {return constant_label;}
  };


  // The learner class for this type of regressor is also a silly
  // algorithm. It consists of computing the average of all the the
  // labels in the dataset.

  // This fits the gaml::concepts::Learner concept.
  class Learner {

  public:

    typedef Predictor predictor_type;

    Learner(void) {}
    Learner(const Learner& other) {}

    // This does the learning, and returns a predictor from the data.
    template<typename DataIterator, typename InputOf, typename OutputOf> 
    Predictor operator()(const DataIterator& begin, const DataIterator& end,
			 const InputOf&, const OutputOf& label_of) const {

      double mean = 0;
      for(DataIterator it = begin; it != end; ++it) 
	mean += label_of(*it); // We retrieve the label from the current data.
      auto nb_elem = std::distance(begin,end);
      if(nb_elem > 0) mean /= nb_elem;
      return Predictor(mean);
    }
  };

  // These are useful type definitions.
  typedef std::pair<int,double> Data;    // We learn from sample/label pairs (int/double here).
  typedef std::vector<Data>     DataSet; // We decide to use a vector for the data set.

  // We need functions to extract input and output from the data.
  int     input_of_data(const Data& data) {return data.first;}
  double output_of_data(const Data& data) {return data.second;}

}
