
#include "program.h"

OperatorNew::OperatorNew(char* t, vector<Node*> p, Expression* ex){
	strncpy(this->type,t,100);
	e = ex;
	for(int i = 0; i < (int)p.size(); i++)
		params.push_back(p[i]);
}

Value* OperatorNew::validate(){
	for (int i = 0; i < (int) params.size(); i++)
		params[i]->validate();
	Class* c = e->parentClass()->getProgram()->findClass(type);
	if (c==NULL)
		throw new ParserException("neznamy identifikator, radek ",line);
	value = new Object(type);
	((Object*)value)->setClass(c);
	return value;
}

Value* OperatorNew::eval(){
	for (int i = 0; i < (int) params.size(); i++)
		params[i]->eval();
	Class* c = e->parentClass()->getProgram()->findClass(type);
	/*seznam instancnich poli*/
	vector<Declaration*> fields = c->getDeclarations(false);
	for (int i = 0; i < (int) fields.size(); i++){
		Declaration* d = fields[i];
		d->execute();
		Value* v = d->getValue();
		Value* val;
		switch(v->getType()){
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
				val = new Object(type);
				((Object*)val)->setClass(c);
				break;
			case STRING_VALUE:
				val = new String();
				break;
			default:
				val = NULL;	
				break;
		}
		/* vytvoreni hodnoty pro kazde pole */
		copyValue(v,val);
		((Object*)value)->initField(d->getName(),val);

	}
	((Object*)value)->setNull(false);
	return value;
}

OperatorNew::~OperatorNew(){
	for(int i = 0; i < (int)params.size(); i++)
		delete params[i];
}

