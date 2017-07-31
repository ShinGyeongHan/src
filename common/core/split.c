 /*/////////////////////////////////////////////////////////////////////////

리눅스의 펄을 사용해보신 분들은 split이라는 함수를 아실껍니다.
이 함수가 무척 편리하죠.

MFC나 STL등의 라이브러리에서는 Token에 관련된 함수나 클래스가 제공되지만 C에서는 strtok()같은 기본함수만 제공되잖아요.

아래는 문자열을 원하는 형태로 자르는데 사용하거든요.

이것을 c로 구현한게 있어서 소개할까합니다.

 

사용방법은 밑에 split.c , split.h를 포함시켜줍니다.
아래는 함수 이용 방법입니다.

 

char **toks;     // split에서 리턴값이 이중포인터라 이걸 받는 변수.
char *ptr = "Under the Hood for Viva!";
int num;            // split함수에서 리턴되는 toks의 갯수.

toks = split(ptr, " ", 10, &num, 0);

 

위와 같이 사용하면

    toks[0] = "Under"
    toks[1] = "the"
    toks[2] = "Hood"
    toks[3] = "for"

    toks[4] = "Viva!"

가 들어가고 num에는 갯수 5가 리턴됩니다.
두번재 인자에는 공백으로 문자열을 자른것이고 원하는 문자열로 자를 수 있습니다.

 

아시는 분은 아시겠지만 이 함수는 리눅스 IDS인 스노트에 있는 소스입니다.
저는 HP UNIX C로 텔넷 패킷 파싱할때 위 함수를 이용했는데 쓸만했습니다.
유용하게 사용하세요. ^^ (그럼 소스 들어갑니다.)
/////////////////////////////////////////////////////////////////////////*/

 

 
 

///////////////
/// split.c  ///
///////////////

#include "split.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
 
char **split(char *str, char *sep, int max_strs, int *toks, char meta)
{
   char **retstr;    /* 2D array which is returned to caller */
   char *idx;         /* index pointer into str */
   char *end;        /* ptr to end of str */
   char *sep_end; /* ptr to end of seperator string */
   char *sep_idx;  /* index ptr into seperator string */
   int len = 0;        /* length of current token string */
   int curr_str = 0;  /* current index into the 2D return array */
   char last_char = (char)0xFF;

#ifdef DEBUG
   printf("[*] Splitting string: %s\n", str);
   printf("curr_str = %d\n", curr_str);
#endif

   /* find the ends of the respective passed strings so our while() 
      loops know where to stop */
   sep_end = sep + strlen(sep);
   end = str + strlen(str);

   /* remove trailing whitespace */
   while(isspace((int)*(end-1)) && ((end-1) >= str)) *(--end) = '\0'; /* -1 because of NULL */

   /* set our indexing pointers */
   sep_idx = sep;
   idx = str;

   /* alloc space for the return string, this is where the pointers to the tokens will be stored */
   retstr = (char **) malloc((sizeof(char **) * max_strs));

   max_strs--;

#ifdef DEBUG
   printf("max_strs = %d  curr_str = %d\n", max_strs, curr_str);
#endif

   /* loop thru each letter in the string being tokenized */
   while (idx < end)
   {
       /* loop thru each seperator string char */
       while (sep_idx < sep_end)
       {
           /* if the current string-indexed char matches the current seperator char... */
           if ((*idx == *sep_idx) && (last_char != meta))
           {
               /* if there's something to store... */
               if (len > 0)
               {
#ifdef DEBUG
                   printf("Allocating %d bytes for token ", len + 1);
                   fflush(stdout);
#endif
                   if(curr_str <= max_strs)
                   {
                       /* allocate space for the new token */
                       retstr[curr_str] = (char *) malloc((sizeof(char) * len) + 1);

                       /* make sure we got a good allocation */
                       if (retstr[curr_str] == NULL)
                       {
                           fprintf(stderr, "msplit() got NULL substring malloc!\n");
                           exit(1);
                       }

                       /* copy the token into the return string array */
                       memcpy(retstr[curr_str], (idx - len), len);
                       retstr[curr_str][len] = 0; 
#ifdef DEBUG
                       printf("tok[%d]: %s\n", curr_str, retstr[curr_str]);
                       fflush(stdout);
#endif
                       /* twiddle the necessary pointers and vars */
                       len = 0;
                       curr_str++;
#ifdef DEBUG
                       printf("curr_str = %d\n", curr_str);
                       printf("max_strs = %d  curr_str = %d\n", max_strs, curr_str);
#endif
                       last_char = *idx;
                       idx++;
                   }

#ifdef DEBUG
                   printf("Checking if curr_str (%d) >= max_strs (%d)\n", curr_str, max_strs);
#endif

                   /* if we've gotten all the tokens requested, return the list */
                   if (curr_str >= max_strs)
                   {
                      while(isspace((int)*idx)) idx++;

                      len = end - idx;
#ifdef DEBUG
                      printf("Finishing up...\n");
                      printf("Allocating %d bytes for last token ", len + 1);
                      fflush(stdout);
#endif
                      retstr[curr_str] = (char *) malloc((sizeof(char) * len) + 1);

                      if (retstr[curr_str] == NULL)
                         printf("Got NULL back from substr malloc\n");

                      memcpy(retstr[curr_str], idx, len);
                      retstr[curr_str][len] = 0; 

#ifdef DEBUG
                      printf("tok[%d]: %s\n", curr_str, retstr[curr_str]);
                      fflush(stdout);
#endif

                       *toks = curr_str + 1;
#ifdef DEBUG
                       printf("max_strs = %d  curr_str = %d\n", max_strs, curr_str);
                       printf("mSplit got %d tokens!\n", *toks);
                       fflush(stdout);
#endif
                       return retstr;
                   }
               }
               else  /* otherwise, the previous char was a seperator as well, and we should just continue */
               {
                   last_char = *idx;
                   idx++;
                   /* make sure to reset this so we test all the sep. chars */
                   sep_idx = sep;
                   len = 0;
               }
           }
           else
           {
               /* go to the next seperator */
               sep_idx++;
           }
       }

       sep_idx = sep;
       len++;
       last_char = *idx;
       idx++;
   }

   /* put the last string into the list */

   if (len > 0)
   {
#ifdef DEBUG
       printf("Allocating %d bytes for last token ", len + 1);
       fflush(stdout);
#endif

       retstr[curr_str] = (char *) malloc((sizeof(char) * len) + 1);

       if (retstr[curr_str] == NULL)
           printf("Got NULL back from substr malloc\n");

       memcpy(retstr[curr_str], (idx - len), len);
       retstr[curr_str][len] = 0; 

#ifdef DEBUG
       printf("tok[%d]: %s\n", curr_str, retstr[curr_str]);
       fflush(stdout);
#endif
       *toks = curr_str + 1;
   }

#ifdef DEBUG
   printf("mSplit got %d tokens!\n", *toks);
   fflush(stdout);
#endif

 

   /* return the token list */
   return retstr;
}

