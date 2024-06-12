#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef int bool;

#define TRUE         1
#define FALSE        0

#define NO_OP_OFFSET        100

#define ERR_INIT            -1
#define NO_ERR              1
#define ERR_REG             500 // If Operand == R8, R9, @R2, @R3...., @(Any other letter than 'R')
#define ERR_HEX             502 // If Hex Num has more than 4 Hexadecimal Digits (eg A5F1AH) OR If a Hex Digit is greater than F
#define ERR_DEC             503 // If Dec Num is more than 65535
#define ERR_VAL_OV          504 // If The Immediate Data or Address overflows 0xFF
#define ERR_WRNG_PARAM      505 // If Wrong Parameter are used
#define ERR_ADDR_OV         506 // If Hex File Address overflows 0xFFFF
#define ERR_OFFSET_OV       507 // If the Jump Instruction Offset is greater than FF
#define ERR_WRNG_INST       508 // An error which occurs if the instruction is not a recognized MCS 8051 instruction. 

long Decode2ndOpRegisters(int OpOffset);
long DecodeINC_DEC(int OpOffset);
long DecodeLongJMPInsts(int NoUse);
long UpdateAddress(int NoUse);
long DecodeMOV(int OpOffSet);
long DecodeOnlyACCInst(int OpOffset);
long DecodeBitJMPInsts(int OpOffset);
long DecodeJMPInsts(int OpOffset);
long DecodeNOP(int NoUse);
long DecodeRET(int NoUse);
long DecodeRETI(int NoUse);
long DecodeMOVC(int NoUse);

int StrBinToDec(char *StrNum);
int DecodeADD_ADDCold();
int GetDecData(char *Data);
int RegisterDecode(char *Op);
int StrHexToDec(char *StrNum);
int StrDecToDec(char *StrNum);
int GetNoOfBytesOfInst();
int DecodeInst();
int GetHexCode(char *HexCode);
int GetOffset(char *LabelAddr, char *CrntLineAddr);

void ReadASMFile(char *FilePath);
void ClearArr(char *ClearArr, int ClearLen);
void CopyArr(char *SourceArr, char *DestArr, int CpyLen);
void SplitInstOprnd(char *String, int StrLen);
void GetLabel(char *String, int StrLen);
void GetOperand2(char *String, int StrLen);
void DecToHexStr(int Data, char *HexNo);
void CopyBufInHexFile();
void UpdateHexFile(char *HexCode);
void AddInAddr(char *Addr, int NumToAdd);
void UpdateLabel();
void DeleteArr(char *ClearArr, int ClearLen);

bool StrEq(char *RefStr, char *ChkStr, int ChkLen);
bool isASMDir();
bool isOpDirectAddr(char *Op);

struct ComndInfo
{
    char Label[20];
    char Inst[10];
    char Op[2][20];
    char Addr[5];
};

struct DecodeInfo
{
    char *Inst;
    int OpOffset;
    long (*fptr)(int);
};

struct Global
{
    int ComndIndx;
    int Error;

    char HexFileCrntAddr[5];
    char HexFileBuf[32];
    int  HexFileBufAddress;
    FILE *HEXFilePointer;

    char CrntLabelAddr[5];
    int CrntLabelNo;
}Glb;

struct LabelAddr
{
    char Name[20];
    char AddrVal[5];
};

const struct DecodeInfo stDecode[] = 
{
    {"INC"  , 0x00,          &DecodeINC_DEC},
    {"DEC"  , 0x10,          &DecodeINC_DEC},
    {"ADD"  , 0x20,          &Decode2ndOpRegisters},
    {"ADDC" , 0x30,          &Decode2ndOpRegisters},
    {"ORL"  , 0x40,          &Decode2ndOpRegisters},
    {"ANL"  , 0x50,          &Decode2ndOpRegisters},
    {"XRL"  , 0x60,          &Decode2ndOpRegisters},
    {"SUBB" , 0x90,          &Decode2ndOpRegisters},
    {"XCH"  , 0xC0,          &Decode2ndOpRegisters},
    {"MOV"  , NO_OP_OFFSET,  &DecodeMOV},
    {"ORG"  , NO_OP_OFFSET,  &UpdateAddress},
    {"LJMP" , NO_OP_OFFSET,  &DecodeLongJMPInsts},
    {"LCALL", NO_OP_OFFSET,  &DecodeLongJMPInsts},

    {"JBC" , 0x10,  &DecodeBitJMPInsts},
    {"JB"  , 0x20,  &DecodeBitJMPInsts},
    {"JNB" , 0x30,  &DecodeBitJMPInsts},
    {"JC"  , 0x40,  &DecodeBitJMPInsts},
    {"JNC" , 0x50,  &DecodeBitJMPInsts},
    {"JZ"  , 0x60,  &DecodeBitJMPInsts},
    {"JNZ" , 0x70,  &DecodeBitJMPInsts},
    {"JMP" , 0x73,  &DecodeBitJMPInsts},
    {"SJMP", 0x80,  &DecodeBitJMPInsts},

    {"RR"  , 0x03,  &DecodeOnlyACCInst},
    {"RRC" , 0x13,  &DecodeOnlyACCInst},
    {"RL"  , 0x23,  &DecodeOnlyACCInst},
    {"RLC" , 0x33,  &DecodeOnlyACCInst},
    {"RLC" , 0x33,  &DecodeOnlyACCInst},
    {"SWAP", 0xC4,  &DecodeOnlyACCInst},
    {"DA"  , 0xD4,  &DecodeOnlyACCInst},

    {"NOP" , NO_OP_OFFSET,  &DecodeNOP},
    {"RET" , NO_OP_OFFSET,  &DecodeRET},
    {"RETI", NO_OP_OFFSET,  &DecodeRETI},
    
    {"MOVC", NO_OP_OFFSET,  &DecodeMOVC}
};