Value* Variable::createValue(Value* v){
	Value* val = NULL;
	switch(v->getType()){
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
			val = new Object(((Object*)v)->getTypeName());
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
Variable::Variable(Node* parentObject, char* name, Expression* e):UnaryOperation(parentObject){
	strncpy(this->name,name,100);
	ex = e;
	cv = NULL;
}

Value* Variable::validate(){
	if (node!=NULL){
		/* pristupuju k poli pres . */
		Value* result = node->validate();
		if (result->getType()==OBJECT_VALUE){
			Object* o = (Object*) result;
			result = o->getClass()->getField(name);		
			if (result==NULL)
				throw new ParserException("nezname pole, radek ",name,line);
			result->setLValue(true);
			return result;
		}
		/*staticka promenna*/
		if (result->getType()==CLASS_VALUE){
			ClassValue* o = (ClassValue*) result;
			result = o->getClass()->getField(name,true);
			if (result==NULL)
				throw new ParserException("nezname pole, radek ",name,line);
			result->setLValue(true);
			return result;			
		}
		throw new ParserException("neni dovolena ., radek ",line);
	}
	else{
		Value* result = ex->findValue(name);
		if (result == NULL){
			Class* cla = ex->parentClass()->getProgram()->findClass(name);
			if (cla!=NULL){
				ClassValue* cv = new ClassValue();
				this->cv = cv;
				cv->setClass(cla);
				return cv;
			}
			throw new ParserException("neznamy identifikator ",name,line);
		}
		result->setLValue(true);
		return result;
	}
}

Value* Variable::eval(){
	if (node!=NULL){
		/* pristupuju k poli pres . */
		Value* result = node->eval();
		if (result->getType()==OBJECT_VALUE){
			Object* o = (Object*) result;
			bool isStaticField = false;
			vector<Declaration*> vd = o->getClass()->getDeclarations(true);
			for (int i = 0; i < (int)vd.size(); i++)
				if (strcmp(name,vd[i]->getName())==0)
					isStaticField = true;
			if (o->isNull()&&!isStaticField)
				throw new ParserException("NullPointerException, radek ",line);;
			if (isStaticField){
				result = o->getClass()->getField(name,true);
			}
			else{
				result = o->getField(name);
			}
			if (assignment){
				/* chci priradit vratim ukazatel na hodnotu ulozenou v deklaraci*/
				return result;
			}
			else{
				value = createValue(result);
				copyValue(result,value);
				return value;
			}	
		}
		else{
			ClassValue* o = (ClassValue*) result;
			result = o->getClass()->getField(name,true);
			if (assignment){
				return result;
			}
			else{
				value = createValue(result);
				copyValue(result,value);
				return value;
			}				
		}
	}
	else{	
		if (cv != NULL){
			/* validace urcila ze pristupuji ke statickemu poli*/
			return cv;
		}
		Value* result = ex->findValue(name,true);
		if (result == NULL){
			result = ex->findValue("this",true);
			if (result!=NULL)
				result = ((Object*)result)->getField(name);
		}
		if (result ==NULL){
			result = ex->findValue(name);
		}
		if (assignment){
			/* chci priradit vratim ukazatel na hodnotu ulozenou v deklaraci*/
			return result;
		}
		else{
			value = createValue(result);
			copyValue(result,value);
			return value;
		}		
	}
}

Variable::~Variable(){
	if (cv!=NULL)
		delete cv;
}

FunctionCall::FunctionCall(Node* parent, char* n, vector<Node*> p, Expression* e):UnaryOperation(parent){
	ex = e;
	strncpy(this->name,n,100);
	for(int i = 0; i < (int)p.size(); i++)
		params.push_back(p[i]);
}

Value* FunctionCall::validate(){
	vector<Value*> v;
	Function* fu = NULL;
	for(int i = 0; i < (int)params.size(); i++)
		v.push_back(params[i]->validate());
	if (node!=NULL){
		Value* val = node->validate();
		if (val->getType()==CLASS_VALUE){
			/* pristupuju ke staticke metode */
			ClassValue* cv = (ClassValue*) val;
			fu = cv->getClass()->getFunction(name,v,true);
			if (fu==NULL)
				throw new ParserException("neznama funkce ",name,line);
		}
		else if (val->getType()==OBJECT_VALUE){
			Object* o = (Object*) val;
			fu = o->getClass()->getFunction(name,v);
			if (fu==NULL)
				throw new ParserException("neznama funkce ",name,line);
		}
		else
			throw new ParserException("nelze volat funkci, radek ",line);
	}
	else{
		fu= ex->parentClass()->getFunction(name,v);
		if (fu==NULL)
				throw new ParserException("neznama funkce ",name,line);
		if (!fu->isStatic()){
			/* volam instancni metodu bez psani this musim ho dodatecne najit, jinak to obstara Variable*/
			Value* t = ex->findValue("this");
			if (t==NULL)
				throw new ParserException("nelze volat ve statickem kontextu, radek ",line);
		}
	}
	if (fu == NULL)
		throw new ParserException("neznama funkce, radek ",line);
	TypeValue t = fu->getType();
	if (t==INT_VALUE)
		value = new Integer();
	else if (t==DOUBLE_VALUE)
		value = new Double();
	else if (t==STRING_VALUE)
		value = new String();
	else if (t==BOOLEAN_VALUE)
		value = new Boolean();
	else if (t==OBJECT_VALUE){
		value = new Object(fu->getTypeName());
		((Object*)value)->setClass(ex->parentClass()->getProgram()->findClass(fu->getTypeName()));
	}
	else
		value = NULL;
	f = fu;
	return value;

}

Value* FunctionCall::eval(){
	vector<Declaration*>* ld = new vector<Declaration*>();
	Block* b = ex->getBlock();
	/* seznam lokalnich deklaraci */
	if (b!=NULL)
		b->getParentFunction()->getLocalDeclaration(ex,ld);
	vector<Value*> params = f->getParams();
	if (node!=NULL){
		Value* val = node->eval();		
		/* pokud nestaticka dosadim implicitni this */
		if (val->getType()==OBJECT_VALUE && !f->isStatic()){
			Object* o = (Object*) val;
			copyValue(o,params[0]);
		}
	}
	else{
		if (!f->isStatic()){
			Value* t = ex->findValue("this");
			copyValue(t,params[0]);
		}		
	}
	/* vyhodnoceni parametru */
	for (int i = f->isStatic()?0:1; i < (int)params.size();i++){
		Value* param = this->params[f->isStatic()?i:i-1]->eval();
		if (param->getType()!=NULL_VALUE)
			copyValue(param,params[i]);
	}
	/* ulozeni promennych na stack */
	for(int i=0; i<(int) ld->size(); i++)
		(*ld)[i]->pushOnStack();
	try{
		f->execute();
	}
	/* return vyhazuje vyjimku typu Value coz je navratova hodnota zde zachytim */
	catch (Value* ret){
		if (ret->getType()!=VOID_VALUE)
			copyValue(ret,value);
		else{
			delete ret;
		}
	}
	/* obnoveni ze zasobniku */
	for(int i=0; i<(int) ld->size(); i++)
		(*ld)[i]->popFromStack();
	delete ld;
	return value;
}

FunctionCall::~FunctionCall(){
	for(int i = 0; i < (int)params.size(); i++)
		delete params[i];
}

Value* UnaryMinus::validate(){
	Value* v = node->validate();
	if (v->getType()==INT_VALUE){			
		value = new Integer();
		return value;
	}
	if (v->getType()==DOUBLE_VALUE){
		value = new Double();
		return value;
	}
	return NULL;
}

Value* UnaryMinus::eval(){
	Value* v = node->eval();
	if (v->getType()==INT_VALUE){			
		((Integer*)value)->setValue(-((Integer*)v)->getValue());			
	}
	else{
		((Double*)value)->setValue(-((Double*)v)->getValue());
	}
	return value;
}

Value* Negation::validate(){
	Value* v = node->validate();
	if (v->getType()==BOOLEAN_VALUE){
		value = new Boolean();
		return value;
	}
	return NULL;
}

Value* Negation::eval(){
	Boolean* v = (Boolean*)node->eval();
	((Boolean*)value)->setValue(!v->getValue());
	return value;
}

Value* Plus::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==INT_VALUE && rv->getType()==INT_VALUE){
		value = new Integer();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Double();
		return value;
	}
	else if ((lv->getType()==STRING_VALUE || rv->getType()==STRING_VALUE) && 
		lv->getType()!=OBJECT_VALUE && lv->getType()!=NULL_VALUE  &&
		rv->getType()!=OBJECT_VALUE && rv->getType()!=NULL_VALUE ){
		value = new String();
		return value;
	}
	throw new ParserException("chybny operand pro operaci +, radek ",line);
}

