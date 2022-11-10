#pragma once

/*
 *   Copyright (C) 2014,  Supelec
 *
 *   Author : Hervé Frezza-Buet
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
 *   Contact : herve.frezza-buet@supelec.fr
 *
 */

#include <functional>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <numeric>
#include <algorithm>
#include <vector>
#include <iterator>
#include <random>
#include <gaml.hpp>

namespace gaml {
  namespace xtree {
    namespace internal {

      class Writeable {
      public:
	Writeable() {}
	virtual ~Writeable() {}
	virtual void write(std::ostream& os) const=0;
	friend std::ostream& operator<<(std::ostream& os, const Writeable& s) {
	  s.write(os);
	  return os;
	}
      };

      class Serializable : public Writeable {
      public:
	Serializable() : Writeable(){}
	virtual ~Serializable() {}
	virtual void read(std::istream& is)=0;
	friend std::istream& operator>>(std::istream& is, Serializable& s) {
	  s.read(is);
	  return is;
	}
      };

      template<typename X>
      class Test : public Serializable {
      public:
	Test(): Serializable() {}
	virtual ~Test() {}

	virtual bool operator()(const X&) = 0;
      };

      template<typename X, typename Y>
      class Tree : public Writeable {
      public:
	Tree() : Writeable() {}
	virtual ~Tree() {}
	virtual Y operator()(const X&) = 0;
      };

      template<typename X, typename Y, typename TEST>
      class Node : public Tree<X,Y> {
      private:
	TEST test;
	std::shared_ptr<Tree<X,Y>> pass;
	std::shared_ptr<Tree<X,Y>> fail;
      public:
	
	Node(std::istream& is,
	     std::shared_ptr<Tree<X,Y>> p,
	     std::shared_ptr<Tree<X,Y>> f) : Tree<X,Y>(), test(), pass(p), fail(f) {
	  is >> test;
	}

	Node(const TEST& t,
	     std::shared_ptr<Tree<X,Y>> p,
	     std::shared_ptr<Tree<X,Y>> f) : Tree<X,Y>(), test(t), pass(p), fail(f) {}
	virtual ~Node() {}
	virtual Y operator()(const X& x) {
	  if(test(x)) return (*pass)(x);
	  else        return (*fail)(x);
	}

	virtual void write(std::ostream& os) const {
	  os << "N"   << std::endl
	     << *pass << std::endl
	     << *fail << std::endl
	     << test;
	}
      };

      
      template<typename X, typename Y, typename T>
      std::shared_ptr<Node<X,Y,T>> make_node(const T& t,
					 std::shared_ptr<Tree<X,Y>> p,
					 std::shared_ptr<Tree<X,Y>> f) {
	return std::make_shared<Node<X,Y,T>>(t,p,f);
      }

      template<typename X, typename Y>
      class Leaf : public Tree<X,Y> {
      public:
	Leaf() : Tree<X,Y>() {}
	virtual ~Leaf() {}

	virtual void write(std::ostream& os) const {
	  os << "L" << std::endl; 
	  write_leaf(os);
	};
	virtual void write_leaf(std::ostream& os) const = 0;
	virtual void read_leaf(std::istream& is) = 0;
      };
      

      template<typename X, typename Y, typename LEAF, typename TEST>
      std::shared_ptr<Tree<X,Y>> tree_factory(std::istream& is) {
	char tag,sep;
	std::shared_ptr<Tree<X,Y>> p;
	std::shared_ptr<Tree<X,Y>> f;
	std::shared_ptr<LEAF> l;
	is >> tag;
	is.get(sep);
	switch(tag) {
	case 'N':
	  p = tree_factory<X,Y,LEAF,TEST>(is);
	  f = tree_factory<X,Y,LEAF,TEST>(is);
	  return std::make_shared<Node<X,Y,TEST>>(is,p,f);
	  break;
	case 'L':
	  l = std::make_shared<LEAF>();
	  l->read_leaf(is);
	  return l;
	  break;
	default:
	  std::ostringstream ostr;
	  ostr << "'" << tag << "' leaf/node tag found, 'L' or 'N' expected.";
	  throw std::runtime_error(ostr.str());
	}
      }

      template<typename X>
      class ThresholdTest : public xtree::internal::Test<X> {
      public:
	double threshold;
	unsigned int attr;

	ThresholdTest() : xtree::internal::Test<X>(), threshold(0), attr(0) {}
	ThresholdTest(double thres, unsigned int a) : xtree::internal::Test<X>(), threshold(thres), attr(a) {}
	ThresholdTest(const ThresholdTest& cp) : threshold(cp.threshold), attr(cp.attr) {}
	ThresholdTest& operator=(const ThresholdTest& cp) {
	  if(this != &cp) {
	    threshold = cp.threshold;
	    attr      = cp.attr;
	  }
	  return *this;
	}

	virtual ~ThresholdTest() {}

	virtual bool operator()(const X& x) {
	  auto it = x.begin();
	  std::advance(it,attr);
	  return *it < threshold;
	}
	virtual void write(std::ostream& os) const {
	  os << threshold << ' ' << attr;
	}
	virtual void read(std::istream& is) {
	  is >> threshold >> attr;
	}
      };


