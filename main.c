//compiling2017
//一条咸鱼的自我救赎
//标识符长度等各种长度的越界都没有考虑，测试过程中需要注意
//这个作业额没那么高级，我也不打算解决参相关的错误了。
/*目前为止的思路如下：在正常进行语法分析的基础上，完成目标代码的生成，在生成目标码的过程中，
通过以某一函数或过程在符号表中的起始位置为起点，结合偏移量实现其中参数和其他相关量的查找，
符号表查找部分通过所得结果大于所属函数或过程的下标来保证查找的正确性，并根据符号表获得其
*/
///测试样例中没有出现<函数标识符> := <表达式>这种形式的语句，因为不清楚其含义
//此代码中认为数组元素下标不会是数组元素
//此代码尚未解决begin和end不匹配的问题。
//此代码还没记录程序的入口。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
//符号表元素的value项没有实际意义，各项的值应通过value去查找
//不纠结了，就假装看不懂文法，函数参数必须是变量了==
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define norw 21//number of reserved words
FILE* fin;

enum code{LIT,OPR,LOD,STO,CAL,CLL,ADD,JMP,JPC,RED,WRT,END};
/*lit 取常量到栈顶
  opr 做运算
  lod 取变量到栈顶
  sto 将栈顶存入变量
  cal 函数调用
  cll 过程调用
  add 栈顶指针自增a
  jmp 无条件跳转
  jpc 条件跳转
  red 读并保存
  wrt 输出栈顶
  end 过程或函数或程序的结尾
*/
enum code codes[1000]={RED};//pcode指令的指令部分

int operand[1000][2]={{0},{0}};//pcode指令的操作数部分
int err = 0;//number of errors
int num_t = 0;//读取的token数量
int num_l = 0;//行数
int num_i = 0;//符号表项数
int num_b = 0;///begin和end对数
int num_p = 0;///小括号的对数
int addr = 0;///虚拟地址空间中的地址
int addr0 = 0;///基地址，主要用于辅助position函数
int addr00= 0;///保存上一级模块的基地址
int top = 0;///运行栈栈顶
int bp = 0;///当前分程序数据区的起始地址
int p0 = 0;///解释执行的pcode下标
int p1 = 0;///解释执行的下一条pcode
int suf_i = 0;//用于表达式处理，分别是符号栈和结果栈的编号

float vm[1000] = {0};//模拟的地址空间-addr
float run_stack[100]={0};///运行栈-top bp p0 p1

char curc;//当前字符current char
char* reserved[norw];//
char* typeof_sym[] = {
    "ident",    "rword",//标识符、保留字
    "illegal",  "string",//非法字符、字符串
    "lparen",   "rparen",//()
    "lsqbra",   "rsqbra",//[]
    "adding",   "multiplying",//加减乘除
    "relation", "assignment",//关系运算符，赋值符号=
    "integer",  "real",     "char",//整数、浮点数、字符
    "procedure","function",//过程、函数
    "comma",    "semicolon","period","colon"//','';''.':'
};///symbol的类型，主要是为了第一次作业服务

typedef struct sym{
    char name[100];///名称，也可以保存一个完整的字符串
    char type[20];//类型int char string float
    char kind[20];//array func pro const var///这个 地方是为了这类特别的元素准备的，其他统称normal，这一位置只需在声明语句中初始化，
    float value;//值
    int level;///所属分程序的起始地址
    int addr;///这里对过程和函数是起始地址，对其内部声明的量是相对地址。
}symbol;//这里只记录symbol的名称和类型，若为变量或常量则以浮点数形式记录其值，字符保存的是ascii码
//对于function 和procedure,value 保存其入口地址

symbol token0;//当前的token
symbol zero;
symbol syms[100];///符号表
symbol suf[100];///suffix expression 保存作为转换结果的后缀表达式