Value* Plus::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE && rv->getType()!=STRING_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Integer*)value)->setValue(ilv->getValue()+irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(ilv->getValue()+drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE && rv->getType()!=STRING_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Double*)value)->setValue(dlv->getValue()+irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(dlv->getValue()+drv->getValue());
		}
	}
	/* pokud je nejaky operand String provedu concat */
	else if (lv->getType()==STRING_VALUE){
		String* slv = (String*) lv;
		if (rv->getType()==STRING_VALUE){
			String* srv = (String*) rv;
			string s = slv->getValue();
			s = s + srv->getValue();
			((String*)value)->setValue(s.c_str());
		}
		else{
			char buf[100];
			int n = 0;
			switch(rv->getType()){
				case INT_VALUE:
					n = sprintf(buf,"%d",((Integer*)rv)->getValue());
					break;
				case DOUBLE_VALUE:
					n = sprintf(buf,"%f",((Double*)rv)->getValue());
					break;
				case BOOLEAN_VALUE:
					if (((Boolean*)rv)->getValue())
						strcpy(buf,"true");
					else
						strcpy(buf,"false");
					break;
				default:
					break;
			}
			if (n<0)
				throw new ParserException("prekrocena delka retezce pri konverzi cisla, radek ",line);
			string s = slv->getValue();
			s = s + buf;
			((String*)value)->setValue(s.c_str());
		}
	}
	else if (rv->getType()==STRING_VALUE){
		String* srv = (String*) rv;
		if (lv->getType()==STRING_VALUE){
			String* slv = (String*) lv;
			string s = slv->getValue();
			s = s + srv->getValue();
			((String*)value)->setValue(s.c_str());
		}
		else{
			char buf[100];
			int n = 0;
			switch(lv->getType()){
				case INT_VALUE:
					n = sprintf(buf,"%d",((Integer*)lv)->getValue());
					break;
				case DOUBLE_VALUE:
					n = sprintf(buf,"%f",((Double*)lv)->getValue());
					break;
				case BOOLEAN_VALUE:
					if (((Boolean*)lv)->getValue())
						strcpy(buf,"true");
					else
						strcpy(buf,"false");
					break;
				default:
					break;
			}
			if (n<0)
				throw new ParserException("prekrocena delka retezce pri konverzi cisla, radek ",line);
			string s = buf;
			s = s + srv->getValue();			
			((String*)value)->setValue(s.c_str());
		}
	}
	return value;
}

Value* Minus::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==INT_VALUE && rv->getType()==INT_VALUE){
		value = new Integer();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Double();
		return value;
	}
	throw new ParserException("chybny operand pro operaci -, radek ",line);
}

Value* Minus::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Integer*)value)->setValue(ilv->getValue()-irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(ilv->getValue()-drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Double*)value)->setValue(dlv->getValue()-irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(dlv->getValue()-drv->getValue());
		}
	}
	return value;
}

