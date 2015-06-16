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
#include <type_traits>
#include <memory>
#include <gamlIterator.hpp>


namespace gaml {

  /*
    TabularIterator for gaml::regular_iterator_tag.
  */
  template<typename Iterator, typename REGULAR_ITERATOR_TAG>
  class TabularIterator 
    : public std::iterator<typename std::iterator_traits<Iterator>::iterator_category,
			   typename std::iterator_traits<Iterator>::value_type>,
      public gaml::virtualized::tabular_base_iterator<typename std::iterator_traits<Iterator>::value_type> {
  private:

    mutable Iterator iit;

  public:

    typedef typename std::iterator_traits<Iterator>::value_type      value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    typedef Iterator                                                 original_iterator_type;
    typedef typename std::vector<unsigned int>::iterator             index_iterator;

    Iterator it;
    index_iterator idx;

    TabularIterator(void) : it(), idx() {}
    TabularIterator(const Iterator& iter, const index_iterator& index_iter) 
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

    virtual const value_type& get_ith_from_root(int i) const override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<regular>::get_ith_from_root(" << i << ")" << std::endl;
#endif
      iit = it;
      std::advance(iit,(difference_type)i);
      return *iit;
    }

    const value_type& operator*(void) const {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<regular>::* : at " << *idx << std::endl;
#endif
      iit = it;
      std::advance(iit,(difference_type)(*idx));
      return *iit;
    }
    bool     operator==(const TabularIterator& i)        const {return it == i.it && idx == i.idx;}
    bool     operator!=(const TabularIterator& i)        const {return it != i.it || idx != i.idx;}
   
    // Virtualization

    typedef gaml::virtualized::tabular_base_iterator<value_type> base_type;
    
    virtual typename base_type::super_type* clone() const override {
      return new TabularIterator<Iterator,REGULAR_ITERATOR_TAG>(*this);
    }
    
    virtual typename std::vector<unsigned int>::iterator idx_iter() const override {
      return idx;
    }

    // The default one is ok.
    // virtual typename base_type::super_type* root_clone() const override {
    //   return this->clone();
    // }

    virtual void increment(void) override {
      ++(*this);
    }

    virtual const value_type& get() const override {
      return *(*this);
    }

    virtual bool is_equal(const typename base_type::super_type& other) const override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,REGULAR_ITERATOR_TAG>*>(&other);
      return (*this) == (*p_iter);
    }

    virtual void decrement(void) override {
      --(*this);
    }

    virtual void increment(int i) override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<regular>::incr(" << i << ")" << std::endl;
#endif
      (*this) += (difference_type)i;
    }

    virtual void decrement(int i) override {
      (*this) -= (difference_type)i;
    }

    virtual int distance(const typename base_type::super_type& other) override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,REGULAR_ITERATOR_TAG>*>(&other);
      return (int)(std::distance(*this,*p_iter));
    }
  };


  /*
    TabularIterator for gaml::tabular_iterator_tag.
  */
  template<typename Iterator>
  class TabularIterator<Iterator,gaml::tabular_iterator_tag>
    : public std::iterator<typename std::iterator_traits<Iterator>::iterator_category,
			   typename std::iterator_traits<Iterator>::value_type>,
      public gaml::virtualized::tabular_base_iterator<typename std::iterator_traits<Iterator>::value_type> {
  public:

    typedef typename Iterator::original_iterator_type original_iterator_type;

  private:

    mutable original_iterator_type iit;

  public:

    typedef typename std::iterator_traits<original_iterator_type>::value_type      value_type;
    typedef typename std::iterator_traits<original_iterator_type>::difference_type difference_type;
    typedef typename std::vector<unsigned int>::iterator                           index_iterator;

    original_iterator_type it;
    index_iterator idx;


    TabularIterator(void) : it(), idx() {}
    TabularIterator(const Iterator& iter, const index_iterator& index_iter) 
      : it(iter.it), idx(index_iter) {}
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

    virtual const value_type& get_ith_from_root(int i) const override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<tabular>::get_ith_from_root(" << i << ")" << std::endl;
#endif
      iit = it;
      std::advance(iit,(difference_type)i);
      return *iit;
    }

    const value_type& operator*(void) const {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<tabular>::* : at " << *idx << std::endl;
#endif
      iit = it;
      std::advance(iit,(difference_type)(*idx));
      return *iit;
    }
    bool     operator==(const TabularIterator& i)        const {return it == i.it && idx == i.idx;}
    bool     operator!=(const TabularIterator& i)        const {return it != i.it || idx != i.idx;}
   
    // Virtualization

    typedef gaml::virtualized::tabular_base_iterator<value_type> base_type;
    
    virtual typename base_type::super_type* clone() const override {
      return new TabularIterator<Iterator,gaml::tabular_iterator_tag>(*this);
    }
    
    virtual typename std::vector<unsigned int>::iterator idx_iter() const override {
      return idx;
    }

    virtual void increment(void) override {
      ++(*this);
    }

    virtual const value_type& get() const override {
      return *(*this);
    }

    virtual bool is_equal(const typename base_type::super_type& other) const override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,gaml::tabular_iterator_tag>*>(&other);
      return (*this) == (*p_iter);
    }

    virtual void decrement(void) override {
      --(*this);
    }

    virtual void increment(int i) override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<tabular>::incr(" << i << ")" << std::endl;
