// Kimberley Trotz

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

bool isFlushed;
bool saveDone=true;
bool fetchDone=false;
bool decodeDone=false; 
bool executeDone=false;

// CPU infrastructure
struct CPU{
    unsigned char memory[1024];
    int registers[16];
    int PC; //program counter 

    int decodeOP1;
    int executeOP1;
    int saveOP1; 

    int decodeOP2;
    int executeOP2;
    int saveOP2;

    int executeResult;
    int saveResult; 

    int currentFetch;
    int currentDecode;
    int currentExecute;
    int currentSave;

    unsigned char currentInstructionFetch[4];
    unsigned char currentInstructionDecode[4];
    unsigned char currentInstructionExecute[4];
    unsigned char currentInstructionSave[4];
}CPU; 



void loadFile(char *filename){

	FILE *src = fopen(filename,"r");
    char line[2];
		int i, instructionSize=0;

		//Loading in instructions from assembler file
		while (NULL != fgets(line, sizeof(line), src))
		{
		    unsigned char temp;

			for(i=0;i<1;i++){
                temp = line[i];
                CPU.memory[instructionSize]=temp;
                instructionSize++;
			}
		}
	// fclose(src);
}


void flushPipeline()
{
	for(int i = 0; i < 4; i++){
		CPU.currentInstructionDecode[1]=0;
		CPU.currentInstructionExecute[1]=1;
		CPU.currentInstructionSave[1]=2;
	}

	CPU.decodeOP1 = 0;
    CPU.executeOP1 = 0; 
    CPU.saveOP1 = 0;
	CPU.decodeOP2 = 0;
    CPU.executeOP2 = 0;
    CPU.saveOP2 = 0;
	CPU.executeResult = 0;
    CPU.saveResult = 0;
	CPU.currentDecode = 0;
    CPU.currentExecute = 0;
    CPU.currentSave = 0;
	saveDone = true;
    fetchDone = false;
    decodeDone = false;
    executeDone = false;
	isFlushed=true;
}

void fetch(){
	if(saveDone!=true){
		return;
	}

    CPU.currentInstructionFetch[0] = CPU.memory[CPU.PC];
    CPU.currentFetch = (int)CPU.currentInstructionFetch[0];
    CPU.currentFetch = CPU.currentFetch/16;

    //3r format
    if(CPU.currentFetch <= 9 || CPU.currentFetch >= 14){
 		CPU.currentInstructionFetch[1]=CPU.memory[CPU.PC+1];
 		CPU.PC=CPU.PC+2;
 	}

 	if(CPU.currentFetch == 10 || CPU.currentFetch == 11 || CPU.currentFetch == 12 || CPU.currentFetch == 13){
 			CPU.currentInstructionFetch[1] = CPU.memory[CPU.PC + 1];
 			CPU.currentInstructionFetch[2] = CPU.memory[CPU.PC + 2];
 			CPU.currentInstructionFetch[3] = CPU.memory[CPU.PC + 3];
 			CPU.PC = CPU.PC + 4;
 	}


 	//send to pipeline
 	for(int i = 0; i < 4; i++){
 		CPU.currentInstructionDecode[i] = CPU.currentInstructionFetch[i];
 	}

 	CPU.currentDecode = CPU.currentFetch;
 	isFlushed=false;
 	fetchDone=true;
 	saveDone=false;
}


