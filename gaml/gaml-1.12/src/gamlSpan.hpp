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
#include <functional>
#include <iostream>
#include <algorithm>

namespace gaml {
  namespace span {

    template<typename INPUT> class Vector;
    template<typename INPUT> class Dictionary;

    /**
     * This is a matrix type for internal use.
     */
    class Matrix {
    public:
      typedef std::vector<double>::size_type index_type;

    private:
    
      index_type row_size,column_size;

    public:
      std::vector<double> coef;

      ~Matrix(void) {}

      Matrix(void) 
	: row_size(0), column_size(0), coef() {}

      Matrix(index_type row, index_type column)  
	: row_size(row), 
	  column_size(column), 
	  coef(row_size*column_size,0){}

      Matrix(const Matrix& m)
	: row_size(m.row_size), 
	  column_size(m.column_size), 
	  coef(m.coef){}

      Matrix& operator=(const Matrix& m) {
      
	if(&m != this) {
	  row_size = m.row_size;
	  column_size = m.column_size;
	  coef = m.coef;
	}

	return *this;
      }

      Matrix(const Matrix&& m)
	: row_size(m.row_size), 
	  column_size(m.column_size), 
	  coef(std::move(m.coef)){}

      Matrix& operator=(const Matrix&& m) {
      
	if(&m != this) {
	  row_size = m.row_size;
	  column_size = m.column_size;
	  coef = std::move(m.coef);
	}

	return *this;
      }

      void resize(index_type row, index_type column) {
	row_size = row;
	column_size = column;
	coef.resize(row_size*column_size);
      }

      void clear() {
	coef.clear();
	row_size = 0;
	column_size = 0;
      }

      bool cleared() {
	return row_size == 0 || column_size == 0;
      }

    

      double& operator()(index_type row, index_type column) {
	return coef[row*column_size+column];
      }

      const double& operator()(index_type row, index_type column) const {
	return coef[row*column_size+column];
      }
    
      void add_row_column(void) {
	Matrix tmp = *this;
	index_type row, column;

	row_size++;
	column_size++;
	coef.resize(row_size*column_size);

	for(row=0;row<tmp.row_size;++row) 
	  for(column=0;column<tmp.column_size;++column) 
	    (*this)(row,column) = tmp(row,column);
      }

      Matrix& operator*=(double arg) {
	typename std::vector<double>::iterator iter,iter_end;
      
	for(iter=coef.begin(),iter_end=coef.end();
	    iter!=iter_end;
	    ++iter)
	  *iter *= arg;

	return *this;
      }


      Matrix& operator/=(double arg) {
	typename std::vector<double>::iterator iter,iter_end;
      
    
	for(iter=coef.begin(),iter_end=coef.end();
	    iter!=iter_end;
	    ++iter)
	  *iter /= arg;


	return *this;
      }

      Matrix& operator+=(const Matrix& op2) {
	typename std::vector<double>::iterator iter1,iter_end;
	typename std::vector<double>::const_iterator iter2;

	for(iter1=coef.begin(),iter_end=coef.end(),iter2=op2.coef.begin();
	    iter1!=iter_end;
	    ++iter1,++iter2)
	  *iter1 += *iter2;
      
	return *this;
      }

      Matrix& operator-=(const Matrix& op2) {
	typename std::vector<double>::iterator iter1,iter_end;
	typename std::vector<double>::const_iterator iter2;

	for(iter1=coef.begin(),iter_end=coef.end(),iter2=op2.coef.begin();
	    iter1!=iter_end;
	    ++iter1,++iter2)
	  *iter1 -= *iter2;
      
	return *this;
      }

      bool operator==(const Matrix& op2) const {
	bool res;
	typename std::vector<double>::const_iterator iter1,iter_end;
	typename std::vector<double>::const_iterator iter2;
      
      
	for(res = true,iter1=coef.begin(),iter_end=coef.end(),iter2=op2.coef.begin();
	    res && (iter1!=iter_end);
	    ++iter1,++iter2)
	  res = *iter1 == *iter2;
      
	return res;
      }

      void multiply(const Matrix& op2, Matrix& res) const {

	typename std::vector<double>::iterator iter;
	double tmp;
	index_type i,j,k;

	res.resize(row_size,op2.column_size);

	for(i=0,iter=res.coef.begin();i<res.row_size;++i)
	  for(j=0;j<res.column_size;++j,++iter) {
	    tmp=0;
	    for(k=0;k<column_size;++k)
	      tmp += (*this)(i,k) * op2(k,j);
	    *iter = tmp;
	  }
      }

      void transpose(Matrix& res) const {
	index_type i,j;
	typename std::vector<double>::iterator iter;
     
	res.resize(column_size,row_size);
      
	for(i=0,iter=res.coef.begin();i<column_size;++i)
	  for(j=0;j<row_size;++j,++iter)
	    *iter = (*this)(j,i);
      }


