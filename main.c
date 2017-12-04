#include <stdio.h>
#include <stdlib.h>

//compiling2017
//一条咸鱼的自我救赎
//标识符长度等各种长度的越界都没有考虑，测试过程中需要注意
//这个作业额没那么高级，我也不打算解决参相关的错误了。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define norw 21//number of reserved words
FILE* fin;

enum code{LIT,OPR,LOD,STO,CAL,CLL,ADD,JMP,JPC,RED,WRT};///这里add是数据栈顶指针增加a,CAL调用函数，CLL调用过程
enum code codes[1000]={RED};//pcode指令的指令部分

int operand[1000][2]={{0},{0}};//pcode指令的操作数部分
int err = 0;//number of errors
int num_t = 0;//读取的token数量
int num_l = 0;//行数
int num_i = 0;//符号表项数
int num_b = 0;///begin和end对数
int addr = 0;///虚拟地址空间中的地址
int depth = 0;///调用层次数
int top = 0;///运行栈栈顶
int bp = 0;///当前分程序数据区的起始地址
int p0 = 0;///解释执行的pcode下标
int p1 = 0;///解释执行的下一条pcode

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
    char type[20];//类型
    float value;//值
    int level;///声明层次
    int addr;///这里对过程和函数是起始地址，对其内部声明的量是相对地址。
}symbol;//这里只记录symbol的名称和类型，若为变量或常量则以浮点数形式记录其值，字符保存的是ascii码
//对于function 和procedure,value 保存其入口地址

symbol token0;//当前的token
symbol syms[100];///符号表


