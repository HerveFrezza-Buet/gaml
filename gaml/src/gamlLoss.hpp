#pragma once
/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Authors : Hervé Frezza-Buet, Frédéric Pennerath 
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

namespace gaml {

  namespace concepts {
    /**
     * @short Concept for the loss function.
     */
    template<typename OUTPUT>
    class Loss {
    public:
      typedef OUTPUT output_type;
      double operator()(const output_type& l1, const output_type& l2) const;
    };
  }

  namespace loss {
    /**
     * @short Quadratic loss
     */
    template<typename OUTPUT>
    class Quadratic {
    public:
      typedef OUTPUT output_type;
      double operator()(const output_type& l1, const output_type& l2) const {
	auto tmp(l2-l1);
	return tmp*tmp;
      }
    };
    
    /**
     * @short Classification loss
     */
    template<typename OUTPUT>
    class Classification {
    public:
      typedef OUTPUT output_type;
      double operator()(const output_type& l1, const output_type& l2) const {
	if(l1 == l2)
	  return 0;
	return 1;
      }
    };

  }
}
