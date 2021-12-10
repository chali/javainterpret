#ifndef __PROGRAM__
#define __PROGRAM__

/*obsahuje vsechny tridy pro prevedeni programu na strom objektu validaci a nasledne vykonani*/

#include <map>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <typeinfo>
#include <string>
#include <iostream>
#include "lexan.h"
#include "exception.h"

using namespace std;

/*vycet moznych typu hodnot*/
enum TypeValue{	INT_VALUE, DOUBLE_VALUE, BOOLEAN_VALUE, STRING_VALUE, VOID_VALUE, NULL_VALUE, CLASS_VALUE, OBJECT_VALUE };

class Class;
class Block;
class Function;
class Expression;
class Declaration;
class WhileCommand;
class Function;
class Program;
class Block;

/*tridy predstavujici jednolive hodnoty pro vypis je pretizen operator <<*/
/*obecna hodnota*/
class Value{
	bool lvalue;
public:
	Value(){ lvalue = false; }
	virtual TypeValue getType(){ return INT_VALUE; }
	void setLValue(bool l){ lvalue = l;	}
	bool getLValue(){ return lvalue; }
};

/* kopiruje hodnotu z v do val, implementace v expression.cpp */
void copyValue(Value* v, Value* val);

class Void: public Value{
public:
	virtual TypeValue getType(){ return VOID_VALUE;	}
};

class Null: public Value{
public:
	virtual TypeValue getType(){ return NULL_VALUE;	}
	friend ostream& operator<<(ostream& o, Null& n){ return o << "null"; }
};

class Boolean: public Value{
	bool value;
public:
	Boolean(){ value = false; }
	Boolean(bool v){ value = v;	}
	void setValue(bool v){ value = v; }
	bool getValue(){ return value; }
	virtual TypeValue getType(){ return BOOLEAN_VALUE; }
	friend ostream& operator<<(ostream& o, Boolean& n){	return o << (n.value?"true":"false"); }
};	

class Integer: public Value{
	int value;
public:
	Integer(){ value = 0; }
	Integer(int v){	value = v; }
	void setValue(int v){ value = v; }
	int getValue(){	return value; }
	virtual TypeValue getType(){ return INT_VALUE; }
	friend ostream& operator<<(ostream& o, Integer& n){	return o << n.value; }
};

class Double: public Value{
	double value;
public:
	Double(){ value = 0; }
	Double(double v){ value = v; }
	void setValue(double v){ value = v; }
	double getValue(){ return value; }
	virtual TypeValue getType(){ return DOUBLE_VALUE; }
	friend ostream& operator<<(ostream& o, Double& n){ return o << n.value;	}
};

class String: public Value{
	string value;
public:
	String(){ value = ""; }
	String(char* v){ value = v;	}
	void setValue(const char* v){ value = v; }
	const char* getValue(){	return value.c_str(); }
	virtual TypeValue getType(){ return STRING_VALUE; }
	friend ostream& operator<<(ostream& o, String& n){ return o << n.value; }
	virtual ~String(){}
};

class Object: public Value{
	/* zda je object null */
	bool null;
	map<string,Value*> fields;
	char type[100];
	/*po validaci obsahuje tridu podle type*/
	Class* c;
public:
	Object(char* type){	null = true; strncpy(this->type,type,100); }
	virtual TypeValue getType(){ return OBJECT_VALUE; }
	void setClass(Class* cl){ c = cl; }
	Class* getClass(){ return c; }
	bool isNull(){ return null;	}
	void setNull(bool n){ null = n;	}
	char* getTypeName(){ return type; }
	/*pouziva pri vytvareni nove instance*/
	void initField(char* name, Value* v){ fields[name]=v; }
	/*nastavovani fieldu behem zivota instance*/
	void setField(char* name, Value* v){ copyValue(v,fields[name]);	}
	Value* getField(char* name){ return (fields)[name];	}
	/*kopirovani*/
	void setValue(map<string,Value*> f){ fields = f; }
	map<string,Value*> getValue(){ return fields; }
	virtual ~Object(){}
};

/*pouziva se kdyz pristupujeme ke statickym clenum trid*/
class ClassValue: public Value{
	Class* c;
public:
	virtual TypeValue getType(){ return CLASS_VALUE; }
	void setClass(Class* cl){ c = cl; }
	Class* getClass(){ return c; }
};