Value* Mul::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==INT_VALUE && rv->getType()==INT_VALUE){
		value = new Integer();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Double();
		return value;
	}
	throw new ParserException("chybny operand pro operaci *, radek ",line);
}

Value* Mul::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Integer*)value)->setValue(ilv->getValue()*irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(ilv->getValue()*drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Double*)value)->setValue(dlv->getValue()*irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(dlv->getValue()*drv->getValue());
		}
	}
	return value;
}

Value* Div::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==INT_VALUE && rv->getType()==INT_VALUE){
		value = new Integer();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Double();
		return value;
	}
	throw new ParserException("chybny operand pro operaci /, radek ",line);
}

Value* Div::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Integer*)value)->setValue(ilv->getValue()/irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(ilv->getValue()/drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Double*)value)->setValue(dlv->getValue()/irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Double*)value)->setValue(dlv->getValue()/drv->getValue());
		}
	}
	return value;
}

Value* Equal::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()== rv->getType() && lv->getType()!=STRING_VALUE){
		value = new Boolean();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
		return value;
	}
	else if ((lv->getType()==OBJECT_VALUE || lv->getType()==NULL_VALUE) && (rv->getType()==OBJECT_VALUE || rv->getType()==NULL_VALUE)){
		value = new Boolean();
		return value;
	}
	throw new ParserException("chybny operand pro operaci ==, radek ",line);
}

Value* Equal::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
		Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()==irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()==drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()==irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()==drv->getValue());
		}
	}
	else if (lv->getType()==BOOLEAN_VALUE){
		Boolean* blv = (Boolean*) lv;
		Boolean* brv = (Boolean*) rv;
		((Boolean*)value)->setValue(blv==brv);
	}
	/* porovnavani objectu mezi sebou a s null  ( mezi sebou je to porovnavani adres ale nebudou urcite stejne i kdyz by se melo
	jednat o jednu a tu samou instaci v ramci programu = nepouzitelne)*/
	if (lv->getType()==OBJECT_VALUE){
		Object* ilv = (Object*)lv;
		if (rv->getType()==OBJECT_VALUE){
			Object* irv = (Object*) rv;
			((Boolean*)value)->setValue(ilv==irv);
		}
		else if (rv->getType()==NULL_VALUE){
			((Boolean*)value)->setValue(ilv->isNull());
		}
	}
	else if (lv->getType()==NULL_VALUE){
		if (rv->getType()==OBJECT_VALUE){
			Object* irv = (Object*) rv;
			((Boolean*)value)->setValue(irv->isNull());
		}
		else if (rv->getType()==NULL_VALUE){
			((Boolean*)value)->setValue(true);
		}
	}
	return value;
}

Value* NotEqual::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()== rv->getType() && lv->getType()!=STRING_VALUE){
		value = new Boolean();
		return value;
	}
	else if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
		return value;
	}
	else if ((lv->getType()==OBJECT_VALUE || lv->getType()==NULL_VALUE) && (rv->getType()==OBJECT_VALUE || rv->getType()==NULL_VALUE)){
		value = new Boolean();
		return value;
	}
	throw new ParserException("chybny operand pro operaci !=, radek ",line);
}

Value* NotEqual::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()!=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()!=drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()!=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()!=drv->getValue());
		}
	}
	else if (lv->getType()==BOOLEAN_VALUE){
		Boolean* blv = (Boolean*) lv;
		Boolean* brv = (Boolean*) rv;
		((Boolean*)value)->setValue(blv!=brv);
	}
	if (lv->getType()==OBJECT_VALUE){
		Object* ilv = (Object*)lv;
		if (rv->getType()==OBJECT_VALUE){
			Object* irv = (Object*) rv;
			((Boolean*)value)->setValue(ilv!=irv);
		}
		else if (rv->getType()==NULL_VALUE){
			((Boolean*)value)->setValue(!ilv->isNull());
		}
	}
	else if (lv->getType()==NULL_VALUE){
		if (rv->getType()==OBJECT_VALUE){
			Object* irv = (Object*) rv;
			((Boolean*)value)->setValue(!irv->isNull());
		}
		else if (rv->getType()==NULL_VALUE){
			((Boolean*)value)->setValue(false);
		}
	}
	return value;
}

Value* Greater::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
	return value;
	}
	throw new ParserException("chybny operand pro operaci >, radek ",line);
}

Value* Greater::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()>irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()>drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()>irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()>drv->getValue());
		}
	}
	return value;
}

