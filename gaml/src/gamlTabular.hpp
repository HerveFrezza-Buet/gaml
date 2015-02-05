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

#include <iterator>
#include <gamlVirtual.hpp>
#include <vector>

namespace gaml {

  template<typename Iterator, typename IndexIterator>
  class TabularIterator 
    : public gaml::virtualized::base_iterator<typename std::random_access_iterator_tag, 
					      typename std::iterator_traits<Iterator>::value_type> {
  private:

    Iterator it;
    mutable Iterator iit;
    IndexIterator idx;

  public:
    typedef typename std::iterator_traits<Iterator>::value_type      value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;

    TabularIterator(void) : it(), idx() {}
    TabularIterator(const Iterator& iter, const IndexIterator& index_iter) 
      : it(iter), idx(index_iter) {}
    TabularIterator(const TabularIterator& cp) : it(cp.it), idx(cp.idx) {}

    TabularIterator& operator=(const TabularIterator& cp)   {
      if(this != &cp) {
	it  = cp.it;
	idx = cp.idx;
      }
      return *this;
    }

    TabularIterator& operator++()                       {++idx; return *this;}
    TabularIterator& operator--()                       {--idx; return *this;}
    TabularIterator& operator+=(difference_type diff)   {idx+=diff; return *this;}
    TabularIterator& operator-=(int diff)               {idx-=diff; return *this;}

    TabularIterator  operator++(int) {
      TabularIterator res = *this;
      ++*this; 
      return res;
    }

    TabularIterator  operator--(int) {
      TabularIterator res = *this; 
      --*this; 
      return res;
    }

    difference_type  operator-(const TabularIterator& i) const {return idx - i.idx;}
    TabularIterator  operator+(difference_type i)        const {return TabularIterator(it,idx+i);}
    TabularIterator  operator-(difference_type i)        const {return TabularIterator(it,idx-i);}
    auto operator*(void) const -> decltype(*it) {
      iit = it;
      std::advance(iit,(difference_type)(*idx));
      return *iit;
    }
    bool     operator==(const TabularIterator& i)        const {return it == i.it && idx == i.idx;}
    bool     operator!=(const TabularIterator& i)        const {return it != i.it || idx != i.idx;}
   
    // Virtualization

    typedef gaml::virtualized::base_iterator<typename std::random_access_iterator_tag, 
					     typename std::iterator_traits<Iterator>::value_type> base_type;
    
    virtual base_type* clone() const {
      return new TabularIterator<Iterator,IndexIterator>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,IndexIterator>*>(&other);
      return (*this) == (*p_iter);
    }

    virtual void decrement(void) {
      --(*this);
    }

    virtual void increment(typename base_type::difference_type i) {
      (*this) += i;
    }

    virtual void decrement(typename base_type::difference_type i) {
      (*this) -= i;
    }

    virtual typename base_type::difference_type distance(const base_type& other) {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,IndexIterator>*>(&other);
      return std::distance(*this,*p_iter);
    }
  };


  template<typename Iterator> 
  class Tabular {
  public:

    typedef std::vector<unsigned int> indices_type;

    Iterator _begin;
    indices_type indices;


    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef TabularIterator<Iterator,typename indices_type::iterator> iterator;
    
    Tabular(const Iterator& begin_iter) : 
      _begin(begin_iter), indices() {
    }

    Tabular() : 
      _begin(), indices() {
    }
    
    Tabular(const Tabular& cp) : _begin(cp._begin), indices(cp.indices) {}

    const Tabular& operator=(const Tabular& cp) {
      if(&cp != this) {
	_begin  = cp._begin;
	indices = cp.indices;
      }
      return *this;
    }
    
    iterator begin() {return TabularIterator<Iterator,typename indices_type::iterator>(_begin, indices.begin()); }
    iterator end()   {return TabularIterator<Iterator,typename indices_type::iterator>(_begin, indices.end()); }
  };
}
