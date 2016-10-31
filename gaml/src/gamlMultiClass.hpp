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
#include <functional>
#include <stdexcept>

#include <gamlFilter.hpp>


namespace gaml {

  namespace concept {
    namespace score {
      
      /**
       * @short This compute a scalar from an input. The scalar value is
       * somehow related to a class detection.
       */ 
      class Scorer {
      public:

	/**
	 * Do not use int in your implementation
	 */
	typedef int input_type;
      
	Scorer(const Scorer& other);
	Scorer& operator=(const Scorer& other);
	double operator()(const input_type& x) const;
      };

      /**
       * @short This learns a scorer from a data set.
       */ 
      class Learner {
      public:

	/**
	 * Do not use int in your implementation
	 */
	typedef int scorer_type;
	typedef int output_type;
      
	Learner(const Learner& other);
	Learner& operator=(const Learner& other);

	template<typename DataIterator, typename InputOf, typename OutputOf> 
	scorer_type operator()(const DataIterator& begin, const DataIterator& end,
			       const InputOf&, const OutputOf&) const;
      };

    }

    /**
     * This finds the two classes in a bi-class dataset
     */
    class FindBiclass {
    public:
      
      /**
       * Do not use int in your implementation
       */
      typedef int output_type;
      
      FindBiclass(const FindBiclass& other);

      /**
       * This returns the two classes used to label the dataset. The dataset is supposed to contain exactly two classes.
       */
      template<typename DataIterator, typename OutputOf> 
      std::pair<output_type,output_type> operator()(const DataIterator& begin, const DataIterator& end,
						    const OutputOf&) const;
    };
  }
  

  namespace score2class {

    template<typename SCORER, typename OUTPUT>
    class Predictor {
    private:

      SCORER scorer;
      std::function<bool (double)> decision;
      OUTPUT pos_class;
      OUTPUT neg_class;
      
    public:
      
      typedef typename SCORER::input_type input_type;
      typedef OUTPUT output_type;

      /**
       * decision is a function telling if the score correspond to the positive class.
       */
      template<typename Decision>
      Predictor(const OUTPUT& positive_class,
		const OUTPUT& negative_class,
		const SCORER& scorer,
		const Decision& decision)
	: scorer(scorer), decision(decision), pos_class(positive_class), neg_class(negative_class) {}
      Predictor()                            = default;
      Predictor(const Predictor&)            = default;
      Predictor& operator=(const Predictor&) = default;
      
      output_type operator()(const input_type& x) const {
	if(decision(scorer(x))) return pos_class;
	return neg_class;
      }
    };
    
    /**
     * This builds a predictor from a scorer. 
     * @param positive_class must be the first argument
     * @param negative_class must be the first argument
     */
    template<typename OUTPUT,typename SCORER, typename Decision>
    auto predictor(const OUTPUT& positive_class,
		   const OUTPUT& negative_class,
		   const SCORER& scorer, const Decision& decision) {
      return Predictor<SCORER,OUTPUT>(positive_class, negative_class,
				      scorer, decision);
    }
    
    
    template<typename SCORE_LEARNER, typename FIND_BICLASS>
    class Learner {
    private:
      
      SCORE_LEARNER algo;
      std::function<bool (double)> decision;
      FIND_BICLASS find;
      
    public:
      
      
      /**
       * @param decision is a function telling if the score correspond to the positive class.
       * @param FIND_CLASS must fit gaml::concept::FindBiclass.
       */
      template<typename Decision>
      Learner(const SCORE_LEARNER& algo,
	      const Decision& decision,
	      const FIND_BICLASS& find)
	: algo(algo), decision(decision), find(find)  {}
      
      Learner()                          = default;
      Learner(const Learner&)            = default;
      Learner& operator=(const Learner&) = default;
      
      typedef Predictor<typename SCORE_LEARNER::scorer_type, typename FIND_BICLASS::output_type> predictor_type;
      
