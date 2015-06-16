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

#include <map>
#include <set>
#include <utility>
#include <gamlException.hpp>


namespace gaml {
      
  namespace classification {

    /**
     * @short This computes a confusion matrix.
     */
    template<typename CLASS>
    class Confusion {
    public:
      
      typedef CLASS class_type;

    private:


      typedef std::map<std::pair<class_type,class_type>,unsigned int> matrix_type;
      typedef std::map<class_type,std::pair<unsigned int,unsigned int> > frequencies_type;

      // matrix[a,b] = probability that classifiers says b whereas real label is a.
      matrix_type matrix;

      // sums[a].first  = \sum_i matrix[a,i]
      // sums[b].second = \sum_i matrix[i,b]
      frequencies_type sums; 
      unsigned int nb_samples;


      friend std::ostream& operator<<(std::ostream& os, const Confusion<CLASS>& m) {
	typename matrix_type::const_iterator      miter,mend;
	typename frequencies_type::const_iterator fiter,fend;

	os << m.nb_samples << std::endl;

	os << m.sums.size() << std::endl;
	for(fiter = m.sums.begin(), fend = m.sums.end(); fiter != fend; ++fiter)
	  os << (*fiter).first << ' ' << (*fiter).second.first << ' ' <<  (*fiter).second.second << ' ';
	os << std::endl;

	os << m.matrix.size() << std::endl;
	for(miter = m.matrix.begin(), mend = m.matrix.end(); miter !=mend; ++miter)
	  os << (*miter).first.first << ' ' << (*miter).first.second << ' ' << (*miter).second << ' ';
	
	return os;
      }

      friend std::istream& operator>>(std::istream& is, Confusion<CLASS>& m) {
	class_type a,b;
	unsigned int x,y;
	unsigned int i,size;

	m.clear();

	is >> m.nb_samples;
	
	is >> size;
	for(i=0;i<size;++i) {
	  is >> a >> x >> y;
	  m.sums[a] = std::pair<unsigned int,unsigned int>(x,y);
	}

	is >> size;
	for(i=0;i<size;++i) {
	  is >> a >> b >> x;
	  m.matrix[std::pair<class_type,class_type>(a,b)] = x;
	}

	return is;
      }
      

      void checkSamples(const char* method) const {
	if(nb_samples==0)
	  throw exception::EmptyConfusionMatrix(std::string("in method Confusion::")+std::string(method));
      }

      unsigned int confus(const class_type& a,const class_type& b) const {
	std::pair<class_type,class_type> key(a,b);
	typename matrix_type::const_iterator coef = matrix.find(key);

	if(coef == matrix.end())
	  return 0;
	else
	  return (*coef).second;
      }

      unsigned int P(const char* method,
		     const class_type& positive,
		     const class_type& negative) const {
	unsigned int res = TP(positive,negative)+FN(positive,negative);
	if(res == 0)
	  throw exception::NoPositiveInData(std::string("in method Confusion::")+std::string(method));
	return res;
      }

      unsigned int N(const char* method,
		     const class_type& positive,
		     const class_type& negative) const {
	unsigned int res = TN(positive,negative)+FP(positive,negative);
	if(res == 0)
	  throw exception::NoNegativeInData(std::string("in method Confusion::")+std::string(method));
	return res;
      }

      unsigned int P_(const char* method,
		      const class_type& positive,
		      const class_type& negative) const {
	unsigned int res = TP(positive,negative)+FP(positive,negative);
	if(res == 0)
	  throw exception::NoPositivePrediction(std::string("in method Confusion::")+std::string(method));
	return res;
      }

      unsigned int N_(const char* method,
		      const class_type& positive,
		      const class_type& negative) const {
	unsigned int res = FN(positive,negative)+TN(positive,negative);
	if(res == 0)
	  throw exception::NoNegativePrediction(std::string("in method Confusion::")+std::string(method));
	return res;
      }


      static std::string percent(double x, bool display_zero=false) {
	std::ostringstream ostr;
	if(x>1)
	  x=1;
	else if(x<0)
	  x=0;
	int xx = (int)(x*1000+.5);
	if(display_zero || xx!=0) {
	  if(xx==1000)
	    ostr << "100.0%";
	  else
	    ostr << std::setw(4) << .1*xx << '%';
	}
	else
	  ostr << "     ";
	return ostr.str();
      }


