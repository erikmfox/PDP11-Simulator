/*
ECE 586 PDP11 project - Portland State University
Professor Yuchen Huang
Due March 15, 2019
Team members:
	Alec Wiese
	Alexandra Pinzon
	Erik Fox
	Jennifer Lara
*/
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

//convert to decimal
#define MASK_OP_UPPER_2 61440 //170000
#define MASK_OP_UPPER_4 65472 //177700

//Memory commands
#define READ	0
#define WRITE	1
#define FETCH	2

//--------Overall Masks----------------
//DOUBLE OPERAND
	//SSDD format
#define DUB_OP_GEN_LOG_MASK	"070000"
#define DUB_OP_GEN_LOG_LOW	"010000"
#define DUB_OP_GEN_LOG_HI	"060000"

//RSS Format
#define DUB_OP_REG_MASK	"077000"
#define DUB_OP_REG_LOW	"070000"
#define DUB_OP_REG_HI	"077000"

//RDD Format
#define DUB_OP_REG_XOR_MASK	"177000"
#define DUB_OP_REG_XOR		"074000"

//OPERAND masks
#define DUB_OP_GEN_LOG_SS_MASK		"000700"
 //007700
#define DUB_OP_GEN_LOG_DD_MASK		"000007"
// 000077

#define DUB_OP_REG_R_MASK		"000700"
#define DUB_OP_REG_XX_MASK		"000007"
//"000077"


//Addressing mode Masks
//SSDD
#define SOURCE_MODE_MASK 		"007000" //one
#define DEST_MODE_MASK 			"000070" //two
//RSS
#define REG_MODE_MASK			"000070"


//SINGLE OPERAND
	//DD format
#define SIN_OP_MASK	"007700"
#define SIN_OP_LOW	"005000"
#define SIN_OP_HI	"006700"

#define SIN_OP_SWAB_MASK	"077700"
#define SIN_OP_SWAB			"000300"

//OPERAND masks
#define SIN_OP_DD_MASK "000007"

//Addressing Mode Masks
#define SIN_MODE_MASK "000070" 



//Conditional Operand
#define COND_MASK "007700"
#define COND_MASK_L "000000"
#define COND_MASK_H "003400"
#define COND_MASK_OFFSET "000377"

//----Single operand (4-oct digit opcode) -----------
////////MODE AND DESTINATION///////
#define SIN_MODE	"000070"
#define SIN_REG		"000007"

////////////OP CODES///////////////
//General
#define CLR_B "005000"
#define CLR_W "105000"

#define COM_B "005100"
#define COM_W "105100"

#define INC_B "005200"
#define INC_W "105200"

#define DEC_B "005300"
#define DEC_W "105300"

#define NEG_B "005400"

#define NEG_W "105400"

#define TST_B "005700"
#define TST_W "105700"

//Shift and rotate
#define ASR_B "006200"
#define ASR_W "106200"

#define ASL_B "006300"
#define ASL_W "106300"

#define ROR_B "006000"
#define ROR_W "106000"

#define ROL_B "006100"
#define ROL_W "106100"

#define SWAB "000300"

//Multiple Precision
#define ADC_B "005500"
#define ADC_W "105500"

#define SBC_B "005600"
#define SBC_W "105600"

#define SXT "006700"


//---------Double operand-----------
////////MODE AND DESTINATION///////
#define DUB_MODE_0	"007000"
#define DUB_SOURCE	"000700"
#define DUB_MODE_1	"000070"
#define DUB_DEST	"000007"

////////////OP CODES///////////////
//General  (2-oct digit opcode) 
#define MOV_B "010000"
#define MOV_W "110000"

#define CMP_B "020000"
#define CMP_W "120000"

#define ADD "060000"
#define SUB "160000"

//Logical  (2-oct digit opcode) 
#define BIT_B "030000"
#define BIT_W "130000"

#define BIC_B "040000"
#define BIC_W "140000"

#define BIS_B "050000"
#define BIS_W "150000"


////////MODE AND DESTINATION///////
#define DUB_REG_REG		"000700"
#define DUB_REG_MODE	"000070"
#define DUB_REG_SRCDST	"000007"

////////////OP CODES///////////////
//Register  (3-oct digit opcode) 
#define MUL "070000" //SOURCE
#define DIV "071000" //SOURCE
#define ASH "072000" //SOURCE
#define ASHC "073000" //SOURCE
#define XOR "074000" //DESTINATION


//----Program control  (4-oct digit opcode) --------
////////MODE AND DESTINATION///////
#define PRG_CTRL_OFFSET	"000377"

////////////OP CODES///////////////
//Branch
#define BR "000400"
#define BNE "001000"
#define BEQ "001400"
#define BPL "100000"
#define BMI "100400"
#define BVC "102000"
#define BVS "102400"
#define BCC "103000"
#define BCS "103400"

//Signed conditional branch
#define BGE "002000"
#define BLT "002400"
#define BGT "003000"
#define BLE "003400"

//Unsigned Conditional Branch
#define BHI "101000"
#define BLOS "101400"
#define BHIS "103000"
#define BLO "103400"

////////MODE AND DESTINATION///////
//Figure out screwy op codes for below#define PRG_CTRL_OFFSET	"000377"

////////////OP CODES///////////////
//Jump & subroutine
#define JMP "000100"
#define JSR "004000"// (3-oct digit opcode) 
#define RTS "000200"
#define MARK "006400"
#define SOB "07700" 


struct opDataStruct {
	uint16_t operand;
	uint16_t address;
	bool isRegister;
	uint16_t *regPoint;
};
bool branch = false;
bool byte = false;
uint16_t source, destination, registerName, offset, ssddModeMask1, ssddModeMask2, rssModeMask, sopModeMask;
uint16_t	R7 = 0,//Program Counter
R6 = 0,//Stack pointer
R5 = 0,
R4 = 0,
R3 = 0,
R2 = 0,
R1 = 0,
R0 = 0
;

int instructionCount = 0;
int code_Z,//if result was zero
code_N,//if result was negative
code_C,//result forces carry from MSB
code_V//if op results in arithmetic overflow
;


ifstream inputFile("test2.ascii");
ofstream traceFile("trace.txt");


#define MEM_SIZE 65536//2^16
uint8_t memory[MEM_SIZE];

int starFlag = 0;


int skipPC = false;
bool isOP, isVar;
bool setAddress, startAddressFound = false;
bool halt = false;

void pr(string input) {
	cout << "\n" << input << "\n";
}

void prLabel(string input0, uint16_t input1) {
	cout << "\n" << input0 << ": " << dec << input1;
}

//#define DEBUG_octStringToNum 1
uint16_t octStringToNum(string input) {
	uint16_t output = 0;
	//setAddress = false;
	//isOP = false;
	//isVar = false;
	int octPos = input.length(),
		octVal = 0;

#ifdef DEBUG_octStringToNum
	pr("\nOctal: " + input);
#endif // DEBUG_octStringToNum


	for (char& c : input) {
		if (c == '7') {
			octVal = 7;
		}
		else if (c == '6') {
			octVal = 6;
		}
		else if (c == '5') {
			octVal = 5;
		}
		else if (c == '4') {
			octVal = 4;
		}
		else if (c == '3') {
			octVal = 3;
		}
		else if (c == '2') {
			octVal = 2;
		}
		else if (c == '1') {
			octVal = 1;
		}
		else if (c == '0') {
			octVal = 0;
		}
		else {
			octVal = 0;
		}
		output += (octVal << ((octPos - 1) * 3));
		octPos--;

	}


#ifdef DEBUG_octStringToNum
	cout << "\nBase 10: " << unsigned(output);
#endif // DEBUG_octStringToNum
	return output;
}



