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

#include <map>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

#include <gamlFilter.hpp>

namespace gaml {

  namespace concept {
    namespace score {
      // learner
      // predictor
    }
  }
  
  namespace biclass {
    // learner/predicor from scorer avec labelisation fct.
  }
  
  namespace multiclass {

    namespace one_vs_one {
      
      /**
       * This predicts a label from a vote of internal bi-class predictors.
       */
      template<typename PREDICTOR>
      class Predictor {
	
	typedef typename PREDICTOR::input_type 	input_type;
	typedef typename PREDICTOR::output_type output_type;
	
      private:
	
	typedef std::vector<PREDICTOR>             predictors_type;
	typedef std::map<output_type,unsigned int> vote_type;
	
	predictors_type predictors;
	vote_type       votes;

	void clear_votes() {for(auto& kv : votes) : kv.second = 0;}
			
	
      public:
	
 

	Predictor()                                  = default;
	Predictor(const Predictor& other)            = default;
	Predictor& operator=(const Predictor& other) = default;

	/**
	 * This adds a bi-class predictor in the list.
	 */
	Predictor<output_type,PREDICTOR>& operator+=(const std::pair<const PREDICTOR&, std::pair<output_type,output_type> >& p) {
	  predictor_type.push_back(p.first);
	  // We create entries in the votes map.
	  votes[p.second.first]  = 0;
	  votes[p.second.second] = 0;
	}

	output_type operator()(const input_type& x) const {
	  clear_votes();
	  for(auto& p : predictors) votes[p(x)]++;
	  auto argmax = std::max_element(votes.begin(), votes.end(),
					 [](const vote_type::value_type& a, const vote_type::value_type& b) -> bool {return a.second < b.second;});
	  return argmax->first;
	}
      };
      
      template<typename LEARNER>
      class Learner {
      private:

	LEARNER algo;
	
      public:

	Learner(const LEARNER& algo) : algo(algo) {}
	Learner()                          = default;
	Learner(const Learner&)            = default;
	Learner& operator=(const Learner&) = default;

	typedef Predictor<typename LEARNER::predictor_type> predictor_type;

	// This does the learning, and returns a predictor from the data.
	template<typename DataIterator, typename InputOf, typename OutputOf> 
	predictor_type operator()(const DataIterator& begin, const DataIterator& end,
				  const InputOf& input_of, const OutputOf& output_of) const {
	  // First, let us collect the labels which are in the data.
	  std::set<typename LEARNER::predictor_type::output_type> labels;
	  for(auto it = begin; it != end; ++it) labels.insert(output_of(*it));
	  
	  // Now, let us learn for each label pair.
	  predictor_type res;
	  for(auto l1 = labels.begin(); l1 != labels.end(); ++l1) {
	    auto l2 = l1;
	    for(++l2; l2 != labels.end(); ++l2) {
	      auto dataset = gaml::filter(begin,end,
					  [output_of, a=*l1, b=*l2](const decltype(*begin)& elem) -> bool {
					    return output_of(elem) == a || output_of(elem) == b;
					  });
	      res += {algo(dataset.begin(),dataset.end(),input_of,output_of),{*l1,*l2}};
	    }
	  }
	  return res;				      
	}
      };

      template<typename LEARNER>
      Learner<LEARNER> learner(const LEARNER& learner) {
	return Learner<LEARNER>(learner);
      }
    }
  }
}
