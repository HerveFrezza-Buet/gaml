#pragma once

/*
 *   Copyright (C) 2012,  Supelec
 *
 *   Author : Herv� Frezza-Buet, Fr�d�ric Pennerath 
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

#include<exception>
#include<iterator>


namespace gaml {


  template<typename Iterator1, typename Iterator2>
  class MergeIterator {
    Iterator1 it1_, end1_;
    Iterator2 it2_, begin2_;

  public:

    using difference_type = long;
    using value_type        = typename std::iterator_traits<Iterator1>::value_type; 
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::input_iterator_tag;

    

    MergeIterator() : it1_(), end1_(), it2_(), begin2_() {}
    MergeIterator(const Iterator1& it1, const Iterator1& end1, const Iterator2& it2, const Iterator2& begin2) : it1_(it1), end1_(end1), it2_(it2), begin2_(begin2) {
    }
    MergeIterator(const MergeIterator& other) : it1_(other.it1_), end1_(other.end1_), it2_(other.it2_), begin2_(other.begin2_) {}
    MergeIterator& operator++(void) {
      if(it1_ != end1_) ++it1_; else ++it2_;
      return *this;
    }

    auto operator*(void) const -> decltype(*it1_)  {
      if(it1_ != end1_) return *it1_;
      else return *it2_;
    }
    
    bool operator!=(const MergeIterator& other) const {
      return (it1_ != other.it1_) || (it2_ != other.it2_);
    }
    bool operator==(const MergeIterator& other) const {
      return (it1_ == other.it1_) && (it2_ == other.it2_);
    }

  };

  template<typename Iterator1, typename Iterator2> class Merge {
    Iterator1 begin1_, end1_;
    Iterator2 begin2_, end2_;
  public:
    using value_type = typename std::iterator_traits<Iterator1>::value_type;
    using iterator = MergeIterator<Iterator1, Iterator2>;

    Merge(const Iterator1& begin1, const Iterator1& end1, const Iterator2& begin2, const Iterator2& end2) : begin1_(begin1), end1_(end1), begin2_(begin2), end2_(end2) {}
    Merge(const Merge& other) : begin1_(other.begin1_), end1_(other.end1_), begin2_(other.begin2_), end2_(other.end2_) {}

    size_t size() { return (end1_ - begin1_) + (end2_ - begin2_); }
    bool empty() { return (end1_ == begin1_) && (end2_ == begin2_); }

    iterator begin() { return MergeIterator<Iterator1,Iterator2>(begin1_, end1_, begin2_, begin2_); }
    iterator end() { return MergeIterator<Iterator1,Iterator2>(end1_, end1_, end2_, begin2_); }
  };

  template<typename Iterator1, typename Iterator2> 
  Merge<Iterator1,Iterator2> merge(const Iterator1& begin1, const Iterator1& end1, 
				    const Iterator2& begin2, const Iterator2& end2) {
    return Merge<Iterator1,Iterator2>(begin1,end1,begin2,end2);
  }

}
