#include<bits/stdc++.h>
using namespace std;
//Which contains the opcodes of the instructions.
map<string,string> op_table;
//Which contains the symbols and their addresses.
map<string,int> sym_table;
map<string,int> ind;
//All errors will be stored here.
vector<string> errors;
//Instruction pointer, used to store the current address location.
int eip;
//It is the starting address of the program.
int start_addr;
//The program name will be stored here.
string start_var;
//The starting directive name will be stored here.
string start_dir;
//It stores the length of the program.
int length;
//Block of the object code will be stored here.
string block;

//It parses the line and gives the string of words.
vector<string> parser(string s){
    vector<string> output;
    string c;
    for(int i=0;i<s.size();i++){
        //If we are encountering . then we will omit the remaining parts of the line as those are treated as comments.
        if(s[i]=='.') break;
        if(s[i]==' ' && c.size()==0) continue;
        if(s[i]==' '){
            output.push_back(c);
            c.clear();
        }
        else{
            c+=s[i];
        }
    }
    if(c.size()!=0) output.push_back(c);
    return output;
}

//Instead of hardcoding the values i am taking those from a file.
void Taking_opcodes(){
    fstream f;
    f.open("opcode.txt",ios::in);
    while(!f.eof()){
        string s;
        string value;
        f>>s>>value;
        op_table[s]=value;
    }
}

//This function will check whether the given instruction is of variable format.
int check_for_variable(vector<string>& words){
    if(words[1]=="WORD" || words[1]=="BYTE" || words[1]=="RESB" || words[1]=="RESW") return 1;
    return 0;
}

//This function will checks whether the symbol is defined or not otherwise it will define it.
int checking_symbol(string symbol, int op){
    if(op && symbol.size()>2){
        if(symbol[symbol.size()-1]=='X' && symbol[symbol.size()-2]==','){
            symbol.pop_back();
            symbol.pop_back();
        }
    }
    if(op){
        ind[symbol]=1;
    }
    if(sym_table.find(symbol)!=sym_table.end()) return 1;
    return 0;
}

//This function checks whethere the opcode is correct or not.
int checking_opcode(string opcode){
    if(op_table.find(opcode)==op_table.end()) return 1;
    return 0; 
}

//This funtion checks whether the operand is correct or not.
int checking_operand(string symbol){
    //Here all possible cases are checked 'X' and 'C' and everything.
    if(symbol.size()<3 || (symbol[0]!='X' && symbol[0]!='C') || symbol[1]!='\'' || symbol[symbol.size()-1]!='\'') return -1;
    if(symbol[0]=='C') return symbol.size()-3;
    for(int i=2;i<symbol.size()-1;i++){
        if((symbol[i]<=57 && symbol[i]>=48) || (symbol[i]<=70 && symbol[i]>=65)) continue;
        else return -1;
    }
    return (symbol.size()-3+1)/2;
}

//It will check whether the given string contains characters corresponding to numbers or not.
int checking_number(string number){
    for(int i=0;i<number.size();i++){
        if(number[i]<=57 && number[i]>=48) continue;
        else return 1;
    }
    return 0;
}

//It will check whether the given string contains characters corresponding to hexadecimal numbers or not.
int checking_hexnumber(string number){
    for(int i=0;i<number.size();i++){
        if((number[i]<=57 && number[i]>=48) || (number[i]<=70 && number[i]>=65)) continue;
        else return 1;
    }
    return 0;
}

//It converts the string to a number.
int convert_to_decimal(string number){
    int x=0;
    stringstream ss;
    ss<<hex<<number;
    ss>>x;
    //cout<<number<<" "<<x<<endl;
    return x;
}

//It converts the decimal number to hexadecimal string of specified length given in length variable.
string convert_to_hexdecimal(int number,int length){
    stringstream ss;
    ss<<hex<<number;
    //cout<<number<<" "<<ss.str()<<endl;
    string s=ss.str();
    for(int i=s.size()+1;i<=length;i++) s="0"+s;
    for(int i=0;i<s.size();i++) s[i]=toupper(s[i]);
    return s;
}

