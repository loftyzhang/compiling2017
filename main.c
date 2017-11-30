//compiling2017
//一条咸鱼的自我救赎
//标识符长度等各种长度的越界都没有考虑，测试过程中需要注意
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define norw 21//number of reserved words

enum code{LIT,OPR,LOD,STO,CAL,CLL,ADD,JMP,JPC,RED,WRT}///这里add是数据栈顶指针增加a,CAL调用函数，CLL调用过程

code codes[1000]={RED};



FILE* fin;
int err = 0;//number of errors
int num_t = 0;//读取的token数量
int num_l = 0;//行数

char curc;//当前字符current char

char* reserved[norw];//
char* typeof_sym[] = {
	"ident",    "rword",//标识符、保留字
    "illegal",  "string",//非法字符、字符串
    "lparen",   "rparen",//()
    "lsqbra",   "rsqbra",//[]
    "adding",   "multiplying",//加减乘除
    "relation", "assignment",//关系运算符，赋值符号=
    "integer",	"real",		"char",//整数、浮点数、字符
    "procedure","function",//过程、函数
    "comma",    "semicolon","period","colon"//','';''.':'
};///symbol的类型，主要是为了第一次作业服务

typedef struct sym{
	char name[100];
	char type[20];
	float value;
}symbol;//这里只记录symbol的名称和类型，若为变量或常量则以浮点数形式记录其值，字符保存的是ascii码
//对于function 和procedure,value 保存其入口

symbol token0;//当前的token
symbol symbols[100];

void error(int a ,int b);
code listcode();
void statement();

symbol get_sym();
int position(symbol sym);

int search_rword(char* s);///确认sym是否是保留字，若是则返回其标号，不是则返回-1



void error(int a,int b){
    switch(b){
        case 1:printf("error in line %d,too long identifier",a);break;
        case 2:printf("error in line %d,illegal real input",a);break;
        case 3:printf("error in line %d,incompleted operator",a);break;
        case 4:printf("error in line %d,unpaired quotation marks",a);break;
        case 5:printf("error in line %d,illegal string",a);break;
        case 6:printf("error in line %d,illegal expression",a);break;
        case 7:printf("error in line %d,illegal step size",a);break;
        case 8:printf("error in line %d,illegal conditions",a);break;
        case 9:printf("error in line %d,illegal function call",a);break;
        case 10:printf("error in line %d,illegal variable declaration",a);break;
        case 11:printf("error in line %d,illegal array declaration",a);break;
        case 12:printf("error in line %d,illegal const declaration",a);break;
        case 13:printf("error in line %d,illegal function declaration",a);break;
        default:break;
    }
    err++;
}////error

code listcode(){

}

void statement(symbol sym){
    int i = 0;
    if(strcmp(sym.type,"ident")==0){

    }
}/* read  
    write
    const declaration
    variable declaration
    function declaration
    procedure declaration
    if
    for
    while
*/
/* 预计若为标识符，则根据是否已经登记入表判断是声明还是赋值
    若为句型标识则判断为对应句型，
    begin和end中递归调用此函数
    需要提前解决标识符登记问题，完善查找标识符的功能。
*/

void read_statement(){

}

void write_statement(){

}



