#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define UBOGON 0xfffd		/* replacement character */
#define BIT8 0x80		/* 8th bit mask */
/* 한글 낱자의 수 */
#define SINGLENUM				51

typedef struct SIZED_TEXT
{
    unsigned char *data;          /* text */
    unsigned long size;           /* size of text in octets */
} SIZEDTEXT;

struct utf8_eucparam
{
    unsigned int base_ku : 8;	/* base row */
    unsigned int base_ten : 8;	/* base column */
    unsigned int max_ku : 8;	/* maximum row */
    unsigned int max_ten : 8;	/* maximum column */
    void *tab;			/* conversion table */
};

/* Character set conversion tables */
#include "ksc_5601.c"		/* Korean */

/* EUC parameters */
static const struct utf8_eucparam ksc_param =
{
    BASE_KSC5601_KU,BASE_KSC5601_TEN,MAX_KSC5601_KU,MAX_KSC5601_TEN,(void *) ksc5601tab
};


/* UTF-8 size and conversion routines from UCS-2 values.  This will need to
 * be changed if UTF-16 data (surrogate pairs) are ever an issue.
 */

#define UTF8_SIZE(c) ((c & 0xff80) ? ((c & 0xf800) ? 3 : 2) : 1)

#define UTF8_PUT(b,c) {					\
  if (c & 0xff80) {		/* non-ASCII? */	\
    if (c & 0xf800) {		/* three byte code */	\
      *b++ = 0xe0 | (c >> 12);				\
      *b++ = 0x80 | ((c >> 6) & 0x3f);			\
    }							\
    else *b++ = 0xc0 | ((c >> 6) & 0x3f);		\
    *b++ = 0x80 | (c & 0x3f); 				\
  }							\
  else *b++ = c;					\
}

typedef int (*fptr)(const void*, const void*);

static void euckr_to_utf8 (SIZEDTEXT *in, SIZEDTEXT *out, void *tab);
static void utf8_to_euckr (SIZEDTEXT *in, SIZEDTEXT *out);
static unsigned short _KssmToKS(unsigned short  code);
static int _CodeCmp(unsigned short *p, unsigned short *q);


/*
int test001 (char *input)
{
  SIZEDTEXT in, out, out2;
  unsigned char *decoded;
  FILE *f;

  in.data = input; in.size = strlen(input);

  euckr_to_utf8(&in, &out, (void *)&ksc_param);
  printf("[%s]\n", out.data);

  f= fopen("./output.txt", "a");
  fwrite(out.data, out.size, 1, f);
  fclose(f);


  decoded = out.data;

  utf8_to_euckr(&out, &out2);
  printf("[%s]\n", out2.data);

  free(out2.data);
  free(decoded);

  return (0);
}

int test002(char *in)
{
	char instr[80], outstr[80];

	strcpy(instr, in);

	euc2utf(outstr, instr);

	printf("--%s--%s--\n", instr, outstr);

	return 0;
}

int ___main (int argc, char *argv[])
{
  if( argc != 2 ) {
    printf("usage : e2u [euc_kr string] then print and output.txt file\n");
    exit(1);
  }else{
	  test001(argv[1]);
  };

	test002(argv[1]);

  	return 0;
}
*/


int euc2utf(char *instr, char *outstr)
{
    SIZEDTEXT in, out;

    if ( instr==NULL ) return 0;

    in.data = (unsigned char *) instr;
    in.size = (unsigned long) strlen(instr);

    out.data = (unsigned char *) malloc (out.size + 1);

    euckr_to_utf8(&in, &out, (void *)&ksc_param);
    sprintf(outstr, "%s", out.data);

    free(out.data);
    return 0;
}



int utf2euc(char *instr, char *outstr)
{
    SIZEDTEXT in, out;

    if ( instr == NULL ) return 0;

    in.data = (unsigned char *) instr;
    in.size = (unsigned long) strlen(instr);

    out.data = (unsigned char *) malloc(in.size + 1);

    utf8_to_euckr(&in, &out);
    sprintf(outstr, "%s", out.data);

    free(out.data);

    return 0;
}


/* Convert ASCII + double-byte sized text to UTF-8
 * Accepts: source sized text
 *	    pointer to return sized text
 *	    conversion table
 */

