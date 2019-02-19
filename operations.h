#ifndef OPERATIONS_H
#define OPERATIONS_H

#include<type_traits>
#include<list>
#include<future>
#include<unistd.h>

#include"matrix.h"


template<typename T, typename U>
struct op_traits {
	typedef decltype(T() + U()) sum_type;
	typedef decltype(T() * U()) prod_type;
};

/*
This class has been developed in the same reasoning of mproxy, basically it holds all the matrices until
the operation has to be performed(when the * is reached) partialy copied from mproxy
*/
template<typename T, unsigned h, unsigned w>
class sproxy {
public:

    static constexpr unsigned H = h;
    static constexpr unsigned W = w;

	//Implicit conversion to matrix with non static dimensions
    operator matrix<T>() {
        resolve();
        matrix_wrap<T> lhs = matrices.front(), rhs = matrices.back();
        const unsigned height = lhs.get_height();
        const unsigned width = lhs.get_width();
        assert(lhs.get_height() == rhs.get_height() && lhs.get_width() == rhs.get_width());
        matrix<T> result(height, width);
        for (unsigned i = 0; i != height; i++){
            for (unsigned j = 0; j != width; j++) {
                result(i, j) = lhs(i, j) + rhs(i, j);
            }
		}
        std::cout<< "sum conversion" <<std::endl;
        return result;
    }

	//Implicit conversion operators to matrix with static dimensions
    template<unsigned h2, unsigned w2>
    operator matrix<T, h2, w2>() {
        static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
        assert(h2 == get_height() && w2 == get_width());
        resolve();
        matrix_wrap<T> lhs = matrices.front(), rhs = matrices.back();
        assert(lhs.get_height() == rhs.get_height() && lhs.get_width() == rhs.get_width());
        matrix<T, h2, w2> result;
        for (unsigned i = 0; i != h2; i++){
            for (unsigned j = 0; j != w2; j++) {
                result(i, j) = lhs(i, j) + rhs(i, j);
            }
		}
        return result;
    }

    unsigned get_height() const { return matrices.front().get_height(); }

    unsigned get_width() const { return matrices.front().get_width(); }

    template<typename U, class L, class R>	friend sproxy<U, matrix_ref<U, L>::H, matrix_ref<U, R>::W>    
    operator+(const matrix_ref<U, L> &lhs, const matrix_ref<U, R> &rhs); //both the cases are friend with the matrix in order to access add method

    template<typename U, unsigned h2, unsigned w2, class R> friend sproxy<U, h2, matrix_ref<U, R>::W>
    operator+(sproxy<U, h2, w2> &&lhs, const matrix_ref<U, R> &rhs);

	//mproxy must be able to access the sproxy private methods!!
    template<typename U, unsigned h2, unsigned w2>
    friend class mproxy;

	//This is the same problem I had with mproxy last assignment
    template<typename U, unsigned h2, unsigned w2>
    friend class sproxy;

    sproxy(sproxy<T, h, w> &&X) = default;

private:
    sproxy() = default;
	
	//Copy constructor
    template<unsigned w2>
    sproxy(sproxy<T, h, w2> &&X) : matrices(std::move(X.matrices)) {}

    template<class matrix_type>
    void add(matrix_ref<T, matrix_type> mat) {
        matrices.emplace_back(mat);
    }


    void resolve() { 
		while (matrices.size() > 2) 
            resolve_one(); 
	}

    void resolve_one() {
        typename std::list<matrix_wrap<T>>::iterator lhs = matrices.begin();
        typename std::list<matrix_wrap<T>>::iterator rhs = lhs;
        rhs++;
        typename std::list<matrix_wrap<T>>::iterator result = matrices.emplace(lhs, matrix<T>(lhs->get_height(), lhs->get_width()));
		sum(*result, *lhs, *rhs);
		matrices.erase(lhs);
        matrices.erase(rhs);
    }

    void sum(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
        const unsigned height = lhs.get_height();
        const unsigned width = lhs.get_width();
		for (unsigned i = 0; i != height; i++){
            for (unsigned j = 0; j != width; j++)
                result(i, j) = lhs(i, j) + rhs(i, j);
		}
    }

    std::list<matrix_wrap<T>> matrices;
};

/*
In the previous assignment we permitted to have sum between different types, however here we use a proxy so
that means that there wouldn't be a omogeneos type in the list of matrices so I decided not to permit sum between different type 
*/

//Case when a chain of sums begins
template<typename T, class L, class R>
sproxy<T, matrix_ref<T, L>::H, matrix_ref<T, R>::W>
operator+(const matrix_ref<T, L> &lhs, const matrix_ref<T, R> &rhs) {
    static_assert((matrix_ref<T, L>::W * matrix_ref<T, R>::W == 0 || matrix_ref<T, L>::W == matrix_ref<T, R>::W) &&
            		(matrix_ref<T, L>::H * matrix_ref<T, R>::H == 0 || matrix_ref<T, L>::H == matrix_ref<T, R>::H) ,
                  "Can't do the sum with different dimensions!");
    if (lhs.get_height() != rhs.get_height() || lhs.get_width() != rhs.get_width())
        throw std::domain_error("Can't do the sum with different dimensions!");
    const unsigned height = lhs.get_height();
    const unsigned width = lhs.get_width();
    sproxy<T, matrix_ref<T, L>::H, matrix_ref<T, R>::W> result;
    result.add(lhs);
    result.add(rhs);
    return result;
}

