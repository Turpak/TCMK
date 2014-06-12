#include "bigNumber.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
using std::ifstream;
using std::ofstream;

bigNumber::bigNumber()
{// конструктор по умолчанию обнуляет число
	this->_sign = 0;
	this->_elements.push_back(0);
}

bigNumber::bigNumber(char* text)
{// конструктор из строки
	if (!text)
		return;

	int strSize = strlen(text);
	int strSign = 0;
	if (text[0] == '-')
	{
		strSize--;
		strSign = 1;
	}

	// проверка входной строки
	const char* pStr = text + strSign;
	while (*pStr)
	{
		if (*pStr < '0' || *pStr > '9')
		{
			// если в строке есть нечисловой символ - обнуляем и выходим
			this->_elements.push_back(0);
			this->_sign = 0;
			return;
		}
		pStr++;
	}

	// разбиваем строку на части по 9 символов
	for (int i = 0; i < (strSize + strSign) / 9; i++)
	{
		pStr -= 9;
		char splStr[10];
		memcpy(splStr, pStr, 9);
		splStr[9] = '\0'; // получили очередную строку из 9 символов
		unsigned int digitI = atol(splStr);
		this->_elements.push_back(digitI);
	}

	// обрабатываем оставшиеся символы (если длина строки не кратна 9)
	char ost[10];
	memset(ost, 0, 10);
	memcpy(ost, text + strSign, pStr - text - strSign); // получили строку - необработанная часть
	if (strlen(ost) > 0)
	{
		unsigned int lastDigit = atol(ost);
		this->_elements.push_back(lastDigit);
	}

	this->_sign = strSign;
	this->_DelZeroes();
}

bigNumber::bigNumber(long long int value)
{
	this->_sign = 0;
	if (value < 0)
	{
		this->_sign = 1;
		value = -value;
	}
	do
	{
		this->_elements.push_back(value % BASE);
		value = value / BASE;
	} while (value != 0);
}


char* bigNumber::getString()
{// возвращает строку, в которой записано число в 10-ричной системе счисления
	char* strBuffer = new char[this->_elements.size() * 9 + 1 + this->_sign]();
	char* pString = strBuffer + this->_elements.size() * 9 + this->_sign; // указатель на текущую позицию для записи числа

	for (int i = 0; i < this->_elements.size(); i++)
	{
		// получаем строковое представление очередного разряда
		char splStr[10];
		sprintf(splStr, "%09u", this->_elements[i]);

		pString -= 9;
		memcpy(pString, splStr, 9);
	}

	// удаление ведущих нулей
	while (*pString == '0' && *(pString + 1))
		pString++;

	if (this->_sign)
	{
		pString--;
		*pString = '-';
	}

	char* string = new char[strlen(pString) + 1]();
	strcpy(string, pString);
	delete[] strBuffer;

	return string;
}

bool bigNumber::getNumberFromTextFile(const char* filename)
{
	ifstream Text_file(filename);
	if (Text_file.fail())
		return false;

	Text_file.seekg(0, std::ios::end);
	int SizeOfFile = Text_file.tellg();
	Text_file.seekg(0, std::ios::beg);

	char* string = new char[SizeOfFile + 1]();
	Text_file >> string;
	Text_file.close();

	*this = bigNumber(string);
	delete[] string;
	return true;
}

bool bigNumber::saveNumberToTextFile(const char* filename)
{
	ofstream Result_file(filename);
	if (Result_file.fail())
		return false;

	char* string = this->getString();
	Result_file << string;
	delete[] string;
	Result_file.close();

	return true;
}

bool bigNumber::saveNumberToBinFile(const char* filename)
{// будем считать, что в бинарный файл необходимо записать число по основанию 256
	// то есть необходимо перейти от BASE к 256 (256 - потому что максимальное значение байта - 255)
	// для этого надо находить остатки от деления на 256
	// пока число не уменьшиться до 0

	ofstream Result_file(filename, std::ios::binary);
	if (Result_file.fail())
		return false;

	bigNumber temp = *this; // число, которое будем делить на 256
	bigNumber b256 = 256; // чтобы каждый раз не вызывать конструктор
	bigNumber b0 = (long long int)0;

	while (temp != b0)
	{
		bigNumber remainder;
		temp = _dividing(temp, b256, remainder); // делим temp на 256, в remainder - остаток

		// то, что осталось в remainder - необходимо записать в файл, точнее самый младший разряд
		// ведь остаток от деления на 256 при BASE > 256 будет занимать один разряд
		unsigned char ost = remainder[0];
		Result_file.write((char*)&ost, 1);
	}

	Result_file.close();
	return true;
}