symbol get_sym(){
    symbol token;
    char c = '\0';
    int i = 0;
    int n = 0;
    float j = 0;
    c = fgetc(fin);
    while((c=='\n')||(c==' ')||(c=='\t')){
        if(c == '\n'){
            num_l ++;
        }
        c = fgetc(fin);
    }
    n = c - '\0';
    if (c==EOF){
    	curc = EOF;
    	strcpy(token.name,"EOF");
    	strcpy(token.type,"EOF");
    	token.value = -1;
    	return token;
    }//文件结尾，停止读取，返回eof，程序结束
////标识符、保留字、运算符、函数、过程、标点符号
    else if(c == '+'||c == '-'){
    	token.name[0] = c;
    	token.name[1] = '\0';
    	strcpy(token.type,"adding");
    	token.value = (float)(c-'\0');//加法运算符，保存ascii码作为其value
    }
    else if(c == '*'||c == '/'){
    	token.name[0] = c;
    	token.name[1] = '\0';
    	strcpy(token.type,"multiplying");
    	token.value = (float)(c - '\0');//乘法运算符
    }
    else if(c == '<'){
    	if((c=fgetc(fin))=='='){
    		token.name[0] = '<';
    		token.name[1] = '=';
    		token.name[2] = '\0';
    		token.value = 0;//关系运算符之<=
    	}
    	else if(c == '>'){
    		token.name[0] = '<';
    		token.name[1] = '>';
    		token.name[2] = '\0';
    		token.value = 0;//关系运算符之<>
    	}
    	else{
    		ungetc(c,fin);
    		token.name[0] = c;
    		token.name[1] = '\0';
    		token.value = (float)(c - '\0');//关系运算符之<
    	}
    	strcpy(token.type,"relation");
    }
    else if(c == '>'){
    	if((c=fgetc(fin))=='='){
    		token.name[0] = '>';
    		token.name[1] = '=';
    		token.name[2] = '\0';
    		token.value = 0;//关系运算符之>=
    	}
    	else {
    		ungetc(c,fin);
    		token.name[0] = '>';
    		token.name[1] = '\0';
    		token.value = (float)(c - '\0');//关系运算符之>
    	}
    	strcpy(token.type,"relation");
    }
    else if(c == '='){
    	token.name[0] = c;
    	token.name[1] = '\0';
    	strcpy(token.type,"relation");
    	token.value = (float)(c - '\0');//关系运算符之=
    }
    else if(c == 34){///双引号
    	char s[100];
    	while((c = fgetc(fin))!='"'){
    		s[i++] = c;
    	}
    	s[i] = '\0';//字符串，没有考虑引号不配对等问题。字符串中没有保留引号
    	strcpy(token.name,s);
    	strcpy(token.type,"string");
    	token.value = 0;
    }
    else if(c == 39){///单引号
    	c = fgetc(fin);
    	token.name[0] = c;
    	token.name[1] = '\0';
    	strcpy(token.type,"char");//////字符
    	token.value = c - '\0';
    	if ((c=fgetc(fin))!=39){
    		error(num_l,4);//引号不匹配
    		strcpy(token.name,"illegal");
    		strcpy(token.type,"illegal");
    		token.value = 0;
    		ungetc(c,fin);
    		ungetc(39,fin);//不知道能不能连着两次ungetc，反正也没有这种奇葩错误把。。
    	}
    }
    else if(c>='0'&&c<='9'){///数字
    	char s[100];
    	s[0] = c;
    	j = j + (c-'0');
    	strcpy(token.type,"integer");
    	while(1){//这里暂时仅对于第一次作业的要求进行设计，实际上需要考虑数字后出现的字符是否合法的问题，
    			//正常来说数字后可以是空格、换行符、运算符等，而不会是eof(小数点算作数字的一部分)
    		c = fgetc(fin);
    		if(c>='0'&&c<='9'){
    			s[++i] = c;
    			j = 10*j + (c-'0');
    		}
    		else if(c == 46){
    			int k=1;
    			strcpy(token.type,"real");
    			s[++i] = c;
    			while(1){
    				c = fgetc(fin);
    				if(c>='0'&&c<='9'){
    					j = j + (float)pow(0.1,k)*(c-'0');
    					s[++i] = c;
    					k = k+1;
    				}
    				else{
    					s[++i] = '\0';//这里没有考虑报错的问题，只是认为数字结束
       					ungetc(c,fin);
    					break;
    				}
    			}//浮点数
    			strcpy(token.name,s);
    			token.value = j;
    			break;
    		}
    		else{
    			s[++i] = '\0';
    			strcpy(token.name,s);
    			token.value = j;///整数
    			ungetc(c,fin);
    			break;
    		}
    	}
    }
    else if(c>='a'&&c<='z'){
    	int k = 0;
    	char s[100];
    	s[0] = c;
    	while(1){
            c = fgetc(fin);
    		if((c<='z'&&c>='a')||(c>='0'&&c<='9')||(c<='Z'&&c>='A')){//标识符
    			s[++i] = c;
    		}
    		else{
    			s[++i] = '\0';
    			ungetc(c,fin);
    			break;///标识符结束
    		}
    	}
    	strcpy(token.name,s);
    	k = search_rword(s);
    	if(k>=0){
    		strcpy(token.type,"rword");
    		token.value = k;
    	}
    	else{
    		strcpy(token.type,"ident");
    		token.value = 0;
    	}
    }
    else if(c>='A'&&c<='Z'){
    	char s[100];
    	s[0] = c;
    	while(1){
    		if((((c=fgetc(fin))<='z')&&(c>='a'))||(c>='0'&&c<='9')||(c<='Z'&&c>='A')){//标识符
    			s[++i] = c;
    		}
    		else{
    			s[++i] = '\0';
    			ungetc(c,fin);
    			break;///标识符结束
    		}
    	}
    	strcpy(token.name,s);
    	strcpy(token.type,"ident");
    	token.value = 0;
    }
    else{
    	strcpy(token.name,"illegal");
    	strcpy(token.type,"illegal");
    	token.value = -1;
    }
    switch(n){
    	case 40:strcpy(token.name,"(");strcpy(token.type,"lparen");token.value = 0;break;///左括号
    	case 41:strcpy(token.name,")");strcpy(token.type,"rparen");token.value = 0;break;///右括号
    	case 44:strcpy(token.name,",");strcpy(token.type,"comma");token.value = 0;break;///逗号
    	case 46:strcpy(token.name,".");strcpy(token.type,"period");token.value = 0;break;///句点
    	case 58:strcpy(token.name,":");strcpy(token.type,"colon");token.value = 0;break;///冒号
    	case 59:strcpy(token.name,";");strcpy(token.type,"semicolon");token.value = 0;break;///分号
    	case 91:strcpy(token.name,"[");strcpy(token.type,"lsqbra");token.value = 0;break;///左方括号
    	case 93:strcpy(token.name,"]");strcpy(token.type,"rsqbra");token.value = 0;break;///右方括号
    	default:n = 0;break;
   	}
    return token;
 }

