
#include "program.h"

void ExpressionCommand::validate(){
	ex->validate();
	if (!ex->getStatement())
		throw new ParserException("vyraz neni prikaz na radku ",line);
}

Value* Declaration::createValue(){
	Value* val = NULL;
	switch(type){
		case INT_VALUE:
			val = new Integer();
			break;
		case BOOLEAN_VALUE:
			val = new Boolean();
			break;
		case DOUBLE_VALUE:
			val = new Double();
			break;
		case OBJECT_VALUE:
			val = new Object(typeName);
			((Object*)val)->setClass(c);
			/*vytvori se object ale je null*/
			((Object*)val)->setNull(true);
			break;
		case STRING_VALUE:
			val = new String();
			break;
		default:
			break;
	}
	return val;
}

Declaration::Declaration(Expression* e, TypeValue tv, char* tn, char* n, bool s){
	expr = e;
	type = tv;
	stat = s;
	val = NULL;
	c = NULL;
	b = NULL;
	if (tn!=NULL)
		strncpy(this->typeName,tn,100);
	strncpy(this->name,n,100);
}

void Declaration::validate(){
	if (c==NULL){
		c = b->getClass();
	}
	if (type==OBJECT_VALUE){
		c = c->getProgram()->findClass(typeName);
		if (c==NULL)
			throw new ParserException("trida neexistuje, radek ",line);
	}
	if (expr!=NULL){
		Value* v = expr->validate();
		TypeValue tv = v->getType();
		/* povolena konverze int na double */
		if (v==NULL || (this->getValue()->getType()!=tv && ((this->getValue()->getType()!=DOUBLE_VALUE && tv!=INT_VALUE)
			 && (tv!=NULL_VALUE && this->getValue()->getType()!=OBJECT_VALUE))))
			throw new ParserException("chybna deklarace na radku ",line);
		else{
			if (v->getType()==OBJECT_VALUE)
				if (strcmp(((Object*)v)->getTypeName(),((Object*)this->getValue())->getTypeName())!=0)
					throw new ParserException("chybna deklarace na radku ",line);
		}
	}
	else{
		this->getValue();
	}
}

void Declaration::execute(){
	if (expr!=NULL){
		Value* v = expr->eval();
		if (v->getType()==val->getType()){
			copyValue(v,val);
		}
		/* typy nejsou sice stejne ale predchozi validace zajisti ze se jedna o kompatibilni konverzi z int na double nebo object a null*/
		else{
			if (v->getType() == NULL_VALUE){
				((Object*)val)->setNull(true);
			}
			else{
				Integer* iv = (Integer*)v;
				Double* ival = (Double*)val;
				ival->setValue(iv->getValue());
			}
		}
	}
}

void Declaration::pushOnStack(){ 
	/* vytvorime novou Value*/
	Value* temp = createValue();
	/* naplnime ji puvodni*/
	copyValue(val,temp);
	/* starou ulozime na stack */
	valStack.push(val);
	/* novou dame na misto stare */
	val = temp;
}

void Declaration::popFromStack(){
	/* vybereme starou */
	Value* temp = valStack.top();
	/* smazeme uz neplatnou */
	delete val;
	valStack.pop();
	/* misto smazana umistime tu ze stacku */
	val = temp;
}

Value* Declaration::getValue(){
	if (val==NULL){
		/* hodnota jeste neexistuje tak vytvorime, k tomuto volani dojede u kazde promenne behem validace*/
		val = createValue();
	}
	if (val->getType()==OBJECT_VALUE && c!=NULL)
		((Object*)val)->setClass(c);
	return val;
}

Value* Block::findValue(char* name, bool local){
	for (int i = 0; i < (int)(declarations.size()); i++)
		if (strcmp(name,declarations[i]->getName())==0)
			return declarations[i]->getValue();
	return parent->findValue(name,local);	
}

