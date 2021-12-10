#include <string.h>
#include <stdio.h>
#include <iostream>
#include "program.h"
#include "exception.h"

using namespace std;

int main(int argc, char* argv[])
{
	if (argc!=3){
		cout << "pro spusteni pouzijte prosim : jaint src_path main_class" << endl;
		return 0;
	}
	if (strlen(argv[1])>200){
		cout << "maximalni delka src_path je 200 znaku" << endl;
		return 0;
	}
	if (strlen(argv[2])>100){
		cout << "maximalni delka main_class je 100 znaku" << endl;
		return 0;
	}
	try{		
		Program p1(argv[1]);
		p1.init();
		p1.runMain(argv[2]);		
	}
	catch (ParserException* e){
		cout << e->getMessage() << endl;
		delete e;
	}	
	return 0;
}

