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

#include <gamlxtreeInternals.hpp>
#include <map>

namespace gaml {
  namespace xtree {
    namespace classification { 
      namespace internal {

	template<typename X, typename Y>
	class Leaf : public xtree::internal::Leaf<X,Y> {
	public:
	  Y value;
	  Leaf() : xtree::internal::Leaf<X,Y>() {}
	  Leaf(Y val) : xtree::internal::Leaf<X,Y>(), value(val){}
	  virtual ~Leaf() {}

	  virtual Y operator()(const X& x) {
	    return value;
	  }

	  virtual void write_leaf(std::ostream& os) const {
	    os << value.size() << std::endl;
	    for(auto& kv : value)
	      os << kv.first << ' ' << kv.second << std::endl;
	  }

	  virtual void read_leaf(std::istream& is) {
	    char sep;
	    unsigned int size;
	    typename Y::key_type   k;
	    typename Y::mapped_type v;
	    is >> size; is.get(sep);
	    value.clear();
	    for(unsigned int i = 0; i < size; ++i) {
	      is >> k; is.get(sep);
	      is >> v; is.get(sep);
	      value[k] = v;
	    }
	  }
	};	

	template<typename X, typename Y, typename DataIterator, typename InputOf, typename OutputOf>
	class MakeLeaf {
	public:
	  std::shared_ptr<xtree::internal::Tree<X,Y>> operator()(const DataIterator& begin, const DataIterator& end,
								 const InputOf& input_of, const OutputOf& output_of) {
	    return std::make_shared<Leaf<X,Y>>(gaml::frequencies<typename Y::key_type,typename Y::key_compare>(begin,end,output_of));
	  }
	};

	
	template<typename X, typename Y,
		 template<typename,typename,typename> class SCORE,
		 typename RANDOM_DEVICE,
		 typename COMP,
		 typename DataIterator, typename InputOf, typename OutputOf>
	std::shared_ptr<xtree::internal::Tree<X,std::map<Y,double,COMP>>> build_tree(const DataIterator& begin, const DataIterator& end,
										     const InputOf& input_of, const OutputOf& output_of,
										     unsigned int nmin,
										     unsigned int k,
										     RANDOM_DEVICE& rd) {
	  return xtree::internal::build_tree<X,std::map<Y,double,COMP>,
	  				     SCORE, RANDOM_DEVICE,
	  				     MakeLeaf>(begin,end,input_of,output_of,nmin,k,rd);
	}
      }
    }
  }
}
