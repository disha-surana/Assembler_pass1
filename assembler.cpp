//		FIRST pass of assembler

//assumption: label and instruction cannot be in same line, dup not implemented

#include <bits/stdc++.h>
#include<algorithm>
#include<conio.h>
using namespace std;

fstream file;
string word;
int len=0,c=0,k,lc=0;
string arrLine[100],line;
string cur_seg;
int off=0;

//Objective : To remove spaces and tab from a string
void trim(string &str) 
{  int i,j=0;
   	 for(i=0;str[i]!='\0';++i)
    {
        if(str[i]!=' '&&str[i]!='\t')
            str[j++]=str[i];
    } 
    str[j]='\0';
    str.erase (j++,i);    
}

//Objective : To convert decimal number to hexa-decimal number
string decTohex(int off)
{
	char arr[50]; 	 
    int i = 0; 
    while(off!=0) 
    {   int temp  = 0; 	 
        temp = off % 16; 	
        if(temp < 10) 
         arr[i++] = temp + 48; //0-48
        else 
         arr[i++] = temp + 55;  //A-65           
        off = off/16; 
    } 
    string hex = "";
    for(int j=4; j>=0; j--)
    {
    	if(j>i-1)
		hex+= "0";
		else
		hex+= arr[j]; }
    return hex;
}

//class for symbol table records
class symb
{
public:
	string name;
	string type;
	string segment;
	int offset;
	int size;
	
	void show(){	
		
	trim(name); trim(type); trim(segment);
	cout<<name<<setw(15)<<size<<setw(15)<<type<<setw(15)<<segment<<setw(15)<<decTohex(offset)<<"\n";
		
	}
	
}s[100];

//Objective : To count number of values of a variable
int count_val(int pos){
    int cval;
	cval=count(arrLine[pos].begin(), arrLine[pos].end(), ',') + 1; //count ',' to check number of elements in variable
	return cval;
}

//Objective : To display symbol table
void display()
{
	cout<<"\nNAME"<<setw(15)<<"SIZE"<<setw(15)<<"TYPE"<<setw(15)<<"SEGMENT"<<setw(15)<<"OFFSET"<<"\n\n";
	
	for(int i=0;i<c;i++)
		s[i].show();
    
}

//Objective : To check if word is present in symbol table or not
int is_found(string word){
	
	int f_index=-1;
	for(int i=0;i<c;i++){
		
		string temp=s[i].name;
		transform(temp.begin(),temp.end(),temp.begin(),::toupper);
		if(word.compare(temp)==0)
		{
			f_index=i;
			break;
		}	
	}
	return f_index;
}

//structure for opcodes and their respective sizes
struct inst_size{
	string op;
	int ins_s;
	
};


inst_size mnemonic[]={{.op = "MOV", .ins_s = 2 },{.op = "LDS", .ins_s = 2 },{.op = "LES", .ins_s = 2 },{.op = "LEA", .ins_s = 2 },{.op = "XCHG", .ins_s = 2 },{.op = "ADD", .ins_s = 2 }};

//array of registers
string reg[]={"AX","BX","CX","DX","SI","DI","BP","AH","AL","BH","BL","CH","CL","DH","DL","DS","ES","SS"};


//Objective : To check if value is present in register array or not
int in_register(string value)
{
	int i;
	transform(value.begin(),value.end(),value.begin(),::toupper);
    for (i = 0; i < 18; i++)
    {
        if (value == reg[i])
        {
            return i;
        }
    }
	i=-1;
    return i;
}

//Objective : To find index at which line contains any opcode (using struct array inst_size)
int find_op(string line){
	
	int i;
	transform(line.begin(),line.end(),line.begin(),::toupper);
	for(i=0;i<6;i++){
		if(line.find(mnemonic[i].op)!=-1)
			return i;
	}
	return -1;
}

//Objective : To calculate the offset of various instructions

/*
1.	Reg - to/from - Memory (without displacement)		:	2-bytes
2.	Reg - to/from - Memory (with displacement)   		:	4-bytes
3.	Reg <- Immediate value						  		:	3-bytes
4.	Memory with 16-bit displacement <- Immediate value	:	5-bytes
5.	Reg - to - Reg										:	2-bytes

*/
void offset_cal(string line){
	
	trim(line);
	
	int op_index=find_op(line);

	if(op_index!=-1){
		//off+= mnemonic[op_index].ins_s;
		op_index=mnemonic[op_index].op.length();
	}
	
	int co_index = line.find(",");
	string dest = line.substr(op_index,co_index-op_index);
	string source = line.substr(co_index+1);

	//register to register-2
	if((in_register(dest)!=-1)&&(in_register(source)!=-1))        
		off+=2;
		
	//memory to register without displacement
	if((in_register(dest)!=-1)&&((is_found(source)!=-1)||((source[0]=='[' && source[source.length()-1]==']'))))            //R<-M without disp
	{
		if((source.find("+")==-1)&&	(count(source.begin(), source.end(), '[')<=1))
			off+=2;
	}
	
	//register to memory without displacement
	if((in_register(source)!=-1)&&((is_found(dest)!=-1)||((dest[0]=='[' && dest[dest.length()-1]==']'))))                 //M<-R without disp
	{
		if((dest.find("+")==-1)&&(count(dest.begin(), dest.end(), '[')<=1))
			off+=2;
	}
	
	//immediate to register
	if(((in_register(dest)!=-1))&&(isdigit(source[0])==1))
			off += 3;
	
	//memory to register with displacement
	//2 braces or +
	if((in_register(dest)!=-1)&&((count(source.begin(), source.end(), '[')>1)||(source.find("+")!=-1)))                        
			off+=4;
	
	//memory to register with displacement
	//case: var[reg] and avoid var1[ax+6](covered above)
	if((in_register(dest)!=-1)&&((count(source.begin(), source.end(), '[')==1)&&((source.find("+")==-1))&&((source[0]=='[' && source[source.length()-1]!=']')||(source[0]!='[' && source[source.length()-1]==']'))))
		    off+=4;
	
	//register to memory with displacement
	//2 braces or +
	if((in_register(source)!=-1)&&((count(dest.begin(), dest.end(), '[')>1)||(dest.find("+")!=-1)))                        
			off+=4;
	
	//register to memory with displacement
	//case: var[reg] and avoid var1[ax+6](covered above)
	if((in_register(source)!=-1)&&((count(dest.begin(), dest.end(), '[')==1)&&((dest.find("+")==-1))&&((dest[0]=='[' && dest[dest.length()-1]!=']')||(dest[0]!='[' && dest[dest.length()-1]==']'))))                        
		    off+=4;
	
	//immediate value to memory with 16 bit displacement and without both 5 bytes
	if((in_register(dest)==-1)&&(isdigit(source[0])==1))
	off+=5;
			
	if((isdigit(dest[0])))     //I<-anything
	{
     	cout<<"ERRROR: immediate cannot be destination";
					getch(); exit(0);
	}
	
}