//It is the phase one of the assembler.
void analysis(char* filename){
    fstream fp,fint;
    fp.open(filename,ios::in);
    //Intermediate code will be stored here.
    fint.open("intermediate.txt",ios::out);
    int line=0;
    while(!fp.eof()){
        line++;     //It stores the current line in the input file.
        string s;
        getline(fp,s);  //Get the string from the input file.
        vector<string> words=parser(s); //Get the words from the input file.
        
        if(line==1){
            // Checking for the start of the first line.
            if(words.size()!=3 || words[1]!="START" || checking_hexnumber(words[2])){
                //Printing the error message if it is not the start of the program.
                errors.push_back("The format should be Name\tSTART\tAddress in hexadecimal at line "+ to_string(line));
            }
            else{
                //Storing the necessary information int the varibales.
                start_var=words[0];
                eip=convert_to_decimal(words[2]);
                //cout<<eip<<endl;
                start_addr=eip;
            }
            fint<<convert_to_hexdecimal(eip,4)<<" "<<s<<"\n";
            continue;
        }

        /*
        for(int i=0;i<words.size();i++){
            cout<<words[i]<<"\t";
        }
        cout<<endl;
        continue;
        */

       //If words size is 0, then it means it is a comment line.
        if(words.size()==0){
            //Printing that line also in intermediate file.
            fint<<"     "<<s<<"\n";
            continue;
        }
        //Printing the same line with allotted address in the intermediate file.
        fint<<convert_to_hexdecimal(eip,4)<<" "<<s<<"\n";
        //If the words size is one which should be RSUB otherwise some error in the format.
        if(words.size()==1){
            if(words[0]=="END"){
                //Checking for errors.
                errors.push_back("Specify the Directive name with the END at line "+ to_string(line));
                return;
            }
            else if(words[0]!="RSUB"){
                //Checking for errors.
                errors.push_back("The format should be Operand memory at line "+ to_string(line));
            }
            eip+=3;
        }
        //If the words size is two which means the it can be END or some instruction operand memory format.
        else if(words.size()==2){
            if(words[0]=="END"){
                if(sym_table.find(words[1])==sym_table.end()){
                    errors.push_back("Should use the correct Directive name at line "+ to_string(line));
                }
                else start_dir=words[1];
                return;
            }
            //Which means it contains error.
            else if(words[1]=="END"){
                //Checking for errors.
                errors.push_back("END should be followed by a Directive name. at line "+ to_string(line));
                return;
            }
            else if(words[1]=="RSUB"){
                //Checking for errors.
                if(checking_symbol(words[0],0)) errors.push_back("Redeclaration of variable/label name at line "+ to_string(line));
                else sym_table[words[0]]=eip;
            }
            else{
                //Checking for errors.
                if(checking_opcode(words[0])) errors.push_back("Opcode is not valid at line "+ to_string(line));
                checking_symbol(words[1],1);
            }
            //fint<<convert_to_hexdecimal(eip,4)<<" "<<s<<"\n";
            eip+=3;
        }
        else if(words.size()==3){
            
            /*
            if(words[1]=="END"){
                if(sym_table.find(words[2])==sym_table.end()) errors.push_back("Specified Directive is not there.");
                else start_dir=words[2];
                checking_symbol(words[0],0);
                return;
            }*/

            //In the three word case the first word should always be the symbol name.
            if(checking_symbol(words[0],0)) errors.push_back("Redeclaration of variable/label name "+words[0]);
            else sym_table[words[0]]=eip;

            //fint<<convert_to_hexdecimal(eip,4)<<" "<<s<<"\n";
            
            //Checking the second is containing any variables or not.
            if(check_for_variable(words)){
                //If the second words is a BYTE.
                if(words[1]=="BYTE"){
                    //Here getting the size of the operand into the variable x.
                    int x=checking_operand(words[2]);
                    if(x<0) errors.push_back("Invalid instruction at line "+ to_string(line));
                    else if(x>0) eip+=x;
                    else eip+=3;
                    //cout<<x<<endl;
                }
                //If the second words is a WORD.
                else if(words[1]=="WORD"){
                    //Here treating the operand as a integer number.
                    int x=0;
                    if(checking_number(words[2])) errors.push_back("Number should be given at line "+ to_string(line));
                    eip+=3;
                }
                //If the second words is a reserve byte type.
                else if(words[1]=="RESB"){
                    int x=0;
                    //Checking for the errors in the number given in operand space.
                    if(checking_number(words[2])) errors.push_back("Number should be given at line "+ to_string(line));
                    else{
                        stringstream ss;
                        ss<<words[2];
                        ss>>x;
                        //cout<<"line "<<line<<" : value of x is "<<x<<endl;
                        eip+=x;
                    }
                }
                else if(words[1]=="RESW"){
                    //Same as RESB but here size is increased in count of three bytes.
                    int x=0;
                    if(checking_number(words[2])) errors.push_back("Number should be given at line "+ to_string(line));
                    else{
                        stringstream ss;
                        ss<<words[2];
                        ss>>x;
                        //cout<<"line "<<line<<" : value of x is "<<x<<endl;
                        eip+=(x*3);
                    }
                }
                else{
                    //Nothing is there then there is an error in the given format.
                    errors.push_back("Invalid datatype is given at line "+ to_string(line));
                    eip+=3;
                }
            }
            //If it is not a variable then it should follow a directive opcode memory type instruction.
            else{
                if(checking_opcode(words[1])) errors.push_back("Opcode is not valid at line "+ to_string(line));
                checking_symbol(words[2],1);
                eip+=3;
            }
        }
        //If it contains more than three words then it is incorrect instruction.
        else{
            errors.push_back("Error occured at line (Invalid Instruction format) "+ to_string(line));
            eip+=3;
        }
    }
    //If program doesn't contains END then it should print the no. lines read and an error message.
    //cout<<"no. of lines "<<line<<endl;
    errors.push_back("Program should contain END DIRECTIVE_NAME instruction to specify from where we have to start.");
}

