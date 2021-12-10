
#include "program.h"

void Program::CD(){
	switch(token.getType()){
		case CLASS:{
				token = lexan->nextToken();
				if (token.getType()!=IDENTIFICATOR){
					setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
					return;
				}
				LexElement name = token;
				token = lexan->nextToken();
				if (token.getType()!=OPEN_BRACE){
					setError(new ParserException("ocekavana { na radku ",lexan->getLine()));
					return;
				}
				token = lexan->nextToken();
				Class* c = new Class(name.getCharValue());
				FFD(c);
				if (token.getType()!=CLOSE_BRACE){
					setError(new ParserException("ocekavana } na radku ",lexan->getLine()));
				}
				c->setProgram(this);
				classes.push_back(c);
				token = lexan->nextToken();
				CD();
				break;
			}
		case END_FILE:
			break;
		default:
			setError(new ParserException("ocekavano class na radku ",lexan->getLine()));
			return;
	}

}

void Program::FFD(Class* c){
	bool stat = false;
	LexElement name;
	if (token.getType()==STATIC){
		stat = true;
		token = lexan->nextToken();
	}
	switch (token.getType()){			
		case IDENTIFICATOR:{
				LexElement typeName = token;
				token = lexan->nextToken();
				if (token.getType()!=IDENTIFICATOR){
					setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
					return;
				}
				name = token;
				token = lexan->nextToken();
				FFDC(c,stat,name.getCharValue(),OBJECT_VALUE,typeName.getCharValue());
				FFD(c);
				break;
			}
		case INT:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return;
			}
			name = token;
			token = lexan->nextToken();
			FFDC(c,stat,name.getCharValue(),INT_VALUE,NULL);
			FFD(c);
			break;
		case DOUBLE_KEY:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return;
			}
			name = token;
			token = lexan->nextToken();
			FFDC(c,stat,name.getCharValue(),DOUBLE_VALUE,NULL);
			FFD(c);
			break;
		case STRING_KEY:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return;
			}
			name = token;
			token = lexan->nextToken();
			FFDC(c,stat,name.getCharValue(),STRING_VALUE,NULL);
			FFD(c);
			break;
		case BOOLEAN:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return;
			}
			name = token;
			token = lexan->nextToken();
			FFDC(c,stat,name.getCharValue(),BOOLEAN_VALUE,NULL);
			FFD(c);
			break;
		case VOID:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return;
			}
			name = token;
			token = lexan->nextToken();
			FFDC(c,stat,name.getCharValue(),VOID_VALUE,NULL);
			FFD(c);
			break;
		case CLOSE_BRACE:
			break;
		default:
			setError(new ParserException("ocekavano int, void, double, boolean, String ,identifikator , } na radku ",lexan->getLine()));
			return;
	}

}

void Program::FFDC(Class* c, bool stat, char* name, TypeValue type, char* typeName){
	switch(token.getType()){
		case ASIGNMENT:{
				token = lexan->nextToken();
				if (type==VOID_VALUE){
					setError(new ParserException("pole nemuze byt typu void na radku ",lexan->getLine()));
					return;
				}
				Expression* e = new Expression(lexan,token);
				setError(e->getError());
				e->setClass(c);
				Declaration* d = new Declaration(e,type,typeName,name,stat);
				d->setLine(lexan->getLine());
				token = e->getToken();
				if (token.getType()!=SEMICOLON)
					setError(new ParserException("ocekavan ; na radku ",lexan->getLine()));
				c->addDeclaration(d);
				token = lexan->nextToken();
				break;
			}
		case SEMICOLON:{
				if (type==VOID_VALUE){
					setError(new ParserException("pole nemuze byt typu void na radku ",lexan->getLine()));
					return;
				}
				Declaration* d = new Declaration(NULL,type,typeName,name,stat);
				d->setLine(lexan->getLine());
				c->addDeclaration(d);
				token = lexan->nextToken();
				break;
			}
		case OPEN_ROUND_BRACKET:{
				token = lexan->nextToken();
				HelperParamsDeclaration* help = new HelperParamsDeclaration(); 
				if (!stat){
					Declaration* d = new Declaration(NULL,OBJECT_VALUE,c->getName(),"this",false);
					d->setLine(lexan->getLine());
					help->addParam(d);
				}
				help = PD(help);
				if (token.getType()!=CLOSE_ROUND_BRACKET)
					setError(new ParserException("ocekavana ) na radku ",lexan->getLine()));
				token = lexan->nextToken();
				if (token.getType()!=OPEN_BRACE)
					setError(new ParserException("ocekavana { na radku ",lexan->getLine()));
				token = lexan->nextToken();
				Function* f = new Function(help->getParams(),type,typeName,name,stat,NULL);
				f->setLine(lexan->getLine());
				Block* b = BO(new Block(f));
				b->setLine(lexan->getLine());
				f->setBlock(b);
				if (token.getType()!=CLOSE_BRACE)
					setError(new ParserException("ocekavana } na radku ",lexan->getLine()));
				token = lexan->nextToken();
				c->addFunction(f);
				delete help;
				break;
			}
		default:
			setError(new ParserException("ocekavano =, ;, ( na radku ",lexan->getLine()));
			return;
	}
}

