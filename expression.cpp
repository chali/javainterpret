
/* implementace metod tridy Expression */

#include "program.h"

Node* Expression::Y(){
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
		case NULL_KEY:
			return YC(X());
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null na radku ",lexan->getLine()));
			return NULL;
	}
}

Node* Expression::YC(Node* n){
	switch(token.getType()){					
		case ASIGNMENT:{
				token = lexan->nextToken();
				statement = true;
				Node* as = new Assignment(n,YC(X()));
				as->setLine(lexan->getLine());
				return as;
			}
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu =, ), \',\' , ; na radku ",lexan->getLine()));
			return n;
	}
}

Node* Expression::X(){
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
		case NULL_KEY:
			return XC(B());
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null na radku ",lexan->getLine()));
			return NULL;
	}
}

Node* Expression::XC(Node* n){
	Node* temp;
	switch(token.getType()){	
		case AND:
			token = lexan->nextToken();
			temp = new And(n,B());
			temp->setLine(lexan->getLine());
			return XC(temp);
		case OR:
			token = lexan->nextToken();
			temp = new Or(n,B());
			temp->setLine(lexan->getLine());
			return XC(temp);		
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}
}

Node* Expression::B(){
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
		case NULL_KEY:
			return BC(E());
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null na radku ",lexan->getLine()));
			return NULL;
	}
}