//The output file will be stored here.
fstream fout;

//Here new block is created, text segment in the object code.
void create_new_block(){
    //If the block size is less than 9 which means that the previously created block is empty.
    //So need of printing.
    if(block.size()<=9){
        block.clear();
        block+="T";
        block+=convert_to_hexdecimal(eip,6);
        block+="00";
        return;
    }
    //Otherwise we have to save that block in the memory and creates a newer ones.
    //Finding block size.
    string s=convert_to_hexdecimal((block.size()-9)/2,2);
    //Storing in the text segment.
    block[7]=s[0];
    block[8]=s[1];
    //Saving it in the output file.
    fout<<block<<"\n";
    block.clear();
    //Creating new block.
    block+="T";
    block+=convert_to_hexdecimal(eip,6);
    block+="00";
    return;
}

//Adding an instruction to an existing block.
void add_to_block(string ins){
    //If block size exceeding the limit, then we need to create a new block.
    if(block.size()+ins.size()<=69) block+=ins;
    else{
        create_new_block();
        block+=ins;
    }
    return;
}

//Phase 2 of the assembler.
void synthesis(char* input,char* output){
    fstream fin;
    fin.open(input,ios::in);
    fout.open(output,ios::out);
    int line=0;
    while(!fin.eof()){
        line++;
        string s;
        //Taking input from the file.
        getline(fin,s);
        vector<string> words=parser(s);
        //If it is the first line then it should contains start.
        if(line==1){
            block+="H";
            for(int i=0;i<6;i++){
                if(i>=start_var.size()) block+=" ";
                else block+=start_var[i];
            }
            //Printing the start segment into the output stream.
            block+=convert_to_hexdecimal(start_addr,6);
            block+=convert_to_hexdecimal(length,6);
            fout<<block<<"\n";
            block.clear();
            //Creating a newer block.
            block+="T";
            eip=convert_to_decimal(words[2]);
            block+=convert_to_hexdecimal(eip,6);
            block+="00";
            //cout<<block<<endl;
            continue;
        }
        //If it doesn't have any words probably it will be the comment line.
        if(words.size()==0){
            //cout<<line<<endl;
            continue;
        }
        //If it has only one word which means it should be the RSUB instruction.
        else if(words.size()==1) {
            //cout<<line<<endl;
            string ins;
            ins+=op_table[words[0]];
            ins+=convert_to_hexdecimal(0,4);
            add_to_block(ins);
            eip+=3;
        }
        //If it has two words which means it can be RSUB, END or normal operation memory instuction.
        else if(words.size()==2){
            //cout<<line<<endl;
            if(words[0]=="END") break;
            string ins;
            ins+=op_table[words[0]];
            int x=0;
            //Checking for the intermediate addressing possibility.
            if(words[1].size()>2 && words[1][words[1].size()-1]=='X' && words[1][words[1].size()-2]==','){
                x+=32768;               //Making the 9th bit as 1.
                words[1].pop_back();
                words[1].pop_back();
            }
            x+=sym_table[words[1]];
            //cout<<words[1]<<endl;
            //Converting it into a hexadecimal instruction.
            ins+=convert_to_hexdecimal(x,4);
            //Adding it into the block of segment.
            add_to_block(ins);
            eip+=3;
        }
        else{
            //Checking for variable.
            if(check_for_variable(words)){
                //cout<<line<<endl;
                //If it is a byte then checks for string constant or hexadecimal constant.
                if(words[1]=="BYTE"){
                    string ins;
                    if(words[2][0]=='X'){
                        //cout<<line<<endl;
                        for(int i=2;i<words[2].size()-1;i++){
                            ins+=words[2][i];
                        }
                        //Should make in byte size.
                        if(words[2].size()%2==0) ins="0"+ins;
                        //Adding to the block.
                        add_to_block(ins);
                        //Incrementing the corresponding length.
                        eip+=(ins.size()/2);
                    }
                    else{
                        for(int i=2;i<words[2].size()-1;i++){
                            //Else should convert to ascii values.
                            ins+=convert_to_hexdecimal(int(words[2][i]),2);
                            //cout<<int(words[2][i])<<endl;
                        }
                        add_to_block(ins);
                        //Incrementing the correspoding length.
                        eip+=words[2].size()-3;
                    }
                }
                else if(words[1]=="WORD"){
                    //Giving the required space in terms of three bytes.
                    stringstream ss;
                    int x=0;
                    ss<<words[2];
                    ss>>x;
                    //Add the instruction to the block.
                    add_to_block(convert_to_hexdecimal(x,6));
                    eip+=3;
                }
                else if(words[1]=="RESB"){
                    //Reserver the space asked and create a new block.
                    stringstream ss;
                    int x=0;
                    ss<<words[2];
                    ss>>x;
                    eip+=x;
                    create_new_block();
                }
                else{
                    //The only case left is often word.
                    stringstream ss;
                    int x=0;
                    ss<<words[2];
                    ss>>x;
                    eip+=(3*x);
                    create_new_block();
                }
            }
            else{
                //cout<<line<<" "<<block<<endl;
                //If is an end then it should break.
                if(words[1]=="END") break;
                string ins;
                ins+=op_table[words[1]];
                int x=0;
                //Normal instruction format like name opcode memory.
                if(words[2].size()>2 && words[2][words[2].size()-1]=='X' && words[2][words[2].size()-2]==','){
                    x+=32768;
                    words[2].pop_back();
                    words[2].pop_back();
                }
                //Creating hexadecimal of required format.
                x+=sym_table[words[2]];
                ins+=convert_to_hexdecimal(x,4);
                //cout<<line<<" "<<block<<" "<<ins<<endl;
                add_to_block(ins);
                //cout<<line<<" "<<block<<endl;
                eip+=3;
            }
        }
    }

    //If the block size is greater than 9, which means it contains some instuctions.
    if(block.size()>9){
        create_new_block();
    }
    block.clear();
    //Creating an end block.
    block="E";
    block+=convert_to_hexdecimal(sym_table[start_dir],6);
    fout<<block<<"\n";
    return;
}

