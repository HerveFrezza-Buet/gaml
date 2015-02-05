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

#include <array>
#include <list>
#include <functional>
#include <exception>
#include <sstream>

namespace gaml {
  namespace multidim {
    
    /**
     * This predicts a vector of value from a scalar predictor.
     */
    template<typename OUTPUT,
	     typename PREDICTOR,
	     int DIM>
    class Predictor {

    public:
      
      typedef OUTPUT output_type;
      typedef typename PREDICTOR::input_type input_type;
      
      typedef std::array<typename PREDICTOR::output_type,DIM> result_type;
      typedef std::function<OUTPUT (const result_type&)>      converter_type;
      
      typedef std::list<PREDICTOR> predictors_type; // A list do not requires default constructor for its elements.

      
    private:
      predictors_type preds;
      converter_type converter;

    public:

      template<typename PredIter>
      Predictor(const converter_type& output_converter,
		const PredIter& begin, const PredIter& end) 
	: preds(),
	  converter(output_converter) {
	unsigned int i =0;
	auto iter = begin;
	for(; i < DIM && iter != end; ++iter, ++i)
	  preds.push_back(*iter);
	
	for(;iter != end; ++iter, ++i);

	if(i != DIM){
	  std::ostringstream ostr;
	  ostr << "gaml::multidim::Predictor : initializing a " << DIM
	       << "-dimension(s) predictor from " << i
	       << " elementary predictors." << std::endl;
	  throw std::runtime_error(ostr.str());
	}
      }

      Predictor(const Predictor& other) 
      : preds(other.preds),
	converter(other.converter) {}

      Predictor& operator=(const Predictor& other) {
	if(this != &other) {
	  preds     = other.preds;
	  converter = other.converter;
	}
	return *this;
      }

      output_type operator()(const input_type& x) const {
	result_type values;
	auto res_iter = values.begin();
	for(auto& pred : preds)
	  *(res_iter++) = pred(x);
	return converter(values);
      }

      /**
       * Returns the collection of all the predictors.
       */
      const predictors_type& predictors(void) {
	return preds;
      }
    };

    template<int DIM,
	     typename LEARNER,
	     typename OUTPUT>
    class Learner {

    public:

      typedef typename LEARNER::predictor_type::input_type input_type;
      typedef OUTPUT output_type;
      typedef gaml::multidim::Predictor<output_type,typename LEARNER::predictor_type,DIM> predictor_type;
      typedef std::function<typename predictor_type::result_type (output_type)> to_array_type;
      typedef std::function<output_type (typename predictor_type::result_type)> to_output_type;
      
    private:

      LEARNER algo;
      to_array_type  to_array;
      to_output_type to_output;
      
    public:
      
      
      Learner(const LEARNER& learner, 
	      const to_array_type& array_of_output,
	      const to_output_type& output_of_array) 
	: algo(learner),
	  to_array(array_of_output),
	  to_output(output_of_array) {}
      
      Learner(const Learner& other) 
	: algo(other.algo),
	  to_array(other.to_array),
	  to_output(other.to_output){}
      
      Learner& operator=(const Learner& other) {
	if(this != &other) {
	  algo      = other.algo;
	  to_array  = other.to_array;
	  to_output = other.to_output;
	}

	return *this;
      }
      
      // This does the learning, and returns a predictor from the data.
      template<typename DataIterator, typename InputOf, typename OutputOf> 
      predictor_type operator()(const DataIterator& begin, const DataIterator& end,
      				const InputOf& input_of, const OutputOf& output_of) const {
	typename std::list<typename LEARNER::predictor_type> preds;

	for(unsigned int dim = 0; dim < DIM; ++dim)
	  preds.push_back(algo(begin,end,input_of,
			       [&output_of,this,dim](const typename DataIterator::value_type& data) -> typename LEARNER::predictor_type::output_type {
				 return to_array(output_of(data))[dim];
			       }));
      
      	return predictor_type(to_output,preds.begin(),preds.end());			  
      }
    };

    
    template<typename OUTPUT,
	     int DIM,
	     typename LEARNER>
    Learner<DIM, LEARNER, OUTPUT> learner(const LEARNER& learner, 
					  const typename Learner<DIM, LEARNER, OUTPUT>::to_array_type& array_of_output,
					  const typename Learner<DIM, LEARNER, OUTPUT>::to_output_type& output_of_array) {
      return Learner<DIM, LEARNER, OUTPUT>(learner,array_of_output,output_of_array);
    }
    
  }
}
