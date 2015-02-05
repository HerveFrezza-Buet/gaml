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

    template<typename X, typename Y>
    class Predictor {
    public:

      typedef X input_type;
      typedef Y output_type;

    protected:

      std::shared_ptr<internal::Tree<X,Y>> tree;

    public:
	
      Predictor() : tree() {}
      Predictor(std::shared_ptr<internal::Tree<X,Y>> t) : tree(t) {}
      Predictor(const Predictor& other) : tree(other.tree) {}
      Predictor& operator=(const Predictor& other) {
	if(this != &other)
	  tree = other.tree;
	return *this;
      }
      // This does the prediction.
      output_type operator()(const input_type& x) const {return (*tree)(x);}
    };
  }
}
