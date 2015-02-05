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

#include <vector>
#include <iterator>
#include <algorithm>
#include <gamlTabular.hpp>

namespace gaml {

  template<typename Iterator> 
  class Split {
  public:

    Tabular<Iterator> true_values, false_values;
    
    template<typename Test>
    Split(const Iterator& begin_iter, 
	  const Iterator& end_iter, 
	  const Test& test) 
      : true_values(begin_iter), false_values(begin_iter) {
      auto out_true  = std::back_inserter(true_values.indices);
      auto out_false = std::back_inserter(false_values.indices);

      unsigned int i=0;
      for(auto it = begin_iter; it != end_iter; ++it)
	if(test(*it))
	  *(out_true++)  = i++;
	else
	  *(out_false++) = i++;
    }
  };
  
  template<typename Iterator, typename Test>
  Split<Iterator> split(const Iterator& begin, const Iterator& end, const Test& test) {
    return Split<Iterator>(begin,end,test);
  }
}