void error(int a ,int b);
enum code listcode();
int statement();
void const_dec(symbol sym);
void var_dec(symbol sym);
void pro_dec(symbol sym);
void func_dec(symbol sym);
void pro_call(int n);
void func_call(int n);
void reading();
void writing();
void if_state();
void for_state();
void while_state();
void expression();//表达式，这里是做一个中缀变后缀的转换，然后计算后缀表达式。用于保存后缀表达式的栈不必是全局变量。
symbol get_sym();
void listcode(int a,int b,int c);///a,b,c对应一条指令的三个参数
int position(int b,symbol sym);
int search_rword(char* s);///确认sym是否是保留字，若是则返回其标号，不是则返回-1



void error(int a,int b){
    switch(b){
        case 1:printf("error in line %d,too long identifier\n",a);break;
        case 2:printf("error in line %d,illegal real input\n",a);break;
        case 3:printf("error in line %d,incompleted operator\n",a);break;
        case 4:printf("error in line %d,unpaired quotation marks\n",a);break;
        case 5:printf("error in line %d,illegal string\n",a);break;
        case 6:printf("error in line %d,illegal expression\n",a);break;
        case 7:printf("error in line %d,illegal step size\n",a);break;
        case 8:printf("error in line %d,illegal conditions\n",a);break;
        case 9:printf("error in line %d,illegal function call\n",a);break;
        case 10:printf("error in line %d,illegal variable declaration\n",a);break;
        case 11:printf("error in line %d,illegal array declaration\n",a);break;
        case 12:printf("error in line %d,illegal const declaration\n",a);break;
        case 13:printf("error in line %d,illegal function declaration\n",a);break;
        case 14:printf("error in line %d,illegal proccedure declaration\n",a);break;
        case 15:printf("error in line %d,unpaired parens\n",a);break;
        case 16:printf("error in line %d,illegal procedure call",a);break;
        default:break;
    }
    err++;
}////error

/*enum code listcode(){

}*/

int statement(symbol sym){
    int i = 0;
    symbol token;
    if(strcmp(sym.name,"const")==0){
        token = get_sym();
        const_dec(token);
        printf("this is a const declaration statement!\n");
        return 0;
    }
    else if(strcmp(sym.name,"var")==0){
        token = get_sym();
        var_dec(token);
        printf("this is a var declaration statement!\n");
        return 0;
    }
    else if(strcmp(sym.name,"procedure")==0){
        token = get_sym();
        printf("this is a procedure declaration statement!\n");
        pro_dec(token);
        return 0;
    }
    else if(strcmp(sym.name,"function")==0){
        token = get_sym();
        printf("this is a function declaration statement!\n");
        func_dec(token);
        return 0;
    }/////根据get_sym函数的特性，先考虑保留字的问题，再故这四个分支是先判断声明再判断调用
    else if(strcmp(sym.name,"read")==0){
        reading();
        printf("this is a read statement!\n");
        return 0;
    }
    else if(strcmp(sym.name,"write")==0){
        writing();
        printf("this is a write statement!\n");
        return 0;
    }
    else if(strcmp(sym.name,"if")==0){
        printf("this is a if statement!\n");///应该顺便解决else和then分支
        if_state();
        return 0;
    }
    else if(strcmp(sym.name,"do")==0){
        printf("this is a while statement!\n");
        while_state();
        return 0;
    }
    else if(strcmp(sym.name,"for")==0){
        printf("this is a for statement!\n");
        for_state();
        return 0;
    }
    else if(strcmp(sym.name,"begin")==0){
        num_b++;
        return 4;//约等于跳过了begin
    }
    else if(strcmp(sym.name,"end")==0){
        token = get_sym();
        if(token.name[0]==46){///.
            num_b--;
            return 1;///end of file
        }
        else if(token.name[0] == 59){//end of a procedure or a function
            num_b--;
            return 2;
        }
        else{
            for(i=strlen(token.name);i>0;i--){
                ungetc(token.name[i-1],fin);
            }
            num_b--;
            return 3;///普通复合语句的结尾
        }
    }
    else if(strcmp(sym.type,"ident")==0){
        i = position(0,sym);
        if(i!=-1){
            if(strcmp(syms[i].kind,"procedure")==0){
                printf("this is a procedure call statement!\n");
                pro_call(i);
                return 0;
            }
            else if(strcmp(syms[i].kind,"function")==0){//这里实际上只是针对函数结尾的保存函数返回值的语句
                token = get_sym();///等号
                expression();//表达式处理
                listcode(3,0,0);//保存函数返回值到函数
                token = get_sym();//分号
                return 0;
            }//若为函数或过程则为调用语句，否则是赋值语句
        }
        else{//一般的赋值语句,需要判断该标识符是否是属于当前层次的
            token = get_sym();
            if(strcmp(token.type,"assignment")==0){///赋值语句肯定是分号结尾没跑了
                printf("this is a assignment statement!\n");
                expression();
                i = position(addr0,sym);
                if(i>=0){
                    listcode(3,0,i-addr0);///根据偏移量保存值
                }
                else{
                    i = position(addr00,sym);///在上一级模块中查找
                    if(i>=0){
                        listcode(3,0,i-addr00);;
                    }
                    else{
                        error(num_l,6);
                    }
                }
                token = get_sym();//分号
                return 0;
            }////这里还需要判断是否为函数或过程的调用语句
        }
    }
    return -1;
}
/* 预计若为标识符，则是赋值,因为不支持隐式声明，相应的应给出错误类型
    若为句型标识则判断为对应句型，
    begin和end中递归调用此函数
    需要提前解决标识符登记问题，完善查找标识符的功能。
    声明语句需要修改标识符的类型，从iden改为函数、过程、常量、变量等
*/

