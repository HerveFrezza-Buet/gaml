#pragma once

/*
 *   Copyright (C) 2014,  Supelec
 *
 *   Author : Hervé Frezza-Buet
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
 *   Contact : herve.frezza-buet@supelec.fr
 *
 */

#include <gamlxtreePredictor.hpp>
#include <gamlxtreeRegressionInternals.hpp>

namespace gaml {
  namespace xtree {
    namespace regression { 
      
      template<typename X, typename Y>
      class Predictor : public xtree::Predictor<X,Y>, public xtree::internal::Serializable {
      public:

	typedef X input_type;
	typedef Y output_type;

      public:
	
	Predictor() : xtree::Predictor<X,Y>(), xtree::internal::Serializable() {}
	Predictor(std::shared_ptr<xtree::internal::Tree<X,Y>> t) : xtree::Predictor<X,Y>(t), xtree::internal::Serializable() {}
	Predictor(const Predictor& other) : xtree::Predictor<X,Y>(other) {}

	virtual void write(std::ostream& os) const {
	  this->tree->write(os);
	}

	virtual void read(std::istream& is) {
	  this->tree = xtree::internal::tree_factory<X,Y,
						     regression::internal::Leaf<X>,
						     xtree::internal::ThresholdTest<X>>(is);
	}
	
      };

      template<typename X, typename Y, template<typename,typename,typename> class SCORE,
	       typename RANDOM_DEVICE>
      class Learner {
      public:
	typedef regression::Predictor<X,Y> predictor_type;
	unsigned int nmin;
	unsigned int k;
	mutable RANDOM_DEVICE rd;

	Learner() = delete;
	Learner(const Learner&) = default;

	Learner(unsigned int min_set_size,
		unsigned int nb_attr_test,
		const RANDOM_DEVICE& rd) : nmin(min_set_size), k(nb_attr_test), rd(rd)  {}
	

	template<typename DataIterator, typename InputOf, typename OutputOf>
	predictor_type operator()(const DataIterator& begin, const DataIterator& end,
			     const InputOf& input_of, const OutputOf& output_of) const {
	  return predictor_type(regression::internal::build_tree<X,Y,SCORE,RANDOM_DEVICE>(begin,end,input_of,output_of,nmin,k,rd));
	}
      };

      /**
       * This builds a extreme tree learner for classification.
       * @param min_set_size If a split leads to a leaf with less that this amount of samples, it will not be splitted further.
       * @param nb_attr_test At each split we test some of the attributes (with a single random threshold). This is the number of tested attributes.
       */
      template<typename X, typename Y, template<typename,typename,typename> class SCORE,
	       typename RANDOM_DEVICE>
      Learner<X,Y, SCORE, RANDOM_DEVICE> learner(unsigned int min_set_size,
				  unsigned int nb_attr_test,
				  const RANDOM_DEVICE& rd) {
	return Learner<X,Y, SCORE, RANDOM_DEVICE>(min_set_size,nb_attr_test,rd);
      }
      
    }
  }
}
