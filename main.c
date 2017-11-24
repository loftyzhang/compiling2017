//compiling2017
//一条咸鱼的自我救赎
//标识符长度等各种长度的越界都没有考虑，测试过程中需要注意
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define norw = 21;//number of reserved words

FILE* fin;
int err = 0;//number of errors
int num_t = 0;//读取的token数量
int num_l = 0;

char curc;//当前字符current char

char* reserved[norw];//   
char* typeof_sym[] = {
	"ident",    "rword",//标识符、保留字
    "illegal",  "string",//非法字符、字符串
    "lparen",   "rparen",//()
    "lbrace",   "rbrace",//{}
    "lsqbra",   "rsqbra",//[]
    "adding",   "multiplying",//加减乘除
    "relation", "assignment",//关系运算符，赋值符号=
    "integer",	"real",		"char",//整数、浮点数、字符
    "procedure","function",//过程、函数
    "comma",    "semicolon","period"，"colon"//','';''.':'
};///symbol的类型，主要是为了第一次作业服务

typedef struct sym{
	char name[20];
	char type[20];
	float value;
}symbol;//这里只记录symbol的名称和类型，若为变量或常量则以浮点数形式记录其值，字符保存的是ascii码

symbol token0;//当前的token

void error(int a ,int b);
int search_rword(char* s);///确认sym是否是保留字，若是则返回其标号，不是则返回-1

symbol get_sym();


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

symbol get_sym(){
    struct symbols token;
    char c = '/0';
    int i = 0;
    int j = 0;
    c = fgetc(fin);
    while(c=='/n'){
    	num_t++;
    	c = fgetc(fin);
    }//跳过连续的换行
    while(c==' '){
    	c = fgetc(fin);
    }//跳过连续的空格 能不能带上制表符不知道
    if ((c= fgetc(fin))==EOF){
    	curc = EOF;
    	strcpy(token.name,"EOF");
    	strcpy(token.type,"EOF");
    	token.value = -1;
    	return token;
    }//文件结尾，停止读取，返回eof，程序结束
////标识符、保留字、运算符、函数、过程、标点符号
    return token;
 }

int main(){
	char fname[100];//文件路径
	FILE* fout;
	symbol cur_token;
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

	printf("Please enter the name of file to compile./n");//使用绝对路径
	scanf("%s",fname);
	if((fin = fopen(fname,"r")==NULL){
		printf("Open failed");
		return 1;
	}
	fout = fopen("the_result.txt","w");
	while(1){
		num_t++;
		get_sym();
		if(curc==EOF){
			break;
		}
		fprinf(fout,"%d %s %s",num_t,token0.type,token0.name);
	}
	fprintf(fout,"end of file");
	fclose(fin);
	fclose(fout);

	return 0;
}