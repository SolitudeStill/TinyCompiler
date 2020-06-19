

 
/**
  * 生产汇编代码
  **/
#include<bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#define MAX_LENGTH 20
using namespace std; 
//四元式类型定义如下
typedef struct Quaternion{
	string op;
	string arg1;
	string arg2;
    string result;
}Quaternion;
//四元式数组，存放产生的四元式
Quaternion quaternion[MAX_LENGTH*2];
//传人一个四元式，输出对应的汇编代码
int cntford=0;//每个显示数字都有标签 
int loopcnt=0;
int eqcnt=0;
int gtcnt=0;
int ltcnt=0;
void print_code(Quaternion qua)
{
	putchar('\n');
	if(qua.op == "+"){
		printf("    mov bl,%s\n    add bl,%s\n    mov %s,bl\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }else if(qua.op == "-"){
        printf("    mov bl,%s\n    sub bl,%s\n    mov %s,bl\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }
    else if(qua.op == "*"){
        printf("    mov ax,%s\n    mov bx,%s\n    mul bx\n    mov %s,al\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }else if(qua.op == "/"){//除法的时候不考虑余数
        printf("    mov ax,%s\n    mov bh,%s\n    div bh\n    mov %s,al\n    mov bh,00h;reset bh to stop loop\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }else if(qua.op == "%"){//余数
        printf("    mov ax,%s\n    mov bh,%s\n    div bh\n    mov %s,ah\n    mov bh,00h;reset bh to stop loop\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }else if(qua.op == "l"){//loop = stop
        printf("    mov bl,%s\n    cmp bl,%s\n    je loop%d\n",qua.arg1.c_str(),qua.arg2.c_str(),loopcnt);
        loopcnt++;
    }else if(qua.op == "L"){//loop start
        printf("    loop%d:\n",loopcnt);
    }else if(qua.op == ">"){//if>
        printf("    mov ax,%s\n    mov bh,%s\n    div bh\n    mov %s,ah\n    mov bh,00h;reset bh to stop loop\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
    }else if(qua.op == "s"){//show
        for(int x=0;x<strlen(qua.arg1.c_str());x++){
            if(qua.arg1[x]=='&'){
                printf("    mov dl,10\n    mov ax,0200h\n    int 21h\n");//要显示的char的int 
            } else if(qua.arg1[x]=='#'){
                printf("    mov dl,' '\n    mov ax,0200h\n    int 21h\n");//要显示的char的int 
            } else{
                printf("    mov dl,'%c'\n    mov ax,0200h\n    int 21h\n",qua.arg1[x]);//要显示的char的int 
            }
        }
    }else if(qua.op == "d"){//show integer
        printf("   ;this big part is to show a 0-99number\n   mov cl,%s\n   cmp cl,9\n   ja dabove%d\n   mov bl,cl;fou ze\n   add bl,30h;ascii correct\n   mov dl,bl\n   mov ax,0200h\n   int 21h\n   jmp dover%d\n",qua.arg1.c_str(),cntford,cntford);//要显示的char的int 
    	printf("   dabove%d:;put the number cl\n   mov ax,cl\n   mov bh,10\n   div bh\n   mov bl,al;the result\n   add bl,30h;ascii correct\n   mov dl,bl;show\n   mov bh,0;reset bh to stop loop\n   mov ax,0200h\n   int 21h\n",cntford);
		printf("   mov ax,cl\n   mov bh,10\n   div bh\n   mov bl,ah\n   add bl,30h\n   mov dl,bl\n   mov bh,0;reset bh to stop loop\n   mov ax,0200h\n   int 21h\n   mov cl,0\n   mov ax,0\n    dover%d:\n    mov al,0\n",cntford);
		cntford++; 
	}else if(qua.op=="rd"){
		printf("    mov ah,01H\n    int 21H\n    mov bl,al\n    sub bl,30h\n    mov %s,bl\n",qua.arg1.c_str());
	}else if(qua.op=="gt"){//  >
		printf("    mov al,%s\n    mov bl,%s\n   cmp al,bl\n    ja gt%d\n    mov %s,0\n    jmp gt_over%d\n    gt%d:mov %s,1\n    gt_over%d:\n"
		          ,qua.arg1.c_str(),qua.arg2.c_str(),  gtcnt,     qua.result.c_str(),         gtcnt,         gtcnt,qua.result.c_str()      ,gtcnt);
		          gtcnt++;
	}else if(qua.op=="lt"){//  >
		printf("    mov al,%s\n    mov bl,%s\n   cmp al,bl\n    jb lt%d\n    mov %s,0\n    jmp lt_over%d\n    lt%d:mov %s,1\n    lt_over%d:\n"
		          ,qua.arg1.c_str(),qua.arg2.c_str(),  ltcnt,     qua.result.c_str(),         ltcnt,         ltcnt,qua.result.c_str()      ,ltcnt);
		          ltcnt++;
	}else if(qua.op=="if_f"){
		printf("    cmp %s,1\n    je %s\n"
		          ,qua.arg1.c_str(),      qua.arg2.c_str());
	}else if(qua.op=="asn"){
		printf("    mov al,%s\n    mov %s,al",qua.arg1.c_str(),qua.arg2.c_str());
	}else if(qua.op=="lab"){
		printf("    %s:\n",qua.arg1.c_str());
	}else if(qua.op=="mul"){
		printf("    mov ax,%s\n    mov bx,%s\n    mul bx\n    mov %s,al\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
	}else if(qua.op=="sub"){
		printf("    mov bl,%s\n    sub bl,%s\n    mov %s,bl\n",qua.arg1.c_str(),qua.arg2.c_str(),qua.result.c_str());
	}else if(qua.op=="eq"){
		printf("    mov al,%s\n    mov bl,%s\n   cmp al,bl\n    je eq%d\n    mov %s,0\n    jmp eq_over%d\n    eq%d:mov %s,1\n    eq_over%d:\n"
		          ,qua.arg1.c_str(),qua.arg2.c_str(),  eqcnt,     qua.result.c_str(),         eqcnt,         eqcnt,qua.result.c_str()      ,eqcnt);
		          eqcnt++;
	}else if(qua.op=="wri"){
		printf("    mov bl,%s\n    add bl,30h\n    mov dl,bl\n    mov ax,0200h\n    int 21h\n",qua.arg1.c_str());//要显示的char的int 
	}else if(qua.op=="halt"){
		puts("    mov ah,4ch\n    int 21h\n");
	} 
}
 
int cnt=0;
//输出全部汇编代码
void testCode()
{
    
    puts("\nassume cs:code,ss:stack,ds:data\ndata segment\n  ");
	puts("  t1 db 1 dup(0)\n t2 db 1 dup(0)\n t3 db 1 dup(0)\n fact db 1 dup(0)\n x db 1  dup(0)\n t4 db 1 dup(0)\n t5 db 1 dup(0)\n \ndata ends\nstack segment\n  s dw 10 dup(0)\nstack ends\ncode segment\nstart:\n");
	puts(";show char segment\n     mov ax,0200h;set once and keep this function\n");
    //遍历实验3中的四元式，输出对应的汇编代码
    for(int i=0;i<cnt;i++)
        print_code(quaternion[i]);
    puts("code ends\nend start");
}
 
//主函数
int main()
{
    string s_tmp;
	streambuf *backup;  
	ifstream fin;  
	fin.open("data.txt");  
	cin.rdbuf(fin.rdbuf()); // assign file's streambuf to cin 
    while(1){
        cout<<"请输入四元式：以$结束";
        cin>>s_tmp;
        if(s_tmp=="$"){
            cout<<"\n已输出到文件";
            break; 
        }
        
        quaternion[cnt].op=s_tmp;
        cin>>quaternion[cnt].arg1>>quaternion[cnt].arg2>>quaternion[cnt].result;
        cnt++;
    } 
    cout<<"将在3秒后自动编译"<<endl;
    //记得修改这里的输出文件地址 
    freopen("F:\\dos\\a.asm","w",stdout);  
    testCode();
    freopen("con", "w", stdout);
    Sleep(3000);
    system("C:\\Users\\Administrator\\Desktop\\自动编译运行.exe"); 
    return 0;
}
/* 输入样例 
&表示换行 


+ 3 2 t+2
d t+2 - -
s & - -
- t+2 1 t+4
d t+4 - -
s & - -
* t+4 6 t+6
d t+6 - -
s & - -
$


+ 1 2 t
L - - -
- t 1 t
l t 0 -
d t - -
$



s &result#of#2+3& - -
+ 2 3 t
d t - -
s &result#of#18-13& - -
- 18 13 t
d t - -
s &result#of#3*3& - -
* 3 3 t
d t - -
s &result#of#24/4& - -
/ 24 4 t
d t - -
s &result#of#15%4& - -
% 15 4 t
d t - -
$

rd x - -
ge x 0 t1
if_f t1 L1 -
asn 1 fact -
lab L2 - -
mul fact x t2
asn t2 fact -
sub x 1 t3
asn t3 x -
eq x 0 t4
if_f t4 L2 -
wri fact - -
lab L1 - -
halt - - -
$




*/