      friend std::ostream& operator<<(std::ostream& os,const Matrix& m) {
      
	index_type i,j;
	os << m.row_size << ' ' << m.column_size << std::endl;
	for(i=0;i<m.row_size;++i, os << std::endl)
	  for(j=0;j<m.column_size;++j)
	    os << ' ' << m(i,j);

	return os;
      }

      friend std::istream& operator>>(std::istream& is,Matrix& m) {
      
	index_type i,j,r,c;
	char sep;

	is >> r >> c;

	m.resize(r,c);
	for(i=0;i<r;++i)
	  for(j=0;j<c;++j)
	    is >> m(i,j);
	is.get(sep);

	return is;
      }
    };


    template<typename INPUT> class Vector {
    private:
      const Dictionary<INPUT> * base;
      Matrix                    coefs;
      friend class Dictionary<INPUT>;

      Vector(const Dictionary<INPUT>& dico, const INPUT& x)
	: base(&dico),  coefs() {
	*this = x;
      }

    public:

      Vector() : base(nullptr), coefs() {}
      ~Vector() {}
      
      Vector(const Vector& c) : base(c.base), coefs(c.coefs) {}  
      Vector& operator=(const Vector& c) {
	if(this != &c) {
	  base  = c.base;
	  coefs = c.coefs;
	}
	return *this;
      }

      Vector(Vector&& c) : base(c.base), coefs(std::move(c.coefs)) {}

      Vector& operator=(Vector&& c) {
	if(this != &c) {
	  base  = c.base;
	  coefs = std::move(c.coefs);
	}
	return *this;
      }


      std::vector<double>& coefficients() {
	return coefs.coef;
      }

      void raz() {
	coefs.clear(); 
      }

      Vector& operator=(const INPUT& x) {
	Matrix k(base->element.size(),1);
	for(Matrix::index_type i = 0; i < base->element.size(); ++i)
	  k(i,0) = base->kernel(x,base->element[i]);
	base->inv_K.multiply(k,coefs);
	return *this;
      }

      Vector& operator+=(const Vector& v) {
	auto y = v.coefs.coef.begin();
	if(coefs.cleared()) {
	  base = v.base;
	  coefs.resize(base->element.size(),1);
	  for(auto& x : coefs.coef) x = *(y++);
	}
	else
	for(auto& x : coefs.coef) x += *(y++);
	return *this;
      }

      Vector& operator-=(const Vector& v) {
	auto y = v.coefs.coef.begin();
	if(coefs.cleared()) {
	  base = v.base;
	  coefs.resize(base->element.size(),1);
	  for(auto& x : coefs.coef) x = -*(y++);
	}
	else
	for(auto& x : coefs.coef) x -= *(y++);
	return *this;
      }

      Vector& operator*=(double a) {
	for(auto& x : coefs.coef) x *= a;
	return *this;
      }

      Vector& operator/=(double a) {
	for(auto& x : coefs.coef) x /= a;
	return *this;
      }

      double operator*(const Vector&  b) const {
	base->K.multiply(coefs, base->k_a);
	b.coefs.transpose(base->trans_b);
	base->trans_b.multiply(base->k_a,base->trans_b_k_a);
	return base->trans_b_k_a(0,0);
      }

      friend Vector operator+(const Vector&  a, const Vector&  b) {
	Vector res = a;
	res += b;
	return res;
      }

      friend Vector operator+(Vector&& a, const Vector&  b) {
	Vector res = std::move(a);
	res += b;
	return res;
      }

      friend Vector operator+(const Vector&  a, Vector&& b) {
	Vector res = std::move(b);
	res += a;
	return res;
      }

      friend Vector operator+(Vector&& a, Vector&& b) {
	Vector res = std::move(a);
	res += b;
	return res;
      }

      friend Vector operator-(const Vector&  a, const Vector&  b) {
	Vector res = a;
	res -= b;
	return res;
      }

      friend Vector operator-(Vector&& a, const Vector&  b) {
	Vector res = std::move(a);
	res -= b;
	return res;
      }

      friend Vector operator-(const Vector&  a, Vector&& b) {
	Vector res = std::move(b);
	res -= a;
	return res;
      }

      friend Vector operator-(Vector&& a, Vector&& b) {
	Vector res = std::move(a);
	res -= b;
	return res;
      }

      friend Vector operator*(const Vector&  x, double a) {
	Vector res = x;
	res *= a;
	return res;
      }

      friend Vector operator*(double a, const Vector&  x) {
	return x*a;
      }

      friend Vector operator*(Vector&&  x, double a) {
	Vector res = std::move(x);
	res *= a;
	return res;
      }

      friend Vector operator*(double a, Vector&&  x) {
	return std::move(x) * a;
      }

      friend Vector operator/(const Vector&  x, double a) {
	Vector res = x;
	res /= a;
	return res;
      }

      friend Vector operator/(Vector&&  x, double a) {
	Vector res = std::move(x);
	res /= a;
	return res;
      }
    
      friend std::ostream& operator<<(std::ostream& os,const Vector&  x) {
	os << x.coefs;
	return os;
      }

      friend std::istream& operator>>(std::istream& is,Vector&  x) {
	is >> x.coefs;
	return is;
      }
      
    };