int atFlag = 0;
uint16_t memPos = 0;
//#define DEBUG_fillMemory 1
int fillMemory() {
	string line;
	int atFlag = 0;
	int octPos, octVal = 0;
	uint16_t output;
	ifstream myfile("test.ascii");

	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			output = 0;
			atFlag = 0;
			cout << line << endl;
			octPos = line.length();
			octVal = 0;
			for (int i = 0; i < line.length(); ++i)
			{
				if (line[i] == '@')
				{
					atFlag = 1;
				}	//Set MemPos
				else if (line[i] == '*')
				{
					starFlag++;
				}	//Set CodeStart Pos
				else
				{
					if (line[i] == '7')
					{
						octVal = 7;
					}
					else if (line[i] == '6')
					{
						octVal = 6;
					}
					else if (line[i] == '5')
					{
						octVal = 5;
					}
					else if (line[i] == '4')
					{
						octVal = 4;
					}
					else if (line[i] == '3')
					{
						octVal = 3;
					}
					else if (line[i] == '2')
					{
						octVal = 2;
					}
					else if (line[i] == '1')
					{
						octVal = 1;
					}
					else
					{
						octVal = 0;
					}
				}	//place new code in memPos
					//increment memPos by two
				output += (octVal << ((octPos - 1) * 3));
				octPos--;
			}
			if (atFlag)
			{
				memPos = output;
				//atFlag=0;
			}
			else {
				if (starFlag == 1)
				{
					//Set Program Counter to output
					R7 = memPos;
					//prLabel("Set starting address to", R7);
					starFlag++;

				}
				if (starFlag > 2) {
#ifdef DEBUG_fillMemory
					pr("************************ERROR - Multiple starting addresses detected************************");
#endif //DEBUG_fillMemory
				}
				//		cout<<output<<endl;
				memory[memPos] = (output >> 8);
				memory[memPos + 1] = output;
				memPos += 2;
			}
		}
		myfile.close();
	}
	else
	{

		cout << "unable to open file" << endl;
		return 0;
	}
	return 1;
}

void writeFile(int instructionType, int address) {
	traceFile << instructionType << "\t" << setfill('0') << setw(6) << oct << address << "\n";
}

void fetch() {
	writeFile(FETCH, R7);//Write the instruction fetch to the ASCII file
}

void incPC() {
	if (!branch)
	{
		if (skipPC == 1) {
			R7 += 4;//increment the PC past data
			//cout<<"incPC (+4): "<<R7<<endl;
		}
		else if (skipPC == 2) {
			R7 += 6;
		}
		else {

			//cout<<"incPC (+2): "<<R7<<endl;
			R7 += 2;//increment the PC
		}
	}
	else { cout << "Branch" << endl; }
}


uint16_t ret_word(uint16_t mem_addr)
{
	uint16_t operand;
	operand = (unsigned)memory[mem_addr] << 8;
	operand += (unsigned)memory[mem_addr + 1];//one memory reference
	writeFile(READ, mem_addr);
	return operand;
}


void write2dest(struct opDataStruct input)//uint16_t mem_addr, uint16_t data)
{
	uint8_t byteData;
	////GET DATA FROM STRUCT PASSED IN

	//reset byte to false at the beginning of decode
	//set byte to true during instruction decode
	if (byte) {
		byteData = 0x00FF & input.operand;

		if (input.isRegister) {
			*input.regPoint = byteData;
		}
		else {
			memory[input.address] = byteData;

			writeFile(WRITE, input.address);//Write the written address to the ASCII file
		}
	}
	else {

		if (input.isRegister) {
			*input.regPoint = input.operand;
		}
		else {
			memory[input.address] = (input.operand >> 8);
			memory[input.address + 1] = input.operand;

			writeFile(WRITE, input.address);//Write the written address to the ASCII file
		}
	}
}

uint16_t * getRegPtr(uint16_t regID) {
	uint16_t * address;
	if (regID == 0)
		address = &R0;
	else if (regID == 1)
		address = &R1;
	else if (regID == 2)
		address = &R2;
	else if (regID == 3)
		address = &R3;
	else if (regID == 4)
		address = &R4;
	else if (regID == 5)
		address = &R5;
	else if (regID == 6)
		address = &R6;
	else
		address = &R7;
	return address;

}

struct opDataStruct ret_operand(uint16_t mode, uint16_t reg)
{
	uint16_t operand;
	uint16_t *regPtr;
	uint16_t increment;
	uint8_t address;
	struct opDataStruct opData;
	opData.isRegister = false;

	if (byte) {
		increment = 1;
	}
	else {
		increment = 2;
	}

	regPtr = getRegPtr(reg);
	opData.regPoint = regPtr;

	if (reg == 7 && (mode == 2 || mode == 3 || mode == 6 || mode == 7))//PC Register Addressing modes
	{
		++skipPC;
		if (mode == 2)
			//Immediate: PC +2 has operand; Next instruction @ PC+ 4
		{
			cout << "PC Register Addressing Mode: Immediate" << endl;
			if (skipPC == 1)
			{
				opData.address = R7 + 2;  // We got the word from R7+2
				operand = ret_word(R7 + 2);
			}
			else
			{
				operand = ret_word(R7 + 4);
				opData.address = R7 + 4;
			}
		}
		else if (mode == 3)
			//Absolute: PC+2 has Address; Operand at location of address; Next instruction @ PC+4
		{
			cout << "PC Register Addressing Mode: Absolute" << endl;
			if (skipPC == 1)
			{
				operand = ret_word(R7 + 2);
				opData.address = operand;
				operand = ret_word(operand);//two memory references
			}
			else
			{
				operand = ret_word(R7 + 4);
				opData.address = operand;
				operand = ret_word(operand);//two memory references
			}
		}
		else if (mode == 6)
			//Relative: PC+2 has a value X; Operand @ X + PC + 4; Next instruction @ PC+4
		{
			cout << "PC Register Addressing Mode: Relative" << endl;
			if (skipPC == 1)
			{
				operand = ret_word(R7 + 2);
				opData.address = operand + R7 + 4;
				operand = ret_word(operand + R7 + 4);//two memory references
			}
			else
			{
				operand = ret_word(R7 + 4);
				opData.address = operand + R7 + 4;
				operand = ret_word(operand + R7 + 4);//two memory reference
			}
		}
		else
			//Mode 7 Relative Deferred: PC+2 has value X; @ PC+4+X is Address A;  Operand @ A ; Next instruction @ PC+4
		{

			cout << "PC Register Addressing Mode: Relative Deferred" << endl;
			if (skipPC == 1)
			{
				operand = ret_word(R7 + 2);
				operand = ret_word(operand + R7 + 4);
				opData.address = operand;
				operand = ret_word(operand);//three memory references
			}
			else
			{
				operand = ret_word(R7 + 4);
				operand = ret_word(operand + R7 + 4);
				opData.address = operand;
				operand = ret_word(operand);//three memory references
			}
		}
	}
	else//General Register Addressing Modes
	{
		//Note: R7 and R6 are always incremented/decremented by 2
		if (mode == 0)
			// Register: Register contains operand; Next instruction @ PC +2
		{
			cout << "Gen Reg Addressing Mode: Register" << endl;
			operand = *regPtr;
			opData.regPoint = regPtr;
			opData.isRegister = true;
		}
		else if (mode == 1)
			//Register Deferred: Register has address location of operand; Next instruction @ PC+2
			//each is one memory reference
		{
			cout << "Gen Reg Addressing Mode: Register Deferred" << endl;
			operand = *regPtr;
			opData.address = operand;
			operand = ret_word(operand);
		}
		else if (mode == 2)
			//Autoincrement: Register has address location of operand.  After reading register increment reg by 1 for byte 2 for word (SEE NOTE); Next instruction @ PC + 2
		{
			cout << "Gen Reg Addressing Mode: Auto Increment" << endl;
			operand = *regPtr;
			opData.address = operand;
			operand = ret_word(operand);
			*regPtr += increment;
		}

		else if (mode == 3)
			//Auto increment deferred: Register contains address of address where operand is located.  increment register by 2; Next instruction @ PC+2
		{
			cout << "Gen Reg Addressing Mode: AutoIncrement deferred" << endl;
			operand = *regPtr;
			operand = ret_word(operand);
			opData.address = operand;
			operand = ret_word(operand);
			*regPtr += increment;
		}
		else if (mode == 4)
			//Auto Decrement: decrement register by 1 for byte and 2 for word (SEE NOTE) then register has the address of operand location. Next instruction @ PC+2
		{

			cout << "Gen Reg Addressing Mode: Auto Decrement" << endl;
			*regPtr -= increment;
			operand = *regPtr;
			opData.address = operand;
			operand = ret_word(operand);

		}
		else if (mode == 5)
			//Auto decrement deferred: Decrement reg by two then R has address of address where operand is located: Next Instruction @ PC+2
		{
			cout << "Gen Reg Addressing Mode: Auto Decrement Deferred" << endl;
			*regPtr -= increment;
			operand = *regPtr;
			operand = ret_word(operand);
			opData.address = operand;
			operand = ret_word(operand);
		}
		else if (mode == 6)
			//Index:R is an address. R+X is location of operand; Next Instruction @ PC+4
		{
			cout << "Gen Reg Addressing Mode: Index" << endl;
			++skipPC;
			if (skipPC == 1)
			{
				uint16_t X = ret_word(R7 + 2);
				uint16_t Rn = *regPtr;
				uint16_t address = Rn + X;
				operand = ret_word(address);
			}
			else
			{
				uint16_t X = ret_word(R7 + 4);
				uint16_t Rn = *regPtr;
				uint16_t address = Rn + X;
				operand = ret_word(address);
			}

		}
		else
			//Mode 7 Index deferred: R is addres. R+X is address of address where operand is located; Next instruction @ PC+4
		{
			cout << "Gen Reg Addressing Mode: Index Deferred" << endl;
			++skipPC;
			if (skipPC == 1)
			{
				uint16_t X = ret_word(R7 + 2);
				uint16_t Rn = *regPtr;
				uint16_t address = Rn + X;
				operand = ret_word(address);
				opData.address = operand;
				operand = ret_word(operand);
			}
			else
			{
				uint16_t X = ret_word(R7 + 4);
				uint16_t Rn = *regPtr;
				uint16_t address = Rn + X;
				operand = ret_word(address);
				opData.address = operand;
				operand = ret_word(operand);
			}
		}
	}
	if (byte)
		opData.operand = 0x00FF & operand;

