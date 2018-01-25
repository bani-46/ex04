#include "compiler.h"
/**
 * アクセスメモ
 * グローバル変数,ローカル変数
 * →アドレスが欲しい時　LAD gr1, $~~
 * →中身が欲しい時 LD gr1,$~~
 *
 * 仮引数
 * →アドレスが欲しい時　LD gr1, $~~
 * →中身が欲しい時 LD gr1,$~~
 *              LD gr1,0,gr1
 *
 */

/* str list*/
struct STRLIST{
    char *str;
    int label;
    struct STRLIST *nextp;
}*str_list;

void init_strlist(){
    struct STRLIST *sl;
    sl = add_strlist("",0);
    str_list = sl;
}

struct STRLIST *add_strlist(char *_str,int _label_num){
    struct STRLIST *new = (struct STRLIST *)malloc(sizeof(struct STRLIST));
    char *c = (char *)malloc(strlen(_str) + 1);
    if(new != NULL && c != NULL){
        strcpy(c,_str);
        new->str = c;
        new->label = _label_num;
        new->nextp = NULL;
    }
    return new;
}

void insert_strlist(char *_str,int _label_num){
    struct STRLIST *sl = str_list;
    while(sl->nextp != NULL)sl = sl->nextp;
    sl->nextp = add_strlist(_str,_label_num);
}

/* for asmprint */
FILE * init_output(char *filename){
    char str[MAXSTRSIZE];
    sprintf(str,"%s.csl",filename);
    fp_out = fopen(str,"w");
    if(fp_out == NULL) {
        return NULL;
    } else {
        printf("[INFO]Make \"%s\".\n",str);
        return fp_out;
    }
}

void asmprint_def_label(char *_name, char *_procname,int is_array){
    if(strlen(_procname) != 0) {
        if(is_array)fprintf(fp_out, "$%s%%%s\tDS\t%d\n", _name, _procname,num_attr);
        else fprintf(fp_out, "$%s%%%s\tDC\t0\n", _name, _procname);
    }
    else {
        if(is_array)fprintf(fp_out,"$%s\tDS\t%d\n",_name,num_attr);
        else fprintf(fp_out,"$%s\tDC\t0\n",_name);
    }
}

void asmprint_ST_label(char *_name, char *_procname){
    fprintf(fp_out,"\tPOP\tgr1\n");
    if(strlen(_procname) != 0)
        fprintf(fp_out,"\tST\tgr1,\t$%s%%%s\n",_name,_procname);
    else fprintf(fp_out,"\tST\tgr1,\t$%s\n",_name);
}

void asmprint_call_WRITE(char *_str, int _label_num, int format, int length){
    switch(format) {
        case FINT:
            if(length <= 0){
                fprintf(fp_out,"\tLD\tgr2,\tgr0\n");
            }else{
                fprintf(fp_out,"\tLAD\tgr2,\t%d\n",length);
            }
            fprintf(fp_out,"\tCALL\tWRITEINT\n");
            break;

        case FCHAR:
            if(length <= 0){
                fprintf(fp_out,"\tLD\tgr2,\tgr0\n");
            }else{
                fprintf(fp_out,"\tLAD\tgr2,\t%d\n",length);
            }
            fprintf(fp_out,"\tCALL\tWRITECHAR\n");
            break;

        case FBOOL:
            if(length <= 0){
                fprintf(fp_out,"\tLD\tgr2,\tgr0\n");
            }else{
                fprintf(fp_out,"\tLAD\tgr2,\t%d\n",length);
            }
            fprintf(fp_out,"\tCALL\tWRITEBOOL\n");
            break;

        case FSTR:
            insert_strlist(_str, _label_num);
            fprintf(fp_out, "\tLAD\tgr1,\tL%04d\n"
                            "\tLD\tgr2,\tgr0\n"
                            "\tCALL\tWRITESTR\n", _label_num);
            break;
    }
}

void asmprint_str_labels(){
    struct STRLIST *sl = str_list->nextp;
    while(sl != NULL) {
        fprintf(fp_out, "L%04d\tDC\t%s\n",sl->label,sl->str);
        sl = sl->nextp;
    }
}

