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

    /**
     * This concept stands for iterator which iterate on data through
     * an intermadiate array of index. The type trait
     * is_secondary_iterator<T>::type is std::true_type if T is a
     * secondary iterator (detected by the existence of
     * T::primary_type) and std::false_type otherwise.
     */
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
  template<typename Iterator>
  class TabularIterator : public std::iterator<std::random_access_iterator_tag,
					       typename Iterator::value_type> {
  private:
    Iterator                                        begin;
    std::vector<tabular_index_type>::const_iterator idx;

    

  public:

    TabularIterator(const Iterator& begin, const std::vector<tabular_index_type>::const_iterator& idx)
      : begin(begin), idx(idx) {}
    
    typedef Iterator primary_type;
    
    TabularIterator()                                                       = default;
    TabularIterator(const TabularIterator<Iterator>&)                       = default;
    TabularIterator<Iterator>& operator=(const TabularIterator<Iterator>&)  = default;
    TabularIterator(TabularIterator<Iterator>&&)                            = default;
    TabularIterator<Iterator>& operator=(TabularIterator<Iterator>&&)       = default;
    
    tabular_index_type index()  const {return *idx;}
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
    
    bool operator==(const TabularIterator<Iterator>& i) const {return idx == i.idx && begin == i.begin;}
    bool operator!=(const TabularIterator<Iterator>& i) const {return idx != i.idx || begin != i.begin;}
    
    const typename Iterator::value_type& operator*() const {
      auto itt = begin;
      std::advance(itt,(typename std::iterator_traits<primary_type>::difference_type)(*idx));
      return *itt;
    }
  };

  
  template<typename T> using test_primary_iterator = void;
  
  template<typename T, typename = void>
  struct is_secondary_iterator : std::false_type {};
  
  template<typename T>
  struct is_secondary_iterator<T, test_primary_iterator<typename T::primary_type> > : std::true_type {};

  /**
   * Base class for tabular acces to data.
   */

  template<typename Iterator> 
  class TabularBase {
  protected:
    
    std::vector<tabular_index_type> indices;    // indices in the primary collection

  public:
    
    TabularBase() = default;
    TabularBase(const TabularBase<Iterator>&) = default;
    TabularBase(TabularBase<Iterator>&&) = default;
    TabularBase<Iterator>& operator=(const TabularBase<Iterator>&) = default;
    TabularBase<Iterator>& operator=(TabularBase<Iterator>&&) = default;

    typedef std::vector<tabular_index_type>::const_iterator index_iterator;
    index_iterator begin_index() const {return indices.begin();}
    index_iterator end_index()   const {return indices.end();}

    bool has(tabular_index_type idx) const {
      for(auto i : indices) if(i == idx) return true;
      return false;
    }
  };

  
  /**
   * Class for tabular acces to data in case of primary iterators.
   */
  template<typename Iterator, typename PrimaryIterator> 
  class Tabular : public TabularBase<Iterator> {
  private:
    
    Iterator start;

  public:

    Tabular() = default;
    Tabular(const Tabular<Iterator,PrimaryIterator>&) = default;
    Tabular(Tabular<Iterator,PrimaryIterator>&&) = default;
    Tabular<Iterator,PrimaryIterator>& operator=(const Tabular<Iterator,PrimaryIterator>&) = default;
    Tabular<Iterator,PrimaryIterator>& operator=(Tabular<Iterator,PrimaryIterator>&&) = default;
    
    template<typename InitIdxFunc>
    Tabular(const Iterator& begin, const InitIdxFunc& init)
      : TabularBase<Iterator>(), start(begin) {
      init(this->indices);
    }

    typedef TabularIterator<Iterator> iterator;

    iterator begin() const {return iterator(start,this->indices.begin());}
    iterator end()   const {return iterator(start,this->indices.end());}
  };
  
  /**
   * Class for tabular acces to data for secondary iterators
   */
  template<typename Iterator> 
  class Tabular<Iterator, std::true_type> : public TabularBase<Iterator> {
  private:
    
    typename Iterator::primary_type start;

  public:
    
    Tabular() = default;
    Tabular(const Tabular<Iterator,std::true_type>&) = default;
    Tabular(Tabular<Iterator,std::true_type>&&) = default;
    Tabular<Iterator,std::true_type>& operator=(const Tabular<Iterator,std::true_type>&) = default;
    Tabular<Iterator,std::true_type>& operator=(Tabular<Iterator,std::true_type>&&) = default;
    
    template<typename InitIdxFunc>
    Tabular(const Iterator& begin, const InitIdxFunc& init)
      : TabularBase<Iterator>(), start(begin.origin()) {
      init(this->indices);
      for(auto& index : this->indices) {
	auto sec_it = begin;
	std::advance(sec_it,index);
	index = sec_it.index();
      }
    }

    typedef TabularIterator<typename Iterator::primary_type> iterator;

    iterator begin() const {return iterator(start,this->indices.begin());}
    iterator end()   const {return iterator(start,this->indices.end());}
  };

  

}
