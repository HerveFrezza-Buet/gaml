#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Hervé Frezza-Buet, Frédéric Pennerath 
 *
 *   Contributor :
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License (GPL) as published by the Free Software Foundation; either
 *   version 3 of the License, or any later version.
 *   
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   Contact : herve.frezza-buet@supelec.fr, frederic.pennerath@supelec.fr
 *
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <iterator>
#include <gamlBootstrap.hpp>

namespace gaml {
  namespace bag {

    /**
     * This predictors handles a collection of predictors. It compute
     * the prediction from a merge of all the predictors output.
     */
    template<typename MergeOutput,
	     typename ElementaryPredictor>
    class Predictor {
    public:
      std::vector<ElementaryPredictor> predictors;
      MergeOutput merge;

      typedef typename ElementaryPredictor::input_type   input_type;
      typedef typename ElementaryPredictor::output_type  elementary_output_type;
      typedef typename MergeOutput::output_type          output_type;

      Predictor() : predictors(), merge() {}
      Predictor(const Predictor& other) : predictors(other.predictors), merge(other.merge) {}
      Predictor(Predictor&& other) 
	: predictors(std::move(other.predictors)),
	  merge(std::move(other.merge)) {}
      Predictor& operator=(const Predictor& other) {
	if(this != &other) {
	  predictors = other.predictors;
	  merge = other.merge;
	}
	return *this;
      }
      Predictor& operator=(const Predictor&& other) {
	if(this != &other) {
	  predictors = std::move(other.predictors);
	  merge = std::move(other.merge);
	}
	return *this;
      }

      output_type operator()(const input_type& x) const {
	auto prediction_of = [&x](const ElementaryPredictor& p) -> elementary_output_type {return p(x);};
	return merge(predictors.begin(),predictors.end(),prediction_of);
      }
    };

    /**
     * This learner produces a collection of predictors by learning
     * each one from a randomization of the input data set.
     */
    template<typename MergeOutput,
	     typename BasisRandomizer,
	     typename ElementaryLearner>
    class Learner {
    public:
      
      typedef gaml::bag::Predictor<MergeOutput, 
				   typename ElementaryLearner::predictor_type> predictor_type;
      ElementaryLearner learner;
      MergeOutput merger;
      BasisRandomizer randomizer;
      unsigned int nb_predictors;
      bool verbosity;
      
      Learner(const ElementaryLearner& l, 
	      const MergeOutput& output_merger,
	      const BasisRandomizer& dataset_randomizer,
	      unsigned int size,
	      bool is_verbose) 
	: learner(l), 
	  merger(output_merger),
	  randomizer(dataset_randomizer),
	  nb_predictors(size),
	  verbosity(is_verbose){}
      Learner(const Learner& other) 
	: learner(other.learner), 
	  merger(other.merger),
	  randomizer(other.randomizer),
	  nb_predictors(other.nb_predictors),
	  verbosity(other.verbosity) {}
      Learner& operator=(const Learner& other) {
	if(this != &other) {
	  learner       = other.learner;
	  merger        = other.merger;
	  randomizer    = other.randomizer;
	  nb_predictors = other.nb_predictors;
	  verbosity     = other.verbosity;
	}
	return *this;
      }

      template<typename DataIterator, typename InputOf, typename OutputOf> 
      predictor_type operator()(const DataIterator& begin, const DataIterator& end,
				const InputOf& input_of, const OutputOf& output_of) const {
	predictor_type predictor;
	auto out = std::back_inserter(predictor.predictors);

	if(verbosity)
	  std::cout << std::endl;
	for(unsigned int i = 0; i < nb_predictors; ++i) {
	  if(verbosity)
	    std::cout << "Learning " << std::setw(3) << i+1 << '/' << nb_predictors << "...                \r" << std::flush;
	  auto randomized_basis = randomizer(begin,end);
	  *(out++) = learner(randomized_basis.begin(),
			     randomized_basis.end(),
			     input_of,output_of);
	}
	if(verbosity)
	  std::cout << std::endl << std::endl;
	return predictor;
      }
    };

    
    template<typename MergeOutput,
	     typename BasisRandomizer,
	     typename ElementaryLearner>
    Learner<MergeOutput,BasisRandomizer,ElementaryLearner> learner(const ElementaryLearner& l, 
								   const MergeOutput& output_merger,
								   const BasisRandomizer& dataset_randomizer,
								   unsigned int size,
								   bool is_verbose) {
      return Learner<MergeOutput,BasisRandomizer,ElementaryLearner>(l,output_merger,dataset_randomizer,size,is_verbose);
    }

    namespace set {

      /**
       * This is data set randomizer for bag learners that builds a
       * bootstrapped set for each learner.
       */
      class Bootstrap {
      private:
	unsigned int size;
      public:
	Bootstrap(unsigned int bootstrap_set_size) : size(bootstrap_set_size) {}
	template<typename DataIterator> 
	auto operator()(const DataIterator& begin, const DataIterator& end) const {
	  return gaml::bootstrap(begin,end,size);
	}
      };
    }
  }
}