void error(int a ,int b);
enum code listcode();
int statement();
void const_dec(symbol sym);
void var_dec(symbol sym);
void pro_dec(symbol sym);
void func_dec(symbol sym);
void pro_call(symbol sym);
void func_call(symbol sym);
void reading();
void writing();
void if_state();
void for_state();
void while_state();
symbol get_sym();
int position(int b,symbol sym);
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
        case 14:printf("error in line %d,illegal proccedure declaration",a);break;
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
    }/////根据getsym函数的特性，先考虑保留字的问题，再故这四个分支是先判断声明再判断调用
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
        /////正常这里需要进行表达式处理，但是这次作业就读就行了。。。
        i = position(0,sym);
        if(1!=-1){
            if(strcmp(syms[i].type,"procedure")==0){
                token = sym;
                printf("this is a procedure call statement!\n");
                pro_call(token);
                return 0;
            }
            else if(strcmp(syms[i].type,"function")==0){
                token = sym;
                printf("this is a function call statement!\n");
                func_call(token);
                return 0;
            }//若为函数或过程则为调用语句，否则是赋值语句
        }
        while(1){
            token = get_sym();
            if(strcmp(token.type,"semicolon")==0){///赋值语句肯定是分号结尾没跑了
                printf("this is a assignment statement!\n");
                return 0;
            }////这里还需要判断是否为函数或过程的调用语句
        }
    }
    return -1;
}/* read
    write
    const_dec
    var_dec
    func_dec
    pro_dec
    func_call
    pro_call
    if
    for
    while
*/
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
    token.value = token1.value;//对字符，将其值保存在value中，读取的token名称是单个字符组成的字符
    token.level = depth;///层次为当前层
    token.addr = addr;
    vm[addr] = token.value;
    addr = addr + 1;
    syms[num_i] = token;
    num_i = num_i + 1;///登入符号表
    get_sym();//分号
}
void var_dec(symbol sym){
    symbol token;
    symbol token1;
    int size = 1;
    int i = 0;
    strcpy(token.name,sym.name);
    token1 = get_sym();//冒号或者逗号，可能是一次对多个变量进行声明
    while(token1.name[0]==44){//逗号
        token1 = get_sym();//下一个变量
        token1 = get_sym();//逗号或冒号
    }
    token1 = get_sym();//数据类型
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
    }

    strcpy(token.type,token1.name);///数据类型
    token.level = depth;
    token.addr = addr;
    for(i=0;i<size;i++){
        vm[addr] = 0;
        addr ++;
    }///初始化
    syms[num_i] = token;
    num_i = num_i + 1;//登入符号表
    get_sym();//分号
}
void pro_dec(symbol sym){
    symbol token;
    symbol token1;
    int n = 10;
    token = sym;
    strcpy(token.type,"procedure");//这里得到了过程名
    token.value = 0;///这里应该是函数在指令序列中的起始位置，由于未实现listcode就放在这
    token.level = depth;
    token.addr = addr;///这里也应该是函数在虚拟内存空间的起始位置，先保存参数再保存过程内生成的量
    depth++;
    addr++;
    syms[num_i++] = token;//将过程登记入符号表
    token1 = get_sym();//开始参数表部分(40 41 91 93
    if(token1.name[0]==40){///左括号，40
        while(token1.name[0]!=59){//分号59
            token1 = get_sym();
            if(strcmp(token1.name,"var")==0){
                token1 = get_sym();
                var_dec(token1);
                token1 = get_sym();//逗号或者是右括号
            }
            else{
                error(num_l,14);
            }
        }
    }///参数表结束
    while(1){
        token1 = get_sym();
        //printf("%s\n",token1.name);
        n = statement(token1);
        if(n==2){//end;时完成分程序分析，退出
            break;
        }
    }//对分程序部分进行分析
    depth = token.level;//把这个层数复位
}
void func_dec(symbol sym){
    symbol token;
    symbol token1;
    int n = 10;
    token = sym;
    strcpy(token.type,"function");//这里得到了函数名
    token.value = 0;///这里应该是函数在指令序列中的起始位置，由于未实现listcode就放在这
    token.level = depth;
    token.addr = addr;///这里也应该是函数在虚拟内存空间的起始位置，先保存参数再保存过程内生成的量
    depth++;
    addr++;
    syms[num_i++] = token;//将过程登记入符号表
    token1 = get_sym();//开始参数表部分(40 41 91 93
    if(token1.name[0]==40){///左括号，40
        while(token1.name[0]!=59){//分号59
            token1 = get_sym();
            if(strcmp(token1.name,"var")==0){
                token1 = get_sym();
                var_dec(token1);
                token1 = get_sym();//逗号或者是右括号
            }
            else{
                error(num_l,14);
            }
        }
    }///参数表结束
    while(1){
        token1 = get_sym();
        n = statement(token1);
        if(n==2){//end;时完成分程序分析，退出
            break;
        }
    }//对分程序部分进行分析
    depth = token.level;//把这个层数复位
}
void pro_call(symbol sym){
    symbol token;
    token = get_sym();
    while(token.name[0]!=59){
        token = get_sym();
    }////这里暂时这样处理

}
void func_call(symbol sym){
    symbol token;
    token = get_sym();
    while(token.name[0]!=59){
        token = get_sym();
    }////这里暂时这样处理
}
void reading(){
    symbol token;
    token = get_sym();
    while(token.name[0]!=59){
        token = get_sym();
    }////这里暂时这样处理
}
void writing(){
    symbol token;
    token = get_sym();
    while(token.name[0]!=59){
        token = get_sym();
    }////这里暂时这样处理
}
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


/////////////////////////////////还应该有表达式三兄弟expression、term和factor


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

 int position(int b,symbol sym){//在符号表中寻找当前标识符
    int i = 0;
    for(i = b;i<num_i;i++){
        if(strcmp(sym.name,syms[i].name)==0){//这里有一些问题，先保证可以用于识别函数调用
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

int main(){
    char fname[100];//文件路径
    FILE *fout;
    int n = 0;
    int i = 0;
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
    //fout = fopen("the_result.txt","w");
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
    //fclose(fout);

    return 0;
}

