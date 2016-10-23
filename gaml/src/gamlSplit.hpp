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
#include <type_traits>
#include <gamlTabular.hpp>

namespace gaml {

  template<typename Tabular>
  struct Split {
    Tabular true_values;
    Tabular false_values;
  };

 
  template<typename Iterator, typename Test>
  Split<Tabular<Iterator, typename is_secondary_iterator<Iterator>::type>> split(const Iterator& begin, const Iterator& end, const Test& test) {
    std::vector<tabular_index_type> false_indices;
    
    auto true_init = [begin,end,test,&false_indices](std::vector<tabular_index_type>& indices) -> void {
      auto out_true  = std::back_inserter(indices);
      auto out_false = std::back_inserter(false_indices);
      tabular_index_type i=0;
      for(auto it = begin; it != end; ++it)
	if(test(*it))
	  *(out_true++)  = i++;
	else
	  *(out_false++) = i++;
    };
    
    auto false_init = [begin,end,&false_indices](std::vector<tabular_index_type>& indices) -> void {
      indices = std::move(false_indices);
    };

    Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> true_table (begin,true_init);
    Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> false_table(begin,false_init);
    

    Split<Tabular<Iterator, typename is_secondary_iterator<Iterator>::type>> result;
    result.true_values  = std::move(true_table);
    result.false_values = std::move(false_table);
    
    return result;
  }
}
