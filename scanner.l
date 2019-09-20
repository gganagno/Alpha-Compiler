%{
#include "parser.h"
#define RED   "\033[1m\033[31m"
#define RESET "\x1B[0m"

YYSTYPE yylval;
int nested_num = 0;
%}


%option yylineno
%option noyywrap
%option outfile = "scanner.c"


int_num 			[0-9]+ 
double_num			[0-9]+\.[0-9]+

id 					[a-zA-Z][a-zA-Z_0-9]*
invalid_id			[_0-9][a-zA-Z_0-9]*
white_space			[\t\n ] 

assign_op 			[=]
add_op 				[+]  
minus_op 			[-]  
star_op 			[*] 
slash_op 			[/] 
mod_op 				[%]  
equal_op    		"==" 
nequal_op  			"!=" 
inc_op 				"++"  
dec_op 				"--"
big_op 				[>] 
less_op 			[<]  
bigeq_op   			">="  
lesseq_op 			"<="

left_brace 			[{]
right_brace 		[}]
left_bracket 		[[]
right_bracket 		[]]
left_parenthesis 	[(]
right_parenthesis 	[)]
semicolon 			[;]
comma 				[,]
colon 				[:]
double_colon 		"::"
dot 				[.]
double_dot 			".."

key_if 				"if"
key_else			"else"
key_while   		"while"
key_for				"for"
key_function 		"function"
key_return			"return"
key_break			"break"
key_continue 		"continue"
key_and 			"and"
key_and_sym			"&&"
key_not 			"not"
key_not_sym			[!]
key_or 				"or"
key_or_sym			"||"
key_local			"local"
key_true 			"true"
key_false 			"false"
key_nil 			"nil"

single_comm_slash 		"//".*

error_string 	"\""([^"\""]*(\\.)*)*
string 			"\""([^"\""]*(\\.)*)*"\""


%x COMMENT
invalid .

%%



"/*"				{BEGIN(COMMENT); }

<COMMENT>"/*"		{++nested_num;}

<COMMENT>"*"+"/"	{ 
						if (nested_num) {
							--nested_num;
						}
						else {
							BEGIN(INITIAL);
						}
					}				
<COMMENT>"*"+		;
<COMMENT>[^"/"*\n]+	;
<COMMENT>[/]		;
<COMMENT>\n			;
<COMMENT><<EOF>>	{BEGIN(INITIAL);}


{white_space}	{}
{key_if} 		{return key_if;}
{key_else} 		{return key_else;}
{key_while} 	{return key_while;}
{key_for} 		{return key_for;}
{key_function} 	{return key_function;}
{key_return} 	{return key_return;}
{key_continue} 	{return key_continue;}
{key_break} 	{return key_break;}
{key_and} 		{return key_and;}
{key_and_sym} 	{return key_and_sym;}
{key_not} 		{return key_not;}
{key_not_sym} 	{return key_not_sym;}
{key_or} 		{return key_or;}
{key_or_sym} 	{return key_or_sym;}
{key_local} 	{return key_local;}
{key_true} 		{return key_true;}
{key_false} 	{return key_false;}
{key_nil} 		{return key_nil;}

{assign_op} 	{return assign_op;}
{add_op} 		{return add_op;}
{minus_op}		{return minus_op;}
{star_op}		{return star_op;}
{slash_op}		{return slash_op;}
{mod_op}		{return mod_op;}
{equal_op} 		{return equal_op;}
{nequal_op}		{return nequal_op;}
{inc_op}		{return inc_op;}
{dec_op}		{return dec_op;}
{big_op}		{return big_op;}
{less_op}		{return less_op;}
{bigeq_op}		{return bigeq_op;}
{lesseq_op}		{return lesseq_op;}

{left_brace}			{return left_brace;}
{right_brace}			{return right_brace;}
{left_bracket}			{return left_bracket;}
{right_bracket}			{return right_bracket;}
{left_parenthesis}		{return left_parenthesis;}
{right_parenthesis}		{return right_parenthesis;}
{semicolon}				{return semicolon;}
{comma}					{return comma;}
{colon}					{return colon;}
{double_colon}			{return double_colon;}
{dot}					{return dot;}
{double_dot}			{return double_dot;}
{single_comm_slash}		{}


{string} 		{

	char * c = (char *) malloc (sizeof(char) * strlen(yytext)+1);
	int i,j;
	int flag=0,flag_num =0, quote=0;
	for( i=0,j=0;i < strlen(yytext)-1;i++){
		
		c[j] = yytext[i];

		if(yytext[i] == '\\'){

			switch (yytext[i+1]){

				case 'n':

					c[j++] = '\n';	
					i++;
					break;

				case '\\':

					c[j++] = '\\';
					i++;
					break;

				case 't':

					c[j++] = '\t';
					i++;
					break;

				case '\"':

					if(i+1 == strlen(yytext)-1){
						flag_num = i+1;
						flag=yylineno;
						quote=1;

					}else{
					
						c[j++] = '\"';
						i++;
						break;
					}
				case 'r':

					c[j++] = '\r';
					i++;
					break;

				case '\'':

					c[j++] = '\'';
					i++;
					break;

				case 'a':

					c[j++] = '\a';
					i++;
					break;

				case 'b':

					c[j++] = '\b';
					i++;
					break;

				case 'f':

					c[j++] = '\f';
					i++;
					break;

				case 'v':

					c[j++] = '\v';
					i++;
					break;

				case '?':

					c[j++] = '\?';
					i++;
					break;

				default:
				flag=yylineno;
				flag_num = i+1;
				break;
					
			}//switch
		
		}else{

			c[j++] = yytext[i];

		}//if

	}//for
		if(!flag){

			c[j++]=yytext[i];
			c[j]=0;
			
			{yylval.strVal = strdup(c); return string;}
		}else{
			if(quote){
			fprintf(stderr,"Escape character %c%c in line %d instead of closing string\n\n",yytext[flag_num-1],yytext[flag_num],flag);

			}else{
			fprintf(stderr,"Invalid escape character %c%c in line %d \n",yytext[flag_num-1],yytext[flag_num],flag);
			}
		}

}
{int_num}				{yylval.intVal =  atoi(yytext); return int_num;}
{double_num}			{yylval.doubleVal =  atof(yytext);return double_num;}
{id} 					{yylval.strVal = strdup(yytext); return id;}

{invalid_id}			{printf(RED"lexical error "RESET": Invalid identifier %s in line %d\n",yytext,yylineno );return invalid_id;} 
{error_string}			{printf(RED"lexical error "RESET": String not closed in line %d\n",yylineno );return error_string;} 
{invalid}				{printf(RED"lexical error "RESET": Invalid character %s in line %d\n",yytext,yylineno ); return invalid;} 

%%
