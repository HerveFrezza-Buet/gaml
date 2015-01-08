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
#include <functional>
#include <gamlVirtual.hpp>

namespace gaml {
  
  template<typename CATEGORY,typename Iterator, typename Function, typename Return>
  class MapIterator 
    : public gaml::virtualized::base_iterator<CATEGORY, Return> {
  private:

    Iterator it;
    Function f;
    mutable Return  content;

  public:
    typedef Return                                                   value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;

    MapIterator(void) : it(), f(), content() {}  
    MapIterator(const Iterator& iter, const Function& fun) : it(iter), f(fun), content() {}
    MapIterator(const MapIterator& cp) : it(cp.it), f(cp.f), content(cp.content) {}

    MapIterator& operator=(const MapIterator& cp)   {
      if(this != &cp) {
	it = cp.it;
	f  = cp.f;
	content = cp.content;
      }
      return *this;
    }

    MapIterator& operator++() {++it; return *this;}

    MapIterator  operator++(int) {
      MapIterator res = *this;
      ++*this; 
      return res;
    }
    const value_type& operator*()                     const {content = f(*it); return content;}

    bool     operator==(const MapIterator& i) const {return it == i.it;}
    bool     operator!=(const MapIterator& i) const {return it != i.it;}

    // Virtualization

    typedef gaml::virtualized::base_iterator<CATEGORY, Return> base_type;
    
    virtual base_type* clone() const {
      return new MapIterator<CATEGORY,Iterator,Function,Return>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const MapIterator<CATEGORY,Iterator,Function,Return>*>(&other);
      return (*this) == (*p_iter);
    }
  };




  
  template<typename Iterator, typename Function, typename Return>
  class MapIterator<std::random_access_iterator_tag,Iterator,Function,Return> 
    : public gaml::virtualized::base_iterator<std::random_access_iterator_tag, Return> {
  private:

    Iterator it;
    Function f;
    mutable Return  content;

  public:
    typedef Return                                                   value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;

    MapIterator(void) : it(), f(), content() {}  
    MapIterator(const Iterator& iter, const Function& fun) : it(iter), f(fun), content() {}
    MapIterator(const MapIterator& cp) : it(cp.it), f(cp.f), content(cp.content) {}

    MapIterator& operator=(const MapIterator& cp)   {
      if(this != &cp) {
	it = cp.it;
	f  = cp.f;
	content = cp.content;
      }
      return *this;
    }

    MapIterator& operator++()                       {++it; return *this;}
    MapIterator& operator--()                       {--it; return *this;}
    MapIterator& operator+=(difference_type diff)   {it+=diff; return *this;}
    MapIterator& operator-=(int diff)               {it-=diff; return *this;}

    MapIterator  operator++(int) {
      MapIterator res = *this;
      ++*this; 
      return res;
    }

    MapIterator  operator--(int) {
      MapIterator res = *this; 
      --*this; 
      return res;
    }

    difference_type   operator-(const MapIterator& i) const {return it - i.it;}
    MapIterator       operator+(difference_type i)    const {return MapIterator(it+i,f);}
    MapIterator       operator-(difference_type i)    const {return MapIterator(it-i,f);}
    const value_type& operator*()                     const {content = f(*it); return content;}
    bool     operator==(const MapIterator& i) const {return it == i.it;}
    bool     operator!=(const MapIterator& i) const {return it != i.it;}

    // Virtualization

    typedef gaml::virtualized::base_iterator<std::random_access_iterator_tag, Return> base_type;
    
    virtual base_type* clone() const {
      return new MapIterator<std::random_access_iterator_tag,Iterator,Function,Return>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const MapIterator<std::random_access_iterator_tag,Iterator,Function,Return>*>(&other);
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
      auto p_iter = reinterpret_cast<const MapIterator<std::random_access_iterator_tag,Iterator,Function,Return>*>(&other);
      return std::distance(*this,*p_iter);
    }
  };









  template<typename Iterator, typename Function> 
  class Map {
  private:
    Iterator _begin, _end;
    Function* dummy_f; 
  public:
    typedef typename std::iterator_traits<Iterator>::value_type  arg_type;
    typedef decltype((*dummy_f)(*_begin))                        value_type;
    typedef std::function<value_type (const arg_type&)>          function_type;
    typedef MapIterator<typename std::iterator_traits<Iterator>::iterator_category,
			Iterator, function_type, value_type>     iterator;

  private:
    function_type f;

  public:
    

    Map(const Iterator& begin_iter, 
	const Iterator& end_iter, 
	const Function& fun) : 
      _begin(begin_iter), _end(end_iter), f(fun) {}
    
    iterator begin() const {return iterator(_begin, f); }
    iterator end() const   {return iterator(_end,   f); }
  };
  
  template<typename Iterator, typename Function>
  Map<Iterator,Function> map(const Iterator& begin, const Iterator& end, 
			     const Function& fun) {
    return Map<Iterator,Function>(begin,end,fun);
  }
}