HelperParamsDeclaration* Program::PD(HelperParamsDeclaration* help){
	LexElement name;
	Declaration* d;
	switch(token.getType()){
		case IDENTIFICATOR:{
				LexElement typeName = token;
				token = lexan->nextToken();
				if (token.getType()!=IDENTIFICATOR){
					setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
					return help;
				}
				name = token;
				token = lexan->nextToken();
				d = new Declaration(NULL,OBJECT_VALUE,typeName.getCharValue(),name.getCharValue(),false);
				d->setLine(lexan->getLine());
				help->addParam(d);
				return PDC(help);
			}
		case INT:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return help;
			}
			name = token;
			token = lexan->nextToken();
			d = new Declaration(NULL,INT_VALUE,NULL,name.getCharValue(),false);
			d->setLine(lexan->getLine());
			help->addParam(d);
			return PDC(help);
		case DOUBLE_KEY:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return help;
			}
			name = token;
			token = lexan->nextToken();
			d = new Declaration(NULL,DOUBLE_VALUE,NULL,name.getCharValue(),false);
			d->setLine(lexan->getLine());
			help->addParam(d);
			return PDC(help);
		case STRING_KEY:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return help;
			}
			name = token;
			token = lexan->nextToken();
			d = new Declaration(NULL,STRING_VALUE,NULL,name.getCharValue(),false);
			d->setLine(lexan->getLine());
			help->addParam(d);
			return PDC(help);
		case BOOLEAN:
			token = lexan->nextToken();
			if (token.getType()!=IDENTIFICATOR){
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return help;
			}
			name = token;
			token = lexan->nextToken();
			d = new Declaration(NULL,BOOLEAN_VALUE,NULL,name.getCharValue(),false);
			d->setLine(lexan->getLine());
			help->addParam(d);
			return PDC(help);
		case CLOSE_ROUND_BRACKET:
			return help;
		default:
			setError(new ParserException("ocekavano int, double, boolean, String ,identifikator , } na radku ",lexan->getLine()));
			return help;
	}
}

HelperParamsDeclaration* Program::PDC(HelperParamsDeclaration* help){
	Declaration* d;
	switch(token.getType()){
		case COMMA:{
				token = lexan->nextToken();
				LexElement name;
				switch(token.getType()){
					case IDENTIFICATOR:{
						LexElement typeName = token;
						token = lexan->nextToken();
						if (token.getType()!=IDENTIFICATOR){
							setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
							return help;
						}
						name = token;
						token = lexan->nextToken();
						d = new Declaration(NULL,OBJECT_VALUE,typeName.getCharValue(),name.getCharValue(),false);
						d->setLine(lexan->getLine());
						help->addParam(d);
						return PDC(help);
					}
					case INT:
						token = lexan->nextToken();
						if (token.getType()!=IDENTIFICATOR){
							setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
							return help;
						}
						name = token;
						token = lexan->nextToken();
						d = new Declaration(NULL,INT_VALUE,NULL,name.getCharValue(),false);
						d->setLine(lexan->getLine());
						help->addParam(d);
						return PDC(help);
					case DOUBLE_KEY:
						token = lexan->nextToken();
						if (token.getType()!=IDENTIFICATOR){
							setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
							return help;
						}
						name = token;
						token = lexan->nextToken();
						d = new Declaration(NULL,DOUBLE_VALUE,NULL,name.getCharValue(),false);
						d->setLine(lexan->getLine());
						help->addParam(d);
						return PDC(help);
					case STRING_KEY:
						token = lexan->nextToken();
						if (token.getType()!=IDENTIFICATOR){
							setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
							return help;
						}
						name = token;
						token = lexan->nextToken();
						d = new Declaration(NULL,STRING_VALUE,NULL,name.getCharValue(),false);
						d->setLine(lexan->getLine());
						help->addParam(d);
						return PDC(help);
					case BOOLEAN:
						token = lexan->nextToken();
						if (token.getType()!=IDENTIFICATOR){
							setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
							return help;
						}
						name = token;
						token = lexan->nextToken();
						d = new Declaration(NULL,BOOLEAN_VALUE,NULL,name.getCharValue(),false);
						d->setLine(lexan->getLine());
						help->addParam(d);
						return PDC(help);					
					default:
						setError(new ParserException("ocekavano int, double, boolean, String ,identifikator na radku ",lexan->getLine()));
						return help;
				}
			}
		case CLOSE_ROUND_BRACKET:
			return help;
		default:
			setError(new ParserException("ocekavano \',\', ) na radku ",lexan->getLine()));
			return help;
	}
}