void Block::validate(){
	for(int i=0;i <(int)(commands.size());i++){
		Command* c = commands[i];
		c->validate();
		/* kontrola zda nemame vice promennych stejneho jmena */
		if (typeid(*c) == typeid(Declaration)){
			Declaration* d = (Declaration*)c;
			if (this->findValue(d->getName(),true)==NULL){
				declarations.push_back(d);
			}
			else
				throw new ParserException("promena uz je deklarovana, radek ", d->getLine());
		}			
	}
}

void Block::execute(){
	for(int i=0;i <(int)commands.size();i++)
		commands[i]->execute();
}

void Block::checkReturn(){
	for (int i=0; i < (int)commands.size(); i++){
		Command* c = commands[i];
		if (typeid(*c)==typeid(Block))
			((Block*)c)->checkReturn();
		else if (typeid(*c)==typeid(WhileCommand))			
			((WhileCommand*)c)->checkReturn();			
		else if (typeid(*c) == typeid(IfCommand))
			((IfCommand*)c)->checkReturn();			
		/* return musi byt na poslednim miste v bloku za nim uz nelze nic volat */
		else if (typeid(*c) == typeid(ReturnCommand))
			if (i+1!=(int)commands.size())
				throw new ParserException("za return nemuzou byt dalsi prikazy, radek ", c->getLine());
	}

}

void Block::checkReturn2(){
	int i = ((int)commands.size())-1;
	if (i < 0)
		throw new ParserException("chybi return, radek ", this->getLine());
	/* jelikoz nemame else tak posledni prikaz ve funkci ktera neni void musi byt return */
	Command* c = commands[i];
	if (typeid(*c) == typeid(Block))
		((Block*)c)->checkReturn2();
	else if (typeid(*c) != typeid(ReturnCommand))
		throw new ParserException("chybi return",c->getLine());
}

bool Block::getLocalDeclaration(Expression* e, vector<Declaration*>* ld){
	for (int i = 0; i < (int) commands.size(); i++){
		Command* c = commands[i];
		if (typeid(*c) == typeid(Declaration)){
			if (!((Declaration*)c)->containExpression(e))
				ld->push_back(((Declaration*)c));
			else
				return true;
		}
		else if (typeid(*c) == typeid(ExpressionCommand)){
			if (((ExpressionCommand*)c)->containExpression(e))
				return true;
		}
		else if (typeid(*c) == typeid(ReturnCommand)){
			if (((ReturnCommand*)c)->containExpression(e))
				return true;
		}
		else if (typeid(*c) == typeid(WhileCommand)){
			bool res = ((WhileCommand*)c)->getLocalDeclaration(e,ld);
			if (res)
				return true;
		}
		else if (typeid(*c) == typeid(IfCommand)){
			bool res = ((IfCommand*)c)->getLocalDeclaration(e,ld);
			if (res)
				return true;
		}
		else if (typeid(*c) == typeid(Block)){
			bool res = ((Block*)c)->getLocalDeclaration(e,ld);
			if (res)
				return true;
		}
	}
	return false;
}

Block::~Block(){
	for(int i=0;i <(int)commands.size();i++)
		if (commands[i]!=NULL)
			delete commands[i];
}

void IfCommand::validate(){
	Value* v = ex->validate();
	if (v->getType()!=BOOLEAN_VALUE)
		throw new ParserException("vyraz musi vracet boolean, radek ",line);
	if (typeid(*command)==typeid(Declaration))
		throw new ParserException("za if nemuze byt deklarace, radek ",line);
	command->validate();
}

void IfCommand::execute(){
	if (((Boolean*)ex->eval())->getValue())
		command->execute();	
}

void IfCommand::checkReturn(){
	if (typeid(*command)==typeid(Block))
		((Block*)command)->checkReturn();
	else if (typeid(*command)==typeid(IfCommand))
		((IfCommand*)command)->checkReturn();
	else if (typeid(*command)==typeid(WhileCommand))
		((WhileCommand*)command)->checkReturn();
}

bool IfCommand::getLocalDeclaration(Expression* e, vector<Declaration*>* ld){
	if (containExpression(e))
		return true;
	if (typeid(*command)==typeid(Block))
		return ((Block*)command)->getLocalDeclaration(e,ld);
	else if (typeid(*command)==typeid(IfCommand))
		((IfCommand*)command)->getLocalDeclaration(e,ld);
	else if (typeid(*command)==typeid(WhileCommand))
		((WhileCommand*)command)->getLocalDeclaration(e,ld);
	return false;
}