struct ComndInfo Command[500];
struct LabelAddr Label[500];

int main(int argc, char *argv[])
{
    if (argc != 3) 
    {
        printf("\nWrong arguments provided\nExpected Usage: %s \"<source.asm>\" \"<output.hex>\"\n\n", argv[0]);
        return EXIT_FAILURE;
    }

    Glb.HEXFilePointer = fopen(argv[2], "wt");
    ReadASMFile(argv[1]);

    DecodeInst();
    CopyBufInHexFile();
    fputs(":00000001FF", Glb.HEXFilePointer);
    
    fclose(Glb.HEXFilePointer);

    int stop = 0;
}

void TestFunc(char *Var)
{
    int Len = strlen(Var);
    char Ch1 = Var[0];
    char Ch2 = Var[1];
    char Ch3 = Var[2];
    int Stop = 9;
}

void ReadASMFile(char *FilePath)
{
    FILE *ASMFilePointer;
    ASMFilePointer = fopen(FilePath , "rt");

    Glb.ComndIndx = 0;
    Glb.CrntLabelNo = 0;

    char CrntChar, PrevChar;
    char Buf[20];
    int BufIndx = 0;
    bool isSingleOprnd = TRUE;

    ClearArr(Buf, 20);

    CrntChar = fgetc(ASMFilePointer);
    Buf[BufIndx++] = CrntChar;

    while(TRUE)
    {
        if(CrntChar == ':')
        {
            Buf[BufIndx - 1] = ' ';
            GetLabel(Buf, 20);
            ClearArr(Buf, 20);
            BufIndx = 0;
        }
        else if(CrntChar == ',')
        {
            Buf[BufIndx - 1] = ' ';
            SplitInstOprnd(Buf, 20);
            ClearArr(Buf, 20);
            BufIndx = 0;
            isSingleOprnd = FALSE;
        }
        else if((CrntChar == '\n') || (CrntChar == EOF))
        {
            if(PrevChar == '\n')
            {
                ClearArr(Buf, 20);
                BufIndx = 0;
            }
            else
            {
                Glb.Error = ERR_INIT;
                Buf[BufIndx - 1] = ' ';
                
                if(isSingleOprnd == TRUE)
                    SplitInstOprnd(Buf, 20);
                else
                    GetOperand2(Buf, 20);

                if(!isASMDir())
                {
                    CopyArr(Glb.CrntLabelAddr, Command[Glb.ComndIndx].Addr, strlen(Glb.CrntLabelAddr));
                    UpdateLabel();
                }
                else
                {
                    if(!(StrEq("END", Command[Glb.ComndIndx].Inst, 3)))
                    {
                        char HexCode[7];

                        GetHexCode(HexCode);

                        int Stop = 9;
                    }
                }

                ClearArr(Buf, 20);
                BufIndx = 0;
                Glb.ComndIndx++;
                isSingleOprnd = TRUE;

                if((CrntChar == EOF) || (StrEq("END", Command[Glb.ComndIndx].Inst, 3)))
                    break;

                Glb.Error = ERR_INIT;
            }
        }
        
        PrevChar = CrntChar;
        CrntChar = fgetc(ASMFilePointer);
        Buf[BufIndx++] = CrntChar;
    }
    
    fclose(ASMFilePointer);

    printf("\n\n");
}

