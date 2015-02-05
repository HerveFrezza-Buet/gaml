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
#include <stdexcept>

namespace gaml {
  namespace virtualized {

    /**
     * This class is a base class for iterators that can be virtualized in gaml.
     */
    template<typename CATEGORY,typename VALUE>
    class base_iterator : public std::iterator<CATEGORY, VALUE> {
    public:
      typedef typename std::iterator<CATEGORY, VALUE>::difference_type difference_type;

      base_iterator() {}
      base_iterator(const base_iterator& cp) {}
      virtual ~base_iterator() {}

      virtual base_iterator* clone()                    const       = 0;
      virtual const VALUE& get()                        const       = 0;
      virtual bool is_equal(const base_iterator& other) const       = 0;
      virtual void increment(void)                                  = 0;
    };


    /**
     * This class is a base class for iterators that can be virtualized in gaml.
     */
    template<typename VALUE>
    class base_iterator<std::random_access_iterator_tag,VALUE> : public std::iterator<std::random_access_iterator_tag, VALUE> {
    public:
      typedef typename std::iterator<std::random_access_iterator_tag, VALUE>::difference_type difference_type;

      base_iterator() {}
      base_iterator(const base_iterator& cp) {}
      virtual ~base_iterator() {}

      virtual base_iterator* clone()                    const       = 0;
      virtual const VALUE& get()                        const       = 0;
      virtual bool is_equal(const base_iterator& other) const       = 0;
      virtual void increment(void)                                  = 0;

      virtual void decrement(void)                                  = 0;
      virtual void increment(difference_type i)                     = 0;
      virtual void decrement(difference_type i)                     = 0;
      virtual difference_type  distance(const base_iterator& other) = 0;
    };


    

    template<typename CATEGORY, typename VALUE>
    class iterator : public base_iterator<CATEGORY, VALUE> {
    public:
      
      typedef base_iterator<CATEGORY, VALUE>                           base_type;
      typedef base_iterator<CATEGORY, VALUE>                           iterator_type;
      typedef VALUE                                                    value_type;
      typedef typename std::iterator<CATEGORY, VALUE>::difference_type difference_type;

    private:
      
      std::unique_ptr<iterator_type> it_ptr;
      
    public:
      
      iterator() : base_type(), it_ptr() {}
      iterator(const iterator_type& iter) : base_type(), it_ptr(iter.clone()) {}
      iterator(const iterator& cp) : base_type(), it_ptr() {
	if(cp.it_ptr != nullptr) {
	  std::unique_ptr<iterator_type> other(cp.it_ptr->clone());
	  it_ptr = std::move(other);
	}
      }
      virtual ~iterator() {}

      
      iterator& operator=(const iterator& cp) {
	if(&cp != this) {
	  if(cp.it_ptr == nullptr)
	    it_ptr = nullptr;
	  else {
	    std::unique_ptr<iterator_type> other(cp.it_ptr->clone());
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


      virtual base_type* clone() const {
	return new iterator(*this);
      }

      virtual void increment(void) {
	++*this; 
      }

      virtual const VALUE& get() const {
	return *(*this);
      };

      virtual bool is_equal(const base_type& other) const {
	return *this == other;
      }
    };
    





    template<typename VALUE>
    class iterator<std::random_access_iterator_tag,VALUE> : public base_iterator<std::random_access_iterator_tag, VALUE> {
    public:
      
      typedef base_iterator<std::random_access_iterator_tag, VALUE>                           base_type;
      typedef base_iterator<std::random_access_iterator_tag, VALUE>                           iterator_type;
      typedef VALUE                                                    value_type;
      typedef typename std::iterator<std::random_access_iterator_tag, VALUE>::difference_type difference_type;

    private:
      
      std::unique_ptr<iterator_type> it_ptr;
      
    public:
      
      iterator() : base_type(), it_ptr() {}
      iterator(const iterator_type& iter) : base_type(), it_ptr(iter.clone()) {}
      iterator(const iterator& cp) : base_type(), it_ptr() {
	if(cp.it_ptr != nullptr) {
	  std::unique_ptr<iterator_type> other(cp.it_ptr->clone());
	  it_ptr = std::move(other);
	}
      }
      virtual ~iterator() {}


      iterator& operator--()   {
	it_ptr->decrement();
	return *this;
      }
      
      iterator& operator+=(difference_type diff)   {
	it_ptr->increment(diff);
	return *this;
      }
      
      iterator& operator-=(difference_type diff)   {
	it_ptr->decrement(diff);
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
	return i.it_ptr->distance(*it_ptr);
      }
      
      iterator  operator+(difference_type i) const {
	if(it_ptr == nullptr)
	  throw std::runtime_error("gaml::virtualized::iterator : operator+(diff) : null iterator");
	iterator res(*this);
	res.it_ptr->increment(i);
	return res;
      }

      iterator operator-(difference_type i) const {
	if(it_ptr == nullptr)
	  throw std::runtime_error("gaml::virtualized::iterator : operator-(diff) : null iterator");
	iterator res(*this);
	res.it_ptr->decrement(i);
	return res;
      }

      iterator& operator=(const iterator& cp) {
	if(&cp != this) {
	  if(cp.it_ptr == nullptr)
	    it_ptr = nullptr;
	  else {
	    std::unique_ptr<iterator_type> other(cp.it_ptr->clone());
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


      virtual base_type* clone() const {
	return new iterator(*this);
      }

      virtual void increment(void) {
	++*this; 
      }

      virtual const VALUE& get() const {
	return *(*this);
      };

      virtual bool is_equal(const base_type& other) const {
	return *this == other;
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
	auto p_iter = reinterpret_cast<const iterator*>(&other);
	return std::distance(*this,*p_iter);
      }
    };

    template<typename CATEGORY,typename VALUE>
    class Sequence {
    public:
      typedef VALUE                                 value_type;
      typedef virtualized::iterator<CATEGORY,VALUE> iterator;
      typedef base_iterator<CATEGORY, VALUE>        base_type;

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
    Sequence<typename std::iterator_traits<Iterator>::iterator_category,
	     typename std::iterator_traits<Iterator>::value_type> 
      sequence(const Iterator& begin, const Iterator& end) {
      return Sequence<typename std::iterator_traits<Iterator>::iterator_category,
		      typename std::iterator_traits<Iterator>::value_type>(begin,end);
    }
  }
}