void WhileCommand::validate(){
	Value* v = ex->validate();
	if (v->getType()!=BOOLEAN_VALUE)
		throw new ParserException("vyraz musi vracet boolean, radek ",line);
	if (typeid(*command)==typeid(Declaration))
		throw new ParserException("za while nemuze byt deklarace, radek ",line);
	command->validate();
}

void WhileCommand::execute(){
	while (((Boolean*)ex->eval())->getValue())
		command->execute();	
}

void WhileCommand::checkReturn(){
	if (typeid(*command)==typeid(Block))
		((Block*)command)->checkReturn();
	else if (typeid(*command)==typeid(IfCommand))
		((IfCommand*)command)->checkReturn();
	else if (typeid(*command)==typeid(WhileCommand))
		((WhileCommand*)command)->checkReturn();
}

bool WhileCommand::getLocalDeclaration(Expression* e, vector<Declaration*>* ld){
	if (containExpression(e))
		return true;
	if (typeid(*command)==typeid(Block))
		return ((Block*)command)->getLocalDeclaration(e,ld);
	else if (typeid(*command)==typeid(IfCommand))
		((IfCommand*)command)->getLocalDeclaration(e,ld);
	else if (typeid(*command)==typeid(WhileCommand))
		((WhileCommand*)command)->getLocalDeclaration(e,ld);
	return false;
}

void ReturnCommand::validate(){
	Function* f = b->getParentFunction();
	if (f->getType()==VOID_VALUE && ex!=NULL){
		throw new ParserException("return nevraci nic pokud je navratova hodnota void, radek ",line);
	}
	else if (ex!=NULL){
		Value* v = ex->validate();
		if (v->getType()!=f->getType())
			throw new ParserException("chybna navratova hodnota, radek ",line);
		else if (v->getType()==OBJECT_VALUE && strcmp(((Object*)v)->getTypeName(),f->getTypeName())!=0)
			throw new ParserException("chybna navratova hodnota, radek ",line);
	}
	else if (f->getType()!=VOID_VALUE)
		throw new ParserException("return musi vracet hodnotu vyrazu, radek ",line);
}

void ReturnCommand::execute(){
	if (ex!=NULL){
		Value* v = ex->eval();
		throw v;
	}
	throw new Void();
}

Function::Function(vector<Declaration*> p, TypeValue tv, char* tn, char* n, bool s, Block* b){
	for (int i = 0; i< (int)p.size(); i++)
		params.push_back(p[i]);
	type = tv;
	stat = s;
	block = b;
	if (b!=NULL)
		block->setParentBlock(this);
	if (tn!=NULL)
		strncpy(this->typeName,tn,100);
	strncpy(this->name,n,100);
}

Value* Function::findValue(char *name, bool local){
	for (int i = 0; i < (int)(params.size()); i++)
		if (strcmp(name,params[i]->getName())==0)
			return params[i]->getValue();
	/*pokud nehledame jen lokalni promenne pokracujeme v hledani ve tride */
	if (!local)
		return c->getField(name,stat);
	return NULL;
}

void Function::validate(){
	if (type==OBJECT_VALUE){
		Class* ret = c->getProgram()->findClass(typeName);
		if (ret == NULL)
			throw new ParserException("neznamy navratovy typ, radek ",line);
	}
	for(int i=0; i< (int)params.size(); i++){
		params[i]->setBlock(this);
		params[i]->validate();
		for(int j=0; j<i; j++){
			if (strcmp(params[i]->getName(),params[j]->getName())==0)
				throw new ParserException("parametr uz existuje, radek ",line);
		}
	}
	/* jsou za return prikazy? */
	block->checkReturn();
	if (type!=VOID_VALUE)
		/* mame return pokud neni navratova hodnota void? */
		block->checkReturn2();
	block->validate();
}