    public:

      Confusion(void) : matrix(), nb_samples(0) {}
      Confusion(const Confusion<CLASS>& cp) 
	: matrix(cp.matrix),sums(cp.sums),nb_samples(cp.nb_samples) {}
      Confusion<CLASS>& operator=(const Confusion<CLASS>& cp) {
	if(this != &cp) {
	  matrix     = cp.matrix;
	  sums        = cp.sums;
	  nb_samples = cp.nb_samples;
	}
	return *this;
      }
      ~Confusion(void) {}

      

      /**
       * This displays the confusion matrix in text mode. The
       * serialization of your classes must take less that 6
       * characters for a nice display.
       */
      void display(std::ostream& os) const {
	unsigned int                                  i;
	std::set<class_type>                          used_classes = this->classes();
	typename std::set<class_type>::const_iterator truth,prediction,end;

	end = used_classes.end();
  
	std::cout << std::endl
		  << std::endl;
	std::cout << "           +";
	for(i = 0; i < used_classes.size(); ++i)
	  std::cout << "-------+";
	std::cout << std::endl;
	std::cout << "     truth |";
	for(truth = used_classes.begin(); truth != end; ++truth)
	  std::cout << std::setw(6) << *truth << " |";
	std::cout << "  sum " << std::endl;
	std::cout << "prediction /";
	for(i = 0; i < used_classes.size(); ++i)
	  std::cout << "========"; 
	std::cout << std::endl;

	for(prediction = used_classes.begin(); prediction != end; ++prediction) {
	  std::cout << "  |" << std::setw(6) << *prediction << " ||";
	  for(truth = used_classes.begin(); truth != end; ++truth)
	    std::cout << " " << percent(this->confusion(*truth,*prediction)) << " |";
	  std::cout << " " << percent(this->predictionFrequency(*prediction),true)
		    << std::endl;
	  std::cout << "  +-------||";
	  for(i = 0; i < used_classes.size(); ++i)
	    std::cout << "-------+"; 
	  std::cout << std::endl;
	}

	std::cout << "      sum   ";
	for(truth = used_classes.begin(); truth != end; ++truth)
	  std::cout << " " << percent(this->truthFrequency(*truth),true) << "  ";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
      }
      

      /**
       * This clears the confusion matrix coefficients.
       */
      void clear(void) {
	matrix.clear();
	sums.clear();
	nb_samples = 0;
      }

      /**
       * This returns the set of classes actually concerned
       * with non null coefficients in the confusion matrix.
       */
       std::set<class_type> classes(void) const {
	std::set<class_type> used;
	typename frequencies_type::const_iterator iter,end;

	for(iter = sums.begin(), end = sums.end();
	    iter != end;
	    ++iter)
	  used.insert(used.begin(),(*iter).first);

	return used;
      }

      /**
       * This considers an example for updating the confusion matrix.
       */
      template<typename Predictor, 
	       typename Input, 
	       typename Output,
	       typename ClassOfOutput>
      void update(const Predictor& f,
		  const Input& input,
		  const Output& output,
		  const ClassOfOutput& class_of) {
	class_type truth      = class_of(output);
	class_type prediction = class_of(f(input));

	std::pair<class_type,class_type> key(truth,prediction);
	typename matrix_type::iterator coef = matrix.find(key);

	if(coef == matrix.end())
	  matrix[key] = 1;
	else
	  (*coef).second++;

	typename frequencies_type::iterator coef_truth = sums.find(truth);
	if(coef_truth == sums.end()) {
	  sums[truth] = std::pair<unsigned int,unsigned int>(0,0);
	  coef_truth = sums.find(truth);
	}

	typename frequencies_type::iterator coef_prediction = sums.find(prediction);
	if(coef_prediction == sums.end()) {
	  sums[prediction] = std::pair<unsigned int,unsigned int>(0,0);
	  coef_prediction = sums.find(prediction);
	}

	(*coef_truth).second.first++;
	(*coef_prediction).second.second++;
	++nb_samples;
      }

      /**
       * This considers a set of axamples to update the confusion matrix.
       */
      template<typename Predictor, 
	       typename DataIterator,
	       typename InputOf, typename OutputOf,
	       typename ClassOfOutput>
      void update(const Predictor& f,
		  const DataIterator& begin, 
		  const DataIterator& end,
		  const InputOf& input_of,
		  const OutputOf& output_of, 
		  const ClassOfOutput& class_of) {
	for(DataIterator it = begin; it != end; ++it) 
	  update(f,input_of(*it),output_of(*it),class_of);
      }

