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
#include <gamlxtreeClassificationInternals.hpp>
#include <map>

namespace gaml {
  namespace xtree {
    namespace classification { 
      
      template<typename X, typename Y, typename COMP = gaml::by_default::LesserThan<Y>>
	class Predictor : public xtree::Predictor<X,std::map<Y,double,COMP>>, public xtree::internal::Serializable {
      public:

	typedef X input_type;
	typedef std::map<Y,double,COMP> output_type;

      public:
	
	Predictor() : xtree::Predictor<X,output_type>(), xtree::internal::Serializable() {}
	Predictor(std::shared_ptr<xtree::internal::Tree<X,output_type>> t) : xtree::Predictor<X,output_type>(t), xtree::internal::Serializable() {}
	Predictor(const Predictor& other) : xtree::Predictor<X,output_type>(other) {}

	virtual void write(std::ostream& os) const {
	  this->tree->write(os);
	}

	virtual void read(std::istream& is) {
	  this->tree = xtree::internal::tree_factory<X,output_type,
	  					     classification::internal::Leaf<X,output_type>,
	  					     xtree::internal::ThresholdTest<X>>(is);
	}
	
      };

      template<typename X, typename Y, 
	       template<typename,typename,typename> class SCORE,
	       typename COMP = gaml::by_default::LesserThan<Y>>
      class Learner {
      public:
	typedef classification::Predictor<X,Y,COMP> predictor_type;
	unsigned int nmin;
	unsigned int k;

	Learner(void) {}
	Learner(const Learner& other) : nmin(other.nmin), k(other.k) {}

	Learner(unsigned int min_set_size,
		unsigned int nb_attr_test) : nmin(min_set_size), k(nb_attr_test)  {}
	

	template<typename DataIterator, typename InputOf, typename OutputOf>
	predictor_type operator()(const DataIterator& begin, const DataIterator& end,
				  const InputOf& input_of, const OutputOf& output_of) const {
	  return predictor_type(classification::internal::build_tree<X,Y,SCORE,COMP>(begin,end,input_of,output_of,nmin,k));
	}
      };

      template<typename X, typename Y, template<typename,typename,typename> class SCORE,
	       typename COMP = gaml::by_default::LesserThan<Y>>
      Learner<X,Y, SCORE> learner(unsigned int min_set_size,
				  unsigned int nb_attr_test) {
	return Learner<X,Y,SCORE,COMP>(min_set_size,nb_attr_test);
      }
      
    }
  }
}
