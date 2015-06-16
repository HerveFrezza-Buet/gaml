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

#include <gamlDimensions.hpp>
#include <vector>
#include <algorithm>
#include <limits>
#include <iterator>

namespace gaml {
  namespace varsel {
    template<typename DataIterator, typename InputOf, typename OutputOf> 
    class CorrelationFilter {
      const DataIterator dataBegin_;
      const DataIterator dataEnd_;
      const InputOf& inputOf_;
      const OutputOf& outputOf_;
      bool verbose_;
      double* means_;
      double* stdDeviations_;
      double* coefs_;
      int attributeNumber_;

      void computeMeanAndVariance() {
	if(verbose_) std::cout << "Computing mean and standard deviations in dataset" << std::endl;
	int n = attributeNumber_ + 1;

	means_ = new double[n];
	stdDeviations_ = new double[n];

	{
	  double *mPtr = means_, *dPtr = stdDeviations_;
	  for(int i = 0; i != n; ++i) {
	    *mPtr++ = 0; *dPtr++ = 0;
	  }
	}

	int m = 0;
	for(DataIterator it = dataBegin_; it != dataEnd_; ++it, ++m) {
	  auto& input = inputOf_(*it);

	  double *mPtr = means_;
	  for(auto attr = input.begin(); attr != input.end(); ++attr) *mPtr++ += *attr;
	  *mPtr += outputOf_(*it);
	}

	{
	  double *mPtr = means_;
	  for(int i = 0; i != n; ++i) *mPtr++ /= m;
	}

	for(DataIterator it = dataBegin_; it != dataEnd_; ++it) {
	  auto& input = inputOf_(*it);

	  double *mPtr = means_, *dPtr = stdDeviations_;
	  for(auto attr = input.begin(); attr != input.end(); ++attr) {
	    double dev = *attr - *mPtr++;
	    *dPtr++ += dev * dev;
	  }
	  double dev = outputOf_(*it) - *mPtr;
	  *dPtr += dev * dev;
	}
	{
	  double *dPtr = stdDeviations_;
	  for(int i = 0; i != n; ++i, ++dPtr) *dPtr = sqrt(*dPtr / m);
	}

	if(verbose_) {
	  std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(3);
	  std::cout << "Means   =";
	  for(int i = 0; i != n; i++) std::cout << " " << means_[i];
	  std::cout << std::endl;
	  std::cout << "StdDevs =";
	  for(int i = 0; i != n; i++) std::cout << " " << stdDeviations_[i];
	  std::cout << std::endl;
	}
      }

      void computeCorrelationCoefficients() {
	computeMeanAndVariance();
	if(verbose_) std::cout << "Computing correlation matrix" << std::endl;

	int n = attributeNumber_ + 1;
	coefs_ = new double[n*n];
	for(int i = 0; i != n; i++) 
	  for(int j = 0; j <= i; j++) 
	    coefs_[i*n+j] = 0.;

	double* data = new double[n];
	int m = 0;
	for(DataIterator it = dataBegin_; it != dataEnd_; ++it, ++m) {
	  auto& input = inputOf_(*it);
	  double* mean = means_;
	  double* value = data;
	  for(auto attr = input.begin(); attr != input.end(); ++attr, ++mean, ++value) *value = *attr - *mean;
	  *value = outputOf_(*it) - *mean;
	  for(int i = 0; i != n; i++) 
	    for(int j = 0; j <= i; j++) 
	      coefs_[i*n+j] += data[i]*data[j];
	}
	delete [] data;

	for(int i = 0; i != n; i++) {
	  for(int j = 0; j <= i; j++) 
	    coefs_[i*n+j] /= m;
	}

	for(int i = 0; i != n; i++) {
	  for(int j = 0; j <= i; j++) 
	    coefs_[i*n+j] /= (stdDeviations_[i] * stdDeviations_[j]);
	}

	if(verbose_) {
	  for(int i = 0; i != n; i++) {
	    for(int j = 0; j <= i; j++) 
	      std::cout << " " << coefs_[i*n+j];
	    std::cout << std::endl;
	  }
	}
      }

    public:
      static const bool toMinimize = false;

      CorrelationFilter(const DataIterator& dataBegin, const DataIterator& dataEnd, const InputOf& inputOf, const OutputOf& outputOf) :
	dataBegin_(dataBegin), dataEnd_(dataEnd), inputOf_(inputOf), outputOf_(outputOf), verbose_(false), 
	means_(0), stdDeviations_(0), coefs_(0) {
	attributeNumber_ = getDimensionNumber(dataBegin_, dataEnd_, inputOf_);
      }

      ~CorrelationFilter() {
	if(means_ != 0) delete [] means_;
	if(coefs_ != 0) delete [] coefs_;
	if(stdDeviations_ != 0) delete [] stdDeviations_;
      }

      CorrelationFilter& verbose(bool verbose = true) { verbose_ = verbose; return *this; }

      int getAttributeNumber() const { 
	return attributeNumber_;
      }

      template<typename AttributeIterator>
      double operator()(const AttributeIterator begin, const AttributeIterator end) const {
	if(means_ == 0) {
	  CorrelationFilter* filter = const_cast<CorrelationFilter*>(this);
	  filter->computeCorrelationCoefficients();
	}
	if(begin == end) return std::numeric_limits<double>::min();

	int n = attributeNumber_ + 1;
	double score = 0.;
	int target = attributeNumber_;
	for(AttributeIterator it = begin; it != end; ++it) {
	  score += coefs_[target * n + (*it)];
	}

	double denominator = 0.;
	for(AttributeIterator it1 = begin; it1 != end; ++it1) {
	  for(AttributeIterator it2 = begin; it2 != end; ++it2) {
	    int i = *it1, j = *it2;
	    if(i >= j)
	      denominator += coefs_[i*n+j];
	    else
	      denominator += coefs_[j*n+i];
	  }
	}

	return score / sqrt(denominator);
      }
    };

    template<typename DataIterator, typename InputOf, typename OutputOf>
    CorrelationFilter<DataIterator,InputOf,OutputOf> 
    make_correlation_filter(const DataIterator& dataBegin, const DataIterator& dataEnd, const InputOf& inputOf, const OutputOf& outputOf) {
      return CorrelationFilter<DataIterator,InputOf,OutputOf>(dataBegin, dataEnd, inputOf, outputOf);
    }
  }
}
