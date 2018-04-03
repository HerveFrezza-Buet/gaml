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
#include <optional>

namespace gaml {
  
  template<typename Iterator> class Cache;

  template<typename Iterator>
  class CacheIterator 
    : public std::iterator<std::random_access_iterator_tag,
			   typename std::iterator_traits<Iterator>::value_type>{
  public:
    typedef typename std::iterator_traits<Iterator>::value_type  value_type;
    typedef int                                                  index_type;
    typedef index_type                                           difference_type;

  private:

    index_type idx;
    Cache<Iterator>* cache;
    // mutable std::optional<value_type> value;
    friend class Cache<Iterator>;

    CacheIterator(index_type index, Cache<Iterator>* c) 
      : idx(index), cache(c) /*, value() */ {}

  public:                                  

    CacheIterator(void)                               = default;
    CacheIterator(const CacheIterator& cp)            = default;
    CacheIterator& operator=(const CacheIterator& cp) = default;  

    CacheIterator& operator++()                       {++idx;     /* value.reset();*/ return *this;}
    CacheIterator& operator--()                       {--idx;     /* value.reset();*/ return *this;}
    CacheIterator& operator+=(difference_type diff)   {idx+=diff; /* value.reset();*/ return *this;}
    CacheIterator& operator-=(int diff)               {idx-=diff; /* value.reset();*/ return *this;}

    CacheIterator  operator++(int) {
      CacheIterator res = *this;
      ++*this; 
      return res;
    }

    CacheIterator  operator--(int) {
      CacheIterator res = *this; 
      --*this; 
      return res;
    }

    difference_type   operator- (const CacheIterator& i) const {return idx - i.idx;}
    CacheIterator     operator+ (difference_type i)      const {return CacheIterator(idx+i,cache);}
    CacheIterator     operator- (difference_type i)      const {return CacheIterator(idx-i,cache);}
    bool              operator==(const CacheIterator& i) const {return cache == i.cache && idx == i.idx;}
    bool              operator!=(const CacheIterator& i) const {return cache != i.cache || idx != i.idx;}


    const value_type& operator*() const {
      /* if(!value)
	value = cache->at(idx);
	return value.value(); */
      return cache->at(idx);
    }
  };


  //#define mlDEBUG_CACHE
  template<typename Iterator> 
  class Cache {

  private:

    typedef int index_type;

    friend class CacheIterator<Iterator>;

    class Page {
    private:

      void scoreInit(void)  {score = 0;}
      void scoreHit(void)   {score++;}
      void scoreNoHit(void) {score *= .9;}
      void scoreLoad(void)  {scoreInit();}

    public:

      typedef typename Iterator::value_type value_type;

      std::vector<typename Iterator::value_type> data;
      index_type begin;
      double score;
      bool used;

      void load(const Iterator& it, index_type idx, unsigned int size) {
	data.resize(size);
	std::copy(it,it+size,data.begin());
	begin = idx;
	scoreLoad();
	used = true;
      }

      bool has(index_type idx) const {
	return used && idx >= begin && idx-begin < (index_type)(data.size());
      }

      void noHit(void) {if(used) scoreNoHit();}

      // has method call must have returned true
      const typename Iterator::value_type& get(index_type idx) {
	scoreHit();
	return data[idx-begin];
      }
    };

    Iterator _begin;
    index_type size;
    index_type psize;
    typedef std::vector<Page> pages_type;

    pages_type pages;

    const typename Iterator::value_type& at(index_type i) {
      bool page_not_found;
      typename std::vector<Page>::iterator pi,pe,pfound;

      for(page_not_found=true,pi=pages.begin(),pe=pages.end();
	  pi != pe;
	  (*pi).noHit(),++pi)
	if(page_not_found && (*pi).has(i)) {
	  page_not_found = false;
	  pfound = pi; // Do not break here, since noHit has to be called.
	}

#ifdef mlDEBUG_CACHE
      if(!page_not_found)
	std::cout << "index " << i << " is cached." << std::endl;
#endif
      
      if(!page_not_found)
	return (*pfound).get(i);

      // Let us find the lowest scored page.

#ifdef mlDEBUG_CACHE
      std::cout << "No page handles index " << i << std::endl;
#endif

      pi = pages.begin();
      bool no_unused_page_found = true;
      double min_score;
      pfound = pi;
      if(!(*pi).used)
	 no_unused_page_found = false;
      else
	min_score = (*pi).score;
	
      for(++pi ; no_unused_page_found && pi != pe ; ++pi) 
	if(!(*pi).used) {
	  no_unused_page_found = false;
	  pfound = pi;
	}
	else if((*pi).score < min_score) {
	  min_score = (*pi).score;
	  pfound = pi;
	}

#ifdef mlDEBUG_CACHE
      if(!no_unused_page_found)
	std::cout << "Unused page " << std::distance(pages.begin(),pfound) << " found" << std::endl;
      else
	std::cout << "Recycling page " << std::distance(pages.begin(),pfound) << std::endl;
#endif
	
      
      // Let us load it.

      index_type first = psize*(i/psize);
      index_type last  = first + psize;
      if(last > size)
	last = size;

#ifdef mlDEBUG_CACHE
      std::cout << "Loading [" << first << ".." << last << "[ into page " << std::distance(pages.begin(),pfound) << std::endl;
#endif

      (*pfound).load(_begin+first,first,last-first);
      
      return (*pfound).get(i);
    }

  public:
    typedef CacheIterator<Iterator> iterator;
    
    Cache(const Iterator& begin_iter, 
	  const Iterator& end_iter,
	  unsigned int page_size,
	  unsigned int nb_pages) : 
      _begin(begin_iter), size((index_type)(std::distance(begin_iter,end_iter))), psize(page_size), pages(nb_pages) {
      
      typename pages_type::iterator pi,pend;
      for(pi = pages.begin(), pend = pages.end(); pi != pend; ++pi) 
	(*pi).used = false;
    }
    
    iterator begin() {return CacheIterator<Iterator>(0,this); }
    iterator end()   {return CacheIterator<Iterator>(size,this); }
  };
  
  /**
   * The Iterator type must be a random access iterator.
   */
  template<typename Iterator>
  Cache<Iterator> cache(const Iterator& begin, const Iterator& end,
			unsigned int page_size,
			unsigned int nb_pages) {
    return Cache<Iterator>(begin,end,page_size,nb_pages);
  }
}