//Case when the sum chain countinues(temp sproxy and matrix)
template<typename T, unsigned h, unsigned w, class R>
sproxy<T, h, matrix_ref<T, R>::W>
operator+(sproxy<T, h, w> &&lhs, const matrix_ref<T, R> &rhs) {
    static_assert((w * matrix_ref<T, R>::W == 0 || w == matrix_ref<T, R>::W) && (h * matrix_ref<T, R>::H == 0 || h == matrix_ref<T, R>::H),
            "Can't do the sum with different dimensions!");
    if (lhs.get_height() != rhs.get_height() || lhs.get_width() != rhs.get_width())
        throw std::domain_error("Can't do the sum with different dimensions!");
    sproxy<T, h, matrix_ref<T, R>::W> result(std::move(lhs));
    result.add(rhs);
    return result;
}

/*
This is needed when the calculus is performed, it basically returns the result of a matrix operation performed by the implicit conversion 
redefinition in the operation, basically is the operation the thread will execute, which force the computation because it will trigger the implicit conversion
and will return a type matrix<T>!
It's a functor
*/

template<typename T, unsigned h, unsigned w>
struct thread_impl_conv {
    static matrix<T> compute(sproxy<T, h, w> &&sp) {
        std::cout<<"Thready here" << std::endl;
        return sp;
    }
};

/*
Proxy used for the multiplication precedence and parallel multiplication
*/
template<typename T,unsigned h, unsigned w>
class mproxy {
	public:
	
	static constexpr unsigned H = h;
	static constexpr unsigned W = w;


	
	operator matrix<T>() {
		resolve();
		matrix_wrap<T> lhs = matrices.front(), rhs=matrices.back();
		const unsigned height = lhs.get_height();
		const unsigned width = rhs.get_width();
		const unsigned span = lhs.get_width();
		assert(span == rhs.get_height());
		matrix<T> result(height, width);
		for (unsigned i = 0; i != height; ++i)
			for (unsigned j = 0; j != width; ++j) {
				result(i, j) = 0;
				for (unsigned k = 0; k != span; ++k) 
					result(i, j) += lhs(i, k)*rhs(k, j);
				}
				
		std::cerr << "product conversion\n";
		return result;				
	}
	
	template<unsigned h2, unsigned w2>
	operator matrix<T, h2, w2>() {
		static_assert((h == 0 || h == h2) && (w == 0 || w == w2), "sized product conversion to wrong sized matrix");
		assert(h2 == get_height() && w2 == get_width());
		resolve();
		matrix_wrap<T> lhs = matrices.front(), rhs = matrices.back();
		const unsigned span = lhs.get_width();
		assert(span == rhs.get_height());
		matrix<T, h2, w2> result;
		for (unsigned i = 0; i != h2; ++i)
			for (unsigned j = 0; j != w2; ++j) {
				result(i, j) = 0;
				for (unsigned k = 0; k != span; ++k) 
					result(i, j) += lhs(i, k)*rhs(k, j);
				}
		
		std::cerr << "sized product conversion\n";
		return result;				
	}
	
	unsigned get_height() const { return matrices.front().get_height(); }
	unsigned get_width() const { return matrices.back().get_width(); }
	
		
	template<typename U, class LType, class RType>
	friend mproxy<U, matrix_ref<U,LType>::H, matrix_ref<U,RType>::W> 
	operator * (const matrix_ref<U,LType>& lhs, const matrix_ref<U,RType>& rhs);
	
	template<typename U, unsigned h2, unsigned w2, class RType>
	friend mproxy<U,h2,matrix_ref<U,RType>::W> 
	operator * (mproxy<U,h2,w2>&& lhs, const matrix_ref<U,RType>& rhs);

    //The new cases of multiplication have to be friend too!!
    template<typename U, unsigned h1, unsigned w1, unsigned h2, unsigned w2>
    friend mproxy<U, h1, w2>
    operator * (sproxy<U, h1, w1> &&lhs, sproxy<U, h2, w2> &&rhs);

    template<typename U, unsigned h1, unsigned w1, unsigned h2, unsigned w2>
	friend mproxy<U, h1, w2>
    operator * (mproxy<U, h1, w1> &&lhs, sproxy<U, h2, w2> &&rhs);

    
    mproxy(mproxy<T,h,w>&& X) = default;
	
	//mproxy must be friend with itself
    template<typename U, unsigned h1, unsigned w1>
    friend class mproxy;

    private:
	
	mproxy() {};
	
	template<unsigned w2>
	mproxy(mproxy<T,h,w2>&& X) : matrices(std::move(X.matrices)), sizes(std::move(X.sizes)) {}
	