Node* Expression::BC(Node* n){
	Node* temp;
	switch(token.getType()){
		case EQUAL:
			token = lexan->nextToken();
			temp = new Equal(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case NOT_EQUAL:
			token = lexan->nextToken(); 
			temp = new NotEqual(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case GREATER:
			token = lexan->nextToken();
			temp = new Greater(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case GREATER_OR_EQUAL:
			token = lexan->nextToken();
			temp = new GreaterOrEqual(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case LESSER:
			token = lexan->nextToken();
			temp = new Lesser(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case LESSER_OR_EQUAL:
			token = lexan->nextToken();
			temp = new LesserOrEqual(n,E());
			temp->setLine(lexan->getLine());
			return temp;
		case AND:
		case OR:
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu ==, !=, <,\n<=, >, >=, &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}
}


Node* Expression::E(){
	Node* temp;
	switch(token.getType()){
		case OPEN_ROUND_BRACKET:
		case INTEGER:
		case DOUBLE:
		case STRING:
		case TRUE:
		case FALSE:
		case NULL_KEY:
		case IDENTIFICATOR:
		case NEW:
			return EC(T());
		case MINUS:
			token = lexan->nextToken();
			temp = new UnaryMinus(T());
			temp->setLine(lexan->getLine());
			return EC(temp);
		case NOT:
			token = lexan->nextToken();
			temp = new Negation(T());
			temp->setLine(lexan->getLine());
			return EC(temp);
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, -, !, null na radku ",lexan->getLine()));
			return NULL;
		
	}
}
Node* Expression::EC(Node* n){
	Node* temp;
	switch (token.getType()) {
		case PLUS:
			token = lexan->nextToken();
			temp = new Plus(n,T());
			temp->setLine(lexan->getLine());
			return EC(temp);
		case MINUS:	
			token = lexan->nextToken();
			temp = new Minus(n,T());
			temp->setLine(lexan->getLine());
			return EC(temp);
		case EQUAL:
		case NOT_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case LESSER:
		case LESSER_OR_EQUAL:
		case AND:
		case OR:
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu +, -, ==, !=, <,\n<=, >, >=, &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}	
}

Node* Expression::T(){	
	switch(token.getType()){
		case OPEN_ROUND_BRACKET:
		case INTEGER:
		case DOUBLE:
		case STRING:
		case TRUE:
		case FALSE:
		case IDENTIFICATOR:
		case NEW:
		case NULL_KEY:
			return TC(F());
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, null na radku ",lexan->getLine()));
			return NULL;
	}
}
Node* Expression::TC(Node* n){
	Node* temp;
	switch (token.getType()) {
		case MUL:
			token = lexan->nextToken();
			temp = new Mul(n,F());
			temp->setLine(lexan->getLine());
			return TC(temp);
		case DIV:
			token = lexan->nextToken();
			temp = new Div(n,F());
			temp->setLine(lexan->getLine());
			return TC(temp);
		case PLUS:
		case MINUS:
		case EQUAL:
		case NOT_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case LESSER:
		case LESSER_OR_EQUAL:
		case AND:
		case OR:
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu *, /, +, -, ==, !=, <,\n<=, >, >=, &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}
}
Node* Expression::F(){
	Node* result;
	switch (token.getType()) {
		case INTEGER:
			result = new Constant(new Integer(token.getIntValue()));
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case DOUBLE:
			result = new Constant(new Double(token.getDoubleValue()));
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case STRING:
			result = new Constant(new String(token.getCharValue()));
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case TRUE:
			result = new Constant(new Boolean(true));
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case FALSE:
			result = new Constant(new Boolean(false));
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case NULL_KEY:
			result = new Constant(new Null());
			result->setLine(lexan->getLine());
			token = lexan->nextToken();
			return result;
		case OPEN_ROUND_BRACKET:
			token = lexan->nextToken();
			result = Y();
			if (token.getType() != CLOSE_ROUND_BRACKET)
				setError(new ParserException("Ocekavana ) na radku ",lexan->getLine()));
			token = lexan->nextToken();
			return result;
		case IDENTIFICATOR:
			return F6(NULL);
		case NEW:{
				statement = true;
				token = lexan->nextToken();
				if (token.getType()!=IDENTIFICATOR){
					setError(new ParserException("Ocekavan identifikator na radku ",lexan->getLine()));
					return NULL;
				}
				LexElement type = token;
				token = lexan->nextToken();
				if (token.getType()!=OPEN_ROUND_BRACKET){
					setError(new ParserException("Ocekavana ( na radku ",lexan->getLine()));
					return NULL;
				}
				token = lexan->nextToken();
				/*pouze konstruktor bez parametru*/
				//HelperParams* parrams = F3(new HelperParams());
				HelperParams* params = new HelperParams();
				if(token.getType()!=CLOSE_ROUND_BRACKET){
					setError(new ParserException("Ocekavana ) na radku ",lexan->getLine()));
				}
				result = new OperatorNew(type.getCharValue(),params->getParams(),this);
				result->setLine(lexan->getLine());
				delete params;
				token = lexan->nextToken();
				return F5(result);
			}	
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, cislo, retezec, true, false, identifikator, new, null na radku ",lexan->getLine()));
			return NULL;
	}
}

Node* Expression::FC(Node* n, char* name){
	Node* result;
	switch(token.getType()){
		case OPEN_ROUND_BRACKET:{
				statement = true;
				token = lexan->nextToken();
				HelperParams* parrams = F3(new HelperParams());
				result = new FunctionCall(n,name,parrams->getParams(),this);
				result->setLine(lexan->getLine());
				delete parrams;
				token = lexan->nextToken();
				return result;
			}
		case DOT:
		case MUL:
		case DIV:
		case PLUS:
		case MINUS:
		case EQUAL:
		case NOT_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case LESSER:
		case LESSER_OR_EQUAL:
		case AND:
		case OR:
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			result = new Variable(n,name,this);
			result->setLine(lexan->getLine());
			return result;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu (, ., *, /, +, -, ==, !=, <,\n<=, >, >=, &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}
}

HelperParams* Expression::F3(HelperParams* hiap){
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
		case NULL_KEY:
			hiap->addParam(Y());
			return F4(hiap);
		case CLOSE_ROUND_BRACKET:
			return hiap;
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu (, ), cislo, retezec, true, false, identifikator, new, -, !, null na radku ",lexan->getLine()));
			return hiap;
	}
}

HelperParams* Expression::F4(HelperParams* hiap){
	switch(token.getType()){
		case COMMA:
			token = lexan->nextToken();
			hiap->addParam(Y());
			return F4(hiap);
		case CLOSE_ROUND_BRACKET:
			return hiap;
		default:
			setError(new ParserException("Ocekavan jeden z nasledujicich elementu \',\', ) na radku ",lexan->getLine()));
			return hiap;
	}
}

Node* Expression::F5(Node* n){
	switch(token.getType()){
		case DOT:
			token = lexan->nextToken();
			return F6(n);
		case MUL:
		case DIV:
		case PLUS:
		case MINUS:
		case EQUAL:
		case NOT_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case LESSER:
		case LESSER_OR_EQUAL:
		case AND:
		case OR:
		case ASIGNMENT:
		case CLOSE_ROUND_BRACKET:
		case COMMA:
		case SEMICOLON:
			return n;
		default:
			setError(new ParserException(
				"Ocekavan jeden z nasledujicich elementu ., *, /, +, -, ==, !=, <,\n<=, >, >=, &&, ||, =, ), \',\' , ; na radku "
				,lexan->getLine()));
			return n;
	}
}

Node* Expression::F6(Node* n){
	switch(token.getType()){
		case IDENTIFICATOR:{
				LexElement name = token;
				token = lexan->nextToken();				
				return F5(FC(n,name.getCharValue()));
			}
		default:
			setError(new ParserException("Ocekavan identifikator na radku ",lexan->getLine()));
			return n;
	}
}

Expression::Expression(){
	error = NULL;
	c = NULL;
	b = NULL;
	statement = true;
}

Expression::Expression(Lexan* lexan, LexElement& token){
	error = NULL;
	this->lexan = lexan;
	this->token = token;
	c = NULL;
	b = NULL;
	statement = false;
	parsedExpression = Y();
}

Value* Expression::findValue(char* name, bool local){
	if (c!=NULL)
		/* deklarace ve tride */
		return c->getField(name);
	if (b!=NULL){
		/* lokalni deklarace */
		return b->findValue(name,local);
	}
	return NULL;
}

Class* Expression::parentClass(){
	if (c!=NULL)
		return c;
	else
		return b->getClass();
}

void copyValue(Value* v, Value* val){
	if (v->getType()==INT_VALUE){
		Integer* iv = (Integer*)v;
		Integer* ival = (Integer*)val;
		ival->setValue(iv->getValue());
	}
	else if (v->getType()==DOUBLE_VALUE){
		Double* iv = (Double*)v;
		Double* ival = (Double*)val;
		ival->setValue(iv->getValue());
	}
	else if (v->getType()==BOOLEAN_VALUE){
		Boolean* iv = (Boolean*)v;
		Boolean* ival = (Boolean*)val;
		ival->setValue(iv->getValue());
	}
	else if (v->getType()==STRING_VALUE){
		String* iv = (String*)v;
		String* ival = (String*)val;
		ival->setValue(iv->getValue());
	}
	else if (v->getType()==OBJECT_VALUE){
		Object* iv = (Object*)v;
		Object* ival = (Object*)val;
		ival->setClass(iv->getClass());
		ival->setValue(iv->getValue());
		ival->setNull(iv->isNull());
	}
}