      /**
       * This returns the frequency of class c in the
       * data.
       */
      const double truthFrequency(const class_type& c) const {
	checkSamples("truthFrequency");
	typename frequencies_type::const_iterator coef = sums.find(c);
	if(coef == sums.end()) 
	  return 0;
	else
	  return (*coef).second.first / (double)nb_samples;
      }

      /**
       * This returns the frequency of class c in the
       * prediction made from data.
       */
      const double predictionFrequency(const class_type& c) const {
	checkSamples("predictionFrequency");
	typename frequencies_type::const_iterator coef = sums.find(c);
	if(coef == sums.end()) 
	  return 0;
	else 
	  return (*coef).second.second / (double)nb_samples;
      }

      /**
       * This is the frequency of predicting class b while example
       * label actually belongs to class a.
       */
      double confusion(const class_type& a,const class_type& b) const {
	checkSamples("confusion");
	return confus(a,b) / (double)nb_samples;
      }

      unsigned int nbSamples(void) const {return nb_samples;}

      /**
       * @returns the number of true positives.
       */
      unsigned int TP(const class_type& positive,
		      const class_type& negative) const {
	return confus(positive,positive);
      }

      /**
       * @returns the number of true negatives.
       */
      unsigned int TN(const class_type& positive,
		      const class_type& negative) const {
	return confus(negative,negative);
      }

      /**
       * @returns the number of false positives.
       */
      unsigned int FP(const class_type& positive,
		      const class_type& negative) const {
	return confus(negative,positive);
      }

      /**
       * @returns the number of false negatives.
       */
      unsigned int FN(const class_type& positive,
		      const class_type& negative) const {
	return confus(positive,negative);
      }

      /**
       * @return TP / (TP + FN)
       */
      double sensitivity(const class_type& positive,
			 const class_type& negative) const {
	return TP(positive,negative)/(double)P("sensitivity",positive,negative);
      }

      /**
       * This returns sensitivity (alias).
       */
      double recall(const class_type& positive,
		    const class_type& negative) const {
	return TP(positive,negative)/(double)P("recall",positive,negative);
      }

      /**
       * @return FP / (FP + TN)
       */
      double fallOut(const class_type& positive,
		     const class_type& negative) const {
	return FP(positive,negative)/(double)N("fallOut",positive,negative);
      }

      /**
       * See also the precision method
       * @return (TP + TN) / all
       */
      double accuracy(const class_type& positive,
		      const class_type& negative) const {
	return (TP(positive,negative) + TN(positive,negative))
	  / (double)(P("accuracy",positive,negative) + V("accuracy",positive,negative));
      }

      /**
       * @return 1-fallOut
       */
      double specificity(const class_type& positive,
			 const class_type& negative) const {
	return 1-FP(positive,negative)/(double)N("specificity",positive,negative);
      }

      /**
       * See also the accuracy method.
       * @return TP / (TP + FP)
       */
      double precision(const class_type& positive,
		       const class_type& negative) const {
	return TP(positive,negative)/(double)P_("precision",positive,negative);
      }
      
      /**
       * @return TN / (TN + FN)
       */
      double negativePredictiveValue(const class_type& positive,
				     const class_type& negative) const {
	return TN(positive,negative)/(double)N_("negativePredictiveValue",positive,negative);
      }

      /**
       * @return FP / (FP + TP)
       */
      double falseDiscovery(const class_type& positive,
			    const class_type& negative) const {
	return FP(positive,negative)/(double)P_("falseDiscovery",positive,negative);
      }

      /**
       * @returns The Matthews correlation coefficient : MCC = (TP*TN - FP*FN)/sqrt(P*N*P_*N_), with P = TP+FN, N = FP+TN, P_ = TP+FP, N_ = FN+TN.
       */
      double MCC(const class_type& positive,
		 const class_type& negative) const {
	return (TP(positive,negative)*TN(positive,negative)
		+ FP(positive,negative)*FN(positive,negative))
	  /sqrt(P("MCC",positive,negative)
		*N("MCC",positive,negative)
		*P_("MCC",positive,negative)
		*N_("MCC",positive,negative));
      }

      
    };
  }

}
