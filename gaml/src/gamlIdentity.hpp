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



  template<typename CATEGORY,typename Iterator>
  class IdentityIterator 
    : public gaml::virtualized::base_iterator<CATEGORY, 
					      typename std::iterator_traits<Iterator>::value_type> {	
  private:
    
    Iterator it;

  public:

    typedef typename std::iterator_traits<Iterator>::value_type     value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    
    IdentityIterator(void) : it() {}  
    IdentityIterator(const Iterator& iter) : it(iter){}
    IdentityIterator(const IdentityIterator& cp) : it(cp.it) {}

    IdentityIterator& operator=(const IdentityIterator& cp)   {
      if(this != &cp)
	it = cp.it;
      return *this;
    }

    IdentityIterator& operator++()  {++it; return *this;}
    IdentityIterator  operator++(int) {
      IdentityIterator res = *this;
      ++*this; 
      return res;
    }
    const value_type& operator*()                          const {return *it;}
    bool     operator==(const IdentityIterator& i)         const {return it == i.it;}
    bool     operator!=(const IdentityIterator& i)         const {return it != i.it;}

    // Virtualization

    typedef gaml::virtualized::base_iterator<CATEGORY, 
					     typename std::iterator_traits<Iterator>::value_type> base_type;
    
    virtual base_type* clone() const {
      return new IdentityIterator<CATEGORY,Iterator>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const IdentityIterator<CATEGORY,Iterator>*>(&other);
      return (*this) == (*p_iter);
    }
  };



  template<typename Iterator>
  class IdentityIterator<std::random_access_iterator_tag,Iterator> 
    : public gaml::virtualized::base_iterator<std::random_access_iterator_tag, 
					      typename std::iterator_traits<Iterator>::value_type> {	
  private:
    
    Iterator it;

  public:

    typedef typename std::iterator_traits<Iterator>::value_type     value_type;
    typedef typename std::iterator_traits<Iterator>::difference_type difference_type;
    
    IdentityIterator(void) : it() {}  
    IdentityIterator(const Iterator& iter) : it(iter){}
    IdentityIterator(const IdentityIterator& cp) : it(cp.it) {}

    IdentityIterator& operator=(const IdentityIterator& cp)   {
      if(this != &cp)
	it = cp.it;
      return *this;
    }

    IdentityIterator& operator++()                       {++it; return *this;}
    IdentityIterator& operator--()                       {--it; return *this;}
    IdentityIterator& operator+=(difference_type diff)   {it+=diff; return *this;}
    IdentityIterator& operator-=(int diff)               {it-=diff; return *this;}

    IdentityIterator  operator++(int) {
      IdentityIterator res = *this;
      ++*this; 
      return res;
    }

    IdentityIterator  operator--(int) {
      IdentityIterator res = *this; 
      --*this; 
      return res;
    }

    difference_type   operator-(const IdentityIterator& i) const {return it - i.it;}
    IdentityIterator       operator+(difference_type i)    const {return IdentityIterator(it+i);}
    IdentityIterator       operator-(difference_type i)    const {return IdentityIterator(it-i);}
    const value_type& operator*()                          const {return *it;}
    bool     operator==(const IdentityIterator& i)         const {return it == i.it;}
    bool     operator!=(const IdentityIterator& i)         const {return it != i.it;}

    // Virtualization

    typedef gaml::virtualized::base_iterator<typename std::iterator_traits<Iterator>::iterator_category, 
					     typename std::iterator_traits<Iterator>::value_type> base_type;
    
    virtual base_type* clone() const {
      return new IdentityIterator<std::random_access_iterator_tag,Iterator>(*this);
    }

    virtual void increment(void) {
      ++(*this);
    }

    virtual const value_type& get() const {
      return *(*this);
    }

    virtual bool is_equal(const base_type& other) const {
      auto p_iter = reinterpret_cast<const IdentityIterator<std::random_access_iterator_tag,Iterator>*>(&other);
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
      auto p_iter = reinterpret_cast<const IdentityIterator<std::random_access_iterator_tag,Iterator>*>(&other);
      return std::distance(*this,*p_iter);
    }
  };





  template<typename Iterator> 
  class Identity {
  private:
    Iterator _begin, _end;
  public:
    typedef IdentityIterator<typename std::iterator_traits<Iterator>::iterator_category,Iterator>  iterator;
    typedef typename iterator::value_type                                                          value_type;


  public:
    

    Identity(const Iterator& begin_iter, 
	     const Iterator& end_iter) : 
      _begin(begin_iter), _end(end_iter) {}
    
    iterator begin() const {return iterator(_begin); }
    iterator end() const   {return iterator(_end); }
  };
  
  template<typename Iterator>
  Identity<Iterator> identity(const Iterator& begin, const Iterator& end) {
    return Identity<Iterator>(begin,end);
  }
}

