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

#include <iterator>
#include <type_traits>
#include <string>

namespace gaml {

  struct is_virtual {};
  struct is_tabular {};
  struct regular_iterator_tag {
    static std::string to_string() {return "gaml::regular_iterator_tag";}
  };
  struct virtualized_iterator_tag {
    static std::string to_string() {return "gaml::virtualized_iterator_tag";}
  };
  struct tabular_iterator_tag {
    static std::string to_string() {return "gaml::tabular_iterator_tag";}
  };

  /**
   * For internal use
   */
  template<typename Iterator, typename False>
  struct virtual_category_of {
    typedef regular_iterator_tag tag_type;
  };

  /**
   * For internal use
   */
  template<typename Iterator>
  struct virtual_category_of<Iterator, std::true_type> {
    typedef virtualized_iterator_tag tag_type;
  };

  /**
   * For internal use
   */
  template<typename Iterator, typename False>
  struct category_of {
    typedef typename virtual_category_of<Iterator,
					 typename std::is_base_of<gaml::is_virtual,
								  Iterator>::type>::tag_type tag_type;
  };

  /**
   * For internal use
   */
  template<typename Iterator>
  struct category_of<Iterator, std::true_type> {
    typedef tabular_iterator_tag tag_type;
  };
  

  template<typename Iterator>
  struct iterator_traits {
    typedef Iterator iterator_type;
    typedef typename category_of<Iterator, 
				 typename std::is_base_of<gaml::is_tabular,
							  Iterator>::type>::tag_type      tag_type;
  };
}