int DecodeInst()
{
    int NoOfInst = Glb.ComndIndx;

    for(Glb.ComndIndx = 0; Glb.ComndIndx < NoOfInst; Glb.ComndIndx++)
    {
        if(isASMDir() == FALSE)
        {
            for(int LabelNo = 0; LabelNo < Glb.CrntLabelNo; LabelNo++)
            {
                for(int Op = 0; Op < 2; Op++)
                {
                    if(StrEq(Label[LabelNo].Name, Command[Glb.ComndIndx].Op[Op], strlen(Label[LabelNo].Name)))
                    {
                        char HexCode[3];
                        int Offset = GetOffset(Label[LabelNo].AddrVal, Command[Glb.ComndIndx + 1].Addr);

                        if(Offset == ERR_INIT)
                            return ERR_INIT;

                        DecToHexStr(Offset, HexCode);
                        
                        DeleteArr(Command[Glb.ComndIndx].Op[Op], 20);

                        Command[Glb.ComndIndx].Op[Op][0] = HexCode[0];
                        Command[Glb.ComndIndx].Op[Op][1] = HexCode[1];
                        Command[Glb.ComndIndx].Op[Op][2] = 'H';
                        Command[Glb.ComndIndx].Op[Op][3] = '\0';
                    }   
                }
            }

            char HexCode[7];

            if(GetHexCode(HexCode) == ERR_INIT)
                return ERR_WRNG_INST;

            UpdateHexFile(HexCode);
        }
    }

    return NO_ERR;
}

long DecodeOnlyACCInst(int OpOffset)
{
    if(StrEq("A\0", Command[Glb.ComndIndx].Op[0], 2) && (Command[Glb.ComndIndx].Op[1][0] == '\0'))
        return OpOffset;

    Glb.Error = ERR_WRNG_PARAM;
    return ERR_INIT;
}

long DecodeNOP(int NoUse)
{
    return 0;
}

long DecodeRET(int NoUse)
{
    return 0x22;
}

long DecodeRETI(int NoUse)
{
    return 0x32;
}


long DecodeBitJMPInsts(int OpOffset)
{
    if(StrEq("JMP", Command[Glb.ComndIndx].Inst, 3))
    {
        if(StrEq("@A+DPTR", Command[Glb.ComndIndx].Op[0], 7) && (strlen(Command[Glb.ComndIndx].Op[1]) == 0))
            return OpOffset;
    }
    else if(strlen(Command[Glb.ComndIndx].Op[1]) == 0)
    {
        return (OpOffset << 8) + GetDecData(Command[Glb.ComndIndx].Op[0]);
    }
    else if(isOpDirectAddr(Command[Glb.ComndIndx].Op[0]) && isOpDirectAddr(Command[Glb.ComndIndx].Op[1]))
    {
        int BitAddr = GetDecData(Command[Glb.ComndIndx].Op[0]);
        if(BitAddr > 0xFF)
        {
            Glb.Error = ERR_VAL_OV;
            return ERR_INIT;
        }

        return (OpOffset << 16) + (BitAddr << 8) + GetDecData(Command[Glb.ComndIndx].Op[1]);
    }

    Glb.Error = ERR_WRNG_PARAM;
    return ERR_INIT;
}

long DecodeLongJMPInsts(int NoUse)
{
    if(StrEq("LJMP", Command[Glb.ComndIndx].Inst, 4))
        return 0x020000 + GetDecData(Command[Glb.ComndIndx].Op[0]);
    
    if(StrEq("LCALL", Command[Glb.ComndIndx].Inst, 5))
        return 0x120000 + GetDecData(Command[Glb.ComndIndx].Op[0]);
}

long DecodeDJNZ(int NoUse)
{

}

