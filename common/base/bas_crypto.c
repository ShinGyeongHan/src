#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h> 


/* =============================================
 *      void PrintString()
 * it will print string but print '\0' as '@' and
 * '\n` as '#' and other as '$'.
 * ============================================= */
void CryptPrintString(FILE* fp, char* str, int len)
{
    int creFlag = 0, i;
    FILE* eFp;

    if (fp == NULL)
    {
        eFp = fopen("/dev/console", "w");
        fp = eFp;
        creFlag = 1;
    }

    fprintf(fp,"[%d][", len);
    for (i = 0; i < len; i++)
    {
        if (isprint(str[i]))
        {
            fprintf(fp, "%c", str[i]);
        }
        else if (str[i] == ' ')
        {
            fprintf(fp, "%c", '*');
        }
        else
        {
            fprintf(fp, "<%02x>", (unsigned char)str[i]);
        }

    }
    fprintf(fp, "]%c", '\n');

    if (creFlag)
    {
        fclose(eFp);
    }
}


static int NorByteOp(char*op1, int sLen1, char*op2, int sLen2, char*op3)
{
    int i, pos=0;

    for (i = 0; i < sLen1; i++)
    {
        op3[i] = op1[i] ^ op2[pos];
        pos ++;
        if (pos >= sLen2)
        {
            pos = 0;
        }
    }
    return 0;
}

/* =============================================
 *          int BinToHexa()
 * binary data to heax ascii.
 * ============================================= */
static int BinToHexa(char* bin, int len, char* asc)
{
    int ind=0, i;
    unsigned char val;

    for (i = 0; i < len; i++)
    {
        val = (bin[i] >> 4) & 0x0f;
        if (val > 9)
        {
            val -= 10;
            asc[ind] = val+'a';
        }
        else
        {
            asc[ind] = val+'0';
        }
        ind ++;

        val = bin[i] & 0x0f;
        if (val > 9)
        {
            val -= 10;
            asc[ind] = val+'a';
        }
        else
        {
            asc[ind] = val+'0';
        }
        ind ++;
    }
    asc[ind] = '\0';
    return ind;
}

/* =============================================
 *          int HexaToBin()
 * binary data to heax ascii.
 * ============================================= */
static int HexaToBin(char* asc, int aln, char* bin)
{
    int ind=0, loop = aln, i;
    unsigned char ch;

    for (i = 0; i < loop; i++)
    {
        if ((i % 2) == 0)
        {
            if (asc[i] < 'a')
            {
                ch = asc[i] - '0';
            }
            else
            {
                ch = 10 + (asc[i] - 'a');
            }
            bin[ind] = (ch << 4) & 0xf0;
        }
        else
        {
            if (asc[i] < 'a')
            {
                ch = asc[i] - '0';
            }
            else
            {
                ch = 10 + (asc[i] - 'a');
            }
            bin[ind] |= (ch & 0x0f);
            ind ++;
        }
    }
    return (ind);
}


int CryptEncodeKey(char* sec, char* sour, int slen, char* seed, int dlen)
{
    char encKey[512];
    char keyBuf[512];
    char resBuf[512];

    strcpy(encKey,
           "?F*|zEnd7I~J5I-,@v_o?)[-6+-%hkZ^MioRS_m,T4RX'0?85e");

    NorByteOp(encKey, slen, seed, dlen, keyBuf);
    /*
    printf("---- fuck:");
    PrintString(stdout, keyBuf, slen);
    printf("---%c,%c,%c\n", encKey[5], seed[5], encKey[5]^seed[5]);
    */
    NorByteOp(sour, slen, keyBuf, slen, resBuf);
    return BinToHexa(resBuf, slen, sec);
}



int CryptDecodeKey(char* sour, char* sec, int slen, char* seed, int dlen)
{
    char encKey[512];
    char keyBuf[512];
    char resBuf[512];

    slen = HexaToBin(sec, slen, resBuf);

    strcpy(encKey,
           "?F*|zEnd7I~J5I-,@v_o?)[-6+-%hkZ^MioRS_m,T4RX'0?85e");

    NorByteOp(encKey, slen, seed, dlen, keyBuf);
    NorByteOp(resBuf, slen, keyBuf, slen, sour);

    return slen;
}
