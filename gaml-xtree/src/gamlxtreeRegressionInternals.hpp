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


namespace gaml {
  namespace xtree {
    namespace regression { 
      namespace internal {

	template<typename X>
	class Leaf : public xtree::internal::Leaf<X,double> {
	public:
	  double value;
	  Leaf() : xtree::internal::Leaf<X,double>() {}
	  Leaf(double val) : xtree::internal::Leaf<X,double>(), value(val){}
	  virtual ~Leaf() {}

	  virtual double operator()(const X& x) {
	    return value;
	  }

	  virtual void write_leaf(std::ostream& os) const {
	    os << value;
	  }

	  virtual void read_leaf(std::istream& is) {
	    is >> value;
	  }
	};	

	template<typename X, typename Y, typename DataIterator, typename InputOf, typename OutputOf>
	class MakeLeaf {
	public:
	  std::shared_ptr<xtree::internal::Tree<X,double>> operator()(const DataIterator& begin, const DataIterator& end,
								      const InputOf& input_of, const OutputOf& output_of) {
	    return std::make_shared<Leaf<X>>(gaml::average(begin,end,output_of));
	  }
	};

	
	template<typename X, typename Y, 
		 template<typename,typename,typename> class SCORE,
		 typename DataIterator, typename InputOf, typename OutputOf>
	std::shared_ptr<xtree::internal::Tree<X,Y>> build_tree(const DataIterator& begin, const DataIterator& end,
							       const InputOf& input_of, const OutputOf& output_of,
							       unsigned int nmin,
							       unsigned int k) {
	  return xtree::internal::build_tree<X,Y,
					     SCORE, 
					     MakeLeaf>(begin,end,input_of,output_of,nmin,k);
	}
      }
    }
  }
}