/*obecny uzel vyrazu*/
class Node{
protected:
	/*radek kde se uzel nachazi*/
	int line;
	/*hodnota co uzel vraci*/
	Value* value;
	/*indikuje zda budeme prirazovat*/
	bool assignment;
public:
	Node(){ value = NULL; assignment = false;}
	/*overeni zda je vyraz korektni , datove typy, lvalue atd.*/
	virtual Value* validate()=0;
	/*vykonani*/
	virtual Value* eval()=0;
	void setLine(int l){ line = l; }
	void setAssignment(bool a){ assignment=a; }
	virtual ~Node(){
		if (value!=NULL)
			delete value;
	}
};

class BinaryOperation: public Node{
protected:
	Node* right;
	Node* left;
public:
	BinaryOperation(Node* left, Node* right){ this->left = left; this->right = right; }
	virtual Value* validate()=0;
	virtual Value* eval()=0;
	~BinaryOperation(){	if (right!=NULL) delete right; if (left!=NULL) delete left; }
};

class UnaryOperation: public Node{
protected:	
	Node* node;
public:
	UnaryOperation(Node* node){	this->node = node; }
	virtual Value* validate()=0;
	virtual Value* eval()=0;	
	~UnaryOperation(){
		if (node!=NULL)
			delete node;
	}
};

class Constant: public Node{
public:
	Constant(Value* v){	value = v; }
	virtual Value* validate(){ return value; }
	virtual Value* eval(){ return value; }	
};

class OperatorNew: public Node{
	char type[100];
	vector<Node*> params;
	Expression* e;
public:
	OperatorNew(char* t, vector<Node*> p, Expression* ex);
	virtual Value* validate();
	virtual Value* eval();
	~OperatorNew();
};

/*trida predstavujici pristup k promennym a to jak deklarovanym tak k polozkam objektu*/
class Variable: public UnaryOperation{
	char name[100];
	Expression* ex;
	ClassValue* cv;
	Value* createValue(Value*);
public:
	Variable(Node* parentObject, char* name, Expression* e);
	virtual Value* validate();
	virtual Value* eval();
	~Variable();
};

class FunctionCall: public UnaryOperation{
	vector<Node*> params;
	char name[100];
	Expression* ex;
	/* po validaci funkce ktera se bude volat */
	Function* f;
public:
	FunctionCall(Node* parent, char* n, vector<Node*> p, Expression* e);
	virtual Value* validate();
	virtual Value* eval();	
	~FunctionCall();
};

class UnaryMinus: public UnaryOperation{
public:
	UnaryMinus(Node* n):UnaryOperation(n){}
	virtual Value* validate();
	virtual Value* eval();
};

class Negation: public UnaryOperation{
public:
	Negation(Node* n):UnaryOperation(n){}
	virtual Value* validate();
	virtual Value* eval();
};