long DecodeMOV(int OpOffSet)
{
    int RegDecodeCode = 0;
    int OtherOp = 0;

    for(int Op = 0; Op < 2; Op++)
    { 
        if((Command[Glb.ComndIndx].Op[Op][0] == 'R') || (Command[Glb.ComndIndx].Op[Op][0] == '@'))
        {
            RegDecodeCode = RegisterDecode(Command[Glb.ComndIndx].Op[Op]);
            OtherOp = 1 - Op;
        }
    }

    if(RegDecodeCode != 0)
    {
        if((Command[Glb.ComndIndx].Op[OtherOp][0] == 'A') && (Command[Glb.ComndIndx].Op[OtherOp][1] == '\0'))
        {
            if(OtherOp == 0)       
                return 0xE0 + RegDecodeCode;
            else
                return 0xF0 + RegDecodeCode;
        }
        else if(isOpDirectAddr(Command[Glb.ComndIndx].Op[OtherOp]) == TRUE)
        {
            if(StrEq("DJNZ", Command[Glb.ComndIndx].Inst, 4) && (OtherOp == 1))
                return (0xD0 << 8) + GetDecData(Command[Glb.ComndIndx].Op[1]);
            else if(OtherOp == 0)
                return ((0x80 + RegDecodeCode) << 8) + GetDecData(Command[Glb.ComndIndx].Op[OtherOp]);
            else
                return ((0xA0 + RegDecodeCode) << 8) + GetDecData(Command[Glb.ComndIndx].Op[OtherOp]);\
        }
        else if(Command[Glb.ComndIndx].Op[OtherOp][0] == '#')
            return ((0x70 + RegDecodeCode) << 8) + GetDecData(&Command[Glb.ComndIndx].Op[OtherOp][1]);
    }

    if(StrEq("A\0", Command[Glb.ComndIndx].Op[0], 2) && isOpDirectAddr(Command[Glb.ComndIndx].Op[1]))
        return 0xE500 + GetDecData(Command[Glb.ComndIndx].Op[1]);
    else if(StrEq("A\0", Command[Glb.ComndIndx].Op[1], 2) && isOpDirectAddr(Command[Glb.ComndIndx].Op[0]))
        return 0xF500 + GetDecData(Command[Glb.ComndIndx].Op[0]);

    // for(int Op = 0; Op < 2; Op++)
    // {
    //     if(StrEq("A\0", Command[Glb.ComndIndx].Op[Op], 2) && isOpDirectAddr(Command[Glb.ComndIndx].Op[1 - Op]))
    //         return 0xE500 + GetDecData(Command[Glb.ComndIndx].Op[1 - Op]);
    // }
    
    if(isOpDirectAddr(Command[Glb.ComndIndx].Op[0]) && isOpDirectAddr(Command[Glb.ComndIndx].Op[1]))
        return 0x850000 + (GetDecData(Command[Glb.ComndIndx].Op[1]) << 8) + GetDecData(Command[Glb.ComndIndx].Op[0]);

    if(isOpDirectAddr(Command[Glb.ComndIndx].Op[0]) && (Command[Glb.ComndIndx].Op[1][0] == '#'))
        return 0x750000 + (GetDecData(Command[Glb.ComndIndx].Op[0]) << 8) + GetDecData(&Command[Glb.ComndIndx].Op[1][1]);

    if(StrEq("A\0", Command[Glb.ComndIndx].Op[0], 2) && (Command[Glb.ComndIndx].Op[1][0] == '#'))
        return 0x7400 + GetDecData(&Command[Glb.ComndIndx].Op[1][1]);

    Glb.Error = ERR_WRNG_PARAM;
    return ERR_INIT;
}

int GetOffset(char *LabelAddr, char *CrntLineAddr)
{
    int Offset = StrHexToDec(LabelAddr) - StrHexToDec(CrntLineAddr);

    if((-128 <= Offset) && (Offset <= 127))
    {
        if(Offset < 0) 
            Offset += 256;

        return Offset;
    }
    else
    {
        Glb.Error = ERR_OFFSET_OV;
        return ERR_INIT;
    }
}

void UpdateLabel()
{
    if(strlen(Command[Glb.ComndIndx].Label) != 0)
    {
        CopyArr(Command[Glb.ComndIndx].Label, Label[Glb.CrntLabelNo].Name, strlen(Command[Glb.ComndIndx].Label));
        CopyArr(Glb.CrntLabelAddr, Label[Glb.CrntLabelNo].AddrVal, strlen(Glb.CrntLabelAddr));
        Glb.CrntLabelNo++;
    }
    AddInAddr(Glb.CrntLabelAddr, GetNoOfBytesOfInst());
}

int GetNoOfBytesOfInst()
{
    if((StrEq("NOP", Command[Glb.ComndIndx].Inst, 3) == TRUE) || (StrEq("RET", Command[Glb.ComndIndx].Inst, 3) == TRUE) || (StrEq("RETI", Command[Glb.ComndIndx].Inst, 4) == TRUE) || (StrEq("MUL", Command[Glb.ComndIndx].Inst, 3) == TRUE) || (StrEq("DIV", Command[Glb.ComndIndx].Inst, 3) == TRUE))
        return 1;
    else if((StrEq("SJMP", Command[Glb.ComndIndx].Inst, 4) == TRUE) || (StrEq("AJMP", Command[Glb.ComndIndx].Inst, 4) == TRUE) || (StrEq("ACALL", Command[Glb.ComndIndx].Inst, 5) == TRUE))
        return 2;
    else if((StrEq("CJNE", Command[Glb.ComndIndx].Inst, 4) == TRUE) || (StrEq("LCALL", Command[Glb.ComndIndx].Inst, 5) == TRUE) || (StrEq("LJMP", Command[Glb.ComndIndx].Inst, 4) == TRUE))
        return 3;
    else if(Command[Glb.ComndIndx].Op[1][0] == '/')
        return 2;
    else if((StrEq("INC", Command[Glb.ComndIndx].Inst, 3) == TRUE) && (StrEq("DPTR", Command[Glb.ComndIndx].Op[0], 4) == TRUE))
        return 1;

    int NoOfBytes = 1;

    for(int Op = 0; Op < 2; Op++)
    {
        if((strlen(Command[Glb.ComndIndx].Op[Op]) == 0) || (Command[Glb.ComndIndx].Op[Op][0] == '@'))
            continue;

        if((strlen(Command[Glb.ComndIndx].Op[Op]) == 1))
            if((Command[Glb.ComndIndx].Op[Op][0] == 'C') || (Command[Glb.ComndIndx].Op[Op][0] == 'A'))
                continue;

        if((strlen(Command[Glb.ComndIndx].Op[Op]) == 2) && (Command[Glb.ComndIndx].Op[Op][0] == 'R'))
            continue;

        NoOfBytes++;
    }
    return NoOfBytes;
}

