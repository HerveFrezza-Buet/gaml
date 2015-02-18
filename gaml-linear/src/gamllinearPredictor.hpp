#pragma once

#include <functional>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <map>

namespace gaml {
  namespace linear {

    template<typename X>
    class Predictor {

    public:
      typedef X input_type;
      typedef double output_type;

    private:
      std::function<void(gsl_vector*, const X& x)> phi;
      gsl_vector* phix;

    public: 
      std::map<unsigned int, double> w;
      double offset_output;

      Predictor() : phi(0), phix(0), offset_output(0) {
      }

      template<typename fctPhi>
      Predictor(const fctPhi& fct_phi, double offset, unsigned int nb_features) : phi(fct_phi), offset_output(offset) {
	phix = gsl_vector_alloc(nb_features);
      }

      Predictor (const Predictor<X> &other) : phi(other.phi), w(other.w), offset_output(other.offset_output)  {
	phix = gsl_vector_alloc(other.phix->size);
	gsl_vector_memcpy(phix, other.phix);
      }

      Predictor& operator= (const Predictor<X> &other) {
	if(this != &other) {
	  phi = other.phi;
	  gsl_vector_free(phix);
	  phix = gsl_vector_alloc(other.phix->size);
	  gsl_vector_memcpy(phix, other.phix);
	  w = other.w;
	  offset_output = other.offset_output;
	}
	return *this;
      }

      ~Predictor() {
	gsl_vector_free(phix);
      }

      output_type operator() (const input_type &x) const {
	phi(phix, x);
	double y = 0;
	for(auto& kv: w) 
	  y += gsl_vector_get(phix, kv.first) * kv.second;

	return y + offset_output;
      }
  
    };

  }
}
