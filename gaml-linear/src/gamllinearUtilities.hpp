#pragma once

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

namespace gaml {
  namespace linear {
    namespace data_matrix {

      void alloc(gsl_matrix*& Phi, gsl_vector*& Y,
		 gsl_vector*& mean_features, gsl_vector*& sigma_features,
		 unsigned int nb_samples,
		 unsigned int nb_features) {
	Phi = gsl_matrix_alloc(nb_samples,nb_features);
	Y   = gsl_vector_alloc(nb_samples);
	mean_features = gsl_vector_alloc(nb_features);
	sigma_features = gsl_vector_alloc(nb_features);
      }
			   
      /**
       * @param label_of have to return a double.
       */
      template<typename DataIter, 
	       typename fctPhi, 
	       typename InputOf, 
	       typename LabelOf>
      void fill(gsl_matrix* Phi, gsl_vector* Y,
		gsl_vector* mean_features, gsl_vector* sigma_features,
		double& mean_y,
		const DataIter& begin,
		const DataIter& end,
		const fctPhi& phi_of_input,
		const InputOf& input_of,
		const LabelOf& label_of,
		bool normalize_data) {

	if(normalize_data) {
	  unsigned int i = 0;
	  mean_y = 0;
	  for(auto it = begin; it != end; ++it, ++i) {
	    auto& d = *it;
	    gsl_vector_view row = gsl_matrix_row(Phi,i);
	    phi_of_input(&(row.vector),input_of(d));

	    auto y = label_of(d);
	    gsl_vector_set(Y,i,y);
	    mean_y += y;
	  }
	  mean_y /= double(Phi->size1);

	  // We now normalize the inputs
	  // computing the mean
	  gsl_vector_set_zero(mean_features);
	  for(unsigned int i = 0 ; i < Phi->size1; ++i) 
	    for(unsigned int j = 0 ; j < Phi->size2; ++j) 
	      gsl_vector_set(mean_features, j, 
			     gsl_vector_get(mean_features, j) +
			     gsl_matrix_get(Phi, i, j));
	  gsl_vector_scale(mean_features, 1.0 / Phi->size1);
	  // the variance
	  gsl_vector_set_zero(sigma_features);
	  for(unsigned int i = 0 ; i < Phi->size1; ++i)
	    for(unsigned int j = 0 ; j < Phi->size2; ++j) {
	      double tmp = gsl_matrix_get(Phi, i, j) - gsl_vector_get(mean_features,j);
	      gsl_vector_set(sigma_features, j,
			     gsl_vector_get(sigma_features, j) +
			     tmp * tmp);
	    }
	  for(unsigned int j = 0 ; j < sigma_features->size ; ++j) {
	    double sigma = gsl_vector_get(sigma_features, j);
	    gsl_vector_set(sigma_features, j, sqrt(sigma));
	  }

	  for(unsigned int i = 0 ; i < Phi->size1; ++i)
	    for(unsigned int j = 0 ; j < Phi->size2; ++j) {
	      double mu = gsl_vector_get(mean_features, j);
	      double sigma = gsl_vector_get(sigma_features, j);
	      if(sigma == 0) {
		// Don't touch Phi[i,j]
		gsl_vector_set(mean_features, j, 0.0);
		gsl_vector_set(sigma_features, j, 1.0);
	      }
	      else {
		gsl_matrix_set(Phi, i, j,
			       (gsl_matrix_get(Phi, i, j) - mu)/sigma);
	      }
	    }
	
	  // Center the output
	  for(unsigned int i = 0 ; i < Y->size; ++i)
	    gsl_vector_set(Y, i, gsl_vector_get(Y, i) - mean_y);
	}
	else {
	  unsigned int i = 0;
	  for(auto it = begin; it != end; ++it, ++i) {
	    auto& d = *it;
	    gsl_vector_view row = gsl_matrix_row(Phi,i);
	    phi_of_input(&(row.vector),input_of(d));
	    gsl_vector_set(Y,i,label_of(d));
	  }

	  // We now fill in the mean and sigma matrices
	  gsl_vector_set_zero(mean_features);
	  gsl_vector_set_all(sigma_features, 1.0);
	  mean_y = 0;


	}
      }
    }
  }
}