bool isASMDir()
{
    if(StrEq("ORG", Command[Glb.ComndIndx].Inst, 3) || StrEq("BIT", Command[Glb.ComndIndx].Inst, 3) || StrEq("EQU", Command[Glb.ComndIndx].Inst, 3) || StrEq("END", Command[Glb.ComndIndx].Inst, 3))
        return TRUE;

    return FALSE;
}

void UpdateHexFile(char *HexCode)
{
    int HexCodeIndx = 0;

    for(int Indx = 0; Indx < strlen(HexCode); Indx++)
    {
        if(Glb.HexFileBufAddress > 31)
        {
            CopyBufInHexFile();
            Glb.HexFileBufAddress = 0;

        }
        
        Glb.HexFileBuf[Glb.HexFileBufAddress] = HexCode[HexCodeIndx];
        HexCodeIndx++;
        Glb.HexFileBufAddress++;
    }
}

void AddInAddr(char *Addr, int NumToAdd)
{
    int DecCode = StrHexToDec(Addr) + NumToAdd;

    char HexBuf[3];
    
    DecToHexStr((DecCode >> 8 & 0x00FF), HexBuf);
    Addr[0] = HexBuf[0];
    Addr[1] = HexBuf[1];

    DecToHexStr(DecCode & 0x00FF, HexBuf);
    Addr[2] = HexBuf[0];
    Addr[3] = HexBuf[1];

    Addr[4] = 0;

    // Addr[3] += NumToAdd;

    // if(Addr[3] > '9')
    //     Addr[3] += 7;

    // if(Addr[3] > 'F')
    // {
    //     Addr[2] = Addr[2] + 1;
    //     Addr[3] = Addr[3] - 'F';

    //     if(Addr[2] > 'F')
    //     {
    //         Addr[1] = Addr[1] + 1;
    //         Addr[2] = Addr[2] - 'F';

    //         if(Addr[1] > 'F')
    //         {
    //             Addr[0] = Addr[0] + 1;
    //             Addr[1] = Addr[1] - 'F';

    //             if(Addr[0] > 'F')
    //                 Glb.Error = ERR_ADDR_OV;
    //         }
    //     }
    // }
}

void CopyBufInHexFile()
{
    Glb.HexFileCrntAddr[4] = '\0';

    char ByteCnt[3], AddrBuf[3];
    DecToHexStr(Glb.HexFileBufAddress / 2, ByteCnt);

    fputs(":", Glb.HEXFilePointer);
    fputs(ByteCnt, Glb.HEXFilePointer);
    fputs(Glb.HexFileCrntAddr, Glb.HEXFilePointer);
    fputs("00", Glb.HEXFilePointer);

    CopyArr(Glb.HexFileCrntAddr, AddrBuf, 2); AddrBuf[2] = '\0';
    int Sum = StrHexToDec(ByteCnt) + StrHexToDec(AddrBuf);
    
    CopyArr(&Glb.HexFileCrntAddr[2], AddrBuf, 2);
    Sum += StrHexToDec(AddrBuf);

    char TempStore[3];
    int TempStoreIndx = 0;
    TempStore[2] = '\0';

    for(int HexFileBufIndx = 0; HexFileBufIndx < Glb.HexFileBufAddress; HexFileBufIndx++)
    {
        if(TempStoreIndx > 1)
        {
            TempStoreIndx = 0;
            Sum += StrHexToDec(TempStore);
        }
        TempStore[TempStoreIndx] = Glb.HexFileBuf[HexFileBufIndx];
        TempStoreIndx++;

        fputc(Glb.HexFileBuf[HexFileBufIndx], Glb.HEXFilePointer);
    }
    
    Sum += StrHexToDec(TempStore);
    char HexNo[3];
    DecToHexStr(((~(Sum & 0x00FF)) & 0x00FF) + 1, HexNo);

    fputs(HexNo, Glb.HEXFilePointer);
    fputc('\n', Glb.HEXFilePointer);

    AddInAddr(Glb.HexFileCrntAddr, Glb.HexFileBufAddress / 2);
}