bool Function::getLocalDeclaration(Expression* e, vector<Declaration*>* ld){
	for (int i=0; i < (int)params.size();i++)
		ld->push_back(params[i]);
	block->getLocalDeclaration(e,ld);
	return true;
}

vector<Value*> Function::getValuesParams(){
	vector<Value*> v;
	/* u instancnich metod je prvni parametr this ten preskakujeme */
	for(int i=stat?0:1; i< (int)params.size(); i++)
		v.push_back(params[i]->getValue());
	return v;
}

vector<Value*> Function::getParams(){
	vector<Value*> r;
	for (int i=0; i < (int) params.size(); i++)
		r.push_back(params[i]->getValue());
	return r;
}

Function::~Function(){
	for(int i=0; i< (int)params.size(); i++)
		if (params[i]!=NULL)
			delete params[i];
	if (block!=NULL)
		delete block;
}

void Class::addDeclaration(Declaration* d){
	d->setClass(this);
	if (d->isStatic()){
		d->validate();
		fields.push_back(d);
	}
	else{
		/* nestaticke deklarace validujeme pozdeji */
		preValidateFields.push_back(d);	
	}
}

void Class::addFunction(Function* f){
	f->setClass(this);
	if (getFunction(f->getName(), f->getValuesParams())!=NULL)
		throw new ParserException("funkce uz existuje, radek ", f->getLine());
	functions.push_back(f);
}

vector<Declaration*> Class::getDeclarations(bool stat){
	vector<Declaration*> res;
	for (int i = 0; i < (int) fields.size(); i++){
		if (stat == fields[i]->isStatic())
			res.push_back(fields[i]);
	}
	return res;
}

Value* Class::getField(char* name, bool stat){
	for (int i = 0; i < (int)fields.size(); i++)
		if (strcmp(name,fields[i]->getName())==0)
			if (stat){
				if (fields[i]->isStatic())
					return fields[i]->getValue();
			}
			else{
				return fields[i]->getValue();
			}
	return NULL;
}

void Class::initStaticField(){
	vector<Declaration*> fields = getDeclarations(true);
	for (int i = 0; i < (int) fields.size(); i++){
		Declaration* d = fields[i];
		d->execute();
	}
}

Function* Class::getFunction(char* name, vector<Value*> values, bool stat){
	/* pri hledani funkce se musi zohlednit i parametry */
	for (int i = 0; i < (int)functions.size(); i++){
		Function* f = functions[i];
		vector<Value*> v = f->getValuesParams();
		bool eq = true;
		if ((int)values.size()==(int)v.size()){
			for (int j=0; j< (int)v.size();j++){
				if (values[j]->getType() != v[j]->getType() && values[j]->getType()!=NULL_VALUE && v[j]->getType()!=OBJECT_VALUE)
					eq = false;
				else
					if (values[j]->getType()==OBJECT_VALUE && strcmp(((Object*)values[j])->getTypeName(),((Object*)v[j])->getTypeName())!=0)
						eq = false;
			}
		}
		else
			eq = false;
		if (strcmp(name,f->getName())==0 && eq)
			if (!stat || f->isStatic())
				return f;
		}
	return NULL;
}

void Class::validateField(){
	for (int i = 0; i < (int)preValidateFields.size(); i++){			
		preValidateFields[i]->validate();
		fields.push_back(preValidateFields[i]);
	}
	preValidateFields.clear();		
}

void Class::validateFunction(){
	for (int i = 0; i < (int)functions.size(); i++)
		functions[i]->validate();
}

Class::~Class(){
	for (int i = 0; i < (int)fields.size(); i++)
		if (fields[i]!=NULL)
			delete fields[i];
	for (int i = 0; i < (int)functions.size(); i++)
		if (functions[i]!=NULL)
			delete functions[i];
}

System::System(Program* pr){
	Expression* e = new Expression();
	OperatorNew* n = new OperatorNew("Stream",vector<Node*>(),e);
	e->setNode(n);
	e->setClass(this);
	Declaration* d = new Declaration(e,OBJECT_VALUE,"Stream","out",true);
	d->setClass(this);
	p = pr;
	d->validate();
	fields.push_back(d);
}
