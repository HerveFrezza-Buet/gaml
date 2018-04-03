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
    
  template<typename Iterator, typename RANDOM_DEVICE>
  Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> shuffle(const Iterator& begin, const Iterator& end, RANDOM_DEVICE& rd) {
    auto init = [begin,end,&rd](std::vector<tabular_index_type>& indices) -> void {
      unsigned int i=0;
      indices.resize(std::distance(begin,end));
      for(auto& idx : indices) idx = i++;
      std::shuffle(indices.begin(), indices.end(), rd);
    };
    return Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> (begin,init);
  }
    
  template<typename Iterator, typename RANDOM_DEVICE>
  Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> packed_shuffle(const Iterator& begin, const Iterator& end, unsigned int pack_size, RANDOM_DEVICE& rd) {
    auto init = [begin,end, pack_size, &rd](std::vector<tabular_index_type>& indices) -> void {
      unsigned int nb_blocks;
      unsigned int i,j,k;

      indices.resize(std::distance(begin,end));
      std::vector<unsigned int> blocks;
      std::vector<unsigned int> begins;
      std::vector<unsigned int> ends;
      std::vector<unsigned int> sizes;
      std::vector<unsigned int> idxs(indices.size());

      nb_blocks = idxs.size() / pack_size;
      if(idxs.size() % pack_size)
	nb_blocks++;
      blocks.resize(nb_blocks);
      begins.resize(nb_blocks);
      ends.resize(nb_blocks);
      sizes.resize(nb_blocks);

      k = 0; for(auto& b : blocks) b = k++;
      std::shuffle(blocks.begin(),blocks.end(), rd);

      i = 0; for(auto& idx : idxs) idx = i++;
      
      auto start = idxs.begin();

      for(i = 0, k = 0; k < nb_blocks; ++k, i = j) {
	j=i+pack_size;
	if(j > idxs.size())
	  j = idxs.size();
	begins[k] = i;
	ends  [k] = j;
	sizes [k] = j-i;
	std::shuffle(start+i,start+j, rd);
      }

      auto out = indices.begin();
      start    = idxs.begin();
      for(auto& b : blocks) {
	std::copy(start + begins[b], start + ends[b], out);
	out += sizes[b];
      }
    };
    return Tabular<Iterator, typename is_secondary_iterator<Iterator>::type> (begin,init);
  }
  

}