void decode(){
	if(isFlushed==true || fetchDone!=true){
		return;
	}

	for(int i = 0; i < 4; i++){
		if(CPU.currentInstructionDecode[i] != CPU.currentInstructionFetch[i]) {
			return;
		}
	}
    
	//3r format
	if(CPU.currentDecode <= 6) {
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[0];
		CPU.decodeOP1 = CPU.decodeOP1%16;
		CPU.decodeOP2 = (int)CPU.currentInstructionDecode[1];
	}

	//ai format
	if(CPU.currentDecode == 9){
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[0];
		CPU.decodeOP1 = CPU.decodeOP1%16;
		CPU.decodeOP2 = (int)CPU.currentInstructionDecode[1];

        //for negative nums
		if(CPU.decodeOP2 > 127){
            CPU.decodeOP2 = CPU.decodeOP2-256;
        }
	}

	//interrupt
	if(CPU.currentDecode == 8){
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[1];
	}

	//jump format
	if(CPU.currentDecode == 12 || CPU.currentDecode == 13){
		//store top bits
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[0];
		CPU.decodeOP1 = CPU.decodeOP1%16;
		CPU.decodeOP1 = CPU.decodeOP1<<8;
		CPU.decodeOP1 = CPU.decodeOP1 | (int)CPU.currentInstructionDecode[1];

		//store bottom bits
		CPU.decodeOP2 = (int)CPU.currentInstructionDecode[2];
		CPU.decodeOP2 = CPU.decodeOP2<<8;
		CPU.decodeOP2 = CPU.decodeOP2 | (int)CPU.currentInstructionDecode[3];
	}

	//br format
	if(CPU.currentDecode == 10||CPU.currentDecode == 11){
        //store bits
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[0]%16;
		CPU.decodeOP1 = CPU.decodeOP1<<4;
		CPU.decodeOP1 = CPU.decodeOP1 | (int)CPU.currentInstructionDecode[1]/16;

		//store address bits
		if((int)CPU.currentInstructionDecode[1]%16 >= 8){
			CPU.decodeOP2 = -1048576;
			CPU.decodeOP2 = CPU.decodeOP2 | (((int)CPU.currentInstructionDecode[1]%16)<<16);
			CPU.decodeOP2 = CPU.decodeOP2 | ((int)CPU.currentInstructionDecode[2])<<8;
			CPU.decodeOP2 = CPU.decodeOP2 | (int)CPU.currentInstructionDecode[3];
		}
		else{
			CPU.decodeOP2 = (int)CPU.currentInstructionDecode[1]%16;
			CPU.decodeOP2 = CPU.decodeOP2<<8;
			CPU.decodeOP2 = CPU.decodeOP2 | (int)CPU.currentInstructionDecode[2];
			CPU.decodeOP2 = CPU.decodeOP2<<8;
			CPU.decodeOP2 = CPU.decodeOP2 | (int)CPU.currentInstructionDecode[3];
		}
	}

	//stack format
	if (CPU.currentDecode==7){	
        //store reg
		CPU.decodeOP1=(int)CPU.currentInstructionDecode[0];
		CPU.decodeOP1=CPU.decodeOP1%16;

		//store instruction
		CPU.decodeOP2=(int)CPU.currentInstructionDecode[1];
		CPU.decodeOP2=CPU.decodeOP2>>6;
	}

	//ls format
	if(CPU.currentDecode == 14 || CPU.currentDecode == 15){
		CPU.decodeOP1 = (int)CPU.currentInstructionDecode[0];
		CPU.decodeOP1 = CPU.decodeOP1%16;
		CPU.decodeOP1 = CPU.decodeOP1<<4;

		CPU.decodeOP1 = CPU.decodeOP1 | ((int)CPU.currentInstructionDecode[1]/16);
		CPU.decodeOP2 = ((int)CPU.currentInstructionDecode[1]%16);

	}

	//Send to pipe
	for(int i = 0; i < 4; i++){
 		CPU.currentInstructionExecute[i] = CPU.currentInstructionDecode[i];
 	}

	CPU.currentExecute = CPU.currentDecode;
	CPU.executeOP1 = CPU.decodeOP1;
	CPU.executeOP2 = CPU.decodeOP2;
	decodeDone = true;
	fetchDone = false;
}

//instructions
void add(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] + (int)CPU.registers[CPU.executeOP2>>4];
}

void and(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] & (int)CPU.registers[CPU.executeOP2>>4];
}

void divide(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] / (int)CPU.registers[CPU.executeOP2>>4];
}

void multiply(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] * (int)CPU.registers[CPU.executeOP2>>4];
}

void subtract(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] - (int)CPU.registers[CPU.executeOP2>>4];
}

void or(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1] | (int)CPU.registers[CPU.executeOP2>>4];
}


void addImmediate(){
    CPU.executeResult = (int)CPU.registers[CPU.executeOP1]+ CPU.executeOP2;
}

void branchIfEqual(){
    if((int)CPU.registers[CPU.executeOP1/16]==(int)CPU.registers[CPU.executeOP1%16])
			{

				CPU.executeResult=CPU.executeOP2;
				CPU.executeResult=CPU.executeResult*2;
			}
			else{CPU.executeResult=1;}

}

void branchIfLess(){
    if((int)CPU.registers[CPU.executeOP1/16]<(int)CPU.registers[CPU.executeOP1%16])
			{

				CPU.executeResult=CPU.executeOP2;
				CPU.executeResult=CPU.executeResult*2;
			}
			else{CPU.executeResult=1;}
}

void jump(){
    CPU.executeResult=CPU.executeOP1;
			CPU.executeResult=CPU.executeResult<<16;
			CPU.executeResult|=CPU.executeOP2;
			CPU.executeResult = CPU.executeResult*2;
}

void call(){
    	CPU.executeResult=CPU.executeOP1;
			CPU.executeResult=CPU.executeResult<<16;
			CPU.executeResult|=CPU.executeOP2;
			CPU.executeResult = CPU.executeResult*2;

			CPU.registers[15]=(int)CPU.registers[15]-4; //move back for push
			int k=0;
			for(int j = 3; j>=0;j--)
			{
				CPU.memory[(int)CPU.registers[15]+k]=(CPU.PC>>(j*8));
				k++;
			}
			k=0;

}

