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
  class Shuffle : public Tabular<Iterator> {

  public:

    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename Tabular<Iterator>::iterator                iterator;
    
    Shuffle(const Iterator& begin_iter, 
	    const Iterator& end_iter) : Tabular<Iterator>(begin_iter) {
      unsigned int i=0;
      this->indices.resize(std::distance(begin_iter,end_iter));
      for(auto& idx : this->indices) idx = i++;
      std::random_shuffle(this->indices.begin(),this->indices.end());
    }
  };
  
  template<typename Iterator>
  Shuffle<Iterator> shuffle(const Iterator& begin, const Iterator& end) {
    return Shuffle<Iterator>(begin,end);
  }

  template<typename Iterator> 
  class PackShuffle : public Tabular<Iterator> {

  public:

    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef typename Tabular<Iterator>::iterator                iterator;
    
    PackShuffle(const Iterator& begin_iter, 
		const Iterator& end_iter,
		unsigned int pack_size) : Tabular<Iterator>(begin_iter) {
      unsigned int nb_blocks;
      unsigned int i,j,k;

      this->indices.resize(std::distance(begin_iter,end_iter));
      std::vector<unsigned int> blocks;
      std::vector<unsigned int> begins;
      std::vector<unsigned int> ends;
      std::vector<unsigned int> sizes;
      std::vector<unsigned int> idxs(this->indices.size());

      nb_blocks = idxs.size() / pack_size;
      if(idxs.size() % pack_size)
	nb_blocks++;
      blocks.resize(nb_blocks);
      begins.resize(nb_blocks);
      ends.resize(nb_blocks);
      sizes.resize(nb_blocks);

      k = 0; for(auto& b : blocks) b = k++;
      std::random_shuffle(blocks.begin(),blocks.end());

      i = 0; for(auto& idx : idxs) idx = i++;
      
      auto start = idxs.begin();

      for(i = 0, k = 0; k < nb_blocks; ++k, i = j) {
	j=i+pack_size;
	if(j > idxs.size())
	  j = idxs.size();
	begins[k] = i;
	ends  [k] = j;
	sizes [k] = j-i;
	std::random_shuffle(start+i,start+j);
      }

      auto out = this->indices.begin();
      start    = idxs.begin();
      for(auto& b : blocks) {
	std::copy(start + begins[b], start + ends[b], out);
	out += sizes[b];
      }
    }
  };
  
  template<typename Iterator>
  PackShuffle<Iterator> packed_shuffle(const Iterator& begin, const Iterator& end, unsigned int pack_size) {
    return PackShuffle<Iterator>(begin,end,pack_size);
  }
}