Block* Program::BO(Block* b){
	switch(token.getType()){
		case OPEN_ROUND_BRACKET:
		case INTEGER:
		case DOUBLE:
		case STRING:
		case TRUE:
		case FALSE:
		case IDENTIFICATOR:
		case NEW:
		case NOT:
		case MINUS:
		case IF:
		case RETURN:
		case WHILE:
		case OPEN_BRACE:
		case SEMICOLON:
		case INT:
		case DOUBLE_KEY:
		case STRING_KEY:
		case NULL_KEY:
		case BOOLEAN:
			b->addCommand(L(b));
			return BO(b);
		case CLOSE_BRACE:
			return b;
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null,\n\
								   int, double, boolean, String, {, if, while, return na radku ",lexan->getLine()));
			return b;
	}
}

Command* Program::L(Block* parent){
	Expression* e;
	Command* temp;
	switch(token.getType()){
		case IF:
			token = lexan->nextToken();
			if (token.getType()!=OPEN_ROUND_BRACKET){
				setError(new ParserException("ocekavana ( na radku ",lexan->getLine()));
				return NULL;
			}
			token = lexan->nextToken();
			e = new Expression(lexan,token);
			setError(e->getError());
			e->setBlock(parent);
			token = e->getToken();
			if (token.getType()!=CLOSE_ROUND_BRACKET){
				setError(new ParserException("ocekavana ) na radku ",lexan->getLine()));
			}
			token = lexan->nextToken();
			temp = new IfCommand(L(parent),e);
			temp->setLine(lexan->getLine());
			return temp;
		case WHILE:
			token = lexan->nextToken();
			if (token.getType()!=OPEN_ROUND_BRACKET){
				setError(new ParserException("ocekavana ( na radku ",lexan->getLine()));
				return NULL;
			}
			token = lexan->nextToken();
			e = new Expression(lexan,token);
			setError(e->getError());
			e->setBlock(parent);
			token = e->getToken();
			if (token.getType()!=CLOSE_ROUND_BRACKET){
				setError(new ParserException("ocekavana ) na radku ",lexan->getLine()));
			}
			token = lexan->nextToken();
			temp = new WhileCommand(L(parent),e);
			temp->setLine(lexan->getLine());
			return temp;
		case RETURN:
			token = lexan->nextToken();
			if (token.getType()==SEMICOLON){
				token = lexan->nextToken();
				temp = new ReturnCommand(NULL,parent);
				temp->setLine(lexan->getLine());
				return temp;
			}
			e = new Expression(lexan,token);
			setError(e->getError());
			e->setBlock(parent);
			token = e->getToken();
			if (token.getType()!=SEMICOLON){
				setError(new ParserException("ocekavana ; na radku ",lexan->getLine()));
			}
			token = lexan->nextToken();
			temp = new ReturnCommand(e,parent);
			temp->setLine(lexan->getLine());
			return temp;
		case OPEN_BRACE:{
				token = lexan->nextToken();
				Block* result = BO(new Block(parent));
				if (token.getType()!=CLOSE_BRACE){
					setError(new ParserException("ocekavana } na radku ",lexan->getLine()));
				}
				result->setLine(lexan->getLine());
				token = lexan->nextToken();
				return result;
			}
		case SEMICOLON:
			token = lexan->nextToken();
			return new EmptyCommand();
		case INT:
			token = lexan->nextToken();
			if (token.getType()==IDENTIFICATOR){
				LexElement name = token;
				token = lexan->nextToken();
				return D(name.getCharValue(),INT_VALUE,NULL,parent);
			}
			else{
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return NULL;
			}
		case DOUBLE_KEY:
			token = lexan->nextToken();
			if (token.getType()==IDENTIFICATOR){
				LexElement name = token;
				token = lexan->nextToken();
				return D(name.getCharValue(),DOUBLE_VALUE,NULL,parent);
			}
			else{
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return NULL;
			}
		case STRING_KEY:
			token = lexan->nextToken();
			if (token.getType()==IDENTIFICATOR){
				LexElement name = token;
				token = lexan->nextToken();
				return D(name.getCharValue(),STRING_VALUE,NULL,parent);
			}
			else{
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return NULL;	
			}
		case BOOLEAN:
			token = lexan->nextToken();
			if (token.getType()==IDENTIFICATOR){
				LexElement name = token;
				token = lexan->nextToken();
				return D(name.getCharValue(),BOOLEAN_VALUE,NULL,parent);
			}
			else{
				setError(new ParserException("ocekavan identifikator na radku ",lexan->getLine()));
				return NULL;
			}
		case IDENTIFICATOR:{
				LexElement type = token;
			    token = lexan->nextToken();
				if (token.getType()==IDENTIFICATOR){
					LexElement name = token;
					token = lexan->nextToken();
					return D(name.getCharValue(),OBJECT_VALUE,type.getCharValue(),parent);
				}
				else{
					lexan->pushBack(token);
					token = type;					
				}
			}	
		case OPEN_ROUND_BRACKET:
		case INTEGER:
		case DOUBLE:
		case STRING:
		case TRUE:
		case FALSE:
		case NULL_KEY:
		case NEW:
		case NOT:
		case MINUS:
			e = new Expression(lexan,token);
			setError(e->getError());
			e->setBlock(parent);
			token = e->getToken();
			if (token.getType()!=SEMICOLON){
				setError(new ParserException("ocekavan ; na radku ",lexan->getLine()));
			}
			token = lexan->nextToken();
			temp = new ExpressionCommand(e);
			temp->setLine(lexan->getLine());
			return temp;
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null,\n\
								   int, double, boolean, String, {, if, while, return na radku ",lexan->getLine()));
			return NULL;
	}
}

Command* Program::D(char* name, TypeValue type, char* typeName, Block* parent){
	switch(token.getType()){
		case ASIGNMENT:{
				token = lexan->nextToken();
				Expression* e = new Expression(lexan,token);
				setError(e->getError());
				e->setBlock(parent);
				token = e->getToken();				
				Declaration* d = new Declaration(e,type,typeName,name,false);
				if (token.getType()!=SEMICOLON){
					setError(new ParserException("ocekavan ; na radku ",lexan->getLine()));
				}
				d->setBlock(parent);
				d->setLine(lexan->getLine());
				token = lexan->nextToken();
				return d;
			}
		case SEMICOLON:{
				token = lexan->nextToken();
				Declaration* d = new Declaration(NULL,type,typeName,name,false);
				d->setBlock(parent);
				d->setLine(lexan->getLine());
				return d;
			}
		default:
			setError(new ParserException("ocekavan ; nebo = na radku ",lexan->getLine()));
			return NULL;
	}
}

Program::Program(char* path){
	error = NULL;
	lexan = new Lexan(path);
	token = lexan->nextToken();
	classes.push_back(new Stream(this));
	classes.push_back(new System(this));	
}

void Program::init(){
	CD();
	if (getError()!=NULL)
		throw getError();
	validate();
}

void Program::runMain(char* className){
	for (int i = 0; i < (int)classes.size(); i++)
		classes[i]->initStaticField();
	Class* mainClass = this->findClass(className);
	if (mainClass == NULL)
		throw new ParserException("ClassNotFoundException ",className);
	Function* main = mainClass->getFunction("main",vector<Value*>(),true);
	if (main==NULL)
		throw new ParserException("NoSuchMethodException main()");
	main->execute();
}

Class* Program::findClass(char* name){
	for (int i = 0; i < (int)classes.size(); i++)
		if (strcmp(name,classes[i]->getName())==0)
			return classes[i];
	return NULL;
}

void Program::validate(){
	for (int i = 0; i < (int)classes.size(); i++)
		classes[i]->validateField();
	for (int i = 0; i < (int)classes.size(); i++)
		classes[i]->validateFunction();
}

Program::~Program(){
	delete lexan;
	for (int i = 0; i < (int)classes.size(); i++)
		if (classes[i]!=NULL)
			delete classes[i];
}