int search_rword(char* s){//保留字数组为字典序
    int high,low,mid;
    high = norw-1;
    low = 0;
    mid = norw/2;
    while(high>=low){
        mid = (high + low)/2;
        if(strcmp(reserved[mid],s)==0)
            return mid;
        if(strcmp(reserved[mid],s)>0)
            high = mid - 1;
        else if(strcmp(reserved[mid],s)<0)
            low = mid + 1;
    }
    return -1;
}///确认sym是否是保留字，若是则返回其标号，不是则返回-1

int main(){
	char fname[100];//文件路径
	FILE *fout;
	//symbol cur_token;
	reserved[0] = "array";	reserved[1] = "begin";
	reserved[2] = "char";	reserved[3] = "const";
	reserved[4] = "do";		reserved[5] = "downto";
	reserved[6] = "else";	reserved[7] = "end";
	reserved[8] = "for";	reserved[9] = "function";
	reserved[10] ="integer";reserved[11]= "if";
	reserved[12]= "of";		reserved[13]="procedure";
	reserved[14]= "read";	reserved[15]="real";
	reserved[16]= "then";	reserved[17]="to";
	reserved[18]= "var";	reserved[19]="while";
	reserved[20]= "write";

	printf("Please enter the name of file to compile.\n");//使用绝对路径
	scanf("%s",fname);
	if((fin = fopen(fname,"r"))==NULL){
		printf("Open failed");
		return 1;
	}
	//fout = fopen("the_result.txt","w");
	while(1){
		num_t++;
		token0 = get_sym();
		if(curc==EOF){
			break;
		}
		//fprintf(fout,"%d %s %s\n",num_t,token0.type,token0.name);
		printf("%d %s %s\n",num_t,token0.type,token0.name);
	}
	//fprintf(fout,"%s","end of file");
	//fprintf(fout,"%d %s %s %s\n",num_t,"123",token0.type,token0.name);
	printf("end of file");
	fclose(fin);
	//fclose(fout);

	return 0;
}