void const_dec(symbol sym){
    symbol token;
    symbol token1;
    strcpy(token.name,sym.name);
    get_sym();///等号
    token1 = get_sym();//常量的值
    strcpy(token.type,token1.type);
    strcpy(token.kind,"const");
    token.value = token1.value;//对字符，将其值保存在value中，读取的token名称是单个字符组成的字符
    token.level = addr0;///记录声明位置
    token.addr = addr;
    vm[addr] = token.value;
    addr = addr + 1;
    syms[num_i] = token;
    num_i = num_i + 1;///登入符号表
    get_sym();//分号
}///const_Dec常量的值直接写入内存，故不会有相应指令。。。。
void var_dec(symbol sym){
    symbol token;
    symbol token1;
    int size = 1;
    int i = 0;
    int j = 0;
    strcpy(token.name,sym.name);
    token.level = addr0;
    token.addr = addr;
    strcpy(token.type,"unknown");////这里暂时不清楚变量类型，为便于处理多个变量的声明，先放一个在这
    syms[num_i++] = token;
    i = 1;
    token1 = get_sym();//冒号或者逗号，可能是一次对多个变量进行声明
    while(token1.name[0]==44){//逗号
        token1 = get_sym();//下一个变量
        token1.level = addr0;
        token1.addr = addr;
        strcpy(token1.type,"unknown");
        syms[num_i++] = token1;
        i++;
        token1 = get_sym();//逗号或冒号
    }
    token1 = get_sym();//数据类型
    strcpy(token1.kind,"var");
    if(strcmp(token1.name,"array")==0){
        get_sym();///[
        token1 = get_sym();
        if(strcmp(token1.type,"integer")==0){
            size = (int)token1.value;///数组大小
        }
        else{
            error(num_l,11);
        }
        get_sym();//]
        get_sym();//of
        token1 = get_sym();////这里可能有很多的错误类型，都归类与error11
        strcpy(token1.kind,"array");
    }
    listcode(6,0,i*size);///在数据栈中申请被声明变量所需的空间
    while(i>=1){
        strcpy(syms[num_i-i].type,token1.name);
        strcpy(syms[num_i-i].kind,token1.kind);
        for(j = 0;j<size;j++){
            vm[addr++] = 0;
        }///初始化
        i--;//补入数据类型
    }
    get_sym();//分号
}///var_dec关于变量声明，相应的动作是在运行栈中预留空间，
 ///仅在赋值语句等需要保存的情形下才将变量写回内存，并在适当时候删除其在运行栈中的数据区