Value* GreaterOrEqual::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
		return value;
	}
	throw new ParserException("chybny operand pro operaci >=, radek ",line);
}

Value* GreaterOrEqual::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()>=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()>=drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()>=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()>=drv->getValue());
		}
	}
	return value;
}

Value* Lesser::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
		return value;
	}
	throw new ParserException("chybny operand pro operaci <, radek ",line);
}

Value* Lesser::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()<irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()<drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()<irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()<drv->getValue());
		}
	}
	return value;
}

Value* LesserOrEqual::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if ((lv->getType()==INT_VALUE || lv->getType()==DOUBLE_VALUE) && (rv->getType()==INT_VALUE || rv->getType()==DOUBLE_VALUE)){
		value = new Boolean();
		return value;
	}
	throw new ParserException("chybny operand pro operaci <=, radek ",line);
}

Value* LesserOrEqual::eval(){
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()==INT_VALUE){
		Integer* ilv = (Integer*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(ilv->getValue()<=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(ilv->getValue()<=drv->getValue());
		}
	}
	else if (lv->getType()==DOUBLE_VALUE){
		Double* dlv = (Double*)lv;
		if (rv->getType()==INT_VALUE){
			Integer* irv = (Integer*) rv;
			((Boolean*)value)->setValue(dlv->getValue()<=irv->getValue());
		}
		else if (rv->getType()==DOUBLE_VALUE){
			Double* drv = (Double*) rv;
			((Boolean*)value)->setValue(dlv->getValue()<=drv->getValue());
		}
	}
	return value;
}

Value* And::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==BOOLEAN_VALUE && rv->getType()==BOOLEAN_VALUE){
		value = new Boolean();
		return value;
	}		
	throw new ParserException("chybny operand pro operaci &&, radek ",line);
}

Value* And::eval(){
	Boolean* lv = (Boolean*)left->eval();
	Boolean* rv = (Boolean*)right->eval();		
	((Boolean*)value)->setValue(lv->getValue()&&rv->getValue());
	return value;
}

Value* Or::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv->getType()==BOOLEAN_VALUE && rv->getType()==BOOLEAN_VALUE){
		value = new Boolean();
		return value;
	}		
	throw new ParserException("chybny operand pro operaci ||, radek ",line);
}

Value* Or::eval(){
	Boolean* lv = (Boolean*)left->eval();
	Boolean* rv = (Boolean*)right->eval();		
	((Boolean*)value)->setValue(lv->getValue()||rv->getValue());
	return value;
}

Value* Assignment::validate(){
	Value* lv = left->validate();
	Value* rv = right->validate();
	if (lv==NULL || !lv->getLValue())
		throw new ParserException("leva strana neni lvalue, radek ",line);
	if (lv->getType()== rv->getType()){
		if (lv->getType()==INT_VALUE)
			value = new Integer();
		else if (lv->getType()==DOUBLE_VALUE)
			value = new Double();
		else if (lv->getType()==STRING_VALUE)
			value = new String();
		else if (lv->getType()==BOOLEAN_VALUE)
			value = new Boolean();
		else if (lv->getType()==OBJECT_VALUE){
			value = new Object(((Object*)lv)->getTypeName());
		}
		else
			throw new ParserException("chybny operand pro operaci =, radek ",line);
		return value;
	}
	else if (lv->getType()==DOUBLE_VALUE && rv->getType()==INT_VALUE){
		value = new Double();
		return value;
	}
	else if (lv->getType()==OBJECT_VALUE && rv->getType()==NULL_VALUE){
		value = new Object(((Object*)lv)->getTypeName());
		return value;
	}
	throw new ParserException("chybny operand pro operaci =, radek ",line);
}

Value* Assignment::eval(){
	/* timto rekneme ze chceme prirazovat a dostaneme ukazatel na hodnotu v deklaraci */
	left->setAssignment(true);
	Value* lv = left->eval();
	Value* rv = right->eval();
	if (lv->getType()== rv->getType()){
		copyValue(rv,value);
		copyValue(value,lv);
		return value;
	}
	else if (lv->getType()==DOUBLE_VALUE && rv->getType()==INT_VALUE){
		((Double*)value)->setValue(((Integer*)rv)->getValue());
		copyValue(value,lv);
		return value;
	}
	else if (lv->getType()==OBJECT_VALUE && rv->getType()==NULL_VALUE){
		((Object*)value)->setNull(true);
		copyValue(value,lv);
		return value;
	}
	return NULL;
}


