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

#include <cmath>
#include <gamlAlgorithms.hpp>
#include <gamlSplit.hpp>

namespace gaml {
  namespace score {

    
    /**
     * This can be used as a "template template" argument for algorithms
     * whose scoring function is a parameter.
     */
    template<typename DataIterator, typename Test, typename ValueOf> 
    class RelativeVarianceReduction {
    public:
      double operator()(const DataIterator& begin, const DataIterator& end, const Test& test, const ValueOf& value_of) {
	double n    = 0;
	double mean = 0;
	double M2   = 0;
	double var;

	double n_t    = 0;
	double mean_t = 0;
	double M2_t   = 0;
	double var_t;

	double n_f    = 0;
	double mean_f = 0;
	double M2_f   = 0;
	double var_f;
      
	for(auto it = begin; it != end; ++it) {
	  double x =  (double)(value_of(*it));
	  double delta = x - mean;
	  mean = mean + delta/(++n);
	  M2 = M2 + delta*(x - mean);
	  if(test(*it)) {
	    double delta_t = x - mean_t;
	    mean_t = mean_t + delta_t/(++n_t);
	    M2_t = M2_t + delta_t*(x - mean_t);
	  }
	  else {
	    double delta_f = x - mean_f;
	    mean_f = mean_f + delta_f/(++n_f);
	    M2_f = M2_f + delta_f*(x - mean_f);
	  }
	}

	if(n < 2) return 0; else var = M2/(n-1);
      
	if(n_t < 2) var_t = 0; else var_t = M2_t/(n_t-1);
	if(n_f < 2) var_f = 0; else var_f = M2_f/(n_f-1);

	return (var - var_t*n_t/n - var_f*n_f/n)/var;
      }
    };

    /**
     * Let \f$d \in S\f$ be the data dataset \f$S\f$. Let us note \f$S^+ = \{d \in S :\; \mathrm{test}(d) = \mathrm{true}\}\f$ and \f$S^- = S \setminus S^+\f$. Let us not \f$\mathrm{var}(X) = \mathrm{variance}(\{\mathrm{value}(d) :\; d \in X\})\f$. The relative variance reduction is \f$\frac{\mathrm{var}(S) - \frac{|S^+|}{|S|}\mathrm{var}(S^+)- \frac{|S^-|}{|S|}\mathrm{var}(S^-)}{\mathrm{var}(S)}\f$
     */
    template<typename DataIterator, typename Test, typename ValueOf> 
    double relative_variance_reduction(const DataIterator& begin, const DataIterator& end, const Test& test, const ValueOf& value_of) {
      RelativeVarianceReduction<DataIterator,Test,ValueOf> rvr;
      return rvr(begin,end,test,value_of);
    }

    
    /**
     * This can be used as a "template template" argument for algorithms
     * whose scoring function is a parameter.
     */
    template<typename DataIterator, typename Test, typename ValueOf> 
    class NormalizedInformationGain {
    public:
      double operator()(const DataIterator& begin, const DataIterator& end, const Test& test, const ValueOf& value_of) {

	auto split = gaml::split(begin,end,test);
	auto size = std::distance(begin,end);
	auto size_true  = std::distance(split.true_values.begin(), split.true_values.end());
	auto size_false = std::distance(split.false_values.begin(),split.false_values.end());

	double Hc = gaml::classification_entropy<decltype(value_of(*begin))>(begin,end,value_of);

	double p_true  = size_true/(double)size;
	double p_false = size_false/(double)size;
	double Ht = - p_true*std::log2(p_true) - p_false*std::log2(p_false);

	double Hct = 0
	  + p_true  * gaml::classification_entropy<decltype(value_of(*begin))>(split.true_values.begin(),  split.true_values.end(), value_of)
	  + p_false * gaml::classification_entropy<decltype(value_of(*begin))>(split.false_values.begin(), split.false_values.end(),value_of);

	double Ict  = Hc - Hct;
	
	return 2*Ict/(Hc+Ht);
      }
    };

    /**
     * See the definition in this paper (eq 11) : L. Wehenkel and 
     M. Pavella, <i>"Decision trees and transient stability of electric power systems"</i>, Automatica 27(1):115–134, 1991.
     */
    template<typename DataIterator, typename Test, typename ValueOf> 
    double normalized_information_gain(const DataIterator& begin, const DataIterator& end, const Test& test, const ValueOf& value_of) {
      NormalizedInformationGain<DataIterator,Test,ValueOf> nig;
      return nig(begin,end,test,value_of);
    }
  }
}
