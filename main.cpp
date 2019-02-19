#include<iostream>

#include"matrix.h"
#include"matrix_wrap.h"
#include"operations.h"


int main() {
matrix<int> A(4,4);
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  A(i,j) = (i+1)*10+j+1;
	  
	  
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << A(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;


auto B=A.transpose();
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << B(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<int> C = A+B+A;
for (int i=0; i!=4; ++i) {
	for(int j=0; j!=4; ++j)
		std::cout << C(i,j) << ' ';
	std::cout << '\n';
}
std::cout << std::endl;

matrix<int> D(4,4);
for (int i=0; i!=4; ++i)
	for(int j=0; j!=4; ++j)
	  D(i,j) = 1;


std::cout<<"======= SUM PARENTHESIS TESTS ========" <<std::endl;

for(int i = 0; i != 4; ++i){
	for(int j = 0; j != 4; ++j)
		std::cout << A(i, j) << " ";
	std::cout<<std::endl;
 }
std::cout << '\n';
std::cout << '\n';
for(int i = 0; i != 4; ++i){
	for(int j = 0; j != 4; ++j)
		std::cout << B(i, j) << " ";
	std::cout<<std::endl;
}
std::cout << '\n';
std::cout << '\n';
for(int i = 0; i != 4; ++i){
	for(int j = 0; j != 4; ++j)
		std::cout << C(i, j) << " ";
	std::cout<<std::endl;
}
std::cout << '\n';
std::cout << '\n';
for(int i = 0; i != 4; ++i){
	for(int j = 0; j != 4; ++j)
		std::cout << D(i, j) << " ";
	std::cout<<std::endl;
}
matrix<int> PS = (A+A) * (+A);
std::cout << '\n';
std::cout << '\n';
for(int i = 0; i != 4; ++i){
	for(int j = 0; j != 4; ++j)
		std::cout << PS(i, j) << " ";
	std::cout<<std::endl;
}
return 0;
}