void pro_dec(symbol sym){
    symbol token;
    symbol token1;
    int n = num_i;
    addr0 = addr;
    token = sym;
    strcpy(token.type,"procedure");//这里得到了过程名
    strcpy(token.kind,"procedure");///过程没有返回值，故type和kind相同
    token.value = 0;///这里应该是函数在指令序列中的起始位置
    token.level = addr0;
    token.addr = addr;///对于procedure这个地址中是没有值的
    vm[addr] = 0;
    addr++;
    syms[num_i++] = token;//将过程登记入符号表
    listcode(6,0,1);//为过程本身申请一个空间
    token1 = get_sym();//开始参数表部分(40 41 91 93
    if(token1.name[0]==40){///左括号，40
        while(token1.name[0]!=59){//分号59
            token1 = get_sym();
            if(strcmp(token1.name,"var")==0){
                token1 = get_sym();
                var_dec(token1);///解决了整个参数表
                token1 = get_sym();//分号
            }
            else{
                error(num_l,14);
            }
        }
    }///参数表结束,将参数表为空的情况视为非法。
    syms[n].value = p0;///将函数的入口设置为参数表之后的语句
    n = 10;///为了避免误会，先把n初始化
    while(1){
        token1 = get_sym();
        //printf("%s\n",token1.name);
        n = statement(token1);///0123
        if(n==2){//end;时完成分程序分析，退出
            break;
        }
    }//对分程序部分进行分析
    addr0 = addr;////过程段结束，基地址复位
    listcode(11,0,1);
}///pro_dec
void func_dec(symbol sym){
    symbol token;
    symbol token1;
    addr0 = addr;
    int n = num_i;
    token = sym;
    strcpy(token.kind,"function");//这里得到了函数名
    token.value = 0;///这里应该是函数在指令序列中的起始位置，由于未实现listcode就放在这
    token.level = addr0;
    token.addr = addr;///这里保存的是function的返回值
    vm[addr] = 0;
    addr++;
    n = num_i;
    syms[num_i++] = token;//将过程登记入符号表
    listcode(6,0,1);//为保存函数值申请一个空间
    token1 = get_sym();//开始参数表部分(40 41 91 93
    if(token1.name[0]==40){///左括号，40
        while(token1.name[0]!=58){//冒号58
            token1 = get_sym();
            if(strcmp(token1.name,"var")==0){
                token1 = get_sym();
                var_dec(token1);///解决整个参数表
                token1 = get_sym();//冒号
            }
            else{
                printf("%c",&token1.name[0]);
                error(num_l,13);
            }
        }
    }///参数表结束
    syms[n].value = p0;///将函数入口设置为参数表之后的语句
    token = get_sym();//函数的返回值类型
    strcpy(syms[n].type,token.name);//前面记录了函数在符号表中的位置，并据此记录其返回值类型
    n = 10;//初始化n
    while(1){
        token1 = get_sym();
        n = statement(token1);//0123
        if(n==2){//end;时完成分程序分析，退出
            break;
        }
    }//对分程序部分进行分析
    addr0 = addr;//函数段结束，基地址复位
    listcode(11,0,2);
}//func_dec
void pro_call(int n){
    symbol token;
    int i = 0;
    int j = 0;
    int p = 0;
    p = (int)syms[n].value;///记录函数入口
    token = get_sym();//开始参数表部分(40 41 91 93
    if(token.name[0]==40){///左括号，40
        while(token.name[0]!=41){///右括号41
            token = get_sym();
            if(strcmp(tokem.tyoe,"var")==0){
                token = get_sym();
                j = position(addr0,token);
                if(j>=0){
                    i++;
                    token = get_sym();
                }
                else{
                    error(num_l,16);
                    while(token.name[0]!=59){
                        token = get_sym();
                    }
                    return;
                }
            }
            token = get_sym();
        }
    token = get_sym();//数据类型
    if(strcmp(token.name,syms[j].tyoe)!=0){
        token = get_sym();
        error(num_l,16);
        return;
    }
    token1 = get_sym();///分号
    listcode(6,0,i);//为这些变量申请空间
    }///参数表结束
    listcode(5,1,p);//过程调用语句这里好像调用和跳转没啥区别。
}////在这一部分首先需要把实际参数加载入数据栈，然后再跳转，这里lod之前应该进行地址的声明
void func_call(int n){//应包括跳转和将参数加载到运行栈两部分，仅处理到参数表结束
    symbol token;
    int i = 0;
    int j = 0;
    int p = 0;
    p = (int)syms[n].value;
    token = get_sym();//(
    if(token.name[0]==40){
        while(token.name[0]!=58){///冒号58
            token = get_sym();
            if(strcmp(tokem.tyoe,"var")==0){
                token = get_sym();
                j = position(addr0,token);
                if(j>=0){
                    i++;
                    token = get_sym();
                }
                else{
                    error(num_l,16);
                    while(token.name[0]!=59){
                        token = get_sym();
                    }
                    return;
                }
            }
            token = get_sym();
        }
    }
}
void reading(){///基于基地址进行变量的查找和赋值，变量名可能是数组元素
    symbol token;
    symbol token1;
    int i = 0;
    int j = 0;
    token = get_sym();///肯定是括号了
    while(token.name[0]!=59){////分号
        token = get_sym();
        i = position(addr0,token);////定位
        token = get_sym();
        if(strcmp(token.name,"[")){//[,表明v是一个数组元素
            token1 = get_sym();
            j = (int)token.value;
            token = get_sym();///]
            token = get_sym();////这个地方比较混乱，但是没啥问题
        }///当标识符后为逗号或括号时直接进入下一次循环即可。
    }////这里暂时这样处理
//需要找到每个 元素的位置。
}////reading
void writing(){
    symbol token;
    //int i = 0;
    char c='\0';
    token = get_sym();///括号无误
    c = fgetc(fin);
    if(c==34){
        ungetc(c,fin);
        token = get_sym();//字符串
        token = get_sym();
        if(strcmp(token.type,"colon")==0){
            expression();/////表达式处理不应该超出表达式
            token = get_sym();///括号
        }
    }
    else if((c>'0'&&c<'9')||(c>'a'&&c<'z')||(c>'A'&&c<'Z')){//若c是字母或数字则是表达式的开头
        ungetc(c,fin);
        expression();///不是字符串就是表达式
        token = get_sym();//应该是括号
    }
    else{//非法
        error(num_l,6);
        while(c!=59){
            c= fgetc(fin);
        }
    }
    token = get_sym();///语句结尾是分号
}////writing
void if_state(){
    symbol token;
    int n = num_b;//记录当前begin-end的对数，用于判断语句结尾
    int i = 0;
    while(strcmp(token.name,"then")!=0){
        token = get_sym();///这里正常是进行条件的分析，涉及表达式
    }
    do{
        token = get_sym();
        statement(token);
    }
    while(num_b!=n);///根据begin-end是否匹配判定then后语句是否结束，同时解决了普通语句及复合语句
    token = get_sym();//判断是否有else分支
    if(strcmp(token.name,"else")==0){
        do{
            token = get_sym();
            statement(token);
        }
        while(num_b!=n);///根据begin-end是否匹配判定else后语句是否结束，同时解决了普通语句及复合语句
    }
    else{//若没有else分支
        for(i=strlen(token.name);i>0;i--){
            ungetc(token.name[i-1],fin);////把多读的字符退回
        }
    }
}
void for_state(){
    symbol token;
    int n = num_b;
    while(strcmp(token.name,"do")!=0){
        token = get_sym();///有关步长部分
    }
    do{
        token = get_sym();
        statement(token);
    }
    while(num_b!=n);//根据begin-end是否匹配判定do后语句是否结束，同时解决了普通语句及复合语句
}
void while_state(){////以do起始
    symbol token;
    int n = num_b;
     do{
        token = get_sym();
        statement(token);

    }
    while(num_b!=n);//根据begin-end是否匹配判定do后语句是否结束，同时解决了普通语句及复合语句
    token = get_sym();//while
    while(token.name[0]!=59){
        token = get_sym();
    }
}