void load(){
    CPU.executeResult = 0;
    for(int j=0; j<4;j++) {
        CPU.executeResult|=CPU.memory[((int)CPU.registers[(CPU.executeOP1%16)]+(2*CPU.executeOP2))+j];
        
        if(j<3){
            CPU.executeResult=CPU.executeResult<<8;
        }
    }
}

void store(){
    CPU.executeResult=(int)CPU.registers[CPU.executeOP1/16];
}

void execute(){
	if(isFlushed==true || decodeDone!=true){
		return;
	}

	for(int i = 0; i < 4; i ++) {
		if(CPU.currentInstructionExecute[i]!=CPU.currentInstructionDecode[i]){
			return;
		}
	}


    switch(CPU.currentExecute){
        case 0: //halt
            printf("%s","HALT");
            exit(0);

        case 1: 
            add();
            break;

        case 2: 
            and();
            break;

        case 3:
            divide();
            break;

        case 4:
            multiply();
            break;

        case 5:
            subtract();
            break;
        
        case 6:
            or();
            break;

        case 7:
            // stack();
            break;

        case 8: // interrupt
            break;

        case 9:
            addImmediate();
            break;

        case 10:
            branchIfEqual();
            break;

        case 11:
            branchIfLess();
            break;
        
        case 12:
            jump();
            break;

        case 13:
            call();
            break;

        case 14:
            load();
            break;

        case 15:
            store();
            break;

    }

    for(int i = 0; i < 4; i++){
        CPU.currentInstructionSave[i] = CPU.currentInstructionExecute[i];
    }

    CPU.currentSave = CPU.currentExecute;
    CPU.saveResult = CPU.executeResult;
    CPU.saveOP1 = CPU.executeOP1;
    CPU.saveOP2 = CPU.executeOP2;
    executeDone = true;
    decodeDone = false;
}

void save()
{
	if(isFlushed==true || executeDone!=true){
		return;
	}

	for(int i = 0; i < 4; i++){
		if(CPU.currentInstructionSave[i]!=CPU.currentInstructionExecute[i]){
			return;
		}
	}

	saveDone=true;

	//3r
	if(CPU.currentSave <= 6){
		CPU.registers[CPU.saveOP2%16]=CPU.saveResult;
	}

	//ai
	if(CPU.currentSave==9){
		CPU.registers[CPU.saveOP1]=CPU.saveResult;
	}

	//jump
	if (CPU.currentSave==12||CPU.currentSave==13){
		CPU.PC=CPU.saveResult;
		flushPipeline();
	}

	//ls
	if (CPU.currentSave==14)
	{
		CPU.registers[CPU.saveOP1/16]=CPU.saveResult;
	}
	if (CPU.currentSave==15)
	{
		int k=0;
		for(int j = 3; j>=0;j--)
		{
			CPU.memory[((int)CPU.registers[CPU.saveOP1%16]+(2*CPU.saveOP2))+k]=(CPU.saveResult>>(j*8));
			k++;
		}
		k=0;
	}

	//br
	if(CPU.currentSave==10 || CPU.currentSave==11){
		if(CPU.saveResult!=1)
		{
			CPU.PC = CPU.PC +CPU.saveResult;
			flushPipeline();
		}
	}

	//int
	if (CPU.currentSave == 8){
        if(CPU.saveOP1 == 0){
            printf("Print Registers\n");
            for(int i = 0; i < 16; i++){
                printf("CPU.registers[%i]: ", i);
                printf("%02x \n",CPU.registers[i]);
	        }
        }

        else if (CPU.saveOP1==1){
            int index=0;
            printf("Prit Memory");
            for(int i = 0; i < 1024; i++){
                    if(index%16 == 0){
                        printf("\n");
                    }
                    printf("%02x", CPU.memory[i]);

                    if(i%2 != 0){
                        printf(" ");
                    }
                    index++;
            }
            printf("\n");
        }
        else {printf("Error\n");}
	}

	saveDone = true;
	executeDone = false;
}


int main(int argc, char **argv)
{
    CPU.PC = 0;

    CPU.decodeOP1 = 0;
    CPU.executeOP1 = 0;
    CPU.saveOP1 = 0; 

    CPU.decodeOP2 = 0;
    CPU.executeOP2 = 0;
    CPU.saveOP2 = 0;

    loadFile(argv[1]);
    flushPipeline();

    while(CPU.PC<1024) {
        execute();
        save();
        decode();
        fetch();
    }

	return 0;
}