// Kimberley Trotz
// Create a command line application in C called siavm 
// that takes as a parameter a filename (argv[1]). 
// It should call memory’s load function then call the run loop.

// IMPORTANT: Typically, students are taught to NEVER use global variables, 
// to pass values as function parameters and to NOT hardcode values. 
// FOR THIS PROJECT, DISREGARD ALL OF THESE RULES. The Fetch, Dispatch, 
// Execute and Save functions should accept no parameters and should return void.

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Memory
// Allocate space for your virtual machine’s memory. 
// 1k or so as unsigned byte should be enough. 
// This can be done on the stack, globally.
unsigned char memory[1000];
bool halt = false;

// CPU infrastructure
struct CPU{
    int registers[16]; // Create 16 registers. 
    int PC; // Create a PC internal register. 
    int OP1; // Create OP1 and OP2 and Result registers. 
    int OP2;
    int result;
    unsigned char currentInstruction[4]; // Create a “current instruction” buffer.
}CPU;

// Create a load function that takes a filename; 
void loadFile(char *filename){
    FILE *src = fopen(filename, "r"); // read the file
    int index = 0;
    while(!feof(src)){
        memory[index] = fgetc(src); // store the file into the virtual machine's memory
        index++;       
    }  
}

// Fetch
void fetch(){
    memcpy(CPU.currentInstruction, memory + CPU.PC, 4); // Read instruction bytes from memory. 
    // printf("%s\n", memory);
    // printf("%s\n", memory + CPU.PC);
}

// Decode
// Read only from the array of bytes from fetch. 
// Populate the OP1 and OP2 registers if appropriate. 
// Fetch any additional memory needed to complete the instruction.
void decode(){
    CPU.OP1 = CPU.registers[(CPU.currentInstruction[0] & 0x0f) - 1];
    CPU.OP2 = CPU.registers[((CPU.currentInstruction[1] >> 4) & 0x0f) - 1];

    // printf("%d\n", CPU.OP1);
    // printf("%d", CPU.OP2);
}


// instructions
// 3R Format
void add(){
    CPU.result = CPU.OP1 + CPU.OP2;
    CPU.PC += 2;
}

void and(){
    CPU.result = CPU.OP1 & CPU.OP2;
    CPU.PC += 2;
}

void divide(){
    // CPU.result = CPU.OP1 / CPU.OP2;
    CPU.PC += 2;
}

void multiply(){
    CPU.result = CPU.OP1 * CPU.OP2;
    CPU.PC += 2;
}

void or(){
    CPU.result = CPU.OP1 | CPU.OP2;
    CPU.PC += 2;
}

void subtract(){
    CPU.result = CPU.OP1 - CPU.OP2;
    CPU.PC += 2;
}


// ai format
void addimmediate(){
    unsigned char value = CPU.currentInstruction[1];
    int x;

    // x = atoi(value);

    if((value & -(0x8)) == 0x8){
        value = value & 0x7f;
        x = value;
        
    }
    else{
        // printf("%x",value);
        value = value & 0x7f;
        x = value;
        x = x * -1;
    }

    CPU.result = x;
    CPU.PC += 2; 

    printf("%d\n", CPU.result);
}


// br format
void branchifequal(){
    unsigned int firstBits = CPU.currentInstruction[1] & 0x0f;
    unsigned int lastBits = (CPU.currentInstruction[2] << 8) | CPU.currentInstruction[3];

    unsigned int OS = firstBits << 16 | lastBits; //offset

    if((OS >> 19) == 1){
        OS = OS << 13;
        OS = OS >> 13;
        OS *= -1;
    }

    if(CPU.OP1 == CPU.OP2){
        CPU.PC += OS;
    }

    else{
        CPU.PC += 4;
    }
}

void branchifless(){
    unsigned int firstBits = CPU.currentInstruction[1] & 0x0f;
    unsigned int lastBits = (CPU.currentInstruction[2] << 8) | CPU.currentInstruction[3];
    
    unsigned int OS = firstBits << 16 | lastBits;

    if((OS >> 19) == 1){
        OS = OS << 13;
        OS = OS >> 13;
        OS *= -1;
    }

    if(CPU.OP1 < CPU.OP2){
        CPU.PC += OS;
    }

    else{
        CPU.PC += 4;
    }
}


// int format
void interrupt(){
    int instruction = ((CPU.currentInstruction[0] & 0x0f) << 8) | CPU.currentInstruction[1];
    int index; 

    if(instruction == 0){
        // display registers
        for(index = 0; index < 16; index++){
            printf("R%d: %d\n", index + 1, CPU.registers[index]);
        }
    }
    else{
        // display memory
        for(index = 0; index < 65; index++){
            printf("$%d: %x\n", index, memory[index]);
        }
    }

    CPU.PC += 2;
}

// jump format
// call
void jump(){
    int firstBits = ((CPU.currentInstruction[0] & 0x0f) << 8) | CPU.currentInstruction[1];
    int lastBits = (CPU.currentInstruction[2] << 8) | CPU.currentInstruction[3];
    int OS = (firstBits << 16) | lastBits;
    CPU.PC = OS;
}

// ls format
void load(){
    signed char OS = (signed char) CPU.currentInstruction[1] & 0x0f;

    if(OS < 0){
        OS = OS << 1;
        OS = OS >> 1;
        OS *= -1;
    }
    
    CPU.result = memory[CPU.OP2 + OS];
    CPU.PC += 2;
}

void store(){
    CPU.result = CPU.OP1;
    CPU.PC += 2;
}

//stack format
//pop
//push
//return



// Execute
// Switch statement that “does the work” and stores the work into Result
void execute(){
    switch((CPU.currentInstruction[0] >> 4) & 0x0f){

        case 0:
            halt = true;
            break;
        
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


        //pop

        //push

        case 8:
            interrupt();
            break;

        case 9:
            addimmediate();
            break;

        case 10:
            branchifequal();
            break;

        case 11:
            branchifless();
            break;

        case 12:
            jump();
            break;

        case 14:
            load();
            break;

            //call

            //return

        case 15:
            store();
            break;
        
    }
}

// Store
// Write result into final register or memory address. 
void save(){
    signed char OS = (signed char) CPU.currentInstruction[1] & 0x0f;

    switch((CPU.currentInstruction[0] >> 4) & 0x0f){
        case 1: // add
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;
            break;

        case 2: // and
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;
            break;

        case 3: // divide
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;
            break;

        case 4: // multiply
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;
            break;

        case 5: // subtract
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;
            break;

        case 6: // or
            CPU.registers[(CPU.currentInstruction[1] & 0x0f)-1] = CPU.result;;
            break;
        
        case 9: // addimmediate
            CPU.registers[(CPU.currentInstruction[0] & 0x0f)-1] = CPU.result;
            break;
        
        case 13: // call
            CPU.registers[(CPU.currentInstruction[0] & 0x0f)-1] = CPU.result;
            break;

        case 14: // load
            CPU.registers[(CPU.currentInstruction[0] & 0x0f)-1] = CPU.result;
            break;     

        case 15: // store
            if(OS < 0){
                OS = OS << 1;
                OS = OS >> 1;
                OS *= -1;
            }
            memory[CPU.OP2 + OS] = CPU.result;
            break;      
    }
}



int main(int argc, char *argv[]){
    CPU.PC = 0; // program counter
    loadFile(argv[1]); //This file should be the OUTPUT from your assembler.

    // Create a run loop that calls fetch, decode, execute, 
    // store until a flag indicating that HALT occurred.
    while(!halt){
        fetch();
        decode();
        execute();
        save(); //store  
    }  
    return 0;
}


