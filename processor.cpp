#include <bits/stdc++.h>
using namespace std;

class processor
{
	short int data[256], R[16], reg_busy[16];
	int inst[128];
	int IR;
	short int PC, PC_valid, validate;
	short int opcode, op1, op2, op3;
	int L1;
	short int A, B, out;
	short int MAR, MBR, reg, val1;
	short int dest, val, temp;
	int stalled;
	int jump;
	int c_stalls;
	int instCount, stalls, cycles;
	int iCount[16];
	float CPI;
	
	bool active[4];

public:
	processor()
	{
		PC = 0;
		PC_valid = 1;
		validate = 0;
	}

	void initialise()
	{
		ifstream fin1;
		ifstream fin2;
		ifstream fin3;
		fin1.open("ICache.txt");
		fin2.open("DCache.txt");
		fin3.open("RF.txt");

		for(int i=0; i<128; i++)
		{
			fin1 >> hex >> inst[i];	
		}
		for(int i=0; i<256; i++)
		{
			fin2 >> hex >> data[i];
		}
		for(int i=0; i<16; i++)
		{
			fin3 >> hex >> R[i];
			reg_busy[i] = 0;
		}

		PC = 0;
		IR = -1;
		opcode = -1;
		MAR = MBR = -1;
		reg = -1;
		dest = -1;
		val1 = 0;
		instCount = 0;
		stalls = 0; 
		cycles = 0;
		jump = 0;
		stalled = 0;
		c_stalls = 0;
	}

	void fetch()
	{
		if(stalled > 0)
		{
			stalls++;
			return;
		}
		if(PC_valid)
		{
			if(PC != -1)
			{
				IR = inst[PC];
				PC += 1;
				instCount++;
			}
			else IR = -1;
		}	
		else
		{ 
			IR = -1;
		}
	}
	void decode()
	{
		if(IR == -1)
		{
			opcode = -1;
			return;
		}

		opcode = IR/(pow(2, 12));
		cout << opcode << " ";
		if(opcode == 10)	//jump
		{
			L1 = (IR&0x0FF0)/16;
			
			if(L1 >= 128)
			{
				//cout << "here" << " ";
				L1 -= pow(2, 8);
			}
			//cout << L1 << " :L1 ";
			PC_valid = 0;
			validate = 0;
		}
		else if(opcode == 11)	//BEQZ
		{
			op1 = (IR&0x0F00)/(16*16);
			L1 = IR&0x00FF;
			if(L1 > 128)
			{
				L1 -=pow(2, 16);
			}
			if(reg_busy[op1]==1)
			{
				stalled = 1;
				opcode = -1;
				return;
			} 
			else
			{
				stalled = 0;
			}
			PC_valid = 0;
			validate = 0;
		}	
		else if(opcode == 15)	//terminate
		{
			PC = -1;
		}
		else if(opcode == 8)	//load
		{
			op1 = (IR&0x0F00)/(16*16);
			op2 = (IR&0x00F0)/16;
			op3 = (IR&0x000F);

			if(reg_busy[op2] == 1)
			{
				stalled = 1;
			}
			else
			{
				stalled = 0;
			}
		}	
		else if(opcode == 9)	//store
		{
			op1 = (IR&0x0F00)/(16*16);
			op2 = (IR&0x00F0)/16;
			op3 = (IR&0x000F);

			if(reg_busy[op1] == 1 || reg_busy[op2] == 1)
			{
				stalled = 1;
			}
			else
			{
				stalled = 0;
			}
		}
		else if(opcode == 3)
		{
			op1 = (IR&0x0F00)/(16*16);
			op2 = (IR&0x00F0)/16;
			op3 = (IR&0x000F);
			if(reg_busy[op1] == 1)
			{
				stalled = 1;
			}
			else
			{
				stalled = 0;
			}
		}
		else
		{
			op1 = (IR&0x0F00)/(16*16);
			op2 = (IR&0x00F0)/16;
			op3 = (IR&0x000F);
			if(reg_busy[op2] == 1 || reg_busy[op3] == 1)
			{
				stalled = 1;	
			}
			else
			{
				stalled = 0;
			}
		}

		if(stalled  > 0)
		{
			opcode = -1;
			return;
		}
	}

	void alu()
	{
		if(opcode == -1)
		{
			MAR = -1;
			MBR = -1;
			reg = -1;
		}
		if(opcode == 0) //add
		{
			A = R[op2];
			B = R[op3];
			out = A+B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 1) //sub
		{
			A = R[op2];
			B = R[op3];
			out = A-B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 2) //mul
		{
			A = R[op2];
			B = R[op3];
			out = A*B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 3) //INC
		{
			A = R[op1];
			out = A+1;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 4) //AND
		{
			A = R[op2];
			B = R[op3];
			out = A&B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 5) //OR
		{
			A = R[op2];
			B = R[op3];
			out = A|B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 6) //NOT
		{
			A = R[op2];	
			out = ~A;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 7) //XOR
		{
			A = R[op2];
			B = R[op3];
			out = A^B;

			MAR = -1;
			MBR = -1;
			reg = op1;
			val1 = out;
			reg_busy[op1] = 1;
		}
		else if(opcode == 8) //load
		{
			A = R[op2];
			B = op3;
			out = A+B;

			MAR = out;
			MBR = -1;
			reg = op1;
			reg_busy[op1] = 1;
		}
		else if(opcode == 9) //store
		{
			A = R[op2];
			B = op3;
			out = A+B;

			MAR = -1;
			MBR = out;
			reg = op1;
			val1 = -1;
		}
		else if(opcode == 10) //JMP
		{	
			

			PC = PC - 1 + L1;
			//cout << "PC=" << PC << " ";
			MAR = -1;
			MBR = -1;
			reg = -1;
			//PC_valid = 1;
			validate = 1;
			c_stalls += 2;
		}
		else if(opcode == 11) //BEQZ
		{
			if(R[op1] == 0)
			{
				PC = PC-1 + L1;
			}
			//cout << "PC=" << PC << " ";
			c_stalls += 2;
			MAR = -1;
			MBR = -1;
			reg = -1;
			validate = 1;
			//PC_valid = 1;
		}
	}

	void mem()
	{
		if(MAR != -1)
		{
			val = data[MAR];
			dest = reg;
		}
		else if(MBR != -1)
		{
			data[MBR] = R[reg];
			dest = -1;
		}
		else
		{
			dest = reg;
			val = val1;
		}
	}

	void wb()
	{
		if(dest!= -1)
		{
			R[dest] = val;
		}
		temp = dest;
	}

	void process()
	{
		while(opcode != 15)
		{	
			cycles++;
			wb();
			mem();
			alu();
			decode();
			fetch();
			if(temp!= -1)
				reg_busy[temp] = 0;
			if(validate)
				PC_valid = 1;

		}
		wb();
		mem();
		wb();

		cycles+= 3;
	}
	void output()
	{
		ofstream f1, f2, f3;
		//f1.open("DCache.txt");

		// for(int i=0; i<256; i+=4)
		// {
		// 	f1 << hex << data[i] <<endl;
		// }
		cout << endl << endl;
		for(int i=0; i<16; i++)
		{
			cout << R[i] << " ";
		}
		//cout << endl;
		f2.open("Output.txt");

		//f2 << 
		f2 << "Number of cycles: " << cycles << endl;
		f2 << "Number of Data Stalls: "<< stalls+c_stalls << endl;
		f2 << "Number of control stalls: " << c_stalls << endl;
		//cout << CPI << endl;

	}
};

int main()
{
	processor i;
	i.initialise();
	i.process();
	i.output();
	return 0;
}