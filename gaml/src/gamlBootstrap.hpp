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
#include <set>
#include <sstream>
#include <iterator>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <gamlAlgorithms.hpp>
#include <gamlException.hpp>
#include <gamlTabular.hpp>

namespace gaml {

  
  template<typename Iterator>
  Tabular<Iterator> bootstrap(const Iterator& begin, const Iterator& end, unsigned int size) {
    auto init = [begin,end,size](std::vector<tabular_index_type>& indices) -> void {
      indices.resize(size);
      unsigned int range = std::distance(begin,end);
      for(auto& idx : indices)
	idx = (tabular_index_type)gaml::random::uniform(0,range);
    };
    return Tabular<Iterator>(begin,init);
  }

  namespace risk {
    namespace bootstrap {

      template<typename LOSS>
      class LeaveOneOut {
      private:

	LOSS loss;
	unsigned int nb_sets;
	bool verbose;

      public:

	LeaveOneOut(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
	LeaveOneOut(const LeaveOneOut& other)
	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			  const InputOf& inputOf, const OutputOf& outputOf) const {
	  unsigned int size = end - begin;
	  std::vector<gaml::Bootstrap<DataIterator> > sets;
	  std::vector<typename Learner::predictor_type> f;
	  std::vector<std::set<int> > C(size);
	  unsigned int b,i;

	  if(verbose)
	    std::cout << "Making " << nb_sets 
		      << " bootstrapped sets." << std::endl;
       
	  for(b = 0; b < nb_sets; ++b)
	    sets.push_back(gaml::bootstrap(begin,end,size));
	  
	  if(verbose) {
	    for(b = 0; b < nb_sets; ++b) {
	      std::cout << "  Set " << std::setw(3) << b+1 << '/' << nb_sets << " :";

	      for(auto elem : sets[b].indices)
	      	std::cout << ' ' << std::setw(3) << elem;
	      std::cout << std::endl;
	    }
	  }
	    

	  if(verbose)
	    std::cout << "Computing the non-belong-to set list for each example." << std::endl;

	  for(i = 0; i < size; ++i) {
	    auto& Ci = C[i];
	    for(b = 0; b < nb_sets; ++b)
	      if(!(sets[b].has(i)))
		Ci.insert(Ci.begin(),b);
	    // Ci contains the indices of sets that can be used to test i.
	  }

	  if(verbose) {
	    for(i = 0; i < size; ++i) {
	      std::cout << "  sample " << std::setw(3) << i << " is not in sets {";
	      for(auto elem : C[i])
	      	std::cout << ' ' << elem+1;
	      std::cout << " }" << std::endl;
	    }
	  }


	  if(verbose)
	    std::cout << "Learning on these sets." << std::endl;
	  for(b = 0; b < nb_sets; ++b) {
	    std::ostringstream ostr;
	    if(verbose) {
	      ostr << "  Set " << std::setw(3) << b+1 << '/' << nb_sets << " : ";
	      std::cout << ostr.str() << "learning...\r" << std::flush;
	    }
	    gaml::Bootstrap<DataIterator>& Zb = sets[b];
	    f.push_back(learner(Zb.begin(),Zb.end(),inputOf,outputOf));
	    if(verbose)
	      std::cout << ostr.str() << "Done.      " << std::endl;
	  }

	  if(verbose)
	    std::cout << "Testing each sample" << std::endl;

	  unsigned int cardD = 0;
	  double       sumD  = 0;

	  DataIterator zi;
	  for(i = 0, zi = begin; i < size; ++i,++zi) {
	    if(verbose)
	      std::cout << "  " << std::setw(4) << i+1 << '/' << size << " : ";

	    std::set<int>& Ci = C[i];

	    if(Ci.empty()) {
	      if(verbose)
		std::cout << "belongs to all sets." << std::endl;
	    }
	    else {
	      const typename Learner::predictor_type::input_type& xi = inputOf(*zi);
	      const typename Learner::predictor_type::output_type yi = outputOf(*zi);
	      double sum = 0;
	      for(auto j : Ci)
		sum += loss(f[j](xi),yi);
	      sum /= Ci.size();
	      if(verbose)
		std::cout << sum << " (using " << Ci.size() << " samples)" << std::endl;
	      ++cardD;
	      sumD += sum;
	    }
	  }

	  if(cardD == 0)
	    throw gaml::exception::Bootstrap("Every sample belongs to all bootstrapped sets");
       
	  return sumD/(double)cardD;
	}
      };
      
      
      template<typename LOSS>
      LeaveOneOut<LOSS> leave_one_out(const LOSS& l, unsigned int nb_sets, bool verbosity) {
	return LeaveOneOut<LOSS>(l,nb_sets,verbosity);
      }
      