long UpdateAddress(int NoUse)
{
    if(Glb.ComndIndx != 0)
    {
        int StAddr = GetDecData(Command[Glb.ComndIndx].Op[0]);
        CopyBufInHexFile();

        Glb.HexFileBufAddress = 0;
    }

    CopyArr(Command[Glb.ComndIndx].Op[0], Glb.HexFileCrntAddr, 5);
    CopyArr(Command[Glb.ComndIndx].Op[0], Glb.CrntLabelAddr, 4);

    return 0;
}

int GetHexCode(char *HexCode)
{
    int Len = sizeof(stDecode) / sizeof(struct DecodeInfo);
    long DecCode = 0;
    bool InstNotRecognised = TRUE;

    for(int InstIndx = 0; InstIndx < Len; InstIndx++)
    {
        if(StrEq(stDecode[InstIndx].Inst, Command[Glb.ComndIndx].Inst, strlen(Command[Glb.ComndIndx].Inst)) == TRUE)
        {
            DecCode = (*stDecode[InstIndx].fptr)(stDecode[InstIndx].OpOffset);
            InstNotRecognised = FALSE;
            break;
        }
    }

    if(InstNotRecognised)
    {
        Glb.Error = ERR_WRNG_INST;
        return ERR_INIT;
    }

    char HexBuf[3];

    if(DecCode > 0xFFFF)
    {
        DecToHexStr((DecCode >> 16) & 0x0000FF, HexBuf);
        HexCode[0] = HexBuf[0];
        HexCode[1] = HexBuf[1];

        DecToHexStr((DecCode >> 8) & 0x0000FF, HexBuf);
        HexCode[2] = HexBuf[0];
        HexCode[3] = HexBuf[1];

        DecToHexStr(DecCode & 0x0000FF, HexBuf);
        HexCode[4] = HexBuf[0];
        HexCode[5] = HexBuf[1];

        HexCode[6] = 0;
        return NO_ERR;
    }
    else if(DecCode > 0xFF)
    {
        DecToHexStr(DecCode >> 8, HexBuf);
        HexCode[0] = HexBuf[0];
        HexCode[1] = HexBuf[1];

        DecToHexStr(DecCode & 0x00FF, HexBuf);
        HexCode[2] = HexBuf[0];
        HexCode[3] = HexBuf[1];

        HexCode[4] = 0;
        return NO_ERR;
    }
    else
    {
        DecToHexStr(DecCode, HexBuf);
        HexCode[0] = HexBuf[0];
        HexCode[1] = HexBuf[1];

        HexCode[2] = 0;
        return NO_ERR;
    }

    return ERR_INIT;
}

void GlobalInit()
{
    Glb.ComndIndx = 0;
    Glb.Error = 0;
}

bool isOpDirectAddr(char *Op)
{
    if((('0' <= Op[0]) && (Op[0] <= '9')) || ('A' <= Op[0]) && (Op[0] <= 'F'))
        return TRUE;

    return FALSE;
}

long DecodeMOVC(int NoUse)
{
    if(StrEq("A\0", Command[Glb.ComndIndx].Op[0], 2))
    {
        if(StrEq("@A+PC", Command[Glb.ComndIndx].Op[1], 5))
            return 0x83;
        else if(StrEq("@A+DPTR", Command[Glb.ComndIndx].Op[1], 7))
            return 0x93;
    }

    Glb.Error = ERR_WRNG_PARAM;
    return ERR_INIT;
}

