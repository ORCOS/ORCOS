/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
#include <stdlib.h>
#include <stdio.h>

extern "C" main(int argc, char** argv) {
    printf("Hello World!\n");
    return 0;
}
*/

#include <cstdio>
#include <iostream>

using namespace std;

class HelloClass {
public:
	HelloClass(char* buffer);
	~HelloClass();
	void hello(const char* msg);
protected:
	char* b;
	char* B;
};

HelloClass::HelloClass(char* buffer) {
	this->b = this->B = buffer;
	int c = sprintf(this->b, "Start\n");
	this->b += c;
}
HelloClass::~HelloClass() {
	int c = sprintf(this->b, "End\n");
	this->b += c;
	cout << this->B;
	cout.flush();
}
void HelloClass::hello(const char* msg) {
	int c = sprintf(this->b, "%s\n", msg);
	this->b += c;
}

int main()
{
	char b[200];
	HelloClass* h = new HelloClass(b);
	h->hello("HELLO");
	delete h;

	cout << "Hello World!" << endl;
	cout << "Welcome to C++ Programming" << endl;
	cout.flush();

	return 0;
}