bool bigNumber::getNumberFromBinFile(const char* filename)
{// будем считать, что в бинарном файле записаны разряды числа по модулю 256.
	// таким образом, чтобы считать число из бинарного файла необходимо каждый байт 
	// умножить на 256 ^ i, где i - позиция байта в файле и всё это сложить
	ifstream Bin_file(filename, std::ios::binary);

	if (Bin_file.fail())
		return false;

	Bin_file.seekg(0, std::ios::end);
	int SizeOfFile = Bin_file.tellg();
	Bin_file.seekg(0, std::ios::beg);

	// считываем содержимое файла
	unsigned char* fileContent = new unsigned char[SizeOfFile];
	Bin_file.read((char*)fileContent, SizeOfFile);
	Bin_file.close();

	// переведём содержимое файла к основанию BASE
	bigNumber res;
	bigNumber b256 = 1;
	for (int i = 0; i < SizeOfFile; i++)
	{
		unsigned int fi = fileContent[i];
		res = res + b256 * fi;
		b256 = b256 * 256;
	}
	*this = res;
	return true;
}

void bigNumber::_DelZeroes()
{
	while (this->_elements.size() > 1 && this->_elements.back() == 0)
		this->_elements.pop_back();

	if (this->_elements.size() == 1 && this->_elements[0] == 0)
		this->_sign = 0;
}

long long int bigNumber::_compare(const bigNumber& B)
{
	// функция возвращает
	// 0 - если числа равны,
	// >0 - если this больше
	// <0 - если this меньше
	int thisSign = 1;
	if (this->_sign == 1)
		thisSign = -1;

	if (this->_sign != B._sign)
		return thisSign;

	if (this->_elements.size() > B._elements.size())
		return thisSign;

	if (this->_elements.size() < B._elements.size())
		return -thisSign;

	int i = this->_elements.size() - 1;

	while (this->_elements[i] == B[i] && i > 0)
	{
		i--;
	}
	return ((long long int) this->_elements[i] - (long long int)B[i])*thisSign;
}

void bigNumber::_shiftLeft(int s)
{// сдвигает число на s разрядов
	std::vector<unsigned int> newElements;

	newElements.insert(newElements.end(), s, 0);
	newElements.insert(newElements.end(), this->_elements.begin(), this->_elements.end());

	this->_elements = newElements;
}

bigNumber bigNumber::_sum(const bigNumber& A, const bigNumber& B) const
{// скложение двух чисел, причём A - не короче B
	bigNumber res;
	res._elements.resize(A._elements.size() + 1);

	unsigned int carry = 0;
	// прибавляем число меньшей размерности к числу большей размерности
	for (int i = 0; i < B._elements.size(); i++)
	{
		unsigned int tmp = A[i] + B[i] + carry;
		res[i] = tmp % BASE;
		carry = tmp / BASE;
	}

	// добавляем перенос к оставшейся части более длинного числа
	for (int i = B._elements.size(); i < A._elements.size(); i++)
	{
		unsigned int tmp = A[i] + carry;
		res[i] = tmp % BASE;
		carry = tmp / BASE;
	}
	res[A._elements.size()] = carry;
	res._sign = A._sign;
	res._DelZeroes();
	return res;
}

bigNumber bigNumber::_sub(const bigNumber& A, const bigNumber& B) const
{// вычитание двух чисел
	bigNumber res;
	res._elements.resize(A._elements.size());

	unsigned int carry = 0;
	for (int i = 0; i < B._elements.size(); i++)
	{
		int tmp = A[i] - B[i] - carry;
		carry = 0;
		if (tmp < 0)
		{
			carry = 1;
			tmp += BASE;
		}
		res[i] = tmp;
	}

	for (int i = B._elements.size(); i < A._elements.size(); i++)
	{
		int tmp = A[i] - carry;
		carry = 0;
		if (tmp < 0)
		{
			carry = 1;
			tmp += BASE;
		}
		res[i] = tmp;
	}
	res._sign = A._sign;
	res._DelZeroes();
	return res;
}