long Decode2ndOpRegisters(int OpOffset)
{
    int DataOrAddrs = 0;

    if((Command[Glb.ComndIndx].Op[0][0] == 'A') && (Command[Glb.ComndIndx].Op[0][1] == '\0'))
    {
        if((Command[Glb.ComndIndx].Op[1][0] == 'R') || (Command[Glb.ComndIndx].Op[1][0] == '@'))
            return OpOffset + RegisterDecode(Command[Glb.ComndIndx].Op[1]);
        else if(Command[Glb.ComndIndx].Op[1][0] == '#')
        {
            if(StrEq("XCH", Command[Glb.ComndIndx].Inst, 3) == TRUE)
            {
                Glb.Error = ERR_WRNG_PARAM;
                return ERR_INIT;
            }

            DataOrAddrs = GetDecData(&Command[Glb.ComndIndx].Op[1][1]);
            OpOffset += 4;
        }
        else if(isOpDirectAddr(Command[Glb.ComndIndx].Op[1]))
        {
            DataOrAddrs = GetDecData(Command[Glb.ComndIndx].Op[1]);
            OpOffset += 5;
        }
        else 
        {
            Glb.Error = ERR_WRNG_PARAM;
            return ERR_INIT;
        }    

        if(Glb.Error != ERR_INIT)
            return ERR_INIT;
        else if(DataOrAddrs > 0xFF)
        {
            Glb.Error = ERR_VAL_OV;
            return ERR_INIT;
        }
        else
            return (OpOffset << 8) + DataOrAddrs;
    }
    else if(StrEq("ORL", Command[Glb.ComndIndx].Inst, 3) || StrEq("XRL", Command[Glb.ComndIndx].Inst, 3) || StrEq("ANL", Command[Glb.ComndIndx].Inst, 3))
    {
        bool IsByteInst = TRUE;
        if(StrEq("C\0", Command[Glb.ComndIndx].Op[0], 2))
            IsByteInst = FALSE;

        if(IsByteInst && isOpDirectAddr(Command[Glb.ComndIndx].Op[0]))
            DataOrAddrs = GetDecData(Command[Glb.ComndIndx].Op[0]);
        else if(!IsByteInst)
        {   
            if(Command[Glb.ComndIndx].Op[1][0] == '/')
            {
                DataOrAddrs = GetDecData(&Command[Glb.ComndIndx].Op[1][1]);
                OpOffset += 0x60;
            }
            else
            {
                DataOrAddrs = GetDecData(Command[Glb.ComndIndx].Op[1]);
                OpOffset += 0x32;
            }
        }
        else
        {
            Glb.Error = ERR_WRNG_PARAM;
            return ERR_INIT;
        }
        
        if(Glb.Error != ERR_INIT)
            return ERR_INIT;
        else if(DataOrAddrs > 0xFF)
        {
            Glb.Error = ERR_VAL_OV;
            return ERR_INIT;
        }

        if(IsByteInst)
        {
            if(StrEq("A\0", Command[Glb.ComndIndx].Op[1], 2))
                return ((OpOffset + 2) << 8) + DataOrAddrs;
            else if(Command[Glb.ComndIndx].Op[1][0] == '#')
            {
                int ImmedData = GetDecData(&Command[Glb.ComndIndx].Op[1][1]);
                
                if(Glb.Error != ERR_INIT)
                    return ERR_INIT;
                else if(ImmedData > 0xFF)
                {
                    Glb.Error = ERR_VAL_OV;
                    return ERR_INIT;
                }

                return ((OpOffset + 3) << 16) + (DataOrAddrs << 8) + ImmedData;
            }   
        }
        else
            (OpOffset << 8) + DataOrAddrs;
    }
    
    Glb.Error = ERR_INIT;
    return ERR_INIT;
}

long DecodeINC_DEC(int OpOffset)
{
    if(Command[Glb.ComndIndx].Op[0][0] == 'A')
        return OpOffset + 0x04;
    else if((Command[Glb.ComndIndx].Op[0][0] == 'R') || (Command[Glb.ComndIndx].Op[0][0] == '@'))
        return OpOffset + RegisterDecode(Command[Glb.ComndIndx].Op[0]);
    else if((Command[Glb.ComndIndx].Op[0][0] != '#') && (Command[Glb.ComndIndx].Op[0][0] != '@'))
    {
        int Address = GetDecData(Command[Glb.ComndIndx].Op[1]);
        OpOffset += 5;

        if(Address != ERR_INIT)
            return ERR_INIT;
        else if(Address > 0xFF)
        {
            Glb.Error = ERR_VAL_OV;
            return ERR_INIT;
        }   
        else
            return (OpOffset << 8) + Address;
    }

    Glb.Error = ERR_WRNG_PARAM;
    return ERR_INIT;
}

int GetDecData(char *Data)
{
    if(Data[strlen(Data) - 1] == 'H')
        return StrHexToDec(Data);

    if(Data[strlen(Data) - 1] == 'B')
        return StrBinToDec(Data);

    return StrDecToDec(Data);
}

int StrDecToDec(char *StrNum)
{
    int DecNum = 0;
    int Len = strlen(StrNum);
    
    if(Len <= 5)
    {
        for(int CharNo = 0; CharNo < Len; CharNo++)
        {
            if(('0' <= StrNum[CharNo]) && (StrNum[CharNo] <= '9'))
                DecNum = (DecNum * 10) + (StrNum[CharNo] - '0');
            else
            {
                Glb.Error = ERR_WRNG_PARAM;
                return ERR_INIT;
            }
        }

        if(DecNum > 0xFFFF)
        {
            Glb.Error = ERR_DEC;
            return ERR_INIT;
        }
    }
    else
    {
        Glb.Error = ERR_DEC;
        return ERR_INIT;
    }

    return DecNum;
}

