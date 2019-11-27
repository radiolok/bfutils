/* Copyright (c) 2016-2019, Artem Kashkanov
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

#include <iostream>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include "Console.h"

#include "Image.h"
#include <fstream>

#include <vector>

#include "bfutils.h"

#include "Command.h"

#include <getopt.h>

using namespace std;

template <typename counterType> class Counter
{
public: 
	Counter(counterType bottom, counterType top = 0, counterType value = 0) : m_bottom(bottom), m_top(top), m_value(value)
    {
    }

	virtual Counter& operator ++ ()
    {
        m_value++;
        if ((m_top) && ((m_value >= m_top) || (m_value < m_bottom)))
        {
            m_value = m_bottom;
        }
        return *this;
    }

	virtual Counter& operator -- ()
    {
        m_value--;
		if ((m_top) && ((m_value >= m_top) || (m_value < m_bottom)))
        {
            m_value = m_top;
        }
        return *this;
    }

	virtual Counter operator++ (int)
    {
       Counter temp(m_bottom, m_top, m_value);
       return ++temp;
    }

	virtual Counter operator-- (int)
    {
       Counter temp(m_bottom, m_top, m_value);
       return --temp;
    }

	virtual counterType pos()
    {
        return m_value;
    }

	virtual counterType size()
	{
		return m_top - m_bottom;
	}

	virtual void pos(counterType val)
    {
        if ((val >= m_bottom) && (val < m_top))
        {
            m_value = val;
        }
    }

	virtual void reset()
    {
        m_value = m_bottom;
    }

private:
	counterType m_bottom;
	counterType m_top;
	counterType m_value;
};

template <typename MemType, typename countType> class Memory : public Counter<countType>
{
public:
	Memory(countType bottom, countType top, MemType* data = NULL) : Counter<countType>(bottom, top)
	{
		m_memory = static_cast<MemType*>(calloc(sizeof(MemType)*(top - bottom), 1));
		if (data)
		{
			memcpy(m_memory, data, top - bottom);
		}
	}

	virtual MemType& operator * ()
	{
		return m_memory[this->pos()];
	}

private:
	MemType * m_memory;
};

int LoopLookup(Memory<char, size_t>& code, Counter<size_t> loopDepth, bool reverse = false)
{
	++loopDepth;
	while (loopDepth.pos())
	{
		if (reverse)
		{
			--code;
			if (*code == ']')
			{
				++loopDepth;
			}
			else if (*code == '[')
			{
				--loopDepth;
			}
		}
		else
		{
			++code;
			if (*code == ']')
			{
				--loopDepth;
			}
			else if (*code == '[')
			{
				++loopDepth;
			}
		}	
	}
	return 0;
}

int ExecCode(char* code, size_t size)
{
    Memory <char, size_t> codeRAM(0, size + 1, code);
	Memory <char, size_t> dataRAM(0, 30000);

	Counter <size_t> LoopDepthCount(0, 100);

    Counter <uint8_t> DataCell();

    bool loopDetection = false;

    while(codeRAM.pos() != size)
    {
		switch (*codeRAM)
		{
		case '>':
			++dataRAM;
			break;
		case '<':
			--dataRAM;
			break;
		case '+':
			++(*dataRAM);
			break;
		case '-':
			--(*dataRAM);
			break;
		case '.':
			cout << *dataRAM;
			break;
		case ',':
			cin >> *dataRAM;
			break;
		case '[':
			if (!(*dataRAM))
			{
				LoopLookup(codeRAM, LoopDepthCount);
			}
			break;
		case ']':
			if (*dataRAM)
			{
				LoopLookup(codeRAM, LoopDepthCount, true);
			}
			break;
		default:
			//NOP
			break;
		}
		++codeRAM;
    }
    return 0;
}

std::ifstream::pos_type filesize(const char* filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

int main(int argc, char **argv)
{
	int status = -1;
	int c = 0;
	char *filePath = NULL;
	while((c = getopt(argc, argv, "f:")) != -1){
		switch(c)
		{
		case 'f':
			filePath = optarg;
			break;
		}
	}

	std::ifstream file(filePath, std::ifstream::ate | std::ifstream::binary);
	if (!file.is_open()){
		cerr << "Input file error, exiting"<<endl;
		return -1;
	}

	  std::streamsize size = filesize(filePath);
	  if (size == 0)
	  {
		  cerr << "Input file " << filePath << " empty, exiting" << endl;
		  return -1;
	  }

	  file.seekg(0, std::ios::beg);

	  std::vector<char> buffer(size);
	  file.read(buffer.data(), size);

	  status = ExecCode(&buffer.front(), size);
	  if (status) {
		  cerr << "Code Execution Error, Status =" << status << endl;
		  return -1;
	  }
	return 0;
}