      // This does the learning, and returns a predictor from the data.
      template<typename DataIterator, typename InputOf, typename OutputOf> 
      predictor_type operator()(const DataIterator& begin, const DataIterator& end,
				const InputOf& input_of, const OutputOf& output_of) const {
	auto scorer  = algo(begin, end, input_of, output_of);
	auto classes = find(begin, end, output_of);

	// Now, we have to know which is the positive class, i.e the class for which decision(score) is true.
	auto b_class    = output_of(*begin);
	auto b_decision = decision(scorer(input_of(*begin)));
	predictor_type res;
	if(b_class == classes.first)
	  if(b_decision)
	    res = predictor(classes.first,  classes.second, scorer, decision);
	  else
	    res = predictor(classes.second, classes.first, scorer, decision);
	else
	  if(b_decision)
	    res = predictor(classes.second, classes.first, scorer, decision);
	  else
	    res = predictor(classes.first,  classes.second, scorer, decision);
	return res;
      }
    };
    
    template<typename SCORE_LEARNER, typename FIND_BICLASS, typename DECISION>
    auto learner(const SCORE_LEARNER& score_learner,
		 const DECISION& decision,
		 const FIND_BICLASS& find) {
      return Learner<SCORE_LEARNER,FIND_BICLASS>(score_learner, decision, find);
    }
  }
  
  namespace classification {

    /**
     * This finds the two classes which are in the data.
     */
    template<typename OUTPUT>
    class FindTwoClasses {
    public:
      
      typedef OUTPUT output_type;
      
      FindTwoClasses()                         = default;
      FindTwoClasses(const FindTwoClasses& other) = default;

      /**
       * This returns the two classes used to label the dataset. The dataset is supposed to contain exactly two classes.
       */
      template<typename DataIterator, typename OutputOf> 
      std::pair<output_type,output_type> operator()(const DataIterator& begin, const DataIterator& end,
						    const OutputOf& output_of) const {
	if(begin == end) throw std::runtime_error("FindTwoClasses : empty dataset");
	
	output_type pos_class = output_of(*begin);
	output_type neg_class = pos_class;
	
	auto it = begin;
	for(++it; it != end; ++it)
	  if((neg_class = output_of(*it)) != pos_class)
	    break;
	if(it == end) throw std::runtime_error("FindTwoClasses : dataset has only one label");

	return {pos_class,neg_class};
      }
    };

    template<typename OUTPUT>
    FindTwoClasses<OUTPUT> find_two_classes() {return FindTwoClasses<OUTPUT>();}
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
	
	predictors_type   predictors;
	mutable vote_type votes;

	void clear_votes() const {for(auto& kv : votes) kv.second = 0;}
			
	
      public:
	
 

	Predictor()                                  = default;
	Predictor(const Predictor& other)            = default;
	Predictor& operator=(const Predictor& other) = default;

	/**
	 * This adds a bi-class predictor in the list.
	 */
	Predictor<PREDICTOR>& operator+=(const std::pair<const PREDICTOR&, std::pair<output_type,output_type> >& p) {
	  predictors.push_back(p.first);
	  // We create entries in the votes map.
	  votes[p.second.first]  = 0;
	  votes[p.second.second] = 0;

	  return *this;
	}

	output_type operator()(const input_type& x) const {
	  clear_votes();
	  for(auto& p : predictors) votes[p(x)]++;
	  auto argmax = std::max_element(votes.begin(), votes.end(),
					 [](const typename vote_type::value_type& a, const typename vote_type::value_type& b) -> bool {return a.second < b.second;});
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
	  // if output_of is a function, it cannot be captured in the
	  // lambda capture block. We use std::bind to solve this.
	  auto get_output = std::bind(output_of,std::placeholders::_1);
      
	  // First, let us collect the labels which are in the data.
	  std::set<typename LEARNER::predictor_type::output_type> labels;
	  for(auto it = begin; it != end; ++it) labels.insert(output_of(*it));
	  
	  // Now, let us learn for each label pair.
	  predictor_type res;
	  for(auto l1 = labels.begin(); l1 != labels.end(); ++l1) {
	    auto l2 = l1;
	    for(++l2; l2 != labels.end(); ++l2) {
	      auto dataset = gaml::filter(begin,end,
					  [get_output, a=*l1, b=*l2](const typename DataIterator::value_type& elem) -> bool {
					    return get_output(elem) == a || get_output(elem) == b;
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