void expression(){////想了想我觉得还是把中缀变后缀的好，然后比较方便生成目标码
    symbol token,token1;    //这个问题里最重要的还是找出表达式的边界
    symbol ops[20];
    int i = 0;
    int j = 0;
    char c = '\0';
    int ad = 0;
    //int op = 1;///用于正负的标识，注意测试样例中不会出现多个连续的正负号
    int lp = 0;//运算符栈中左括号的数量，用于判断表达式结尾。
    c = fgetc(fin);//用于判断
    if(c == '-'){
        suf[suf_i++] = zero;
        ungetc(c,fin);
        token = get_sym();
        ops[j++] = token;//若为负号则添加一个零和一个减法
        token = get_sym();
    }
    else if(c == '+'){
        token = get_sym();//正号忽略
    }
    else{
        ungetc(c,fin);//若无前导符号则退回，读取第一个标识符
        token = get_sym();
    }
    while(1){
        if(token.name[0]==59||token.name[0]==41||token.name[0]==93){//表达式结束
            for(i=0;i<suf_i;i++){
                if(suf[suf_i].name[0]==91){//表明当前表达式是数组下标，不应该进行代码生成，直接结束函数
                    ungetc(suf[suf_i].name[0],fin);
                    return;
                }
            }
            ungetc(token.name[0],fin);
            break;
        }
        else if(strcpy(token.type,"real")==0||(strcpy(token.type,"integer")==0)){//若为数字，即若为常量，直接入栈
            suf[i++] = token;
        }
        else if(strcmp(token.type,"ident")==0){//若为标识符，则需判断是常量、变量、数组，或是函数调用
            ad = position(addr0,token);
            if(strcmp(syms[ad].kind,"array")==0){
                suf[suf_i++] = syms[ad];
                token = get_sym();
                suf[suf_i++] = token;//将中括号保存在结果中用于标识数组下标的边界
                expression();///这里假设不会发生中括号的嵌套
                token = get_sym();
                suf[suf_i++] = token;//]作为数组元素标识符结束的标记
            }
            else if(strcmp(syms[ad].kind,"function")==0){
                suf[suf_i++] = syms[ad];///函数名
                func_call(ad);
            }
            else{
                suf[suf_i++] = syms[ad];
            }
        }
        else if(token.name[0]==40){///(
            ops[j++] = token;//小括号直接进
            num_p++;
        }
        else if(token.name[0]==41){
            for(j = j-1;(ops[i].name[0]!=40)&&j>=0;j--){
                suf[suf_i++] = ops[j];
            }///这里j没有再减一，相当于将左括号弹出。
            num_p--;
            if(j<0){
                error(num_l,15);
                c = fgetc(fin);
                while(c!=59){
                    c = fgetc(fin);
                }
            }
        }
        else if(token.name[0]=='*'||token.name[0]=='/'){//由于只有两种优先级，故乘除法直接进
            ops[j++] = token;
        }
        else if(token.name[0]=='+'||token.name[0]=='-'){//由于只有两种优先级，故加减法在不为空且栈顶不是括号的时候直接弹出
            for(j=j-1;(ops[j].name[0]!='(')&&(j>=0);j--){
                    suf[suf_i++] = ops[j];
            }
            j++;
            ops[j++] = token;///弹出之后把自己放进去，若未进行弹出即未进入循环直接压入
        }
        else{
            error(num_l,6);
            while((c=fgetc(fin))!=59){
                c = fgetc(fin);//跳过当前行
            }
        }
        token = get_sym();
    }

}///这还需要注意的是，对于数组下标位置上的表达式，可以使用递归的方法分析，对于小括号则不应该递归调用此函数


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
    else if(c == 58){///冒号
        c = fgetc(fin);
        if(c == '='){
            strcpy(token.name,":=");
            strcpy(token.type,"assignment");//赋值符号
            token.value = 0;
        }
        else{
            ungetc(c,fin);
            strcpy(token.name,":");
            strcpy(token.type,"colon");//冒号
            token.value = 0;
        }
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
       // case 58:strcpy(token.name,":");strcpy(token.type,"colon");token.value = 0;break;///冒号
        case 59:strcpy(token.name,";");strcpy(token.type,"semicolon");token.value = 0;break;///分号
        case 91:strcpy(token.name,"[");strcpy(token.type,"lsqbra");token.value = 0;break;///左方括号
        case 93:strcpy(token.name,"]");strcpy(token.type,"rsqbra");token.value = 0;break;///右方括号
        default:n = 0;break;
    }
    return token;
 }

 int position(int b,symbol sym){//在符号表中寻找当前标识符
    int i = 0;
    for(i = b;i<num_i;i++){
        if(strcmp(sym.name,syms[i].name)==0){
            return i;
        }////一种可以考虑的办法是给出一个查找起点，用于解决不同层次间变量同名可能带来的问题
    }
    return -1;
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

void listcode(int a,int b,int c){
    enum code code0;
    switch(a){
        case 0:codes[p0]=LIT;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("LIT");break;
        case 1:codes[p0]=OPR;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("OPR");break;
        case 2:codes[p0]=LOD;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("LOD");break;
        case 3:codes[p0]=STO;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("STO");break;
        case 4:codes[p0]=CAL;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("CAL");break;
        case 5:codes[p0]=CLL;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("CLL");break;
        case 6:codes[p0]=ADD;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("ADD");break;
        case 7:codes[p0]=JMP;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("JMP");break;
        case 8:codes[p0]=JPC;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("JPC");break;
        case 9:codes[p0]=RED;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("RED");break;
        case 10:codes[p0]=WRT;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("WRT");break;
        case 11:codes[p0]=END;operand[p0][0]=b;operand[p0][1]=c;p0++;printf("END");break;
        default:break;
    }
    printf(" %d %d\n",&a,&b);///用于测试
}

int main(){
    char fname[100];//文件路径
    FILE *fout;
    int n = 0;
    int i = 0;

    strcpy(zero.name,"0");
    strcpy(zero.type,"float");
    strcpy(zero.kind,"const");
    zero.value = 0;
    zero.level = 0;
    zero.addr = -1;///初始化一个常量零用于解决表达式前可能存在的运算符的问题、
    //symbol cur_token;
    reserved[0] = "array";  reserved[1] = "begin";
    reserved[2] = "char";   reserved[3] = "const";
    reserved[4] = "do";     reserved[5] = "downto";
    reserved[6] = "else";   reserved[7] = "end";
    reserved[8] = "for";    reserved[9] = "function";
    reserved[10] ="integer";reserved[11]= "if";
    reserved[12]= "of";     reserved[13]="procedure";
    reserved[14]= "read";   reserved[15]="real";
    reserved[16]= "then";   reserved[17]="to";
    reserved[18]= "var";    reserved[19]="while";
    reserved[20]= "write";

    printf("Please enter the name of file to compile.\n");//使用绝对路径
    scanf("%s",fname);
    if((fin = fopen(fname,"r"))==NULL){
        printf("Open failed");
        return 1;
    }
    fout = fopen("the_result.txt","w");
    while(1){
        token0 = get_sym();
        n = statement(token0);
        if(n == 1){
            break;
        }
        i++;
        //fprintf(fout,"%d %s %s\n",num_t,token0.type,token0.name);
        //printf("%d\n",i);
    }
    //fprintf(fout,"%s","end of file");
    //fprintf(fout,"%d %s %s %s\n",num_t,"123",token0.type,token0.name);
    printf("end of file");
    fclose(fin);
    fclose(fout);

    return 0;
}