void DecToHexStr(int Data, char *HexNo)
{
    char HexCode[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    HexNo[0] = HexCode[(Data >> 4) & 0x0F];
    HexNo[1] = HexCode[Data & 0x0F];

    HexNo[2] = 0;
}

int StrHexToDec(char *StrNum)
{
    int HexPow[] = {1, 16, 256, 4096};

    if(strlen(StrNum) > 5)
    {
        Glb.Error = ERR_HEX;
        return ERR_INIT;
    }

    int Len = strlen(StrNum) - 2;

    if(StrNum[strlen(StrNum) - 1] != 'H')
        Len++;

    int Num = 0;
    int Val;
    int PowIndx = 0;

    for(int CharNo = Len; CharNo >= 0; CharNo--)
    {   
        if(('0' <= StrNum[CharNo]) && (StrNum[CharNo] <= '9'))     Val = StrNum[CharNo] - '0';
        else if(('A' <= StrNum[CharNo]) && (StrNum[CharNo] <= 'F'))     Val = StrNum[CharNo] - 'A' + 10;
        else
        {
            Glb.Error = ERR_HEX;
            return ERR_INIT;
        }

        Num += HexPow[PowIndx] * Val;
        PowIndx++;
    }

    return Num;
}

int StrBinToDec(char *StrNum)
{   
    int Len = strlen(StrNum) - 2;
    int Num = 0;
    int BitPos = 1;

    for(int CharNo = Len; CharNo >= 0; CharNo--)
    {
        if(StrNum[CharNo] == '1')
            Num |= BitPos;
        
        BitPos <<= 1;
    }

    return Num;
}

int RegisterDecode(char *Op)
{
    if((Op[0] == 'R') && (Op[2] == '\0') && ((Op[1] - 40) <= 0xF))
        return Op[1] - 40;
    else if((Op[0] == '@') && (Op[1] == 'R') && (Op[3] == '\0') && ((Op[2] - 42) < 8))
        return Op[2] - 42;

    Glb.Error = ERR_REG;
    return ERR_INIT;
}

void ClearArr(char *ClearArr, int ClearLen)
{
    for(int Indx = 0; Indx < ClearLen; Indx++)
    {
        ClearArr[Indx] = ' ';
    }
}

void DeleteArr(char *ClearArr, int ClearLen)
{
    for(int Indx = 0; Indx < ClearLen; Indx++)
    {
        ClearArr[Indx] = '\0';
    }
}

void CopyArr(char *SourceArr, char *DestArr, int CpyLen)
{
    for(int Indx = 0; Indx < CpyLen; Indx++)
        DestArr[Indx] = SourceArr[Indx];
}

void SplitInstOprnd(char *String, int StrLen)
{
    bool isCharTxt = FALSE;
    int TextHit = 1;
    int InstIndx = 0, OprndIndx = 0;

    for(int Indx = 0; Indx < StrLen; Indx++)
    {
        if((String[Indx] != ' ') && (isCharTxt == FALSE))
            isCharTxt = TRUE;

        if((String[Indx] == ' ') && (isCharTxt == TRUE))
        {    
            if(TextHit == 1)
                Command[Glb.ComndIndx].Inst[InstIndx] = '\0';
            else
                Command[Glb.ComndIndx].Op[0][OprndIndx] = '\0';

            isCharTxt = FALSE; 
            TextHit++;
        }

        if((isCharTxt == TRUE) && (TextHit == 1))
        {    
            Command[Glb.ComndIndx].Inst[InstIndx] = String[Indx];
            InstIndx++;
        }

        if((isCharTxt == TRUE) && (TextHit == 2))
        {    
            Command[Glb.ComndIndx].Op[0][OprndIndx] = String[Indx];
            OprndIndx++;
        }
    }
}

void GetLabel(char *String, int StrLen)
{
    int CpyIndx = 0;

    for(int Indx = 0; Indx < StrLen; Indx++)
    {
        if(String[Indx] != ' ')
        {
            Command[Glb.ComndIndx].Label[CpyIndx] = String[Indx];
            CpyIndx++;
        }
    }

    String[CpyIndx] = '\0';
}

void GetOperand2(char *String, int StrLen)
{
    int CpyIndx = 0;

    for(int Indx = 0; Indx < StrLen; Indx++)
    {
        if(String[Indx] != ' ')
        {
            Command[Glb.ComndIndx].Op[1][CpyIndx] = String[Indx];
            CpyIndx++;
        }
    }

    String[CpyIndx] = '\0';
}

bool StrEq(char *RefStr, char *ChkStr, int ChkLen)
{
    for(int CharIndx = 0; CharIndx < ChkLen; CharIndx++)
    {
        if(ChkStr[CharIndx] != RefStr[CharIndx])
            return FALSE;
    }

    return TRUE;
}