	template<class matrix_type>
	void add(matrix_ref<T,matrix_type> mat) {
		matrices.emplace_back(mat);
		sizes.push_back(mat.get_width());
	}
	
	
	void resolve() { while(matrices.size()>2) resolve_one(); }
	void resolve_one() {
		typename std::list<matrix_wrap<T>>::iterator lhs = find_max();
		typename std::list<matrix_wrap<T>>::iterator rhs = lhs;
		++rhs;
		typename std::list<matrix_wrap<T>>::iterator result = matrices.emplace(lhs,matrix<T>(lhs->get_height(),rhs->get_width()));
		do_multiply(*result,*lhs,*rhs);
		matrices.erase(lhs);
		matrices.erase(rhs);
	}
	
	typename std::list<matrix_wrap<T>>::iterator find_max() {
		typename std::list<matrix_wrap<T>>::iterator mat_iter = matrices.begin();
		typename std::list<matrix_wrap<T>>::iterator mat_max = mat_iter;
		std::vector<unsigned>::iterator size_iter = sizes.begin();
		std::vector<unsigned>::iterator last = --(sizes.end());
		unsigned size_max = *size_iter;
		while (size_iter != last) {
			if(*size_iter > size_max) {
				size_max = *size_iter;
				mat_max = mat_iter;
			}
			++mat_iter;
			++size_iter;
		}
		return mat_max;
	}
	
	void do_multiply(matrix_wrap<T> result, matrix_wrap<T> lhs, matrix_wrap<T> rhs) {
		const unsigned height = result.get_height();
		const unsigned width = result.get_width();
		const unsigned span = lhs.get_width();
		assert(span == rhs.get_height());
		for (unsigned i = 0; i != height; ++i)
			for (unsigned j = 0; j != width; ++j) {
				result(i,j) = 0;
				for (unsigned k = 0; k != span; ++k) 
					result(i, j) += lhs(i, k)*rhs(k, j);
				}					
	}
	
	
	std::list<matrix_wrap<T>> matrices;
	std::vector<unsigned> sizes;
};
	
template<typename T, class LType, class RType>
mproxy<T, matrix_ref<T,LType>::H, matrix_ref<T,RType>::W> 
operator * (const matrix_ref<T,LType>& lhs, const matrix_ref<T,RType>& rhs) {
	static_assert(matrix_ref<T,LType>::W*matrix_ref<T,LType>::H==0 || matrix_ref<T,LType>::W==matrix_ref<T,LType>::H,
			"Can't do the sum with different dimensions!");
	if (lhs.get_width()!=rhs.get_height())
		throw std::domain_error("Can't do the sum with different dimensions!");
	mproxy<T, matrix_ref<T,LType>::H, matrix_ref<T,RType>::W> result;
	result.add(lhs);
	result.add(rhs);
	return result;
}


template<typename T, unsigned h, unsigned w, class R>
mproxy<T,h,matrix_ref<T,R>::W> 
operator * (mproxy<T,h,w>&& lhs, const matrix_ref<T,R>& rhs) {
	static_assert(w*matrix_ref<T, R>::H==0 || w==matrix_ref<T, R>::H, 
			"Can't do the sum with different dimensions!");
	if (lhs.get_width() != rhs.get_height())
		throw std::domain_error("Can't do the sum with different dimensions!");
	mproxy<T,h,matrix_ref<T,R>::W> result(std::move(lhs));
	result.add(rhs);
	return result;
}

//This case is when we have a chain ff sum
template<typename T, unsigned h, unsigned w, unsigned h1, unsigned w1>
mproxy<T, h, w1>
operator*(sproxy<T, h, w> &&lhs, sproxy<T, h1, w1> &&rhs) {
    static_assert(w * h1 == 0 || w == h1, "Can't do the multiplication with different dimensions!");
    if (lhs.get_width() != rhs.get_height())
        throw std::domain_error("Can't do the multiplication with different dimensions!");
    
	auto f1 = std::async(std::launch::async, thread_impl_conv<T, h, w>::compute, std::move(lhs));
    auto f2 = std::async(std::launch::async, thread_impl_conv<T, h1, w1>::compute, std::move(rhs));

    matrix<T> A = f1.get();
    matrix<T> B = f2.get();

    mproxy<T, h, w1> result;
    result.add(A);
    result.add(B);
    return result;
}


template<typename T, unsigned h, unsigned w, unsigned h1, unsigned w1>
mproxy<T, h, w1>
operator*(mproxy<T, h, w> &&lhs, sproxy<T, h1, w1> &&rhs) {
    static_assert(w * h1 == 0 || w == h1, "Can't do the multiplication with different dimensions!");
    if (lhs.get_width() != rhs.get_height())
        throw std::domain_error("Can't do the multiplication with different dimensions!");
    auto f1 = std::async(std::launch::async, thread_impl_conv<T, h1, w1>::compute, std::move(rhs));
	matrix<T> A = f1.get();
    mproxy<T, h, w1> result(std::move(lhs));
    result.add(A);
    return result;
}
	
//In order to compile this program on Linux I had to change makefile options and add the -pthread option

#endif // OPERATIONS_H 
