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

class Counter 
{
public: 
    Counter(uint32_t bottom, uint32_t top, uint32_t value = 0) : m_bottom(bottom), m_top(top), m_value(value)
    {

    }
    Counter& operator ++ ()
    {
        m_value++;
        if ((m_value >= m_top) || (m_value < m_bottom))
        {
            m_value = m_bottom;
        }
        return *this;
    }

    Counter& operator -- ()
    {
        m_value--;
        if ((m_value >= m_top) || (m_value < m_bottom))
        {
            m_value = m_top;
        }
        return *this;
    }

    Counter operator++ (int)
    {
       Counter temp(m_bottom, m_top, m_value);
       return ++temp;
    }

    Counter operator-- (int)
    {
       Counter temp(m_bottom, m_top, m_value);
       return --temp;
    }

    uint32_t get()
    {
        return m_value;
    }

    void set(uint32_t val)
    {
        if ((val >= m_bottom) && (val < m_top))
        {
            m_value = val;
        }
    }

    void reset()
    {
        m_value = m_bottom;
    }

private:
    uint32_t m_bottom;
    uint32_t m_top;
    uint32_t m_value;
};



int ExecCode(char* code, size_t size)
{
    Counter IpCount(0, size + 1);
    Counter LoopDepthCount(0, 100);
    Counter ApCount(0, 30000);

    char RAM[30000];
    memset(RAM, 0, 30000);
    Counter DataCell(0,256);

    bool loopDetection = false;

    while(IpCount.get() != size)
    {
        if (loopDetection == true)
        {
          switch (code[IpCount.get()])
          {
            case '[':

              break;
            case ']':
              
              break;
          }
          --IpCount;  
        }
        else
        {//Regular execution
            switch (code[IpCount.get()])
            {
                case '>':
                    ++ApCount;
                break;
                case '<':
                    ++ApCount;
                    break;
                case '+':
                    DataCell.set(RAM[ApCount.get()]);
                    ++DataCell;
                    RAM[ApCount.get()] = DataCell.get();
                    break;
                case '-':
                    DataCell.set(RAM[ApCount.get()]);
                    --DataCell;
                    RAM[ApCount.get()] = DataCell.get();
                    break;
                case '.':
                    cout << RAM[ApCount.get()];
                    break;
                case ',':
                    cin >> RAM[ApCount.get()];
                    break;
                case '[':
                    break;
                case ']':
                    break;
                default:
                    //NOP
                    break;   
            }
            ++IpCount;

        }
        
       
    }
    return 0;
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

	std::fstream file(filePath, std::fstream::in | std::fstream::binary);
	if (!file.good()){
		cerr << "Input file error, exiting"<<endl;
		return -1;
	}
  
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (file.read(buffer.data(), size))
  {
        /* worked! */
  }

	status = ExecCode(&buffer[0], size);
	if (status){
		cerr << "Code Execution Error, Status =" << status << endl;
		return -1;
	}

	return 0;
}
