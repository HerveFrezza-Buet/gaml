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
#include <memory>
#include <vector>
#include <stdexcept>
#include <gamlIterator.hpp>
#include <sstream>

//#define DEBUG_VIRTUALIZED

namespace gaml {
  namespace virtualized {

    
    /**
     * This class is a base class for iterators that can be virtualized in gaml.
     */
    template<typename VALUE>
    class base_iterator : public gaml::is_virtual {
    public:
      typedef VALUE value_type;

      base_iterator() {}
      base_iterator(const base_iterator& cp) {}
      virtual ~base_iterator() {}

      virtual base_iterator* clone()                    const       = 0;
      virtual const VALUE& get()                        const       = 0;
      virtual bool is_equal(const base_iterator& other) const       = 0;
      virtual void increment(void)                                  = 0;

      virtual bool is_a_tabular_iterator() const {
	return false;
      }

      virtual const VALUE& get_ith_from_root(int i) const {
	std::ostringstream ostr;
	ostr << "gaml::virtualized::iterator : get_ith_from_root(" << i << ") : default method should never be called (sub class has to override it).";
	throw std::runtime_error(ostr.str());
	return get(); // This silly return is never called....
      }

      virtual base_iterator* root_clone() const {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  base::root_clone()" << std::endl;
#endif
	return clone();
      }

      virtual typename std::vector<unsigned int>::iterator idx_iter() const {
	throw std::runtime_error("gaml::virtualized::iterator : idx_iter() : default method should never be called (sub class has to override it).");
	return std::vector<unsigned int>::iterator();
      }

      virtual void increment(int i) {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  base::increment(" << i << ")" << std::endl;
#endif
	for(int j=0; j<i;++j) increment();
      }

      virtual void decrement(void) {
	throw std::runtime_error("gaml::virtualized::base_iterator : decrement : not implemented");
      }

      virtual void decrement(int i) {
	for(int j=0; j<i;++j) decrement();
      }

      virtual int distance(const base_iterator& other) {
	int d = 0;
	for(base_iterator* it = this->clone();
	    !(it->is_equal(other));
	    it->increment(),++d);
	return d;
      }
    };



    /**
     * For internal use
     */
    template<typename VALUE>
    class tabular_base_iterator 
      : public gaml::virtualized::base_iterator<VALUE>,
	public gaml::is_tabular {
    public:
      typedef gaml::virtualized::base_iterator<VALUE> super_type; 
      typedef VALUE value_type;
      
      virtual bool is_a_tabular_iterator() const override {
	return true;
      }
    };



