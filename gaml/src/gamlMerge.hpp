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

#include<exception>
#include<iterator>
#include <gamlVirtual.hpp>


namespace gaml {


  template<typename Iterator1, typename Iterator2>
  class MergeIterator : public gaml::virtualized::base_iterator<std::input_iterator_tag, 
								typename std::iterator_traits<Iterator1>::value_type> {
    Iterator1 it1_, end1_;
    Iterator2 it2_, begin2_;

  public:
    typedef typename std::iterator_traits<Iterator1>::value_type value_type;

    MergeIterator() : it1_(), end1_(), it2_(), begin2_() {}
    MergeIterator(const Iterator1& it1, const Iterator1& end1, const Iterator2& it2, const Iterator2& begin2) : it1_(it1), end1_(end1), it2_(it2), begin2_(begin2) {
    }
    MergeIterator(const MergeIterator& other) : it1_(other.it1_), end1_(other.end1_), it2_(other.it2_), begin2_(other.begin2_) {}
    MergeIterator& operator++(void) {
      if(it1_ != end1_) ++it1_; else ++it2_;
      return *this;
    }

    // MergeIterator& operator--(void) {
    //   if(it2_ != begin2_) --it2_;
    //   else --it1_;
    //   return *this;
    // }

    auto operator*(void) const -> decltype(*it1_)  {
      if(it1_ != end1_) return *it1_;
      else return *it2_;
    }

    // typename MergeIterator::difference_type operator-(const MergeIterator& other) const {
    //   return (it1_ - end1_) + (it2_ - begin2_) - (other.it1_ - other.end1_) - (other.it2_ - other.begin2_);
    // }

    // MergeIterator& operator+=(typename MergeIterator::difference_type i) {
    //   typename MergeIterator::difference_type n1 = end1_ - it1_;
    //   typename MergeIterator::difference_type n2 = it2_ - begin2_;
    //   if(n1 != 0) {
    // 	if(i < n1) it1_ += i; else { it1_ = end1_; it2_ += (i - n1); }
    //   } else {
    // 	if(i > -n2) it2_ += i; else { it2_ = begin2_; it1_ += (i + n2); }
    //   }
    //   return *this;
    // }
    // MergeIterator& operator-=(typename MergeIterator::difference_type i) {
    //   return (*this += (-i));
    // }

    // MergeIterator operator+(typename MergeIterator::difference_type i) const {
    //   MergeIterator it(*this);
    //   it += i;
    //   return it;
    // }
    // MergeIterator operator-(typename MergeIterator::difference_type i) const {
    //   return *this + (-i);
    // }

    bool operator!=(const MergeIterator& other) const {
      return (it1_ != other.it1_) || (it2_ != other.it2_);
    }
    bool operator==(const MergeIterator& other) const {
      return (it1_ == other.it1_) && (it2_ == other.it2_);
    }

    // Virtualization

    typedef gaml::virtualized::base_iterator<std::input_iterator_tag, 
					     typename std::iterator_traits<Iterator1>::value_type> base_type;
    
    virtual base_type* clone() const {
      return new MergeIterator<Iterator1,Iterator2>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const MergeIterator<Iterator1,Iterator2>*>(&other);
      return (*this) == (*p_iter);
    }

    // virtual void decrement(void) {
    //   --(*this);
    // }

    // virtual void increment(typename base_type::difference_type i) {
    //   (*this) += i;
    // }

    // virtual void decrement(typename base_type::difference_type i) {
    //   (*this) -= i;
    // }

    // virtual typename base_type::difference_type distance(const base_type& other) {
    //   auto p_iter = reinterpret_cast<const MergeIterator<Iterator1,Iterator2>*>(&other);
    //   return std::distance(*this,*p_iter);
    // }
  };

  template<typename Iterator1, typename Iterator2> class Merge {
    Iterator1 begin1_, end1_;
    Iterator2 begin2_, end2_;
  public:
    typedef typename Iterator1::value_type value_type;
    typedef MergeIterator<Iterator1, Iterator2> iterator;

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
