/*dodelat elemetnty &&, ||, new*/ 

#ifndef __LEXAN__
#define __LEXAN__

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <queue>
#include "exception.h"

using namespace std;

enum LexElementType{
	END_FILE,
	ERROR,
	PLUS,
	MINUS,
	MUL,
	DIV,
	ASIGNMENT,
	EQUAL,
	NOT_EQUAL,
	GREATER,
	GREATER_OR_EQUAL,
	LESSER,
	LESSER_OR_EQUAL,
	NOT,
	INTEGER,
	DOUBLE,
	STRING,
	IDENTIFICATOR,
	OPEN_ROUND_BRACKET,
	CLOSE_ROUND_BRACKET,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	OPEN_BRACE,
	CLOSE_BRACE,
	SEMICOLON,
	COMMA,
	IF,
	ELSE,
	FOR,
	WHILE,
	CLASS,
	STATIC,
	VOID,
	INT,
	DOUBLE_KEY,
	STRING_KEY,
	TRUE,
	FALSE,
	DOT,
	NULL_KEY,
	BOOLEAN,
	NEW,
	OR,
	AND,
	RETURN
};

static int nrKeyWords = 16;
static struct{                             
   LexElementType keyWord;              
   char  word[20];
} keywords[] = {
    { IF, "if" },
	{ ELSE, "else" },
	{ FOR, "for" },
	{ WHILE, "while" },
	{ CLASS, "class" },
	{ STATIC, "static" },
	{ VOID, "void" },
	{ INT, "int" },
	{ DOUBLE_KEY, "double" },
	{ TRUE , "true" },
	{ FALSE , "false" },
	{ NULL_KEY, "null" },
	{ BOOLEAN, "boolean" },
	{ NEW, "new" },
	{ RETURN, "return" },
	{ STRING_KEY, "String"}
  };

class LexElement{
	LexElementType type;
	int intValue;
	double doubleValue;
	char charValue[100];
public:
	LexElement(){}
	LexElement(LexElementType type){
		this->type = type;
	}
	LexElement(LexElementType type, char* s){
		this->type = type;
		strncpy(this->charValue,s,100);
	}
	LexElement(LexElementType type, int intValue){
		this->type = type;
		this->intValue = intValue;
	}
	LexElement(LexElementType type, double doubleValue){
		this->type = type;
		this->doubleValue = doubleValue;
	}
	LexElementType getType() const{
		return type;
	}
	int getIntValue() const{
		return intValue;
	}
	double getDoubleValue() const{
		return doubleValue;
	}
	char* getCharValue(){
		return charValue;
	}
};

class Lexan{
	queue<LexElement> back;
	FILE* source;
	bool init;
	int line;
public:
	Lexan(char* path){
		source =  fopen(path, "r");
		line = 1;
		if (source == NULL) {
			throw new ParserException("Soubor se nepodarilo otevrit");
			init = false;
		}
		init = true;
	}
	LexElement nextToken();
	void pushBack(LexElement e){
		back.push(e);
	}
	int getLine(){ return line;}
	~Lexan(){
		fclose(source);
	}
};

#endif
