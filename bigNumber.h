#pragma once
#include <iostream>

#define BASE 1000000000
#define DIV_BY_ZERO 1

class bigNumber
{
private:
	int _size;
	unsigned int* _digits;
	int _sign;

public:
	bigNumber();
	bigNumber(const char* String);
	bigNumber(const bigNumber& RightHandValue);
	bigNumber(long long int RightHandValue);
	~bigNumber();

	char* getString();
	char* __str__(); // для вывода числа в Python
	char* __repr__();
	bool getNumberFromTextFile(const char* FileName);
	bool saveNumberToTextFile(const char* FileName);
	bool getNumberFromBinFile(const char* FileName);
	bool saveNumberToBinFile(const char* FileName);


	bigNumber& operator=(const bigNumber& RightHandValue);

	bigNumber operator+(const bigNumber& right) const;
	bigNumber operator-() const;
	bigNumber operator-(const bigNumber& right) const;
	bigNumber operator*(const bigNumber& right) const;
	bigNumber operator/(const bigNumber& right) const;
	bigNumber operator%(const bigNumber& right) const;
	bigNumber& operator^(const bigNumber& right) const;


	bool operator>(const bigNumber& B);
	bool operator>=(const bigNumber& B);
	bool operator<(const bigNumber& B);
	bool operator<=(const bigNumber& B);
	bool operator==(const bigNumber& B);
	bool operator!=(const bigNumber& B);

	// перегрузка операторов для Python
	bigNumber operator+(const int& right) const;
	bigNumber operator-(const int& right) const;
	bigNumber operator*(const int& right) const;
	bigNumber operator/(const int& right) const;
	bigNumber operator%(const int& right) const;

	bool operator>(const int& B);
	bool operator>=(const int& B);
	bool operator<(const int& B);
	bool operator<=(const int& B);
	bool operator==(const int& B);
	bool operator!=(const int& B);


	friend std::ostream& operator<<(std::ostream &out, bigNumber A);
	friend std::istream& operator>>(std::istream &is, bigNumber &A);

private:
	void _setSize(int size);
	unsigned int & operator[](int i);
	unsigned int operator[](int i) const;
	void _copy(const bigNumber &rhv);
	void _DelZeroes();
	long long int _compare(const bigNumber& B);
	void _shiftLeft(int s);


	bigNumber _sumAndSub(const bigNumber& left, const bigNumber& right) const;
	bigNumber _multiplication(const bigNumber A, const bigNumber B) const;
	bigNumber _dividing(const bigNumber& A, const bigNumber& B, bigNumber &remainder) const;

};

bigNumber Pow(const bigNumber& A, const bigNumber& B, bigNumber& modulus);