	else
		opData.operand = operand;
	return opData;

}
/*functions*/
uint16_t jmp(uint16_t offset)
{
	/* this is like a branch unconditional */
	// PC = dest
	//since offset = 8bytes, take lowest 6 bytes
	R7 = offset >> 2; //shift by two bits to get lower 6 bits 

	return R7;

}

uint16_t mark(uint16_t offset)
{
	//if offset == NN ; NN = 6 bits 
	R6 = R6 + (2 * offset);
	R7 = R5;
	R5 = (R6)++;
	return R7;
}
uint16_t sob(uint16_t offset)
{
	//description:  register is decremented. if it's not equal to 0, twice the offset is substracted from PC ( now pointing to following word); 
	//need access to R 
	uint16_t temp = 0;
	temp = registerName - 1;
	//should decrement register?
	if (temp != 0)
	{
		R7 = R7 - (2 * offset);
		branch = true;
	}
	return R7;

}
/* subroutine and jmp end*/
uint16_t br_BNE(uint16_t offset) {
	// branch if not equal (to zero) so if Z = 0
	if (code_Z == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "\nERR: Z != 0\n" << endl;
}
uint16_t br_BPL(uint16_t offset)
{
	// branch depending on state of N bit, causes branch if N = positive result
	if (code_N > 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "\nERR: N < 0\n" << endl;


}
uint16_t br_BVC(uint16_t offset)
{
	//branch if overflow is clear
	//tests state of V bit and causes branch if V bit is = 0
	if (code_V == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false; //In case offset is zero
		return R7;
	}
	else
		cout << "\nERR: V != 0\n" << endl;



}

uint16_t br_BCC(uint16_t offset)
{
	//branch if C is = 0
	if (code_C == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "\nERR: C != 0\n" << endl;


}
uint16_t br_BGE(uint16_t offset)
{
	//branch if N or V = 0
	//this is one is a bit weird, might actually be xor, but description was weird
	if (code_N && code_V)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else if ((code_N == 0) && (code_V == 0))
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "N or V are neither set or clear at the same time" << endl;

}

uint16_t br_BGT(uint16_t offset)
{
	//branch if great than 0
	// if z or (NxorV) = 0
	if ((code_N ^ code_V) == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	if (code_Z == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "Z != 0 or (NxorV) != 0" << endl;


}
uint16_t br_BHI(uint16_t offset)
{
	//branch if higher
	//C=0 and Z=0
	if ((code_C == 0) && (code_Z == 0))
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "Z != 0 and C != 0" << endl;


}

uint16_t br_BHIS(uint16_t offset)
{
	//branch if higher or same
	//C = 0
	if (code_C == 0)
	{
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		cout << "C != 0 " << endl;



}
uint16_t br_unc(uint16_t offset)
{
	/* Branch unconditional, update PC */

	R7 = (R7 + 2) + (2 * offset);
	branch = true;
	return R7; /* Where R7 is program counter */
}

uint16_t br_minus(uint16_t offset) {

	/* Branch if previous operation caused the negative flag to be =1 */

	if (code_N) {
		/* Negative flag is set */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		std::cout << "\nERR: N != 1\n";

	return R7;
}

uint16_t br_eq(uint16_t offset) {

	/* Branch if previous operation caused the zero flag to be =1 */

	if (code_Z) {
		/* Zero flag is set */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		std::cout << "\nERR: Z != 1\n";

	return R7;
}

uint16_t br_overfl(uint16_t offset) {

	/* Branch if previous operation caused overflow to be =1 */

	if (code_V) {
		/* Overflow flag is set */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		std::cout << "\nERR: V != 1\n";

	return R7;
}

uint16_t br_ltz(uint16_t offset) {

	/* Branch if previous operation caused overflow and negative to be opposites */

	if (code_V ^ code_N) {
		/* Overflow XOR Negative flag = 1 */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		std::cout << "\nERR: V XOR N != 1\n";

	return R7;
}

uint16_t br_lteq(uint16_t offset) {

	/* Branch if previous operation caused zero to be =1 or negative =1 or overflow =1 */

	if ((code_V ^ code_N) | code_Z) {
		/* Overflow XOR Negative flag then OR'd with Zero flag */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
		{
			cout << "my friend" << endl;
			branch = false;
		}
		return R7;
	}
	else
		std::cout << "\nERR: (V XOR N) | Z != 1\n";

	return R7;
}

uint16_t br_lwsam(uint16_t offset) {

	/* Branch if previous operation caused carry or zero to be =1 */
	cout << "BLOS" << endl;
	if (code_C | code_Z) {
		/* carry or negative flag is set */
		cout << "C or z" << endl;
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
		{
			cout << "my friend" << endl;
			branch = false;
		}
		return R7;
	}
	else
		std::cout << "\nERR: Z | C != 1\n";

	return R7;
}

uint16_t br_lw(uint16_t offset) {

	/* Branch if previous operation caused carry to be =1 */

	if (code_C) {
		/* carry flag is set */
		branch = true;
		R7 = R7 + (2 * offset);
		if (!offset)
			branch = false;
		return R7;
	}
	else
		std::cout << "\nERR: C != 1\n";

	return R7;
}
/*branchs end*/
void clr() {
	/* Clear destination */

	opDataStruct Ry; //Struct objects

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	Ry.operand = 0; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	code_Z = 1;

	code_N = 0;

	code_C = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void clrB() {
	/* Clear destination */

	opDataStruct Ry; //Struct objects

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	Ry.operand = 0; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	code_Z = 1;

	code_N = 0;

	code_C = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void inc() {
	/* Increment destination */

	opDataStruct Ry; //Struct objects
	int16_t result, y; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = (int16_t)Ry.operand;
	result = (int16_t)Ry.operand + 1; //Result to be used for flag calculations
	Ry.operand = Ry.operand + 1; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (Ry.operand == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0)
		code_N = 1;
	else
		code_N = 0;

	if ((Ry.operand == y) && (y == 32767)) // if dest held 077777
		code_V = 1;
	else
		code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void incB() {
	/* Increment destination */

	opDataStruct Ry; //Struct objects
	int8_t result, y; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = (int8_t)Ry.operand;
	result = (int8_t)Ry.operand + 1; //Result to be used for flag calculations
	Ry.operand = Ry.operand + 1; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (Ry.operand == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0)
		code_N = 1;
	else
		code_N = 0;

	if ((Ry.operand == y) && (y == 127)) // if dest held 177
		code_V = 1;
	else
		code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void neg() {
	/* Negate destination */

	opDataStruct Ry; //Struct objects
	int16_t result; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	result = -(int16_t)Ry.operand; //Result to be used for flag calculations
	Ry.operand = -Ry.operand; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0)
		code_N = 1;
	else
		code_N = 0;

	if (Ry.operand == 100000)
		code_V = 1;
	else
		code_V = 0;

	if (result == 0)
		code_C = 0;
	else
		code_C = 1;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void negB() {
	/* Negate destination */

	opDataStruct Ry; //Struct objects
	int16_t result; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	result = -(int16_t)Ry.operand; //Result to be used for flag calculations
	Ry.operand = -Ry.operand; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0)
		code_N = 1;
	else
		code_N = 0;

	if (Ry.operand == 200)
		code_V = 1;
	else
		code_V = 0;

	if (result == 0)
		code_C = 0;
	else
		code_C = 1;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void asr() {
	/* Arithmetic shift right destination */

	opDataStruct Ry; //Struct objects
	int16_t result; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	result = (int16_t)Ry.operand >> 1; //Result to be used for flag calculations
	Ry.operand = (uint16_t)result; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result & (1 << 15))
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	code_C = result & 1;

	code_V = code_N ^ code_C;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void asrB() {
	/* Arithmetic shift right destination */

	opDataStruct Ry; //Struct objects
	int8_t result; //temp variable
	int x;

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	x = (int)Ry.operand & 0x0001;
	code_C = x;
	result = (int16_t)Ry.operand >> 1; //Result to be used for flag calculations
	Ry.operand = (uint16_t)result; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result & (1 << 7))
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;


	code_V = code_N ^ code_C;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void ror() {
	/* Rotate right destination */

	opDataStruct Ry; //Struct objects
	uint16_t result; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	result = Ry.operand >> 1; //Result to be used for flag calculations
	code_C = Ry.operand & 1;
	Ry.operand = result | (code_C << 15);
	result = Ry.operand;

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result & (1 << 15))
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	code_V = code_N ^ code_C;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void rorB() {
	/* Rotate right destination */

	opDataStruct Ry; //Struct objects
	uint16_t result; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	result = Ry.operand >> 1; //Result to be used for flag calculations
	code_C = Ry.operand & 1;
	Ry.operand = result | (code_C << 7);
	result = Ry.operand;

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result & (1 << 7))
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	code_V = code_N ^ code_C;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void swapb() {
	/* Swap bytes destination */

	opDataStruct Ry; //Struct objects
	uint16_t upper, lower; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information

	upper = Ry.operand >> 8;
	lower = (Ry.operand & 0x00FF) << 8;
	Ry.operand = lower + upper;

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (upper & (1 << 7))
		code_N = 1;
	else
		code_N = 0;

	if (upper == 0)
		code_Z = 1;
	else
		code_Z = 0;

	code_V = 0;
	code_C = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void sbc() {
	/* Subtract carry destination */

	opDataStruct Ry; //Struct objects
	uint16_t result, dest; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	dest = Ry.operand;
	result = dest - (uint16_t)code_C; //Result to be used for flag calculations
	Ry.operand = result;

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if ((int16_t)result < 0)
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (dest == (1 << 15))
		code_V = 1;
	else
		code_V = 0;

	if ((dest == 0) && code_C)
		code_C = 0;
	else
		code_C = 1;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void sbcB() {
	/* Subtract carry destination */

	opDataStruct Ry; //Struct objects
	uint16_t result, dest; //temp variable

	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	dest = Ry.operand;
	result = dest - (uint16_t)code_C; //Result to be used for flag calculations
	Ry.operand = result;

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if ((int16_t)result < 0)
		code_N = 1;
	else
		code_N = 0;

	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (dest == (1 << 8))
		code_V = 1;
	else
		code_V = 0;

	if ((dest == 0) && code_C)
		code_C = 0;
	else
		code_C = 1;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void add() {
	/* Add two operands and update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int16_t result, x, y; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int16_t)Rx.operand; //to be used for flag calculations
	y = (int16_t)Ry.operand;

	Ry.operand = Rx.operand + Ry.operand; // Actual result to be used for program
	result = x + y; //result to be used for flag calculations

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0)
		code_N = 1;
	else
		code_N = 0;

	if (result >= 65536 && result <= 98303)
		code_C = 1;
	else
		code_C = 0;

	/* arithmetic overflow: both operands were of the same sign and
		the result was of the opposite sign */
	if ((x > 0 && y > 0) && result < 0)
		code_V = 1;
	else if ((x < 0 && y < 0) && result > 0)
		code_V = 1;
	else
		code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}
//General
void com() {
	//instruction X051DD ; X indicating if byte enabled instruction or not
	//replaces the contents of the destination address by their 
	//logical complement( each bit equal to 0 is set and each bit equal to
	//1 is cleared. 
	//N:set if most significant bit of the result is set; cleared otherwise
	//Z: set if result is 0; clear otherwise
	//V: cleared
	//C: set
	opDataStruct Ry; //Struct object
	uint16_t result, y;    //temp variable

	Ry = ret_operand(sopModeMask, destination); //fill the struct with destination information; SIN_OP_LOW should be mask for instructions 005000

	y = Ry.operand;    //to be used for flag calculation

	Ry.operand = ~Ry.operand;   //actual result to be used for program
	result = ~y;                //result to be used for flag calculation

	if (result < 0)    //if result < 0 ; N  = 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 0)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	code_C = 1;
	code_V = 0;
	write2dest(Ry); //send out destination data to be updated in memory
	return;
}

void dec() {
	/*
	 * Instruction X053DD; decrements destination register given *
	 * N: set if result is < 0; cleared otherwise *
	 * Z: set if result is 0; cleared otherwise *
	 * V: set if (dest? was 100000; cleared otherwise
	 * C: not affected
	 */
	opDataStruct Ry; //Struct objects
	int result, y;
	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = (int)Ry.operand;

	Ry.operand = Ry.operand - 1;
	result = y - 1;

	/*update flags*/
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;
	if (result < 0)
		code_N = 1;
	else
		code_N = 0;
	if (result == 32768)
		code_V = 1;
	else
		code_V = 0;
	//code_C not affected;
	write2dest(Ry);    //send out destination data to be updated in mem
	return;
}

void tst() {
	/* instruction x057DD; Sets the condition codes N and Z according to the contents of the destination operand *
	 * N: set if result is < 0; cleared otherwise
	 * Z: set if result is 0; cleared otherwise
	 * V: cleared
	 * C: cleared
	 */
	opDataStruct Ry; //Struct objects
	int result, y;
	Ry = ret_operand(sopModeMask, destination); //Fill structn with destination information
	y = (int)Ry.operand;
	result = y;
	//no need to update operand
	/* update flags */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;
	if (result < 0)
		code_N = 1;
	else
		code_N = 0;
	code_V = 0;
	code_C = 0;
	//dont need to write to destination
	return;
}
//Shift & Rotate
void asl() {
	/* instruction x063DD; Destination is shifted one place to the left *
	 * LSB = 0
	 * N: set if the high-order bit of the result is set (result < 0; cleared otherwise *
	 * Z: set if result = 0; cleared otherwise
	 * V: laoded from the exclusive OR of the B_bit and C-bit (as set by the completion of the shift operation)
	 * C: loaded with the high-order bit of the destination
	 */
	opDataStruct Ry; //Struct objects
	uint16_t result, y_u;
	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information; SIN_OP_HI - 006000
	y_u = Ry.operand;
	//before we shift, we need to check if the C flag will be set
	if (y_u && 40000)    //bit 5 will be in code_C bc bit 6 is - bit
		code_C = 1;
	else
		code_C = 0;

	Ry.operand = Ry.operand << 1;
	result = y_u << 1; //result is used for flag calculation 
	/*update flags*/
	if (result == 00000)
		code_Z = 1;
	else
		code_Z = 0;
	if (result && 100000)
		code_N = 1;
	else
		code_N = 0;
	if (code_C ^ code_N)
		code_V = 1;
	else
		code_V = 0;
	write2dest(Ry);    //send out destination data to be updated in mem
	return;


}

void rol() {
	/* instruction x061DD ; rotate all the bits of the destination left one place *
	 * Bit 15 is loaded into the C-bit of the status word and the previous contents of C-bit are loaded into bit 0 of the destination *
	 *  N: set if the high order bit of the result word is set, (result < 0); cleared otherwise *
	 *  Z: set if all bits = 0; cleared otherwise *
	 *  V: loaded with XOR of n-bit and c-bit (as set by completion of rotate operation
	 *  C: loaded with high order bit of the destination
	 */
	opDataStruct Ry; //Struct objects
	int result, y;
	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = (int)Ry.operand;
	//before doing changes, lets check C_flag and highest bit
	if (code_C == 1) {
		code_C = 1;      //for next rotate
		result = y << 1; //result is used for flag calculation 
		result++;        //add one to result to include bit in C_flag
		Ry.operand = Ry.operand << 1;
		Ry.operand = Ry.operand + 1;
	}
	else
	{
		code_C = 0;      //for next rotate
		result = y << 1; //result is used for flag calculation 
		Ry.operand = Ry.operand << 1;
	}
	/* update rest of the flags */
	if (result < 0)
		code_N = 1;
	else
		code_N = 0;
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;
	if (code_N ^ code_C)
		code_V = 1;
	else
		code_V = 0;
	write2dest(Ry);    //send out destination data to be updated in mem
	return;


}
//Multiple Precision
void adc() {
	/* instruction x055DD; Adds the contents of the C-bit into the DD *
	 * this allows the carry from the addition of the low-order words to be carried into the high-order result *
	 * dest = dest + c *
	 * N: set if result < 0; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: set if (dst) was 077777 and (C) was 1; cleared otherwise
	 * C: set if (dst) was 177777 and (C) was 1; cleared otherwise
	 */
	opDataStruct Ry; //Struct objects
	uint16_t result, y;
	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = Ry.operand;
	//update flags after checking current flags and DD register 
	if (code_C == 1) {
		if (y && 177777)
			code_C = 1;
		else
			code_C = 0;
		if (y && 077777)
			code_V = 1;
		else
			code_V = 0;
		result = y + 1; //result is used for flag calculation 
		Ry.operand = Ry.operand + 1;
	}
	else
		result = y + 0; //result is used for flag calculation 
 /* update rest of flags N and Z */
	if (result && 100000)
		code_N = 1;
	else
		code_N = 0;
	if (result == 00000)
		code_Z = 1;
	else
		code_Z = 0;
	write2dest(Ry);    //send out destination data to be updated in mem
	return;
}

void sxt() {
	/* instruction 0067DD; N-bit is replicated through dest: dest = 0 if N is clear; dest = -1 if N is set
	 * N: unaffected
	 * Z: set if Nbit is clear
	 * V: clear
	 * C: unaffected
	 */
	opDataStruct Ry; //Struct objects
	int result, y;
	Ry = ret_operand(sopModeMask, destination); //Fill struct with destination information
	y = (int)Ry.operand;
	//update flags after checking current flags and DD register 
	if (code_N == 0) {
		y = 0;
		result = y;
		Ry.operand = 0;
	}
	else {
		y = -1;
		result = y;
		Ry.operand = -1;
	}
	if (code_N == 0)
		code_Z = 1;
	else
		code_Z = 0;
	code_V = 0;

	write2dest(Ry);    //send out destination data to be updated in mem
	return;

}

/*single op end */
void mov() {
	/* Move source to destination, update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int x; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations

	Ry.operand = Rx.operand; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (x == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (x < 0)
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void movB() {
	/* Move source to destination, update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int x; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations

	Ry.operand = Rx.operand; // Actual result to be used for program

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (x == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (x < 0)
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void bit() {
	/* bit test and update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int result, x, y; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations
	y = (int)Ry.operand;

	Ry.operand = Rx.operand & Ry.operand; // Actual result to be used for program
	result = x & y; //result to be used for flag calculations

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result & (1 << 15))
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	return;
}

void bitB() {
	/* bit test and update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int result, x, y; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations
	y = (int)Ry.operand;

	Ry.operand = Rx.operand & Ry.operand; // Actual result to be used for program
	result = x & y; //result to be used for flag calculations

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result & (1 << 7))
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	return;
}

void bis() {
	/* bit set and update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int result, x, y; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations
	y = (int)Ry.operand;

	Ry.operand = Rx.operand | Ry.operand; // Actual result to be used for program
	result = x | y; //result to be used for flag calculations

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result & (1 << 15))
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void bisB() {
	/* bit set and update flags as needed */

	opDataStruct Rx, Ry; //Struct objects

	int result, x, y; //Temp variable

	Rx = ret_operand(ssddModeMask1, source); //Fill struct with source information
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with destination information

	x = (int)Rx.operand; //to be used for flag calculations
	y = (int)Ry.operand;

	Ry.operand = Rx.operand | Ry.operand; // Actual result to be used for program
	result = x | y; //result to be used for flag calculations

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (result == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (result & (1 << 7))
		code_N = 1;
	else
		code_N = 0;

	code_V = 0;

	write2dest(Ry); //Send out destination data to be updated in memory
	return;
}

void divide() {
	/* Divide and update flags as needed */

	opDataStruct Rx; //Struct objects

	int16_t x, y, rem; //Temp variables


	Rx = ret_operand(rssModeMask, source); //Fill struct with source information

	y = (int16_t)registerName;

	rem = y % 2; // Figure out if register is even or odd
	if (rem == 0) {
		//If even, result is saved in R, R+1
		if (y == 0) {
			R0 = R0 / Rx.operand;
			R1 = R0;
			x = (int16_t)R0;
		}
		if (y == 2) {
			R2 = R2 / Rx.operand;
			R3 = R2;
			x = (int16_t)R2;
		}
		if (y == 4) {
			R4 = R4 / Rx.operand;
			R5 = R4;
			x = (int16_t)R4;
		}
	}
	else {
		//If odd, result is saved in R
		if (y == 1) {
			R1 = R1 / Rx.operand;
			x = (int16_t)R1;
		}
		if (y == 3) {
			R3 = R3 / Rx.operand;
			x = (int16_t)R3;
		}
		if (y == 5) {
			R5 = R5 / Rx.operand;
			x = (int16_t)R5;
		}
		else
			std::cout << "ERR: not a valid Register to divide\n";
	}

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (x == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if (x < 0)
		code_N = 1;
	else
		code_N = 0;

	if (Rx.operand == 0)
		code_C = 1;
	else
		code_C = 0;

	if (Rx.operand == 0)
		code_V = 1;
	else if (abs(Rx.operand) < y)
		code_V = 1;
	else
		code_V = 0;

	return;
}

void ashc() {
	/* Arithmetic shift combined and update flags as needed */

	opDataStruct Rx; //Struct objects

	int16_t x, y, v, a; //Temp variables
	uint16_t c;

	Rx = ret_operand(rssModeMask, source); //Fill struct with source information
	x = (int16_t)Rx.operand;
	y = (int16_t)registerName;

	if (y == 0) {
		if (x < 0) { //If operand is negative then right shift
			v = R0 & (1 << 15); //To be used for flag calculation
			R0 = R0 >> Rx.operand;
			a = R0 & (1 << 15); //To be used for flag calculation
			c = R1 & 1; //To be used for flag calculation
			R1 = R1 >> Rx.operand;
		}
		else { //if operand is positive then left shift
			c = R0 & (1 << 15);
			R0 = R0 << Rx.operand;
			R1 = R1 << Rx.operand;
		}
	}
	if (y == 2) {
		if (x < 0) {
			v = R2 & (1 << 15);
			R2 = R2 >> Rx.operand;
			a = R2 & (1 << 15);
			c = R3 & 1;
			R3 = R3 >> Rx.operand;
		}
		else {
			c = R2 & (1 << 15);
			R2 = R2 << Rx.operand;
			R3 = R3 << Rx.operand;
		}
	}
	if (y == 4) {
		if (x < 0) {
			v = R4 & (1 << 15);
			R4 = R4 >> Rx.operand;
			a = R4 & (1 << 15);
			c = R5 & 1;
			R5 = R5 >> Rx.operand;
		}
		else {
			c = R4 & (1 << 15);
			R4 = R4 << Rx.operand;
			R5 = R5 << Rx.operand;
		}
	}
	if (y == 1) {
		if (x < 0) {
			v = R1 & (1 << 15);
			R1 = R1 >> Rx.operand;
			a = R1 & (1 << 15);
			c = R2 & 1;
			R2 = R2 >> Rx.operand;
		}
		else {
			c = R1 & (1 << 15);
			R1 = R1 << Rx.operand;
			R2 = R2 << Rx.operand;
		}
	}
	if (y == 3) {
		if (x < 0) {
			v = R3 & (1 << 15);
			R3 = R3 >> Rx.operand;
			a = R3 & (1 << 15);
			c = R4 & 1;
			R4 = R4 >> Rx.operand;
		}
		else {
			c = R3 & (1 << 15);
			R3 = R3 << Rx.operand;
			R4 = R4 << Rx.operand;
		}
	}
	if (y == 5) {
		if (x < 0) {
			v = R5 & (1 << 15);
			R5 = R5 >> Rx.operand;
			a = R5 & (1 << 15);
			c = R6 & 1;
			R6 = R6 >> Rx.operand;
		}
		else {
			c = R5 & (1 << 15);
			R5 = R5 << Rx.operand;
			R6 = R6 << Rx.operand;
		}
	}

	/* Update flags, if condition is true set flag
		otherwise clear flag */
	if (R0 == 0 && R1 == 0)
		code_Z = 1;
	else if (R1 == 0 && R2 == 0)
		code_Z = 1;
	else if (R2 == 0 && R3 == 0)
		code_Z = 1;
	else if (R3 == 0 && R4 == 0)
		code_Z = 1;
	else if (R4 == 0 && R5 == 0)
		code_Z = 1;
	else if (R5 == 0 && R6 == 0)
		code_Z = 1;
	else
		code_Z = 0;

	if ((int16_t)R0 < 0 && (int16_t)R1 < 0)
		code_N = 1;
	else if ((int16_t)R1 < 0 && (int16_t)R2 < 0)
		code_N = 1;
	else if ((int16_t)R2 < 0 && (int16_t)R3 < 0)
		code_N = 1;
	else if ((int16_t)R3 < 0 && (int16_t)R4 < 0)
		code_N = 1;
	else if ((int16_t)R4 < 0 && (int16_t)R5 < 0)
		code_N = 1;
	else if ((int16_t)R5 < 0 && (int16_t)R6 < 0)
		code_N = 1;
	else
		code_N = 0;

	if (c)
		code_C = 1;
	else
		code_C = 0;

	if (v ^ a)
		code_V = 1;
	else
		code_V = 0;

	return;
}


//General
void cmp() {
	/* instruction X2SSDD
	 * compares the SS and DD operands and sets codition codes *
	 * SS and DD are unaffected *
	 * N: set if result < 0; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: set if arithmetic overflow; cleared otherwise
	 * C: cleared if carry is true from MSB of result; set otherwise
	 */
	opDataStruct Rx, Ry; //Struct objects
	uint16_t  result, x, y; //temp variables
	uint16_t temp;
	Rx = ret_operand(ssddModeMask1, source); //Fill Struct with source info
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with dest info

	x = Rx.operand; // to be used for flag calc
	y = Ry.operand;
	if (!((x && 100000) ^ (y && 100000)))   //if opposite signs
	{
		temp = x + ~y + 1;
		if ((temp && 100000) ^ (x && 100000))   //if these are the same
			code_V = 1;
		else
			code_V = 0;
		temp = temp - 1;
		if (temp && 40000)   //checks if carry from most MSB
			code_C = 0;
		else
			code_C = 1;
	}
	result = x + ~y + 1; // or SS - DD 
	/* update flags */
	if (result && 100000)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 00000)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	// write2dest(Ry); //send out destination data to be updated in memory
	//nothing in DD is changed
	return;

}

void sub() {
	/* 16SSDD
	 * Substract the SS from DD and leaves result in DD. original contents of DD are lost. SS not affected. in double precision arithmetic the c-bit, when set indicates a 'borrow'
	 * N: set if result < 0; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: set if there was a arithmetic overflow as result of operation.
	 * So if operands were opposite signes and the sign of the source was the same as the sign of the result; cleared otherwise
	 * this is like cmp function except we actually write to memory DD *
	 * and we complement x instead of y *
	 */
	opDataStruct Rx, Ry; //Struct objects
	uint16_t result, x, y; //temp variables
	uint16_t temp;
	Rx = ret_operand(ssddModeMask1, source); //Fill Struct with source info
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with dest info

	x = Rx.operand; // to be used for flag calc
	y = Ry.operand;
	if (!((x && 100000) ^ (y && 100000)))   //if opposite signs
	{
		temp = ~x + y + 1;
		if ((temp && 100000) ^ (x && 100000))   //if these are the same
			code_V = 1;
		else
			code_V = 0;
		temp = temp - 1;
		if (temp && 70000)   //checks if carry from most MSB
			code_C = 0;
		else
			code_C = 1;
	}
	result = ~x + y + 1; // or SS - DD 
	Ry.operand = ~Rx.operand + Ry.operand + 1;
	/* update flags */
	if (result && 100000)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 00000)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	write2dest(Ry); //send out destination data to be updated in memory
	return;
}
//Logical
void bic() {
	/*X4SSDD; operation DD = ~SS && DD
	 * Clears each bit in the destination that corresponds to a set bit in the source. The original contents of the destination are lost. SS unaffectedi *
	 * N: set if high order of result set; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: cleared
	 * C: not affected
	 */
	opDataStruct Rx, Ry; //Struct objects
	uint16_t result, x, y; //temp variables
	Rx = ret_operand(ssddModeMask1, source); //Fill Struct with source info
	Ry = ret_operand(ssddModeMask2, destination); //Fill struct with dest info

	x = Rx.operand; // to be used for flag calc
	y = Ry.operand;
	Ry.operand = ~Rx.operand && Ry.operand;
	result = ~x & y;
	/* update flags */
	if (result && 100000)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 0)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	code_V = 0;
	//code_C not affected; 

	write2dest(Ry); //send out destination data to be updated in memory
	return;

}
//Register
void mul() {
	/*instruction 070RSS; The contents of the DD register and SS taken as 2's complement int are multiplied and stored in the DD register and succeding register *
	 * N: set if product is < 0; cleared otherwise
	 * Z: set if product is = 0; cleared otherwise
	 * V: cleared
	 * C; set if the result is less than -2^15 or greater than or = 2^15-1
	 */
	opDataStruct Rx, Ry; //Struct objects
	int result, x, y; //temp variables
	Rx = ret_operand(000, registerName); //Fill Struct with source info
	Ry = ret_operand(rssModeMask, source); //Fill struct with dest info

	x = (int)Rx.operand; // to be used for flag calc
	y = (int)Ry.operand;
	result = x * y;
	Ry.operand = Rx.operand * Ry.operand;

	/* update flags */
	if (result < 0)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 0)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	code_V = 0;
	if (result < -32768)
		code_C = 1;
	else if (result > 32767)
		code_C = 1;
	else
		code_C = 0;
	write2dest(Ry); //send out destination data to be updated in memory
	return;
}

void ash() {
	/* 072RSS; The contents of Register are shifted right or left the # of time specied by SS
	 * N: set if result < 0; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: set if sign of the register changed during shift; cleared otherwise
	 * C: loaded from last bit shifted out of register
	 */
	opDataStruct Rx, Ry; //Struct objects
	int result, x, y; //temp variables
	Rx = ret_operand(000, registerName); //Fill Struct with source info
	//registerName - R - a pointer to register, might need to dereference it 
	//destination = source = XX = SS
	//if register used is even, store result in Register, Register+1
	//else if register used is odd store result in Register
	Ry = ret_operand(rssModeMask, source); //Fill struct with dest info

	x = (int)Rx.operand; // to be used for flag calc
	y = (int)Ry.operand;
	if (y > 28672) //implied that bit will be shifted out
		code_C = 1;
	else
		code_C = 0;
	if (x < 0) //negative shift, right shift
	{
		result = y >> x;
		Ry.operand = Ry.operand >> Rx.operand;
	}
	else
	{
		result = y << x;
		Ry.operand = Ry.operand << Rx.operand;
	}

	/* update flags */
	if (result < 0)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 0)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;

	if (result < 0 && y > 0)
		code_V = 1;
	else if (result > 0 && y < 0)
		code_V = 1;
	else
		code_V = 0;

	write2dest(Ry); //send out destination data to be updated in memory
	return;
}
void xorfun() {
	/* instruction 074RDD *
	 * exclusive OR of the register and destination operand is stored in the destination. contents of register are unaffected. *
	 * N: set if result < 0; cleared otherwise
	 * Z: set if result = 0; cleared otherwise
	 * V: cleared
	 * C: unaffected *
	 */
	opDataStruct Rx, Ry; //Struct objects
	uint16_t result, x, y; //temp variables
	Rx = ret_operand(000, registerName); //Fill Struct with source info
	//registerName - R - a pointer to register, might need to dereference it 
	//destination = source = XX = SS
	//if register used is even, store result in Register, Register+1
	//else if register used is odd store result in Register
	Ry = ret_operand(rssModeMask, source); //Fill struct with dest info
	x = Rx.operand;
	y = Ry.operand;
	Ry.operand = Rx.operand ^ Ry.operand;
	result = x ^ y;
	/* update flags */
	if (result && 100000)    //if result has a set N if largest bit is 1
		code_N = 1;
	else
		code_N = 0;
	if (result == 00000)         //if result is 0
		code_Z = 1;
	else
		code_Z = 0;
	code_V = 0;
	//code_C unaffected
	write2dest(Ry);
	return;
}
/*double op end*/
/*functions end*/
bool is_SSDD_DoubleOperand(uint16_t word) {
	uint16_t checkDouble;
	bool status = false;

	checkDouble = word & octStringToNum(DUB_OP_GEN_LOG_MASK);
	if (checkDouble >= octStringToNum(DUB_OP_GEN_LOG_LOW)) {
		if (checkDouble <= octStringToNum(DUB_OP_GEN_LOG_HI)) {
			status = true; //It's a double operand general or logical instruction!
		}
	}

	if (status) {
		source = ((octStringToNum(DUB_OP_GEN_LOG_SS_MASK) & word) >> (6));
		destination = octStringToNum(DUB_OP_GEN_LOG_DD_MASK) & word;
		ssddModeMask1 = (octStringToNum(SOURCE_MODE_MASK) & word) >> 9;
		ssddModeMask2 = (octStringToNum(DEST_MODE_MASK) & word) >> 3;
		cout << "SSDD Double Operand Instruction: " << word << endl;
		checkDouble = checkDouble >> 12;
		if (checkDouble == 1) {
			cout << "MOV" << endl;
			if (word >> 15)
				movB();
			else
				mov();
		}
		else if (checkDouble == 2) {
			cout << "CMP" << endl;
			cmp();
		}
		else if (checkDouble == 3) {
			cout << "BIT" << endl;
			if (word >> 15)
				bitB();
			else
				bit();
		}
		else if (checkDouble == 4) {
			cout << "BIC" << endl;
			bic();
		}
		else if (checkDouble == 5) {
			cout << "BIS" << endl;
			if (word >> 15)
				bisB();
			else
				bis();
		}
		else if (checkDouble == 6) {
			cout << "ADD/SUB" << endl;
			if (word >> 15)
				sub();
			else
				add();
		}
		else
		{
			//Not one of the valid SSDD double operands
		}


	}
	return status;
}

bool is_RXX_DoubleOperand(uint16_t word) {
	uint16_t checkDouble;
	bool status = false;

	checkDouble = word & octStringToNum(DUB_OP_REG_MASK);
	if (checkDouble >= octStringToNum(DUB_OP_REG_LOW)) {
		if (checkDouble <= octStringToNum(DUB_OP_REG_HI)) {
			status = true; //It's a double operand general or logical instruction!
		}
	}

	if (status) {
		registerName = ((octStringToNum(DUB_OP_REG_R_MASK) & word) >> (3 * 2));
		destination = octStringToNum(DUB_OP_REG_XX_MASK) & word;
		rssModeMask = (octStringToNum(REG_MODE_MASK) & word) >> 3;
		//cout<<"RXX Double operand Instruction: "<<word<<endl;
		checkDouble = checkDouble >> 9;
		if (checkDouble == 56) {
			cout << "MUL" << endl;
			mul();
		}
		else if (checkDouble == 57) {
			cout << "DIV" << endl;
			divide();
		}
		else if (checkDouble == 58) {
			cout << "ASH" << endl;
			ash();
		}
		else if (checkDouble == 59) {
			cout << "ASHC" << endl;
			ashc();
		}
		else if (checkDouble == 60) {
			cout << "XOR" << endl;
			xorfun();
		}
		else if (checkDouble == 63) {
			cout << "SOB" << endl;
			sob(word & 0x003F);
		}
		else {
			//not valid RXX double operand
		}
	}

	return status;
}

bool isSingleOperand(uint16_t word) {
	uint16_t checkSingle;
	bool status = false;
	checkSingle = word & octStringToNum(SIN_OP_MASK);
	if (checkSingle >= octStringToNum(SIN_OP_LOW)) {
		if (checkSingle <= octStringToNum(SIN_OP_HI)) {
			status = true; //It's a single operand instruction!
		}
	}

	checkSingle = word & octStringToNum(SIN_OP_SWAB_MASK);
	if (checkSingle == octStringToNum(SIN_OP_SWAB)) {
		status = true; //It's a single operand SWAB instruction
	}

	if (status) {
		destination = octStringToNum(SIN_OP_DD_MASK) & word;
		sopModeMask = (octStringToNum(SIN_MODE_MASK)& word) >> 3;

		cout << "Single Operand Instruction Instruction: " << word << endl;
		checkSingle = checkSingle >> 6;
		if (checkSingle == 3) {
			cout << "SWAB" << endl;
			swapb();
		}
		else if (checkSingle == 40) {
			if (word >> 15)
			{
				cout << "CLRB" << endl;


				clrB();
			}
			else {
				cout << "CLR" << endl;
				clr();
			}
		}
		else if (checkSingle == 41) {
			cout << "COM" << endl;
			com();
		}
		else if (checkSingle == 42) {
			cout << "INC" << endl;
			if (word >> 15)

				incB();
			else
				inc();
		}
		else if (checkSingle == 43) {
			cout << "DEC" << endl;
			dec();
		}
		else if (checkSingle == 44) {
			cout << "NEG" << endl;
			if (word >> 15)

				negB();
			else
				neg();
		}
		else if (checkSingle == 45) {
			cout << "ADC" << endl;
			adc();
		}
		else if (checkSingle == 46) {
			cout << "SBC" << endl;
			if (word >> 15)

				sbcB();
			else
				sbc();
		}
		else if (checkSingle == 47) {
			cout << "TST" << endl;
			tst();
		}
		else if (checkSingle == 48) {
			cout << "ROR" << endl;
			if (word >> 15)

				rorB();
			else
				ror();
		}
		else if (checkSingle == 49) {
			cout << "ROL" << endl;
			rol();
		}
		else if (checkSingle == 50) {
			if (word >> 15) {
				cout << "ASRB" << endl;


				asrB();
			}
			else {
				asr();
				cout << "ASR" << endl;
			}
		}
		else if (checkSingle == 51) {
			cout << "ASL" << endl;
			asl();
		}
		else if (checkSingle == 52) {
			cout << "MARK" << endl;
			mark(word & 0x003F);
		}
		else if (checkSingle == 55) {
			cout << "SXT" << endl;
			sxt();
		}
		else
			cout << "Default" << endl;

	}

	return status;
}

bool isCondOperand(uint16_t word) {
	uint16_t checkCond;
	bool status = false;
	checkCond = word & octStringToNum(COND_MASK);
	if (checkCond >= octStringToNum(COND_MASK_L)) {
		if (checkCond <= octStringToNum(COND_MASK_H)) {
			status = true;
		}
	}
	if (status) {
		offset = octStringToNum(COND_MASK_OFFSET)& word; //Multiply by two and add to PC
		cout << "Conditional Operand Instruction: " << word << endl;
		checkCond = checkCond >> 6;
		if (checkCond == 0) {
			if (word >> 15)
			{
				cout << "BPL" << endl;
				br_BPL(offset);
			}
		}
		else if (checkCond == 4) {
			cout << "BR/BMI" << endl;
			if (word >> 15)
				br_minus(offset);
			else
				br_unc(offset);
		}
		else if (checkCond == 8) {
			cout << "BNE/BHI" << endl;
			if (word >> 15)
				br_BHI(offset);
			else
				br_BNE(offset);
		}
		else if (checkCond == 12) {
			cout << "BEQ/BLOS" << endl;
			if (word >> 15)
				br_lwsam(offset);
			else
				br_eq(offset);
		}
		else if (checkCond == 16) {
			cout << "BGE/BVC" << endl;
			if (word >> 15)
				br_BVC(offset);
			else
				br_BGE(offset);
		}
		else if (checkCond == 20) {
			cout << "BLT/BVS" << endl;
			if (word >> 15)
				br_overfl(offset);
			else
				br_ltz(offset);
		}
		else if (checkCond == 24) {
			cout << "BGT/BCC/BHIS" << endl;
			if (word >> 15)
				br_BHIS(offset);
			else
				br_BGT(offset);
		}
		else if (checkCond == 28) {
			cout << "BLE/BCS/BLO" << endl;
			if (word >> 15)
				br_lw(offset);
			else
				br_lteq(offset);
		}
		else {
			cout << "Default" << endl;
		}
	}
	return status;

}
//#define DEBUG_decodeAndExecute 1
void decodeAndExecute(uint16_t word) {
	uint16_t address = 0;
	uint16_t opcode = MASK_OP_UPPER_2 & word;

#ifdef DEBUG_decodeAndExecute

	prLabel("Opcode", opcode);

#endif // DEBUG_decodeAndExecute

	byte = false;
	if (word == 0) {
		halt = true;
	}
	else if (is_SSDD_DoubleOperand(word)) {
		//Source and destination values have been set
		//addressing modes has been set
	}
	else if (is_RXX_DoubleOperand(word)) {
		//Register and source/destination have been set
		//addressing mode has been set
	}
	else if (isSingleOperand(word)) {
		//Destination has been set
		//addressing mode has been set
	}
	else if (isCondOperand(word)) {
		//offset has been set
	}

}


#define DEBUG_Instructions
#define DEBUG_Flags			   

int main(void) {
	bool start = false;
	uint16_t word;
	bool getAddress = true;
	string startingAddress;

	fillMemory();
	if (starFlag) {
		cout << "Starting address found in file" << endl;
	}

	pr("Enter a 6 digit octal starting address or type n to auto-detect from ASCII file\n");
	while (getAddress) {
		cin >> startingAddress;

		if (startingAddress.find("n") != string::npos) {
			pr("Getting starting address from file");
			getAddress = false;
		}
		else if (startingAddress.length() > 6) {
			pr("ERROR - address is too long, please re-enter");

		}
		else {
			startAddressFound = true;
			starFlag++;
			R7 = octStringToNum(startingAddress);
			getAddress = false;
		}
	}
	uint16_t instruction;
	pr("********************Main loop***********************");
	while (!halt) {
		instruction = 0;
		instruction = memory[R7] << 8;
		instruction += memory[R7 + 1];

		instructionCount += 1;

#ifdef DEBUG_Instructions
		cout << "\n\n*****New Instruction: " << oct << instruction << " *****\n";
		prLabel("PC", R7);
		prLabel("Instruction Count", instructionCount);
		prLabel("R0", R0);
		prLabel("R1", R1);
		prLabel("R2", R2);
		prLabel("R3", R3);
		prLabel("R4", R4);
		prLabel("R5", R5);
		prLabel("R6", R6);
		prLabel("R7", R7);
		prLabel("C", code_C);
		prLabel("N", code_N);
		prLabel("V", code_V);
		prLabel("Z", code_Z);
		cout << "\n" << endl;
#endif
		if (instruction >> 15)
		{
			byte = true;
		}

		fetch();
		decodeAndExecute(instruction);

		//if branch, skip the increment
		incPC();
		skipPC = 0;
		byte = false;
		branch = false;
	}

	pr("********************HALT Instruction Detected***********************");
#ifdef DEBUG_Flags
	prLabel("C", code_C);
	prLabel("N", code_N);
	prLabel("V", code_V);
	prLabel("Z", code_Z);
#endif


	inputFile.close();
	traceFile.close();


	pr("\nEnter anything to exit");
	string userEnter;
	cin >> userEnter;

	return 0;
}