//Objective : To decode assembly instructions and insert records in symbol table
void check_name(string line,int i)
{
	string vname="";
	
	// segment
	if(line.find("SEGMENT")!=-1)
	{
	 k=line.find("SEGMENT");
	 vname=arrLine[i].substr(0,k); 
	 trim(vname);
	if(is_found(vname)!=-1)
	{
		cout<<"\nERROR:segment name already declared";
		getch();
		exit(0);
	}
	 s[c].name=vname;
	 s[c].type="SEGMENT";
	 s[c].segment="(ITSELF)";
	 s[c].size=-5;
	 cur_seg=arrLine[i].substr(0,k);
	 s[c].offset=0;
	 off=0;
	 c++;
	}
	// label
	else if(line.find(":") != -1)
	{
	 k=line.find(":");
	 vname=arrLine[i].substr(0,k); 
	trim(vname);
	if((is_found(vname)!=-1))
	{
		cout<<"\nERROR: label name already declared";
		getch();
		exit(0);
	}
	 s[c].name=vname;
	 s[c].type="LABEL";
	 s[c].segment=cur_seg;
	 s[c].size=-1;	
	 s[c].offset=off;
	 c++;
	}
	// data variable db
	else if(line.find(" DB ")!=-1 )
	{
	 k=line.find(" DB ");
	  vname=arrLine[i].substr(0,k); 
	trim(vname);
	if(is_found(vname)!=-1)
	{
		cout<<"\nERROR: variable name already declared";
		getch();
		exit(0);
	}
	 s[c].name=vname;
	 s[c].segment=cur_seg;
	 s[c].type="VAR";
	 s[c].size= 1;
	 s[c].offset=off;
	 off += (1 * count_val(i));
	 c++;
	}
	// data variable dw
	else if(line.find(" DW ")!=-1){
		k=line.find(" DW ");
	 vname=arrLine[i].substr(0,k); 
	 trim(vname);
	if(is_found(vname)!=-1)
	{
		cout<<"ERROR: variable name already declared";
		getch();
		exit(0);
	}
	 s[c].name=vname;
	 s[c].type="VAR";
	 s[c].segment=cur_seg;
	 s[c].size= 2;
	 s[c].offset=off;
	 off += (2 * count_val(i));
	 c++;
	}
	// data variable dd
	else if(line.find(" DD ")!=-1){
		k=line.find(" DD ");
		 vname=arrLine[i].substr(0,k); 
	trim(vname); 
	if(is_found(vname)!=-1)
	{
		cout<<"\nERROR: variable name already declared";
		getch();
		exit(0);
	}
	 s[c].name=vname;
	 s[c].type="VAR";
	 s[c].segment=cur_seg;
	 s[c].size= 4;
	 s[c].offset=off;
	 off += (4 * count_val(i));
	 c++;
	}
	else 
		offset_cal(line);      //if not segment or label or variable
}

//Objective : To read assembly code file and store in arrLine[] array
void read()
{
	file.open("QUES1.txt");
    if(file.is_open())
    while ( getline (file,arrLine[len]) )
     len++;
		
	for(int j = 0; j < len; ++j)
    {
    	word=arrLine[j];
    	transform(word.begin(),word.end(),word.begin(),::toupper);
    	k=word.find(";");
	    if(k!=0)
			word=word.substr(0,k-1); 
		else
			word="";
			
		string tem_line=word;
		trim(tem_line);
		
    	if((word.find("ASSUME")!=-1) || (word.find("END")!=-1) || tem_line=="")
		continue;
		
		check_name(word,j);   //call to check segment label or var
	} 
	
	file.close();
}

//Objective : To write symbol table 
void write(){
	
	file.open("Symbol_Table.txt",ios::out);
	
	file<<"\n"<<setw(40)<<"SYMBOL TABLE \n";
	
	file<<"=======================================================================\n\n";
	
	file<<"NAME"<<setw(15)<<"SIZE"<<setw(15)<<"TYPE"<<setw(15)<<"SEGMENT"<<setw(15)<<"OFFSET"<<"\n\n";
	
	file<<"=======================================================================\n";
	
	for(int i=0;i<c;i++)
		file<<s[i].name<<setw(15)<<s[i].size<<setw(15)<<s[i].type<<setw(15)<<s[i].segment<<setw(15)<<decTohex(s[i].offset)<<"\n";
		
	cout<<"\nSymbol table created!";
	
	file.close();
	
}
    
int main()
{
	
read();
//display();
write();

return 0;
}