    template<typename VALUE>
    class iterator: 
      public std::iterator<std::random_access_iterator_tag,
			   VALUE>,
      public base_iterator<VALUE> {
    public:
      
      typedef base_iterator<VALUE>                                      base_type;
      typedef VALUE                                                     value_type;
      typedef typename std::iterator<std::random_access_iterator_tag,
				     VALUE>::difference_type            difference_type;

    private:
      
      std::unique_ptr<base_type> it_ptr;
      
    public:
      
      iterator() : base_type(), it_ptr() {}
      iterator(const base_type& iter) : base_type(), it_ptr(iter.clone()) {}
      iterator(const iterator& cp) : base_type(), it_ptr() {
	if(cp.it_ptr != nullptr) {
	  std::unique_ptr<base_type> other(cp.it_ptr->clone());
	  it_ptr = std::move(other);
	}
      }
      virtual ~iterator() {}

      virtual bool is_a_tabular_iterator() const override {
	return it_ptr->is_a_tabular_iterator();
      }

      iterator& operator--()   {
	it_ptr->decrement();
	return *this;
      }
      
      iterator& operator+=(difference_type diff)   {
	it_ptr->increment((int)diff);
	return *this;
      }
      
      iterator& operator-=(difference_type diff)   {
	it_ptr->decrement((int)diff);
	return *this;
      }
      
      iterator  operator--(int) {
	iterator res = *this;
	--*this; 
	return res;
      }
      
      difference_type  operator-(const iterator& i) const {
	if(it_ptr == nullptr || i.it_ptr == nullptr)
	  throw std::runtime_error("gaml::virtualized::iterator : operator-(iter) : null iterator");
	return (difference_type)(i.it_ptr->distance(*it_ptr));
      }
      
      iterator  operator+(difference_type i) const {
	if(it_ptr == nullptr)
	  throw std::runtime_error("gaml::virtualized::iterator : operator+(diff) : null iterator");
	iterator res(*this);
	res.it_ptr->increment((int)i);
	return res;
      }

      iterator operator-(difference_type i) const {
	if(it_ptr == nullptr)
	  throw std::runtime_error("gaml::virtualized::iterator : operator-(diff) : null iterator");
	iterator res(*this);
	res.it_ptr->decrement((int)i);
	return res;
      }

      iterator& operator=(const iterator& cp) {
	if(&cp != this) {
	  if(cp.it_ptr == nullptr)
	    it_ptr = nullptr;
	  else {
	    std::unique_ptr<base_type> other(cp.it_ptr->clone());
	    it_ptr = std::move(other);
	  }
	}
	return *this;
      }

      iterator& operator++() {
	it_ptr->increment();
	return *this;
      }

      iterator  operator++(int) {
	iterator res = *this;
	++*this; 
	return res;
      }

      const value_type& operator*() const {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  seq::*" << std::endl;
#endif
	return it_ptr->get();
      }

      bool operator==(const iterator& other) const {
	if(it_ptr == nullptr) 
	  if(other.it_ptr == nullptr)
	    return true;
	  else
	    return false;
	else
	  if(other.it_ptr == nullptr)
	    return false;
	  else
	    return (*it_ptr).is_equal(*(other.it_ptr));
      }

      bool operator!=(const iterator& other) const {
	return !(*this == other);
      }

      virtual base_type* clone() const override {
	return new iterator(*this);
      }

      virtual base_type* root_clone() const override {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  seq::root_clone()" << std::endl;
#endif
	return it_ptr->root_clone();
      }

      virtual const value_type& get_ith_from_root(int i) const override {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  seq::get_ith_from_root(" << i << ")" << std::endl;
#endif
	return it_ptr->get_ith_from_root(i);
      }

      virtual typename std::vector<unsigned int>::iterator idx_iter() const override {
	return it_ptr->idx_iter();
      }

      virtual void increment(void) override {
	++*this; 
      }

      virtual const VALUE& get() const override {
	return *(*this);
      };

      virtual bool is_equal(const base_type& other) const override {
	return *this == other;
      }

      virtual void decrement(void) override {
	--(*this);
      }

      virtual void increment(int i) override {
#ifdef DEBUG_VIRTUALIZED
	std::cout << "### (" << this << ")  seq::increment(" << i << ")" << std::endl;
#endif
	(*this) += i;
      }

      virtual void decrement(int i) override {
	(*this) -= i;
      }

      virtual int distance(const base_type& other) override {
	auto p_iter = reinterpret_cast<const iterator*>(&other);
	return (int)(std::distance(*this,*p_iter));
      }
    };

    template<typename VALUE>
    class Sequence {
    public:
      typedef VALUE                                 value_type;
      typedef virtualized::iterator<VALUE>          iterator;
      typedef typename iterator::base_type          base_type;

    private:

      iterator _begin;
      iterator _end;

    public:

      Sequence(const base_type& begin, 
	       const base_type& end) 
	: _begin(begin), _end(end) {}

      Sequence(const Sequence& copy) : _begin(copy._begin), _end(copy.end()) {}
      
      const iterator& begin() const {return _begin;}
      const iterator& end()   const {return _end;}
    };

    template<typename Iterator>
    Sequence<typename std::iterator_traits<Iterator>::value_type> 
    sequence(const Iterator& begin, const Iterator& end) {
      return Sequence<typename std::iterator_traits<Iterator>::value_type>(begin,end);
    }
  }
}