      template<typename DataIterator, typename AttrIterator, typename InputOf, typename OutputOf>
      bool should_split(const DataIterator& begin, const DataIterator& end,
			const InputOf& input_of, const OutputOf& output_of,
			unsigned int nmin,
			AttrIterator& out_non_constant_attr) {
	if(std::distance(begin,end) < (typename DataIterator::difference_type)nmin)
	  return false;
	
	auto it = begin;
	
	const auto& x0 = input_of(*it);
	auto  y0 = output_of(*it);

	std::vector<bool> is_attr_constant(std::distance(x0.begin(),x0.end()),true);
	bool is_output_constant = true;

	auto nb_attr_constant = is_attr_constant.size();
	for(++it; it != end && (nb_attr_constant > 0 || is_output_constant); ++it) {
	  if(is_output_constant)
	    is_output_constant = output_of(*it) == y0;
	  if(nb_attr_constant > 0) {
	    auto iac = is_attr_constant.begin();
	    const auto& x = input_of(*it);
	    for(auto attr_it = x.begin(); attr_it != x.end(); ++attr_it,++iac)
	      if(*iac) {
		auto attr_it0 = x0.begin();
		std::advance(attr_it0,std::distance(is_attr_constant.begin(),iac));
		if(*attr_it0 != *attr_it) {
		  *iac = false;
		  --nb_attr_constant;
		}
	      }
	  }
	}

	if(is_output_constant || nb_attr_constant == is_attr_constant.size())
	  return false;
	
	for(auto iac = is_attr_constant.begin(); iac != is_attr_constant.end(); ++iac)
	  if(!(*iac))
	    *(out_non_constant_attr++) = std::distance(is_attr_constant.begin(),iac);
	return true;
      }
      

      // template<typename X, typename Data>
      // class LambdaValueOf {
      // public:
      // 	unsigned int a;
      // 	std::function<X (const Data&)> input_of;

      // 	template<typename InputOf>
      // 	LambdaValueOf(unsigned int attr, const InputOf& iof) : a(attr), input_of(iof) {}
      // 	double operator()(const Data& data) {
      // 	  auto input = input_of(data);
      // 	  auto it = input.begin();
      // 	  std::advance(it,a);
      // 	  return *it;
      // 	}
      // };
      // template<typename X, typename Data, typename InputOf>
      // LambdaValueOf<X,Data> lambda_value_of(unsigned int a, const InputOf& iof) {
      // 	return LambdaValueOf<X,Data>(a,iof);
      // }
      

      template<typename X, typename Y,
	       template<typename,typename,typename>                   class SCORE,
	       typename RANDOM_DEVICE,
	       template<typename,typename,typename,typename,typename> class MakeLeaf,
	       typename DataIterator, typename InputOf, typename OutputOf>
      std::shared_ptr<xtree::internal::Tree<X,Y>> build_tree(const DataIterator& begin, const DataIterator& end,
							     const InputOf& input_of, const OutputOf& output_of,
							     unsigned int nmin,
							     unsigned int k,
							     RANDOM_DEVICE& rd) {

	std::vector<unsigned int> non_constant_attr;
	auto out_attr = std::back_inserter(non_constant_attr);

	if(should_split(begin,end,input_of,output_of,nmin,out_attr)) {

	  unsigned int K = std::min(k,(unsigned int)non_constant_attr.size());
	  std::shuffle(non_constant_attr.begin(),non_constant_attr.end(), rd);
	  auto attr_begin = non_constant_attr.begin();
	  auto attr_end   = attr_begin+K;


	  SCORE<DataIterator,
		std::function<bool (const typename std::iterator_traits<DataIterator>::value_type&)>,
		OutputOf> score;
	  ThresholdTest<X> best_test;

	  // Let us consider K random non constant attributes for
	  // splitting, and keep the one with the highest score.

	  double best_score = std::numeric_limits<double>::lowest();

	  for(auto attr_it = attr_begin; attr_it != attr_end; ++attr_it) {
	    auto a = *attr_it;
	  
	    // The following code make a bug when input_of(data) is an
	    // array and when g++ optimization is invoked. 
	    //
	    auto value_of = [a,&input_of](const typename std::iterator_traits<DataIterator>::value_type& data) -> double {
	      auto input = input_of(data);
	      auto it = input.begin();
	      std::advance(it,a);
	      return *it;
	    };

	    //auto value_of = lambda_value_of<X,const typename std::iterator_traits<DataIterator>::value_type>(a,input_of);

	    auto values = gaml::map(begin,end,value_of);

	    auto bounds = std::minmax_element(values.begin(),values.end());
	    std::uniform_real_distribution<double> uniform(*(bounds.first),*(bounds.second));
	    double threshold = uniform(rd);
	    // We need to ensure that threshold > min(bounds) in order
	    // to have non empty collections when splitting the values
	    while(threshold == *(bounds.first))
	      threshold = uniform(rd);

	    ThresholdTest<X> test(threshold, a);

	    auto score_test = [&test,&input_of](const typename std::iterator_traits<DataIterator>::value_type& data) -> bool {
	      return test(input_of(data));
	    };

	    double s = score(begin,end,score_test,output_of);
	    if(s > best_score) {
	      best_score = s;
	      best_test = test;
	    }
	  }

	  auto filter_function = [&input_of,&best_test](const typename std::iterator_traits<DataIterator>::value_type& data) -> bool {return best_test(input_of(data));};
	  auto split       = gaml::split(begin, end, filter_function);
	  auto left        = split.true_values;
	  auto right       = split.false_values;
	  return make_node(best_test,
			   build_tree<X,Y,SCORE,RANDOM_DEVICE,MakeLeaf>(left.begin(),  left.end(),  input_of, output_of, nmin, k, rd),
			   build_tree<X,Y,SCORE,RANDOM_DEVICE,MakeLeaf>(right.begin(), right.end(), input_of, output_of, nmin, k, rd));
	}
	else {
	  MakeLeaf<X,Y,DataIterator,InputOf,OutputOf> make_leaf;
	  return make_leaf(begin,end,input_of,output_of);
	}
      }

    }
  }
}