bigNumber bigNumber::_dividing(const bigNumber& A, const bigNumber& B, bigNumber &remainder) const
{// возвращает целую часть от деления, в remainder - остаток
	remainder = A;
	remainder._sign = 0;

	bigNumber divider = B;
	divider._sign = 0;

	if (divider == bigNumber((long long int) 0))
	{
		remainder = (long long int) 0;
		return bigNumber((long long int)(0));
	}

	if (remainder < divider)
	{
		remainder = A;
		return bigNumber((long long int) 0);
	}


	// деление на одноразрядное число
	// ОЧЕНЬ СИЛЬНО ускоряет работу с бинарными файлами (точнее, при записи в бинарный файл)
	if (divider._elements.size() == 1)
	{
		bigNumber res;
		res._elements.resize(A._elements.size());
		unsigned long long int carry = 0;
		for (int i = A._elements.size() - 1; i >= 0; i--)
		{
			unsigned long long int temp = carry;
			temp *= BASE;
			temp += A[i];
			res[i] = temp / divider[0];
			carry = (unsigned long long int)  temp - (unsigned long long int) res[i] * (unsigned long long int) divider[0];
		}
		remainder._sign = (!A._sign != B._sign);
		remainder._elements.clear();
		remainder._elements.push_back(carry);
		remainder._DelZeroes();
		res._sign = (!A._sign != B._sign);
		res._DelZeroes();
		return res;
	}


	bigNumber res;
	res._elements.resize(A._elements.size() - B._elements.size() + 1);

	for (int i = A._elements.size() - B._elements.size() + 1; i; i--)
	{
		long long int qGuessMax = BASE; // для того, чтобы qGuessMin могло быть равно BASE - 1
		long long int qGuessMin = 0;
		long long int qGuess = qGuessMax;

		// цикл - подбор бинарным поиском числа qGuess
		while (qGuessMax - qGuessMin > 1)
		{
			qGuess = (qGuessMax + qGuessMin) / 2;

			// получаем tmp = qGuess * divider * BASE^i;
			bigNumber tmp = divider * qGuess;
			tmp._shiftLeft(i - 1);	// сдвигает число на (i - 1) разрядов вправо
			// то есть, по сути, это умножение на BASE^(i - 1)
			if (tmp > remainder)
				qGuessMax = qGuess;
			else
				qGuessMin = qGuess;
		}
		bigNumber tmp = divider * qGuessMin;
		tmp._shiftLeft(i - 1); // умножение на BASE ^ (i - 1)
		remainder = remainder - tmp;
		res[i - 1] = qGuessMin;
	}

	res._sign = (A._sign != B._sign);
	remainder._sign = (A._sign != B._sign);
	remainder._DelZeroes();
	res._DelZeroes();

	return res;
}

bigNumber Pow(const bigNumber& A, const bigNumber& B, bigNumber& modulus)
{// возведение A в степень B по модулю modulus
	if (modulus <= (long long int) 0)
		return A ^ B;

	bigNumber base = A % modulus;
	bigNumber res = 1;

	for (bigNumber i = B; i > (long long int) 0; i = i - 1)
		res = (res * base) % modulus;

	return res;
}


unsigned int & bigNumber::operator[](int i)
{
	return this->_elements[i];
}

unsigned int bigNumber::operator[](int i) const
{
	return this->_elements[i];
}

bigNumber bigNumber::operator+(const bigNumber& right) const
{
	// для вызова функций _sub, _sub необходимо найти большее по модулю число
	bigNumber A = *this;
	bigNumber B = right;
	A._sign = 0;
	B._sign = 0;
	if (A > B)
	{
		A._sign = this->_sign;
		B._sign = right._sign;
	}
	else
	{
		A = right;
		B = *this;
	}

	if (A._sign == B._sign)
	{// если числа одного знака, их надо сложить
		return _sum(A, B);
	}
	else
	{// если разных - отнять
		return _sub(A, B);
	}
}

bigNumber bigNumber::operator-() const
{// унарный минус
	bigNumber res = *this;
	res._sign = !res._sign;
	return res;
}

bigNumber bigNumber::operator-(const bigNumber& right) const
{
	return bigNumber(*this + (-right));
}

bigNumber bigNumber::operator*(const bigNumber& B) const
{
	bigNumber A = *this;
	bigNumber res;
	res._elements.resize(A._elements.size() + B._elements.size());
	unsigned int carry = 0;
	for (int i = 0; i < B._elements.size(); i++)
	{
		carry = 0;
		for (int j = 0; j < A._elements.size(); j++)
		{
			// такая страшная и длинная строка потому, что разряды имеют тип int, при умножении которых может произойти переполнение. 				//Вот чтобы не было переполнения необходимо явное приведение типов
			unsigned long long int tmp = (unsigned long long int) B[i] * (unsigned long long int) A[j]
				+ carry + (unsigned long long int) res[i + j];
			carry = tmp / BASE;
			res[i + j] = tmp % BASE;
		}
		res[i + A._elements.size()] = carry;
	}

	res._sign = (A._sign != B._sign);
	res._DelZeroes();
	return res;
}

bigNumber bigNumber::operator/(const bigNumber& right) const
{
	bigNumber rem;
	return _dividing(*this, right, rem);
}

bigNumber bigNumber::operator%(const bigNumber& right) const
{
	bigNumber rem;
	_dividing(*this, right, rem);
	return rem;
}

bigNumber& bigNumber::operator^(const bigNumber& right) const
{// возведение *this в степень right
	bigNumber* res = new bigNumber(1);
	bigNumber base = *this;
	for (bigNumber i = right; i > (long long int) 0; i = i - 1)
		*res = *res * base;
	return *res;
}


bool bigNumber::operator>(const bigNumber& B)
{
	return this->_compare(B) > 0;
}

bool bigNumber::operator>=(const bigNumber& B)
{
	return this->_compare(B) >= 0;
}

bool bigNumber::operator<(const bigNumber& B)
{
	return this->_compare(B) < 0;
}

bool bigNumber::operator<=(const bigNumber& B)
{
	return this->_compare(B) <= 0;
}

bool bigNumber::operator==(const bigNumber& B)
{
	return this->_compare(B) == 0;
}

bool bigNumber::operator!=(const bigNumber& B)
{
	return this->_compare(B) != 0;
}

