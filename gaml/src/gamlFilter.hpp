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
#include <functional>

namespace gaml {
  
  template<typename Iterator>
  class FilterIterator {
  private:

    Iterator it;
    Iterator end;

  public:

    using difference_type = long;
    using value_type        = typename Iterator::value_type; 
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::input_iterator_tag;


  private:

    std::function<bool (const value_type&)> filter;

  public:
    FilterIterator(void) : it(), end(), filter() {}
    FilterIterator(const Iterator& iter, const Iterator& last, const std::function<bool (const value_type&)>& f) 
      : it(iter), end(last), filter(f) {
      for(; it != end && !filter(*it); ++it);
    }
    FilterIterator(const FilterIterator& cp) : it(cp.it), end(cp.end), filter(cp.filter) {}

    FilterIterator& operator=(const FilterIterator& cp)   {
      if(this != &cp) {
	it     = cp.it;
	end    = cp.end;
	filter = cp.filter;
      }
      return *this;
    }

    FilterIterator& operator++() {
      for(++it; it != end && !filter(*it); ++it);
      return *this;
    }

    FilterIterator  operator++(int) {
      FilterIterator res = *this;
      ++*this; 
      return res;
    }
    const value_type& operator*()                const {return *it;}
    bool     operator==(const FilterIterator& i) const {return it == i.it;}
    bool     operator!=(const FilterIterator& i) const {return it != i.it;}

  };

  template<typename Iterator> 
  class Filter {
  public:
    typedef typename Iterator::value_type value_type;
    typedef FilterIterator<Iterator>      iterator;

  private:
    
    Iterator _begin;
    Iterator _end;
    std::function<bool (const value_type&)> filter;
    std::function<bool (const value_type&)> _filter;
    bool keep;

  public:

    
    template<typename fctFilter>
    Filter(const Iterator& begin, 
	   const Iterator& end,
	   const fctFilter& fct_filter,
	   bool keep_true) : 
      _begin(begin), _end(end), filter(fct_filter), keep(keep_true) {
      auto f = filter;
      _filter = [f](const value_type& x) -> bool {return !f(x);};
    }
    
    iterator begin() const {
      if(keep)
	return FilterIterator<Iterator>(_begin, _end, filter);
      else
	return FilterIterator<Iterator>(_begin, _end, _filter);
    }
    iterator end() const {return FilterIterator<Iterator>(_end,   _end, filter);}
  };
  
  /**
   * Keep data for which the filter function returns true.
   */
  template<typename Iterator,typename fctFilter>
  Filter<Iterator> filter(const Iterator& begin, const Iterator& end, const fctFilter& fct_filter) {
    return Filter<Iterator>(begin,end,fct_filter,true);
  }

  /**
   * Keep data for which the filter function returns false.
   */
  template<typename Iterator,typename fctFilter>
  Filter<Iterator> reject(const Iterator& begin, const Iterator& end, const fctFilter& fct_filter) {
    return Filter<Iterator>(begin,end,fct_filter,false);
  }

}