int main(int argv,char* argc[]){
    //Generating error if input file is not given.
    if(argv<=1){
        cout<<"Please give file input while running the program!"<<endl;
        return 0;
    }
    //Generating error if output file is not given.
    else if(argv<=2){
        cout<<"Please specify the ouput file name!"<<endl;
    }

    Taking_opcodes();

    //cout<<"Taking opcodes is completed"<<endl;
    //system("pause");

    //Running phase 1 of the assembler.
    analysis(argc[1]);

    //Checking whether the used label is declared somewhere or not.
    for(auto it=ind.begin();it!=ind.end();it++){
        if(sym_table.find(it->first)==sym_table.end()){
            errors.push_back("Specified variable/label "+(it->first)+" does not exists");
        }
    }

    //Printing all the errors in the compiler fashion.
    for(int i=0;i<errors.size();i++){
        cout<<"ERROR : "<<errors[i]<<endl;
    }

    //If errors are there then no output file will be generated.
    if(errors.size()) return 0;

    //Printing the tables in tables.txt file.
    fstream ft;
    ft.open("tables.txt",ios::out);
    ft<<"Op Table is : "<<endl;

    //Printing the op table into the file.
    for(auto it=op_table.begin();it!=op_table.end();it++){
        ft<<(it->first)<<"  "<<(it->second)<<endl;
    }
    ft<<endl;

    //Printing the symbol into the file.
    ft<<"Symbol Table is : "<<endl;
    for(auto it=sym_table.begin();it!=sym_table.end();it++){
        ft<<(it->first)<<"  "<<convert_to_hexdecimal((it->second),4)<<endl;
    }
    

    //Calculating the length of the program.
    length=(eip-start_addr);

    //system("pause");

    //Running phase 2 of the assembler.
    synthesis(argc[1],argc[2]);

    //Output on successfull completion of the program.
    cout<<"Program executed successfully!"<<endl;
    return 0;
}