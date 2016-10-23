#pragma once


erreur_faite_expres

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

#include <exception>
#include <iterator>
#include <gamlTabular.hpp>



namespace gaml {

  
  template<typename Iterator>
  Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> identity(const Iterator& begin, const Iterator& end) {
    auto init = [begin,end](std::vector<tabular_index_type>& indices) -> void {
      auto size = std::distance(begin,end);
      auto out = std::back_inserter(indices);
      for(tabular_index_type i = 0; i < size; ++i) *(out++) = i;
    };
    return Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> (begin,init);
  }


}

