# 8051 Assembler

## Overview
The choice of this project topic was to get a very good insight into coding professional software that addresses all user cases and product requirements. The main and foremost requirement of the project was to develop an assembler that creates a .hex file after decoding a .asm file (containing Intel MCS-8051 instructions).

## Disclaimer
As I upload this project to GitHub along with my other older projects, it's important to note that I am presenting it in its original form, without modifications. Therefore, the concepts and coding style reflect my skills and knowledge as of 2021, rather than my current capabilities. Please overlook any technical or logical errors in the project.

## Capabilities
The assembler has the following capabilities:

### Instruction Decoding
To convert an assembly language file into a hex code file, the key parameters considered are:
- **Arithmetic Instructions:** ADD, SUBB, MUL, etc.
- **Data Transfer / Data Copy Instructions:** MOV, etc.
- **Logical Instructions:** ORL, ANL, etc.
- **Jump Instructions:** SJMP, etc.
- **Call Instructions:** LCALL, etc.
- And all other supported instructions by MCS-8051

### Addressing Modes
Decoding addressing modes, categorized as:
- **Immediate Addressing:** e.g., ADD A, #56H
- **Direct Addressing:** e.g., ADD A, 56H
- **Register Addressing:** e.g., MOV A, R5
- **Register Indirect:** e.g., MOV A, @R0
- And all other supported addressing modes by MCS-8051

### Hex File Creation
Creating a .hex file (in Intel Hex 16 format) in accordance with decoded opcodes from the Intel 8051 instruction set manual.

### Label Handling
- Getting addresses of lines defined by labels and replacing operands containing a label with the corresponding absolute address or offset.
  
### Assembler Directives
Handling assembler directives, including:
- **ORG:** Specifies the address of the instructions that follow.
- **END:** Specifies the end of the ASM file.
- **EQU:** Defines a constant without occupying a memory location.
- **DB:** Define byte directive.

## Development Process
The development process involved defining and coding various independent modules:

### Line and Command Segment Separator
This module reads individual characters from the ASM file and separates lines by detecting the ‘\n’ character. It then breaks each line into its individual segments or parts:
- **Label:** By detecting the ‘:’ character
- **Instruction:** e.g., “MOV”
- **Operands:** Max. 3 operands
- **Comments:** By detecting the ‘;’ character

### Label Decoder
This module identifies the address of the current line where a label occurs and stores the label and its respective line address in a global array. It handles:
- Calculating the address of the current line by adding the number of bytes of each instruction.
- Replacing label occurrences in short jumps with a signed offset.
- Replacing label occurrences in long jumps or calls with the 16-bit absolute address.
- Returning an error if the jump offset overflows the limit of the instruction.

### Support Functions
The program includes various support functions to convert string data to decimal data and vice versa, such as:
- `long StrHexToDec (char *HexStr)`: Converts string hex data (“35H”) to decimal.
- `long StrBinToDec (char *HexStr)`: Converts string binary data (“1010B”) to decimal.
- `long StrDecToDec (char *HexStr)`: Converts string decimal data (“78”) to decimal.

These functions are crucial for converting data forms in the instruction decode module.

### Instruction Decoder
This module processes each instruction, extracts operands and their addressing modes, and returns the hex code. It includes:
- A global structure consisting of the instruction and a function pointer for each instruction.
- Common functions for similar operand patterns and hex code units.

### Write To Hex File
This module stores decoded hex bytes in a buffer and handles:
- Storing bytes into the output hex file at the 16-bit beginning memory address of the current record/buffer.
- Updating the address value for the next record/buffer.
- Handling ORG directives to store and clear the buffer for new byte locations.

### Assembler Directive Decode
This module checks for assembler directives in the current line/command and calls appropriate functions. When the END directive occurs, it stops reading from the ASM file and writes the buffer bytes into the hex file followed by an end-of-file record (“:00000001FF”).

## Usage
- **Build the Assembler:**
- **Run the Assembler:** The assembler expects two command-line arguments: the source .asm file and the output .hex file.
- **Format:** "./assembler <source.asm> <output.hex>"
- **Example:** If you have a source file named program.asm and you want to generate an output file named program.hex, you would run: ./assembler program.asm program.hex
- **Error Handling:** If incorrect arguments are provided, the assembler will display the following usage message:
Wrong arguments provided
Expected Usage: ./assembler "<source.asm>" "<output.hex>"