    /**
     * @short Engels dictionary
     *
     * This is not thread safe (one dictionary per thread).<br>
     */
    template<typename INPUT>
    class Dictionary {
    public:
      typedef INPUT                   input_type;
      typedef std::vector<input_type> container_type;
      typedef Vector<INPUT>           vector_type;

      /**
       * The error tolerence.
       */
      double nu;

     
    private:

      friend vector_type;
      
      container_type element;
      Matrix inv_K,K;

    public:
      /**
       * This is the kernel that is used. It can be changed.
       */
      std::function<double (const input_type&,const input_type&)> kernel;

    private:

      // temporary variables....
      mutable Matrix res;
      mutable Matrix k;
      mutable Matrix trans_k;
      mutable Matrix trans_k_a;
      mutable Matrix a,aa,at;
      mutable Matrix k_a, trans_b, trans_b_k_a;
      mutable double delta,xx;
      mutable typename Matrix::index_type i,last_index;

      
    public:

      /**
       * @param nu_tolerance is the error tolerance for the spanning.
       * @param dot is the dot product, taking two const references on input_type and returning a double.
       */
      template<typename fctDotProduct>
      Dictionary(double nu_tolerance, const fctDotProduct& dot) 
	: nu(nu_tolerance), 
	  element(),
	  inv_K(1,1),
	  K(1,1),
	  kernel(dot) {}
      ~Dictionary(void) {}

      void clear() {
	element.clear();
	inv_K.resize(1,1);
	K.resize(1,1);
      }
      
      /**
       * @return The samples stored in the dictionary.
       */
      const container_type& container() const {return element;}

      /**
       * Returns a vector in the spanned space that corresponds to the
       * projection of x.
       */
      vector_type operator()(const INPUT& x) const {
	return vector_type(*this,x);
      }

      /**
       * Submit an example to the dictionary.
       * <b>Don't use null vector in first submission.</b>
       * @return true if example has been actually added.
       */
      bool submit(const INPUT& x) {
	xx = kernel(x,x);

	if(element.size()==0) {
	  element.push_back(x);
	  inv_K(0,0) = 1/xx;
	  K(0,0) = xx;

	  return true;
	}

	k.resize(element.size(),1);
       
	i=0;
	for(auto& elem : element) k(i++,0) = kernel(elem,x);
	k.transpose(trans_k);
	inv_K.multiply(k,a);
	trans_k.multiply(a,trans_k_a);
	delta = xx - trans_k_a(0,0);

	if(delta <= nu)
	  return false;

	last_index = element.size();
	element.push_back(x);

	// Update K
	K.add_row_column();
	K(last_index,last_index) = xx;
	for(i = 0; i < last_index; ++i) K(last_index,i) = K(i,last_index) = k(i,0);

	// Update inv_K

	a.transpose(at);
	a.multiply(at,aa);
	aa /= delta;
	inv_K += aa;
	inv_K.add_row_column();
	for(i = 0; i < last_index; ++i) inv_K(last_index,i) = inv_K(i,last_index) = -a(i,0)/delta;
	inv_K(last_index,last_index) = 1.0/delta;

	return true;
      }

      friend std::ostream& operator<<(std::ostream& os, const Dictionary<INPUT>& p) {
	os << p.nu << ' ' << p.element.size() << ' ';
	for(auto& elem : p.element) os << elem << ' ';
	os << p.inv_K << p.K;
	return os;
      }

      friend std::istream& operator>>(std::istream& is, Dictionary<INPUT>& p) {
	char sep;
	int size,i;
	INPUT x;

	is >> p.nu >> size;
	is.get(sep);
	for(i = 0; i < size; ++i) {
	  is >> x;
	  is.get(sep);
	  p.element.push_back(x);
	}
	is >> p.inv_K >> p.K;
	return is;
      }
    };
  }

  namespace by_default {
    template<typename INPUT>
    struct DotProduct {
      double operator()(const INPUT& x1, const INPUT& x2) const {
	return x1*x2;
      }
    };
  }

  namespace span {
    template<typename INPUT>
    Dictionary<INPUT> dictionary(double nu_tolerance, std::function<double (const INPUT&,const INPUT&)> dot = gaml::by_default::DotProduct<INPUT>()) {
      return Dictionary<INPUT>(nu_tolerance,dot);
    }	
  }


}
