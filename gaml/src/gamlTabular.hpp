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
#include <vector>


namespace gaml {

  typedef unsigned int tabular_index_type;
  
  namespace concept {

    class SecondaryIterator {
    public:
      /**
       * This is the type of the iterator for the primary collection.
       */
      typedef int primary_type;

      /**
       * @return the index in the primary collection
       */
      tabular_index_type index() const;

      /**
       * @return the begin iterator in the primary collection.
       */
      primary_type origin() const;
      
    };
    
  }


  /**
   * Tabular iterator. It fits concept::SecondaryIterator.
   */
  template<Iterator>
  class TabularIterator : public std::iterator<std::random_access_iterator_tag,
					       typename Iterator::value_type> {
  private:
    Iterator                                  begin;
    std::vector<tabular_index_type>::iterator idx;

    friend class Tabular<Iterator>;
    
    TabularIterator(const Iterator& begin, const std::vector<tabular_index_type>::iterator& idx)
      : begin(begin), idx(idx) {}

  public:

    typedef Iterator primary_type;
    
    TabularIterator()                                   = default;
    TabularIterator(const TabularIterator&)             = default;
    TabularIterator& operator=(const TabularIterator&)  = default;
    TabularIterator(TabularIterator&&)                  = default;
    TabularIterator& operator=(const TabularIterator&&) = default;
    
    tabular_index_type index()  const {return idx;}
    primary_type       origin() const {return begin;};
    
    TabularIterator<Iterator>& operator++()         {++idx;       return *this;}
    TabularIterator<Iterator>& operator--()         {--idx;       return *this;}
    TabularIterator<Iterator>& operator+=(int diff) {idx += diff; return *this;}
    TabularIterator<Iterator>& operator-=(int diff) {idx -= diff; return *this;}
    
    TabularIterator<Iterator> operator++(int) {TabularIterator<Iterator> res = *this; ++*this; return res;}
    TabularIterator<Iterator> operator--(int) {TabularIterator<Iterator> res = *this; --*this; return res;}
    
    int operator-(const TabularIterator<Iterator>& i) const {return idx - i.idx;}
    TabularIterator<Iterator> operator+(int i)        const {return TabularIterator<Iterator>(begin,idx+i);}
    TabularIterator<Iterator> operator-(int i)        const {return TabularIterator<Iterator>(begin,idx-i);}
    
    bool operator==(const TabularIterator<Iterator>& i) const {return idx == i.idx && begin == it.begin;}
    bool operator!=(const TabularIterator<Iterator>& i) const {return idx != i.idx || begin != it.begin;}
    
    const Iterator::value_type& operator*() const {
      auto itt = begin;
      std::advance(iit,(std::iterator_traits<primary_type>::distance_type)(*idx));
      return *itt;
    }
  };
  
  /**
   * Class for tabular acces to data. SFINAE here. Secondary Iterator must fit concept::SecondaryIterator.
   */
  template<typename SecondaryIterator> 
  class Tabular {
  private:
    
    typename SecondaryIterator::primary_type begin;
    std::vector<tabular_index_type> indices;    // indices in the primary collection

  public:
    
    template<typename InitIdxFunc>
    Tabular(const SecondaryIterator& begin, const InitIdxFunc& init)
      : begin(begin.origin()) {
      init(indices);
      for(auto& index : indices) index = (begin + index).index();
    }

    typedef TabularIterator<typename SecondaryIterator::primary_type> iterator;

    iterator begin() const {return iterator(begin,indices.begin());}
    iterator end()   const {return iterator(begin,indices.end());}
  };
  
  /**
   * Class for tabular acces to data in case of primary iterators (SFINAE rescue case).
   */
  template<typename Iterator> 
  class Tabular {
  private:
    
    typename Iterator begin;
    std::vector<tabular_index_type> indices;    // indices in the primary collection

  public:
    
    template<typename InitIdxFunc>
    Tabular(const Iterator& begin, const InitIdxFunc& init)
      : begin(begin) {
      init(indices);
    }

    typedef TabularIterator<Iterator> iterator;

    iterator begin() const {return iterator(begin,indices.begin());}
    iterator end()   const {return iterator(begin,indices.end());}
  };

  

}
