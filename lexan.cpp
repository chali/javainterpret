#include "lexan.h"

LexElement Lexan::nextToken() {
	if (!init)
		return LexElement(ERROR);
	if ((int)back.size()>0){
		LexElement result = back.front();
		back.pop();
		return result;
	}
	int state = 0, token;                            
	/* strPos = pocet nactenych znaku identifikatoru */
	int i, strPos = 0;
	/* Velikost a znamenko exponentu */
	int exp = 0, expSign;
	/* prave zpracovavany desetinny rad */
	double dmsPos = 0.1;    
	int intValue;
	double floatValue;
	char charValue[100];
	while ( 1 ){
		token = fgetc (source);
		switch (state){
			case 0:
				if ( token == '\n')
					line++;
				if ( token == ' ' || token == '\n' || token == '\t' || token == '\r' || token == 13 ) 
					break;
				if ( token == '/' ){
					state = 1;
					break;
				} 
				if ( token == '0' ){
					intValue = 0;
					floatValue = 0.0;
					state = 13;
					break;
				}
				if ( token == '.' ){
					floatValue = 0.0;
					dmsPos = 0.1;
					state = 12;
					break;
				}
				if ( isdigit(token)){
					state = 7;
					intValue   = token - '0';
					floatValue = token - '0';
					break;
				}
				if ( isalpha ( token ) || token == '_' ){
					/* pocatek retezce identifikatoru*/
					charValue[0] = token; 
					strPos = 1;
					state = 6;
					break;
				}
				/* zacatek retezce */
				if (token == '\"'){
					state = 17;
					strPos = 0;
					break;
				}
				if (token == '+')
					return LexElement(PLUS);
				if (token == '-')
					return LexElement(MINUS);
				if (token == '*')
					return LexElement(MUL);
				if (token == '(')
					return LexElement(OPEN_ROUND_BRACKET);
				if (token == ')')
					return LexElement(CLOSE_ROUND_BRACKET);
				if (token == '[')
					return LexElement(OPEN_BRACKET);
				if (token == ']')
					return LexElement(CLOSE_BRACKET);
				if (token == '{')
					return LexElement(OPEN_BRACE);
				if (token == '}')
					return LexElement(CLOSE_BRACE);
				if (token == ',')
					return LexElement(COMMA);
				if (token == ';')
 					return LexElement(SEMICOLON);
				if (token == '='){
					state = 21;
					break;
				}
				if (token == '!'){
					state = 22;
					break;
				}
				if (token == '<'){
					state = 23;
					break;
				}
				if (token == '>'){
					state = 24;
					break;
				}
				if (token == '|'){
					state = 25;
					break;
				}
				if (token == '&'){
					state = 26;
					break;
				}
				if ( token == EOF )
					return LexElement(END_FILE);
				/* nepoznali jsme zacatek zadneho lex. elementu */
				return LexElement(ERROR);  
      
			/* deleni, radkovy nebo blokovy komentar */
			case 1:
				if ( token == '*' ){
					state = 3;
					break;
				}
				if ( token == '/' ) 
				{
					state = 4;
					break;
				}          
				/* Lexan jiz nemuze pokracovat zadnym prechodem. Protoze jsme v 'koncovem' */
				/* stavu, vratim ezpet nacteny znak a navratova hodnota bude DIV */
				ungetc(token,source);
				return LexElement(DIV);        
			
			/* blokovy komentar */
			case 3:
				if ( token == '*' ){
					state = 5;
					break;
				}
				/* pokud by uprostred komentare nastal EOF */
				/* a neosetrili bychom jej, Lexan by se zacyklil */
				if ( token == EOF ) 
					return LexElement(ERROR);
				if ( token == '\n')
					line++;
				/* zustavame ve stavu 3 */
				break; 
         
			/* blokovy komentar */
			case 5:
				if ( token == '*' )
					break;
				if ( token == '/' ){
					state = 0;
					break;
				}
				if ( token == '\n')
					line++;
				state = 3;
				break;          
			
			/* radkovy komentar */
			case 4:
				if ( token == '\n' ){
					line++;
					state = 0;
					break;
				}
				if ( token == EOF )
					/* kontrola EOF stejne jako ve stavu 3 */
					return LexElement(ERROR); 
				/* zustavame ve stavu 4 */
				break; 
         
			/* identifikator */
			case 6:
				if ( isalpha (token) || isdigit (token) || token == '_' ){
					/* HINT: podminka zajistuje, ze nevyjedeme mimo pole */ 
					/* navic si rezervujeme 1 pozici pro ukonceni retezce */
					/* znakem '\0' */
					if ( strPos < (int)sizeof (charValue) - 1 )
						charValue [strPos ++] = token;
					else
						cout << "Pozor max delka identifikatoru je 100 (radek " << line << ")" << endl;
					break;
				}   
				/* konec identifikatoru */
				ungetc ( token, source );
				/* semanticka akce v tomto stavu: doplneni 0 na konec retezce */
				/* test, zda retezec neni identifikator */
				charValue[strPos] = 0;
				for ( i=0; i < nrKeyWords; i ++ )
					if ( !strcmp ( charValue, keywords[i].word ) )
						return LexElement(keywords[i].keyWord);
				/* nenalezli jsme mezi klicovymi slovy -> je to identifikator */
				return LexElement(IDENTIFICATOR,charValue);  
         
			/* cele nebo desetinne cislo */
			case 7:
				if ( isdigit(token)){
					intValue = intValue * 10 + token - '0';
					floatValue = floatValue * 10.0 + token - '0';
					break;
				} 
				if ( token == '.' ){
					dmsPos = 0.1;
					state  = 8;
					break;
				}
				if ( token == 'e' || token == 'E' ){
					exp     = 0;
					expSign = 1;
					state   = 9;
					break;
				}
				/* nalezen integer */
				ungetc(token, source); 
				return LexElement(INTEGER,intValue);
			
			/* desetinne cislo */
			case 8:
				if ( isdigit(token)){
					floatValue += dmsPos * ( token - '0'); 
					dmsPos /= 10.0;
					break;
				}  
				if ( token == 'e' || token == 'E'){ 
					exp     = 0;
					expSign = 1;
					state   = 9;
					break;
				}
				/* desetinne cislo */
				ungetc ( token, source ); 
				return LexElement(DOUBLE,floatValue);

			/* exponent pro des. cislo */
			case 9:
				if ( token == '+' || token == '-' ){
					/* znamenko exponentu si zapanmatujeme */
					expSign = (token == '-') ? -1 : 1; 
					state   = 10;
					break;
				}
				if ( isdigit(token)){
					exp     = token - 0x30;
					state   = 11;
					break;
				}
				/* cislo nemuze koncit znakem 'e' */
				return LexElement(ERROR);
			
			/* exponent po znamenku */
			case 10:
				if ( isdigit(token )){
					/* je vyzadovana alespon 1 cifra exponentu */
					exp = token - 0x30; 
					state   = 11;
					break;
				} 
				return LexElement(ERROR);
			
			/* exponent */
			case 11:
				if ( isdigit(token)){
					/* dalsi cifry exponentu */
					exp = 10 * exp + token - 0x30; 
					break;
				} 
				/* zkompletujeme desetinne cislo z Lex_Float (mantisa), exp a expSign */
				ungetc ( token, source );
				floatValue *= pow ( 10.0, exp * expSign );
				return LexElement(DOUBLE,floatValue);

			/* desetinne cislo zacinajici .*/
			case 12:
				/* alespon 1 cirfa za desetinnou teckou */
				if ( isdigit(token)){
					state     = 8;
					floatValue = 0.1 * ( token - '0' );
					/* pristi zpracovavany rad jsou setiny */
					dmsPos    = 0.01; 
					break;
				}
				ungetc ( token, source );
				return LexElement(DOT);
			
			/* nula, desetinne, oktanove nebo hexa cislo */
			case 13:
				if ( token == 'x' ){
					/* nasleduje hex. cislo */
					state = 14; 
					break;
				} 
				if ( isdigit(token) && token != '8' && token != '9'){
					/* oktalove cifry */
					intValue = token - '0'; 
					state = 16;
					break;
				} 
				ungetc ( token, source );
				/* nalezenno cele cislo - nula */
				return LexElement(INTEGER,intValue); 
			
			/* hexa cislo */
			case 14:
				if ( isxdigit(token)){
					/* hex. cifry prevedeme na velka pismena */
					token  = toupper(token);
					/* musime rozdelit cifry a pismena, 'A' odpovida 10 */
					intValue  = ( token >= 'A' ) ? token - 'A' + 10 : token - '0';
					state    = 15;
					break;
				}
				/*  alespone 1 hex. cifra */
				return LexElement(ERROR); 
			
			/* hexa cislo */
			case 15:
				if (isxdigit(token)){
					/* podobne jako ve stavu 14 */
					token = toupper(token);
					/* nasobeni 16 je ekvivalentni posunu vlevo o 4 mista */
                    /* pozor: priorita operatoru << je nizsi nez priorita + ! */
					intValue  = ( intValue << 4 ) + (( token >= 'A' ) ? token - 'A' + 10 : token - '0');
					break;
				}  
				ungetc ( token, source );
				/* nalezli jsme cele cislo. Vyssi vrstve     */
				/* (syntaktickemu analyzatoru) je lhostejne, zda bylo zadano jako  */
				/* desitkove, osmickove nebo sestnactkove. Dulezite je, aby byla   */
				/* spravne vypoctena hodnota atributu Lex_Int                     */
				return LexElement(INTEGER,intValue);
			
			/* oktanove cislo */
			case 16:
				if ( isdigit(token) && token != '8' && token != '9'){
					/* nasobeni 8 je exvivalentni posunu 0o 3 vlevo */
					/* pozor na prioritu + */
					intValue = ( intValue << 3 ) + ( token - '0' );
					break;
				} 
				ungetc (token, source);
				return LexElement(INTEGER,intValue);
			
			/* stringy */
			case 17:
				if (token == '\\'){
					state = 18;
					break;
				}
				if (token == '\"'){
					charValue[strPos] = 0;
					return LexElement(STRING,charValue); 
				}
				if ( token == EOF || token == '\n') 
					return LexElement(ERROR);
				if ( strPos < (int)sizeof (charValue) - 1 )
					charValue [strPos ++] = token;
				else
					cout << "Pozor max delka retezce je 100 (radek " << line << ")" << endl;
				break;
			
			/* escape sekvence */
			case 18: 
				if (token == 'n'){
					charValue [strPos ++] = '\n';
					state = 17;
					break;
				}
				if (token == 'r'){
					charValue [strPos ++] = '\r';
					state = 17;
					break;
				}
				if (token == 't'){
					charValue [strPos ++] = '\t';
					state = 17;
					break;
				}
				if (token == 'a'){
					charValue [strPos ++] = '\a';
					state = 17;
					break;
				}
				if (token == '\\'){
					charValue [strPos ++] = '\\';
					state = 17;
					break;
				}
				if (token == '\"'){
					charValue [strPos ++] = '\"';
					state = 17;
					break;
				}
				return LexElement(ERROR);

			case 21:
				if (token == '=')
					return LexElement(EQUAL);
				ungetc ( token, source );
				return LexElement(ASIGNMENT);

		    case 22:
				if (token == '=')
					return LexElement(NOT_EQUAL);
				ungetc ( token, source );
				return LexElement(NOT);

			case 23:
				if (token == '=')
					return LexElement(LESSER_OR_EQUAL);
				ungetc ( token, source );
				return LexElement(LESSER);

			case 24:
				if (token == '=')
					return LexElement(GREATER_OR_EQUAL);
				ungetc ( token, source );
				return LexElement(GREATER);

			case 25:
				if (token == '|')
					return LexElement(OR);
				return LexElement(ERROR);

			case 26:
				if (token == '&')
					return LexElement(AND);
				return LexElement(ERROR);

			/* neznamy stav */
			default:
				return LexElement(ERROR);
				break;  
		}
	}
}