class Plus: public BinaryOperation{
public:
	Plus(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class Minus: public BinaryOperation{
public:
	Minus(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();	
};

class Mul: public BinaryOperation{
public:
	Mul(Node* l, Node* r): BinaryOperation(l,r){
	}
	virtual Value* validate();
	virtual Value* eval();
};

class Div: public BinaryOperation{
public:
	Div(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class Equal: public BinaryOperation{
public:
	Equal(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class NotEqual: public BinaryOperation{
public:
	NotEqual(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class Greater: public BinaryOperation{
public:
	Greater(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class GreaterOrEqual: public BinaryOperation{
public:
	GreaterOrEqual(Node* l, Node* r): BinaryOperation(l,r){	}
	virtual Value* validate();
	virtual Value* eval();
};

class Lesser: public BinaryOperation{
public:
	Lesser(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class LesserOrEqual: public BinaryOperation{
public:
	LesserOrEqual(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class And: public BinaryOperation{
public:
	And(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();
};

class Or: public BinaryOperation{
public:
	Or(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();	
};

class Assignment: public BinaryOperation{
public:
	Assignment(Node* l, Node* r): BinaryOperation(l,r){}
	virtual Value* validate();
	virtual Value* eval();	
};

/*pomocna trida, pri prekladu se do ni uklada seznam parametru pro volani funkce*/
class HelperParams{
	vector<Node*> params;
public:
	void addParam(Node* n){	params.push_back(n); }
	vector<Node*> getParams(){	return params; }
};

class Expression{
	/*pokud dojde k chybe ulozi se zde*/
	ParserException* error;
	/* je vyraz prikaz */
	bool statement;
	/* pro fieldy je zde trida ve ktere jsou deklarovany */
	Class* c;
	/* pro lokalni a parametry funkce se nastavuje blok (funkce je take blok)*/
	Block* b;
	Lexan* lexan;
	LexElement token;
	Node* parsedExpression;
	/* metody pro expanze neterminalu */
	Node* Y();
	Node* YC(Node* n);
	Node* X();
	Node* XC(Node* n);
	Node* B();
	Node* BC(Node* n);
	Node* E();
	Node* EC(Node*);
	Node* T();
	Node* TC(Node*);
	Node* F();
	Node* FC(Node*,char*);
	HelperParams* F3(HelperParams*);
	HelperParams* F4(HelperParams*);
	Node* F5(Node*);
	Node* F6(Node*);
public:
	Expression();
	Expression(Lexan* lexan, LexElement& token);
	void setClass(Class* c){ this->c = c; }
	void setBlock(Block* b){ this->b = b; }
	Block* getBlock(){ return b; }
	Value* eval(){ return parsedExpression->eval();	}
	Value* validate(){ return parsedExpression->validate(); }
	bool getStatement(){ return statement; }
	void setNode(Node* n){ parsedExpression=n; }
	/* najde nejblizsi deklarovanou promennou*/
	Value* findValue(char* name){ return this->findValue(name,false); }
	/* najde nejblizsi deklarovanou promennou, muzeme urcit zda chceme pouze localni promenne*/
	Value* findValue(char* name, bool local);
	/* trida ve ktere je vyraz pouzit */
	Class* parentClass();
	/* po dokonceni prekladu vyrazu vraci posledni token aby se mohlo navazat v prekladu programu*/
	LexElement getToken(){ return token; }
	ParserException* getError() { return error; }
	void setError(ParserException* e){ if (error==NULL) error = e; }
	~Expression(){ delete parsedExpression; }
};

/*obecny prikaz*/
class Command{
protected:
	/*radek kde se nachazi*/
	int line;
public:
	/* vykonani */
	virtual void execute()=0;
	/* validace */
	virtual void validate()=0;
	void setLine(int l){ line = l; }
	int getLine(){ return line; }
	virtual ~Command(){
	}
};

/*prikaz obsahujici pouze vyraz*/
class ExpressionCommand: public Command{
	Expression* ex;
public:
	ExpressionCommand(Expression* e){ ex = e; }
	virtual void execute(){ ex->eval();	}
	virtual void validate();
	/*obsahuje prikaz dany vyraz pouziva se pro hledani deklaraci ktere se maji vlozit na zasobnik pri volani funkce*/
	bool containExpression(Expression* e){ return ex==e; }
	~ExpressionCommand(){ if(ex!=NULL) delete ex; }
	
};

class Declaration : public Command{
	/* blok kde se deklarace nachazi*/
	Block* b;
	/* inicializacni vyraz */
	Expression* expr;
	/* kazda promenna ma svuj zasobnik ... to je luxus co :) */
	stack<Value*> valStack;
	/* pred validaci trida ve ktere je deklarace po ni je to trida deklarovaneho objecktu*/
	Class* c;
	/*identifikace typu*/
	TypeValue type;
	/*pokud je object tak jeho jmeno*/
	char typeName[100];
	/* jmeno deklarovane promenne*/
	char name[100];
	/* je to staticka promenna */
	bool stat;
	Value* val;
	/*vytvari novou Value na zaklade deklarace*/
	Value* createValue();
public:
	Declaration(Expression* e, TypeValue tv, char* tn, char* n, bool s);
	virtual void validate();
	virtual void execute();
	void setClass(Class* cl){ c = cl; }
	void setBlock(Block* bl){ b = bl; }
	char* getName(){ return name; }
	bool isStatic(){ return stat; }
	/*obsahuje prikaz dany vyraz pouziva se pro hledani deklaraci ktere se maji vlozit na zasobnik pri volani funkce*/
	bool containExpression(Expression* e){ return expr==e; }
	Expression* getExpression(){ return expr; }
	/*kazda vkladani a vybirani promennych ze zasobniku pri volani funkce*/
	void pushOnStack();
	void popFromStack();
	Value* getValue();
	~Declaration(){ if(expr!=NULL) delete expr; if(val!=NULL) delete val; }
};

class Block: public Command{
	/* nadrazeny blok */
	Block* parent;
	/* seznam deklaraci v bloku*/
	vector<Declaration*> declarations;
	/* seznam prikazu */
	vector<Command*> commands;
public:
	Block(){ parent = NULL;	}
	Block(Block* p){ parent = p; }
	void setParentBlock(Block* b){ parent = b; }
	virtual Class* getClass(){ return parent->getClass(); }
	/* najde promennou, muzeme urcit zda chceme jen lokalni*/
	virtual Value* findValue(char* name, bool local);
	void addCommand(Command* c){ commands.push_back(c);	}
	virtual void validate();
	virtual void execute();
	/* zjistuje zda za return nejsou dalsi prikazy*/
	void checkReturn();
	/* zjistuje zda funkce ktera neni void ma return*/
	void checkReturn2();
	virtual Function* getParentFunction(){ return parent->getParentFunction(); }
	/* ziskani seznamu lokalnich deklaraci, vytvorenych pred pouzitim vyrazu e*/
	virtual bool getLocalDeclaration(Expression* e, vector<Declaration*>* ld);	
	~Block();
};

/* pouze strednik */
class EmptyCommand: public Command{
	virtual void execute(){}
	virtual void validate(){}
};

class IfCommand: public Command{
	Command* command;
	Expression* ex;
public:
	IfCommand(Command* c, Expression* e){ ex = e; command = c; }	
	virtual void validate();
	virtual void execute();
	/* zjistuje zda za return nejsou dalsi prikazy*/
	void checkReturn();
	/* ziskani seznamu lokalnich deklaraci, vytvorenych pred pouzitim vyrazu e*/
	virtual bool getLocalDeclaration(Expression* e, vector<Declaration*>* ld);
	/*obsahuje prikaz dany vyraz pouziva se pro hledani deklaraci ktere se maji vlozit na zasobnik pri volani funkce*/
	bool containExpression(Expression* e){ return ex==e; }
	~IfCommand(){ if(command!=NULL) delete command; if(ex!=NULL) delete ex; }	
};

class WhileCommand: public Command{
	Command* command;
	Expression* ex;
public:
	WhileCommand(Command* c, Expression* e){ ex = e; command = c; }	
	virtual void validate();
	virtual void execute();
	/* zjistuje zda za return nejsou dalsi prikazy*/
	void checkReturn();
	/* ziskani seznamu lokalnich deklaraci, vytvorenych pred pouzitim vyrazu e*/
	virtual bool getLocalDeclaration(Expression* e, vector<Declaration*>* ld);
	/*obsahuje prikaz dany vyraz pouziva se pro hledani deklaraci ktere se maji vlozit na zasobnik pri volani funkce*/
	bool containExpression(Expression* e){ return ex==e; }
	~WhileCommand(){ if(command!=NULL) delete command; if(ex!=NULL) delete ex; }
};

class ReturnCommand: public Command{
	Expression* ex;
	Block* b;
public:
	ReturnCommand(Expression* e, Block* bl){ b = bl; ex = e; }
	virtual void validate();
	virtual void execute();
	/*obsahuje prikaz dany vyraz pouziva se pro hledani deklaraci ktere se maji vlozit na zasobnik pri volani funkce*/
	bool containExpression(Expression* e){ return ex==e; }
	~ReturnCommand(){ if(ex!=NULL) delete ex; }
};

class Function: public Block{
protected:
	/*trida kde je fukce deklarovana*/
	Class* c;
	/* blok nasledujici za deklaraci*/
	Block* block;
	/*deklarovane parametry*/
	vector<Declaration*> params;
	/*je static*/
	bool stat;
	/*jmeno funkce*/
	char name[100];
	/* druh navratoveho typu*/
	TypeValue type;
	/* pokud je navratovy typ object tak jaka je to trida*/
	char typeName[100];
public:
	Function(){ block=NULL;	c=NULL;	stat = false; }
	Function(vector<Declaration*> p, TypeValue tv, char* tn, char* n, bool s, Block* b);
	void setClass(Class* cl){ c = cl; }
	void setBlock(Block* b){ block=b; block->setParentBlock(this); }
	virtual Class* getClass(){ return c; }
	virtual char* getName(){ return name; }
	virtual TypeValue getType(){ return type; }
	char* getTypeName(){ return typeName; }
	/* volani zacina na instanci Blok dokud se nedostane az sem*/
	virtual Function* getParentFunction(){ return this; }
	/* najde promennou, muzeme urcit zda chceme jen lokalni*/
	virtual Value* findValue(char* name, bool local);
	virtual void validate();
	virtual void execute(){ block->execute(); }
	bool isStatic(){ return stat; }	
	/* ziskani seznamu lokalnich deklaraci, vytvorenych pred pouzitim vyrazu e*/
	virtual bool getLocalDeclaration(Expression* e, vector<Declaration*>* ld);
	/* seznam "viditelnych" parametru bez implicitniho this u instancnich metod, pro validacni cast*/
	vector<Value*> getValuesParams();
	/* seznam vsech parametru i implicitnich, pro vykonnou cast*/
	vector<Value*> getParams();
	~Function();
};

class Class{
protected:
	/* seznam deklaraci poli*/
	vector<Declaration*> fields;
	/* pred validaci jsou zde */
	vector<Declaration*> preValidateFields;
	/* seznam funkci */
	vector<Function*> functions;
	/* jmeno tridy */
	char name[100];
	/* program kde je pouzita */
	Program* p;
public:
	Class(){}
	Class(char* name){ strncpy(this->name,name,100); }
	void addDeclaration(Declaration* d);
	void addFunction(Function* f);
	virtual char* getName(){ return name; }
	/* seznam deklaraci instacni nebo staticka pole */
	vector<Declaration*> getDeclarations(bool stat);
	/* hledani hodnot promennych*/
	Value* getField(char* name){ return this->getField(name,false);	}
	Value* getField(char* name, bool stat);
	void initStaticField();
	/* hledani funkce podle parametru a jmena*/
	virtual Function* getFunction(char* name, vector<Value*> values){ return getFunction(name,values,false); }
	virtual Function* getFunction(char* name, vector<Value*> values, bool stat);
	void setProgram(Program* pr){ p = pr; }
	Program* getProgram(){ return p; }
	void validateField();
	void validateFunction();
	virtual ~Class();
};

/* pomocna trida pro nacitani seznamu deklarovanych parametru */
class HelperParamsDeclaration{
	vector<Declaration*> params;
public:
	void addParam(Declaration* n){ params.push_back(n);	}
	vector<Declaration*> getParams(){ return params; }
};

/* knihovni funkce a tridy */
class FunctionPrintInt: public Function{
public:
	FunctionPrintInt(){
		params.push_back(new Declaration(NULL,OBJECT_VALUE,"Stream","this",false));
		params.push_back(new Declaration(NULL,INT_VALUE,NULL,"s",false));
	}
	virtual void execute(){	cout<< *((Integer*)params[1]->getValue()) << endl; }
	virtual void validate(){}
	virtual char* getName(){ return "println"; }
};

class FunctionPrintDouble: public Function{
public:
	FunctionPrintDouble(){
		params.push_back(new Declaration(NULL,OBJECT_VALUE,"Stream","this",false));
		params.push_back(new Declaration(NULL,DOUBLE_VALUE,NULL,"s",false));
	}
	virtual void execute(){	cout<< *((Double*)params[1]->getValue()) << endl; }
	virtual void validate(){}
	virtual char* getName(){ return "println"; }
};

class FunctionPrintBoolean: public Function{
public:
	FunctionPrintBoolean(){
		params.push_back(new Declaration(NULL,OBJECT_VALUE,"Stream","this",false));
		params.push_back(new Declaration(NULL,BOOLEAN_VALUE,NULL,"s",false));
	}
	virtual void execute(){	cout<< *((Boolean*)params[1]->getValue()) << endl; }
	virtual void validate(){}
	virtual char* getName(){ return "println"; }
};

class FucntionPrintString: public Function{
public:
	FucntionPrintString(){
		params.push_back(new Declaration(NULL,OBJECT_VALUE,"Stream","this",false));
		params.push_back(new Declaration(NULL,STRING_VALUE,NULL,"s",false));
	}
	virtual void execute(){	cout<< *((String*)params[1]->getValue()) << endl; }
	virtual void validate(){}
	virtual char* getName(){ return "println"; }
};

class Stream: public Class{
public:
	Stream(Program* pr){
		p = pr;
		functions.push_back(new FunctionPrintInt());
		functions.push_back(new FunctionPrintDouble());
		functions.push_back(new FunctionPrintBoolean());
		functions.push_back(new FucntionPrintString());
	}
	virtual char* getName(){ return "Stream"; }
};

class System: public Class{
public:
	System(Program* pr);
	virtual char* getName(){ return "System"; }
};

class Program{
	/* pokud dojde k chybe pri prekladu je ulozena zde*/
	ParserException* error;
	Lexan* lexan;
	LexElement token;
	vector<Class*> classes;
	void CD();
	void FFD(Class*);
	void FFDC(Class* c, bool stat, char* name, TypeValue type, char* typeName);
	HelperParamsDeclaration* PD(HelperParamsDeclaration*);
	HelperParamsDeclaration* PDC(HelperParamsDeclaration*);

	Block* BO(Block*);
	Command* L(Block*);
	Command* D(char* name, TypeValue type, char* typeName, Block* parent);
public:
	Program(char* path);
	void init();
	void runMain(char* className);
	/* najde tridu deklarovanou v programu*/
	Class* findClass(char* name);
	ParserException* getError(){ return error; }
	void setError(ParserException* e){ if (error==NULL) error = e; }
	void validate();
	~Program();
};

#endif