      template<typename LOSS>
      class R632 {
      private: 

      	LOSS loss;
      	unsigned int nb_sets;
      	bool verbose;

      public:

      	R632(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
      	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
      	R632(const R632& other)
      	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
      	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
			  const InputOf& inputOf, const OutputOf& outputOf) const {
	  auto predictor = learner(begin,end,inputOf,outputOf);
      	  auto empirical = gaml::risk::empirical(loss);
      	  double Rn      = empirical(predictor,begin,end,inputOf,outputOf);
      	  auto estimated = gaml::risk::bootstrap::leave_one_out(loss,nb_sets,false);
      	  double R       = estimated(learner,begin,end,inputOf,outputOf);
      	  if(verbose)
      	    std::cout << "Empirical risk    : Rn     : " << Rn << std::endl
      		      << "Bootstrapped risk : R      : " << R << std::endl
      		      << "                  : R - Rn : " << R-Rn << std::endl;
      	  return 0.368*Rn + 0.632*R;
      	}
      };

      template<typename LOSS>
      R632<LOSS> r632(const LOSS& l, unsigned int nb_sets, bool verbosity) {
      	return R632<LOSS>(l,nb_sets,verbosity);
      }


      template<typename LOSS>
      class R632Plus {
      private:

      	LOSS loss;
      	unsigned int nb_sets;
      	bool verbose;

      public:

      	R632Plus(const LOSS& l, unsigned int nb_bootstrapped_sets, bool verbosity) 
      	  : loss(l), nb_sets(nb_bootstrapped_sets), verbose(verbosity) {}
      	R632Plus(const R632Plus& other)
      	  : loss(other.loss), nb_sets(other.nb_sets), verbose(other.verbose) {}
	
      	template<typename Learner, typename DataIterator, typename InputOf, typename OutputOf> 
      	double operator()(const Learner& learner,const DataIterator& begin, const DataIterator& end,
      			      const InputOf& inputOf, const OutputOf& outputOf) const {
	  auto predictor = learner(begin,end,inputOf,outputOf);
      	  auto empirical = gaml::risk::empirical(loss);
      	  double Rn      = empirical(predictor,begin,end,inputOf,outputOf);
      	  auto estimated = gaml::risk::bootstrap::leave_one_out(loss,nb_sets,false);
      	  double R       = estimated(learner,begin,end,inputOf,outputOf);
      	  unsigned int N = end-begin;
      
      	  if(N == 0)
      	    throw gaml::exception::Bootstrap("Empty database provided.");
	

      	  DataIterator i,j;

      	  double gamma = 0;

      	  for(i = begin; i != end; ++i) {
      	    const typename Learner::predictor_type::output_type yi = predictor(inputOf(*i));
      	    for(j = begin; j != end; ++j)
      	      gamma += loss(outputOf(*j),yi);
      	  }
      	  gamma /= N*N;

      	  double r = (R-Rn)/(gamma-Rn);
      	  double omega = 0.632/(1-0.368*r);

      	  if(verbose)
      	    std::cout << "Empirical risk           : Rn                        : " << Rn << std::endl
      		      << "Bootstrapped risk        : R                         : " << R << std::endl
      		      << "0-information error rate : gamma                     : " << gamma << std::endl
      		      << "Overfitting rate         : r = (R-Rn)/(gamma-Rn)     : " << r << std::endl
      		      << "Weight                   : omega = 0.632/(1-0.368*r) : " << omega << std::endl;
      	  return (1-omega)*Rn + omega*R;
      	}
      };

      template<typename LOSS>
      R632Plus<LOSS> r632plus(const LOSS& l, unsigned int nb_sets, bool verbosity) {
      	return R632Plus<LOSS>(l,nb_sets,verbosity);
      }
    }
  }
}