static void euckr_to_utf8 (SIZEDTEXT *in, SIZEDTEXT *out, void *tab)
{
    unsigned long i;
    unsigned char *s;
    unsigned int c,c1,ku,ten;
    struct utf8_eucparam *p1 = (struct utf8_eucparam *) tab;
    unsigned short *t1 = (unsigned short *) p1->tab;

    for (out->size = i = 0; i < in->size; out->size += UTF8_SIZE (c))
        if ((c = in->data[i++]) & BIT8)
            c = ((i < in->size) && (c1 = in->data[i++]) &&
                 ((ku = c - p1->base_ku) < p1->max_ku) &&
                 ((ten = c1 - p1->base_ten) < p1->max_ten)) ?
                t1[(ku*p1->max_ten) + ten] : UBOGON;

    s = out->data ;

    /* add : kcshin : 2003/0217 */
    memset(s, '\0', out->size+1);
    /**/

    for (i = 0; i < in->size;)
    {
        if ((c = in->data[i++]) & BIT8)
            c = ((i < in->size) && (c1 = in->data[i++]) &&
                 ((ku = c - p1->base_ku) < p1->max_ku) &&
                 ((ten = c1 - p1->base_ten) < p1->max_ten)) ?
                t1[(ku*p1->max_ten) + ten] : UBOGON;
        UTF8_PUT (s,c)		/* convert Unicode to UTF-8 */
    };
}

static void utf8_to_euckr (SIZEDTEXT *in, SIZEDTEXT *out)
{
    int  stringlen, newlen;
    unsigned char *s;
    unsigned short c;
    short cho, chung, chong;


    newlen = 0;
    s = out->data ;

    /* add : kcshin : 2003/0217 */
    memset(s, '\0', in->size + 1);
    /**/
    stringlen = in->size;

    while (stringlen > 0)
    {

        c = *(in->data);

        if (c >= 0xf0)
        {

            c = ((in->data[0]&7)<<18) | ((in->data[1]&63)<<12) |
                ((in->data[2]&63)<<6) | (in->data[3]&63);
            in->data += 4;
            stringlen -= 4;

        }
        else if (c >= 0xe0)
        {

            c = ((in->data[0]&63)<<12) | ((in->data[1]&63)<<6) | (in->data[2]&63);

            in->data += 3;
            stringlen -= 3;

        }
        else if (c >= 0xc0)
        {

            c = ((in->data[0]&63)<<6) | (in->data[1]&63);

            in->data += 2;
            stringlen -= 2;

        }
        else
        {
            in->data++;
            stringlen--;
        }

        /*******************************/
        /* 한글코드 영역 0xAC00~0xD7A3 */
        /*******************************/

        if ( (c >= 0xAC00) && (c <= 0xD7A3))
        {

            c -= 0xAC00;

            cho   = ((c/28)/21)%19 + 2;

            chung = (c/28)%21 + 3;

            if (chung > 19)
            {
                chung += 6;
            }
            else if (chung > 13)
            {
                chung += 4;
            }
            else if (chung > 7)
            {
                chung += 2;
            }

            if ((chong = c%28 + 1) > 17)
                chong++;

            c = (cho << 10) | (chung << 5) | chong | 0x8000;

            c =  _KssmToKS(c);

            s[newlen++] = (unsigned char)(c>>8);
            s[newlen++] = (unsigned char)(c&0x00ff);

        }
        else
        {
            s[newlen++] = c;
        }
    }

    if (newlen)
    {
        s[newlen] = '\0';
        return;
    }
    else
    {
        return;
    }
}

static unsigned short _KssmToKS(unsigned short  code)
{
    short index;
    unsigned short   *p;

    if ((p = (unsigned short *)bsearch(&code, KStbl,
                                       sizeof(KStbl) / sizeof(unsigned short),
                                       sizeof(unsigned short), (fptr)_CodeCmp)) != NULL)
    {
        index = p - KStbl;
        return (((index / 94) + 0xb0) << 8) + (index % 94) + 0xa1;
    }

    for (index = 0; index < SINGLENUM; index++)
    {
        if (Single[index] == code) return (0xa4a1 + index);
    }

    return 0x2000; /*  space in reverse order */
}

static int _CodeCmp(unsigned short *p, unsigned short *q)
{

    unsigned short *pp = (unsigned short *)p;
    unsigned short *qq = (unsigned short *)q;

    return (*pp - *qq);
}