void asmprint_library(){
    fprintf(fp_out,"EOVF\n"
            "  CALL  WRITELINE\n"
            "  LAD  gr1, EOVF1\n"
            "  LD  gr2, gr0\n"
            "  CALL  WRITESTR\n"
            "  CALL  WRITELINE\n"
            "  SVC  1  ;  overflow error stop\n"
            "EOVF1    DC  '***** Run-Time Error : Overflow *****'\n"
            "E0DIV\n"
            "  JNZ  EOVF\n"
            "  CALL  WRITELINE\n"
            "  LAD  gr1, E0DIV1\n"
            "  LD  gr2, gr0\n"
            "  CALL  WRITESTR\n"
            "  CALL  WRITELINE\n"
            "  SVC  2  ;  0-divide error stop\n"
            "E0DIV1    DC  '***** Run-Time Error : Zero-Divide *****'\n"
            "EROV\n"
            "  CALL  WRITELINE\n"
            "  LAD  gr1, EROV1\n"
            "  LD  gr2, gr0\n"
            "  CALL  WRITESTR\n"
            "  CALL  WRITELINE\n"
            "  SVC  3  ;  range-over error stop\n"
            "EROV1    DC  '***** Run-Time Error : Range-Over in Array Index *****'\n"
            "WRITECHAR\n"
            "  RPUSH\n"
            "  LD  gr6, SPACE\n"
            "  LD  gr7, OBUFSIZE\n"
            "WC1\n"
            "  SUBA  gr2, ONE  ; while(--c > 0) {\n"
            "  JZE  WC2\n"
            "  JMI  WC2\n"
            "  ST  gr6, OBUF,gr7  ;  *p++ = ' ';\n"
            "  CALL  BOVFCHECK\n"
            "  JUMP  WC1  ; }\n"
            "WC2\n"
            "  ST  gr1, OBUF,gr7  ; *p++ = gr1;\n"
            "  CALL  BOVFCHECK\n"
            "  ST  gr7, OBUFSIZE\n"
            "  RPOP\n"
            "  RET\n"
            "WRITESTR\n"
            "  RPUSH\n"
            "  LD  gr6, gr1  ; p = gr1;\n"
            "WS1\n"
            "  LD  gr4, 0,gr6  ; while(*p != '\\0') {\n"
            "  JZE  WS2\n"
            "  ADDA  gr6, ONE  ;  p++;\n"
            "  SUBA  gr2, ONE  ;  c--;\n"
            "  JUMP  WS1  ; }\n"
            "WS2\n"
            "  LD  gr7, OBUFSIZE  ; q = OBUFSIZE;\n"
            "  LD  gr5, SPACE\n"
            "WS3\n"
            "  SUBA  gr2, ONE  ; while(--c >= 0) {\n"
            "  JMI  WS4\n"
            "  ST  gr5, OBUF,gr7  ;  *q++ = ' ';\n"
            "  CALL  BOVFCHECK\n"
            "  JUMP  WS3  ; }\n"
            "WS4\n"
            "  LD  gr4, 0,gr1  ; while(*gr1 != '\\0') {\n"
            "  JZE  WS5\n"
            "  ST  gr4, OBUF,gr7  ;  *q++ = *gr1++;\n"
            "  ADDA  gr1, ONE\n"
            "  CALL  BOVFCHECK\n"
            "  JUMP  WS4  ; }\n"
            "WS5\n"
            "  ST  gr7, OBUFSIZE  ; OBUFSIZE = q;\n"
            "  RPOP\n"
            "  RET\n"
            "BOVFCHECK\n"
            "    ADDA  gr7, ONE\n"
            "    CPA   gr7, BOVFLEVEL\n"
            "    JMI  BOVF1\n"
            "    CALL  WRITELINE\n"
            "    LD gr7, OBUFSIZE\n"
            "BOVF1\n"
            "    RET\n"
            "BOVFLEVEL  DC 256\n"
            "WRITEINT\n"
            "  RPUSH\n"
            "  LD  gr7, gr0  ; flag = 0;\n"
            "  CPA  gr1, gr0  ; if(gr1>=0) goto WI1;\n"
            "  JPL  WI1\n"
            "  JZE  WI1\n"
            "  LD  gr4, gr0  ; gr1= - gr1;\n"
            "  SUBA  gr4, gr1\n"
            "  CPA  gr4, gr1\n"
            "  JZE  WI6\n"
            "  LD  gr1, gr4\n"
            "  LD  gr7, ONE  ; flag = 1;\n"
            "WI1\n"
            "  LD  gr6, SIX  ; p = INTBUF+6;\n"
            "  ST  gr0, INTBUF,gr6  ; *p = '\\0';\n"
            "  SUBA  gr6, ONE  ; p--;\n"
            "  CPA  gr1, gr0  ; if(gr1 == 0)\n"
            "  JNZ  WI2\n"
            "  LD  gr4, ZERO  ;  *p = '0';\n"
            "  ST  gr4, INTBUF,gr6\n"
            "  JUMP  WI5  ; }\n"
            "WI2      ; else {\n"
            "  CPA  gr1, gr0  ;  while(gr1 != 0) {\n"
            "  JZE  WI3\n"
            "  LD  gr5, gr1  ;   gr5 = gr1 - (gr1 / 10) * 10;\n"
            "  DIVA  gr1, TEN  ;   gr1 /= 10;\n"
            "  LD  gr4, gr1\n"
            "  MULA  gr4, TEN\n"
            "  SUBA  gr5, gr4\n"
            "  ADDA  gr5, ZERO  ;   gr5 += '0';\n"
            "  ST  gr5, INTBUF,gr6  ;   *p = gr5;\n"
            "  SUBA  gr6, ONE  ;   p--;\n"
            "  JUMP  WI2  ;  }\n"
            "WI3\n"
            "  CPA  gr7, gr0  ;  if(flag != 0) {\n"
            "  JZE  WI4\n"
            "  LD  gr4, MINUS  ;   *p = '-';\n"
            "  ST  gr4, INTBUF,gr6\n"
            "  JUMP  WI5  ;  }\n"
            "WI4\n"
            "  ADDA  gr6, ONE  ;  else p++;\n"
            "    ; }\n"
            "WI5\n"
            "  LAD  gr1, INTBUF,gr6  ; gr1 = p;\n"
            "  CALL  WRITESTR  ; WRITESTR();\n"
            "  RPOP\n"
            "  RET\n"
            "WI6\n"
            "  LAD  gr1, MMINT\n"
            "  CALL  WRITESTR  ; WRITESTR();\n"
            "  RPOP\n"
            "  RET\n"
            "MMINT    DC  '-32768'\n"
            "WRITEBOOL\n"
            "  RPUSH\n"
            "  CPA  gr1, gr0  ; if(gr1 != 0)\n"
            "  JZE  WB1\n"
            "  LAD  gr1, WBTRUE  ;  gr1 = \"TRUE\";\n"
            "  JUMP  WB2\n"
            "WB1      ; else\n"
            "  LAD  gr1, WBFALSE  ;  gr1 = \"FALSE\";\n"
            "WB2\n"
            "  CALL  WRITESTR  ; WRITESTR();\n"
            "  RPOP\n"
            "  RET\n"
            "WBTRUE    DC  'TRUE'\n"
            "WBFALSE    DC  'FALSE'\n"
            "WRITELINE\n"
            "  RPUSH\n"
            "  LD  gr7, OBUFSIZE\n"
            "  LD  gr6, NEWLINE\n"
            "  ST  gr6, OBUF,gr7\n"
            "  ADDA  gr7, ONE\n"
            "  ST  gr7, OBUFSIZE\n"
            "  OUT  OBUF, OBUFSIZE\n"
            "  ST  gr0, OBUFSIZE\n"
            "  RPOP\n"
            "  RET\n"
            "FLUSH\n"
            "  RPUSH\n"
            "  LD gr7, OBUFSIZE\n"
            "  JZE FL1\n"
            "  CALL WRITELINE\n"
            "FL1\n"
            "  RPOP\n"
            "  RET\n"
            "READCHAR\n"
            "  RPUSH\n"
            "  LD  gr5, RPBBUF  ; if(RPBBUF != '\\0') {\n"
            "  JZE  RC0\n"
            "  ST  gr5, 0,gr1  ;  *gr1 = RPBBUF;\n"
            "  ST  gr0, RPBBUF  ;  RPBBUF = '\\0'\n"
            "  JUMP  RC3  ;  return; }\n"
            "RC0\n"
            "  LD  gr7, INP  ; inp = INP;\n"
            "  LD  gr6, IBUFSIZE  ; if(IBUFSIZE == 0) {\n"
            "  JNZ  RC1\n"
            "  IN  IBUF, IBUFSIZE  ;  IN();\n"
            "  LD  gr7, gr0  ;  inp = 0;\n"
            "    ; }\n"
            "RC1\n"
            "  CPA  gr7, IBUFSIZE  ; if(inp == IBUFSIZE) {\n"
            "  JNZ  RC2\n"
            "  LD  gr5, NEWLINE  ;  *gr1 = '\\n';\n"
            "  ST  gr5, 0,gr1\n"
            "  ST  gr0, IBUFSIZE  ;  IBUFSIZE = INP = 0;\n"
            "  ST  gr0, INP\n"
            "  JUMP  RC3  ; }\n"
            "RC2      ; else {\n"
            "  LD  gr5, IBUF,gr7  ;  *gr1 = *inp++;\n"
            "  ADDA  gr7, ONE\n"
            "  ST  gr5, 0,gr1\n"
            "  ST  gr7, INP  ;  INP = inp;\n"
            "RC3      ; }\n"
            "  RPOP\n"
            "  RET\n"
            "READINT\n"
            "  RPUSH\n"
            "RI1      ; do {\n"
            "  CALL  READCHAR  ;  ch = READCHAR();\n"
            "  LD  gr7, 0,gr1\n"
            "  CPA  gr7, SPACE  ; } while(ch == ' ' || ch == '\\t' || ch == '\\n');\n"
            "  JZE  RI1\n"
            "  CPA  gr7, TAB\n"
            "  JZE  RI1\n"
            "  CPA  gr7, NEWLINE\n"
            "  JZE  RI1\n"
            "  LD  gr5, ONE  ; flag = 1\n"
            "  CPA  gr7, MINUS  ; if(ch == '-') {\n"
            "  JNZ  RI4\n"
            "  LD  gr5, gr0  ;  flag = 0;\n"
            "  CALL  READCHAR  ;  ch = READCHAR();\n"
            "  LD  gr7, 0,gr1\n"
            "RI4      ; }\n"
            "  LD  gr6, gr0  ; v = 0;\n"
            "RI2\n"
            "  CPA  gr7, ZERO  ; while('0' <= ch && ch <= '9') {\n"
            "  JMI  RI3\n"
            "  CPA  gr7, NINE\n"
            "  JPL  RI3\n"
            "  MULA  gr6, TEN  ;  v = v*10+ch-'0';\n"
            "  ADDA  gr6, gr7\n"
            "  SUBA  gr6, ZERO\n"
            "  CALL  READCHAR  ;  ch = READSCHAR();\n"
            "  LD  gr7, 0,gr1\n"
            "  JUMP  RI2  ; }\n"
            "RI3\n"
            "  ST  gr7, RPBBUF  ; ReadPushBack();\n"
            "  ST  gr6, 0,gr1  ; *gr1 = v;\n"
            "  CPA  gr5, gr0  ; if(flag == 0) {\n"
            "  JNZ  RI5\n"
            "  SUBA  gr5, gr6  ;  *gr1 = -v;\n"
            "  ST  gr5, 0,gr1\n"
            "RI5      ; }\n"
            "  RPOP\n"
            "  RET\n"
            "READLINE\n"
            "  ST  gr0, IBUFSIZE\n"
            "  ST  gr0, INP\n"
            "  ST  gr0, RPBBUF\n"
            "  RET\n"
            "ONE    DC  1\n"
            "SIX    DC  6\n"
            "TEN    DC  10\n"
            "SPACE    DC  #0020  ; ' '\n"
            "MINUS    DC  #002D  ; '-'\n"
            "TAB    DC  #0009  ; '\\t'\n"
            "ZERO    DC  #0030  ; '0'\n"
            "NINE    DC  #0039  ; '9'\n"
            "NEWLINE    DC  #000A  ; '\\n'\n"
            "INTBUF    DS  8\n"
            "OBUFSIZE  DC  0\n"
            "IBUFSIZE  DC  0\n"
            "INP    DC  0\n"
            "OBUF    DS  257\n"
            "IBUF    DS  257\n"
            "RPBBUF    DC  0\n");
}