#endif
      (*this) += (difference_type)i;
    }

    virtual void decrement(int i) override {
      (*this) -= (difference_type)i;
    }

    virtual int distance(const typename base_type::super_type& other) override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,gaml::tabular_iterator_tag>*>(&other);
      return (int)(std::distance(*this,*p_iter));
    }
  };


  /*
    TabularIterator for gaml::virtualized_iterator_tag.
  */
  template<typename Iterator>
  class TabularIterator<Iterator,gaml::virtualized_iterator_tag>
    : public std::iterator<typename std::iterator_traits<Iterator>::iterator_category,
			   typename std::iterator_traits<Iterator>::value_type>,
      public gaml::virtualized::base_iterator<typename std::iterator_traits<Iterator>::value_type> {
  public:

    typedef typename std::iterator_traits<Iterator>::value_type      value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    typedef typename gaml::virtualized::base_iterator<value_type>    base_iterator_type;
    typedef typename std::vector<unsigned int>::iterator             index_iterator;


  public:

    std::unique_ptr<base_iterator_type> root_it_ptr;
    index_iterator idx;

    TabularIterator(void) : root_it_ptr(), idx() {}
    TabularIterator(base_iterator_type* iter_clone, const index_iterator& index_iter) 
      : root_it_ptr(iter_clone), idx(index_iter) {
    }
    TabularIterator(const TabularIterator& cp) : root_it_ptr(), idx(cp.idx) {
      if(cp.root_it_ptr != nullptr) {
	std::unique_ptr<base_iterator_type> other(cp.root_it_ptr->clone());
	root_it_ptr = std::move(other);
      }
    }

    TabularIterator& operator=(const TabularIterator& cp)   {
      if(this != &cp) {
	idx = cp.idx;
	if(cp.root_it_ptr == nullptr)
	  root_it_ptr = nullptr;
	else {
	  std::unique_ptr<base_iterator_type> other(cp.root_it_ptr->clone());
	  root_it_ptr = std::move(other);
	}
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
    TabularIterator  operator+(difference_type i)        const {return TabularIterator(root_it_ptr->clone(),idx+i);}
    TabularIterator  operator-(difference_type i)        const {return TabularIterator(root_it_ptr->clone(),idx-i);}

    virtual const value_type& get_ith_from_root(int i) const override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<virtual>::get_ith_from_root(" << i << ")" << std::endl;
#endif
      if(root_it_ptr->is_a_tabular_iterator())
	return root_it_ptr->get_ith_from_root(i);
      else {
	std::unique_ptr<base_iterator_type> iit(root_it_ptr->clone());
	iit->increment(i);
	return iit->get();
      }
    }

    const value_type& operator*(void) const {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<virtual>::* : at " << *idx << std::endl;
#endif
      if(root_it_ptr->is_a_tabular_iterator())
	return root_it_ptr->get_ith_from_root(*idx);
      else {
	std::unique_ptr<base_iterator_type> iit(root_it_ptr->clone());
	iit->increment(*idx);
	return iit->get();
      }
    }


    /*
      == only compares indices. The Tabular::end() design depend on this (see the comment).
    */
    bool     operator==(const TabularIterator& i)        const {return /* it == i.it && */ idx == i.idx;} 
    bool     operator!=(const TabularIterator& i)        const {return /* it != i.it || */ idx != i.idx;}
   
    // Virtualization

    typedef gaml::virtualized::tabular_base_iterator<value_type> base_type;
    
    virtual typename base_type::super_type* clone() const override {
      return new TabularIterator<Iterator,gaml::virtualized_iterator_tag>(*this);
    }

    virtual typename std::vector<unsigned int>::iterator idx_iter() const override {
      return idx;
    }

    virtual typename base_type::super_type* root_clone() const override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<virtual>::root_clone()" << std::endl;
#endif
      return root_it_ptr->clone();
    }

    virtual void increment(void) override {
      ++(*this);
    }

    virtual const value_type& get() const override {
      return *(*this);
    }

    virtual bool is_equal(const typename base_type::super_type& other) const override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,gaml::virtualized_iterator_tag>*>(&other);
      return (*this) == (*p_iter);
    }

    virtual void decrement(void) override {
      --(*this);
    }

    virtual void increment(int i) override {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "### (" << this << ")  tabular<virtual>::incr(" << i << ")" << std::endl;
#endif
      (*this) += (difference_type)i;
    }

    virtual void decrement(int i) override {
      (*this) -= (difference_type)i;
    }

    virtual int distance(const typename base_type::super_type& other) override {
      auto p_iter = reinterpret_cast<const TabularIterator<Iterator,gaml::virtualized_iterator_tag>*>(&other);
      return (int)(std::distance(*this,*p_iter));
    }
  };

  /*
    Tabular for gaml::regular_iterator_tag.
  */
  template<typename Iterator, 
	   typename REGULAR_ITERATOR_TAG> 
  class Tabular {
  public:

    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef TabularIterator<Iterator,REGULAR_ITERATOR_TAG>      iterator;

    typedef std::vector<unsigned int> indices_type;

    Iterator _begin;
    indices_type indices;


    
    Tabular(const Iterator& begin_iter) : 
      _begin(begin_iter), indices() {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "Tabular<gaml::regular_iterator_tag>" << std::endl;
#endif
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

    void process_indices() {}
    
    iterator begin() {return TabularIterator<Iterator,REGULAR_ITERATOR_TAG>(_begin, indices.begin()); }
    iterator end()   {return TabularIterator<Iterator,REGULAR_ITERATOR_TAG>(_begin, indices.end()); }
  };


  /*
    Tabular for gaml::tabular_iterator_tag.
  */
  template<typename Iterator> 
  class Tabular<Iterator,gaml::tabular_iterator_tag> {
  public:

    typedef std::vector<unsigned int> indices_type;

    Iterator _begin;
    indices_type indices;


    typedef typename std::iterator_traits<Iterator>::value_type value_type;
    typedef TabularIterator<Iterator,gaml::tabular_iterator_tag> iterator;
    
    Tabular(const Iterator& begin_iter) : 
      _begin(begin_iter), indices() {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "Tabular<gaml::tabular_iterator_tag>" << std::endl;
#endif
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

    void process_indices() {
      for(auto& idx : indices) idx = *(_begin.idx + idx);
    }
    
    iterator begin() {return TabularIterator<Iterator,gaml::tabular_iterator_tag>(_begin, indices.begin()); }
    iterator end()   {return TabularIterator<Iterator,gaml::tabular_iterator_tag>(_begin, indices.end()); }
  };




  /*
    Tabular for gaml::virtualized_iterator_tag.
  */
  template<typename Iterator> 
  class Tabular<Iterator,gaml::virtualized_iterator_tag> {
  public:

    typedef std::vector<unsigned int> indices_type;

    typedef typename std::iterator_traits<Iterator>::value_type       value_type;
    typedef TabularIterator<Iterator,gaml::virtualized_iterator_tag>  iterator;
    typedef typename gaml::virtualized::base_iterator<value_type>     base_iterator_type;
    
    std::unique_ptr<base_iterator_type> _begin_ptr;
    indices_type indices;



    Tabular(const Iterator& begin_iter) : 
      _begin_ptr(begin_iter.clone()), indices() {
#ifdef DEBUG_VIRTUALIZED
      std::cout << "Tabular<gaml::virtualized_iterator_tag>" << std::endl;
#endif
    }

    Tabular() : 
      _begin_ptr(), indices() {
    }
    
    Tabular(const Tabular& cp) : _begin_ptr(), indices(cp.indices) {
      if(cp._begin_ptr != nullptr) {
	std::unique_ptr<base_iterator_type> other(cp._begin_ptr->clone());
	_begin_ptr = std::move(other);
      }
    }

    const Tabular& operator=(const Tabular& cp) {
      if(&cp != this) {
	indices = cp.indices;
	if(cp._begin_ptr == nullptr)
	  _begin_ptr = nullptr;
	else {
	  std::unique_ptr<base_iterator_type> other(cp._begin_ptr->clone());
	  _begin_ptr = std::move(other);
	}
      }
      return *this;
    }

    void process_indices() {
      if(_begin_ptr->is_a_tabular_iterator()) {
	auto tab_idx = _begin_ptr->idx_iter();
	for(auto& idx : indices) idx = *(tab_idx + idx);
      }
    }
    
    iterator begin() {return TabularIterator<Iterator,gaml::virtualized_iterator_tag>(_begin_ptr->root_clone(), indices.begin()); }

    /* a clone (as for begin) would be useless since end is only used with ==, and == only compares indices. */
    iterator end()   {return TabularIterator<Iterator,gaml::virtualized_iterator_tag>(nullptr, indices.end()); }
  };
}
