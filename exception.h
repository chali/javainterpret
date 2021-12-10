
#ifndef __ParserException__
#define __ParserException__

#include<string>
using namespace std;

class ParserException{
	string message;
	string name;
	int line;
	char buff[10];
public:
	ParserException(string mess){
		message = mess;
		this->name = "";
		this->line = 0;
	}
	ParserException(string mess, int line){
		message = mess;
		this->name = "";
		this->line = line;
	}
	ParserException(string mess, string name){
		message = mess;
		this->name = name;
	}
	ParserException(string mess, string name, int line){
		message = mess;
		this->name = name;
		this->line = line;
	}
	string getMessage(){
		message = message + name + " radek ";
		if (line>0){
			sprintf(buff,"%d",line);
			message =message + buff;
		}
		return message;		
	}
	~ParserException(){}
};

#endif
