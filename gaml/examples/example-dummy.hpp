#pragma once

#include <random>
#include <algorithm>
#include <gaml.hpp>

// Do not start to read this file. It is included by some of our
// examples. Read the examples, and jump to this file when you are
// invited to do so.


namespace dummy {

  // Data are artificially generated for testing the process of
  // variable selection The idea is to generate a dataset with a large
  // number of variables and then to consider an output which is the
  // average of a small subset of variables.

  // In this dummy case, the RELEVANT_ATTRIBUTE_NUMBER first attributes
  // will be relavant, by construction. Indeed, the label for each data
  // is related to the sum of the RELEVANT_ATTRIBUTE_NUMBER first
  // attributes, so that the remaining attributes are meaningless.

  const int    DATA_SIZE                         = 10000;
  const int    ATTRIBUTE_NUMBER                  = 100;
  const int    RELEVANT_ATTRIBUTE_NUMBER         =   5;
  const double NOISE_RATIO_ON_RELEVANT_ATTRIBUTE =   0.1;

  namespace numeric {
    typedef std::array<double,ATTRIBUTE_NUMBER> input_type;
    typedef std::pair<input_type ,double> data_type;
    typedef std::vector<data_type> dataset_type;

    template<typename RANDOM_DEVICE>
    dataset_type build_dataset(RANDOM_DEVICE& rd) {
      dataset_type data(DATA_SIZE);
      std::uniform_real_distribution<double> uniform_value(0,1);
      std::uniform_real_distribution<double> uniform_coef(1-NOISE_RATIO_ON_RELEVANT_ATTRIBUTE, 1+NOISE_RATIO_ON_RELEVANT_ATTRIBUTE);
      for(auto& d : data)  { 
	// Let us put a random value on each attributes.
	for(auto& attr : d.first) attr = uniform_value(rd);
	// let us compute the label as the sum of the first RELEVANT_ATTRIBUTE_NUMBER attributes...
	double total = std::accumulate(d.first.begin(), d.first.begin() + RELEVANT_ATTRIBUTE_NUMBER, 0.);
	// ... and noisify a bit the result.
	d.second = total * uniform_coef(rd);
      }

      return data;
    }
    const input_type&  input_of_data(const data_type& data) {return data.first;}
    double            output_of_data(const data_type& data) {return data.second;}
  }

  namespace nominal {

    typedef std::array<bool,ATTRIBUTE_NUMBER> input_type;
    typedef std::pair<input_type ,bool> data_type;
    typedef std::vector<data_type> dataset_type;

    template<typename RANDOM_DEVICE>
    dataset_type build_dataset(RANDOM_DEVICE& rd) {
      dataset_type data(DATA_SIZE);
      std::bernoulli_distribution attr_proba(0.5);
      std::bernoulli_distribution label_proba(NOISE_RATIO_ON_RELEVANT_ATTRIBUTE);
      for(auto& d : data)  { 
	// Let us put a random value on each attributes.
	for(auto& attr : d.first)
	  attr = attr_proba(rd);
	// let us compute the label as the sum of the first RELEVANT_ATTRIBUTE_NUMBER attributes...
	d.second = std::accumulate(d.first.begin(), d.first.begin() + RELEVANT_ATTRIBUTE_NUMBER, true,
				   [&label_proba, &rd](bool a, bool b) -> bool {
				     if(label_proba(rd))
				       return a;
				     else
				       return a && b;
				   });
      }

      return data;
    }

    const input_type&  input_of_data(const data_type& data) {return data.first;}
    double            output_of_data(const data_type& data) {return data.second;}
  }
}
