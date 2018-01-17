#include "compiler.h"
/**
 * アクセスメモ
 * グローバル変数
 * ローカル変数
 * →アドレスが欲しい時　LAD gr1, $~~
 * →中身が欲しい時 LD gr1,$~~
 *
 * 仮引数
 * →アドレスが欲しい時　LD gr1, $~~
 * →中身が欲しい時 LD gr1,$~~
 *              LD gr1,0,gr1
 *
 */

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

FILE * init_output(char *filename){
    char str[127];
    sprintf(str,"%s.csl",filename);
    fp_out = fopen(str,"w");
    if(fp_out == NULL) {
        return NULL;
    } else {
        printf("[INFO]Make \"%s\".\n",str);
        return fp_out;
    }
}

void out_def_label(char *_name, char *_procname){
    if(strlen(_procname) != 0)
        fprintf(fp_out,"$%s%%%s\tDC\t0\n",_name,_procname);
    else fprintf(fp_out,"$%s\tDC\t0\n",_name);
}

void out_val_label(char *_name, char *_procname){
    fprintf(fp_out,"\tPOP\tgr1\n");
    if(strlen(_procname) != 0)
        fprintf(fp_out,"\tST\tgr1,\t$%s%%%s\n",_name,_procname);
    else fprintf(fp_out,"\tST\tgr1,\t$%s\n",_name);
}

void out_call_WRITE(char *_str, int _label_num,int format,int length){
    switch(format) {
        case FINT:
            if(length == 0){
                fprintf(fp_out,"\tLD\tgr2,\tgr0\n");
            }else{
                fprintf(fp_out,"\tLAD\tgr2,\t%d\n",length);//todo
            }
            fprintf(fp_out,"\tCALL\tWRITEINT\n");
            break;
        case FCHAR:
            fprintf(fp_out,"[PUT HERE WRITECHAR]\n");
            break;
        case FBOOL:
            fprintf(fp_out,"[PUT HERE WRITEBOOL]\n");
            break;
        case FSTR:
            insert_strlist(_str, _label_num);
            fprintf(fp_out,
                    "\tLAD\tgr1,\tL%04d\n"
                            "\tLD\tgr2,\tgr0\n"
                            "\tCALL\tWRITESTR\n", _label_num);
            break;
    }
}

void out_call_READ(char *_name,int _is_para,int format){
    if(_is_para)
        fprintf(fp_out,"\tLD");
    else
        fprintf(fp_out,"\tLAD");
    fprintf(fp_out,
            "\tgr1,\t$%s\n"
            "\tCALL\t",_name);
    if(format == FINT)fprintf(fp_out,"READINT\n");
    else fprintf(fp_out,"READCHAR\n");
}

void out_str_labels(){
    struct STRLIST *sl = str_list->nextp;
    while(sl != NULL) {
        fprintf(fp_out, "L%04d\tDC\t%s\n",sl->label,sl->str);
        sl = sl->nextp;
    }
}

void out_commmon_library(){
    fprintf(fp_out,
            "\nEOVF\n"
            "\tCALL\tWRITELINE\n"
            "\tLAD\tgr1,EOVF1\n"
            "\tLD\tgr2,gr0\n"
            "\tCALL\tWRITESTR\n"
            "\tCALL\tWRITELINE\n"
            "\tSVC 1 ; overflow error stop\n"

            "EOVF1\tDC\t'***** Run-Time Error : Overflow *****'\n"

            "E0DIV\n"
            "\tJNZ\tEOVF\n"
            "\tCALL\tWRITELINE\n"
            "\tLAD\tgr1,E0DIV1\n"
            "\tLD\tgr2,gr0\n"
            "\tCALL\tWRITESTR\n"
            "\tCALL\tWRITELINE\n"
            "\tSVC\t2 ; 0-divide error stop\n"

            "E0DIV1\tDC\t'***** Run-Time Error : Zero-Divide *****'\n"

            "EROV\n"
            "\tCALL\tWRITELINE\n"
            "\tLAD\tgr1,EROV1\n"
            "\tLD\tgr2,gr0\n"
            "\tCALL\tWRITESTR\n"
            "\tCALL\tWRITELINE\n"
            "\tSVC\t3 ; range-over error stop\n"

            "EROV1\tDC\t'***** Run-Time Error : Range-Over in Array Index *****'\n"

            "WRITECHAR\n"
//            gr1 の値（文字）を gr2 のけた数で出力する．\n"
//            gr2 が 0 なら必要最小限の桁数で出力する\n"
            "\tRPUSH\n"
            "\tLD\tgr6,SPACE\n"
            "\tLD\tgr7,OBUFSIZE\n"

            "WC1\n"
            "\tSUBA\tgr2,ONE ; while(--c > 0) {\n"
            "\tJZE\tWC2\n"
            "\tJMI\tWC2\n"
            "\tST\tgr6,OBUF,gr7 ; *p++ = ' ';\n"
            "\tCALL\tBOVFCHECK\n"
            "\tJUMP\tWC1 ; }\n"

            "WC2\n"
            "\tST\tgr1,OBUF,gr7 ; *p++ = gr1;\n"
            "\tCALL\tBOVFCHECK\n"
            "\tST\tgr7,\tOBUFSIZE\n"
            "\tRPOP\n"
            "\tRET\n"

            "WRITESTR\n"
//            gr1 が指す文字列を gr2 のけた数で出力する．\n"
//            gr2 が 0 なら必要最小限の桁数で出力する\n"
            "\tRPUSH\n"
            "\tLD\tgr6,gr1 ; p = gr1;\n"

            "WS1\n"
            "\tLD\tgr4,0,gr6 ; while(*p != '¥0') {\n"
            "\tJZE\tWS2\n"
            "\tADDA\tgr6,ONE ; p++;\n"
            "\tSUBA\tgr2,ONE ; c--;\n"
            "\tJUMP\tWS1 ; }\n"

            "WS2\n"
            "\tLD\tgr7, OBUFSIZE ; q = OBUFSIZE;\n"
            "\tLD\tgr5, SPACE\n"

            "WS3\n"
            "\tSUBA\tgr2, ONE ; while(--c >= 0) {\n"
            "\tJMI\tWS4\n"
            "\tST\tgr5, OBUF,gr7 ; *q++ = ' ';\n"
            "\tCALL\tBOVFCHECK\n"
            "\tJUMP\tWS3 ; }\n"

            "WS4\n"
            "\tLD\tgr4, 0,gr1 ; while(*gr1 != '¥0') {\n"
            "\tJZE\tWS5\n"
            "\tST\tgr4, OBUF,gr7 ; *q++ = *gr1++;\n"
            "\tADDA\tgr1, ONE\n"
            "\tCALL\tBOVFCHECK\n"
            "\tJUMP\tWS4 ; }\n"

            "WS5\n"
            "\tST\tgr7, OBUFSIZE ; OBUFSIZE = q;\n"
            "\tRPOP\n"
            "\tRET\n"

            "BOVFCHECK\n"
            "\tADDA\tgr7, ONE\n"
            "\tCPA\tgr7, BOVFLEVEL\n"
            "\tJMI\tBOVF1\n"
            "\tCALL\tWRITELINE\n"
            "\tLD\tgr7, OBUFSIZE\n"

            "BOVF1\n"
            "\tRET\n"

            "BOVFLEVEL\tDC\t256\n"

            "WRITEINT\n"
//            gr1 の値（整数）を gr2 のけた数で出力する．\n"
//            gr2 が 0 なら必要最小限の桁数で出力する\n"
            "\tRPUSH\n"
            "\tLD\tgr7, gr0 ; flag = 0;\n"
            "\tCPA\tgr1, gr0 ; if(gr1>=0) goto WI1;\n"
            "\tJPL\tWI1\n"
            "\tJZE\tWI1\n"
            "\tLD\tgr4, gr0 ; gr1= - gr1;\n"
            "\tSUBA\tgr4, gr1\n"
            "\tCPA\tgr4, gr1\n"
            "\tJZE\tWI6\n"
            "\tLD\tgr1, gr4\n"
            "\tLD\tgr7, ONE ; flag = 1;\n"

            "WI1\n"
            "\tLD\tgr6, SIX ; p = INTBUF+6;\n"
            "\tST\tgr0, INTBUF,gr6 ; *p = '¥0';\n"
            "\tSUBA\tgr6, ONE ; p--;\n"
            "\tCPA\tgr1, gr0 ; if(gr1 == 0)\n"
            "\tJNZ\tWI2\n"
            "\tLD\tgr4, ZERO ; *p = '0';\n"
            "\tST\tgr4, INTBUF,gr6\n"
            "\tJUMP\tWI5 ; }\n"

            "WI2 ; else {\n"
            "\tCPA\tgr1, gr0 ; while(gr1 != 0) {\n"
            "\tJZE\tWI3\n"
            "\tLD\tgr5, gr1 ; gr5 = gr1 - (gr1 / 10) * 10;\n"
            "\tDIVA\tgr1, TEN ; gr1 /= 10;\n"
            "\tLD\tgr4, gr1\n"
            "\tMULA\tgr4, TEN\n"
            "\tSUBA\tgr5, gr4\n"
            "\tADDA\tgr5, ZERO ; gr5 += '0';\n"
            "\tST\tgr5, INTBUF,gr6 ; *p = gr5;\n"
            "\tSUBA\tgr6, ONE ; p--;\n"
            "\tJUMP\tWI2 ; }\n"

            "WI3\n"
            "\tCPA\tgr7, gr0 ; if(flag != 0) {\n"
            "\tJZE\tWI4\n"
            "\tLD\tgr4, MINUS ; *p = '-';\n"
            "\tST\tgr4, INTBUF,gr6\n"
            "\tJUMP\tWI5 ; }\n"

            "WI4\n"
            "\tADDA\tgr6, ONE ; else p++;\n"
            "\t; }\n"

            "WI5\n"
            "\tLAD\tgr1,INTBUF,gr6 ; gr1 = p;\n"
            "\tCALL\tWRITESTR ; WRITESTR();\n"
            "\tRPOP\n"
            "\tRET\n"

            "WI6\n"
            "\tLAD\tgr1, MMINT\n"
            "\tCALL\tWRITESTR ; WRITESTR();\n"
            "\tRPOP\n"
            "\tRET\n"

            "MMINT\tDC\t'-32768'\n"

            "WRITEBOOL\n"
//            gr1 の値（真理値）が 0 なら'FALSE'を\n"
//            0 以外なら'TRUE'を gr2 のけた数で出力する．\n"
//            gr2 が 0 なら必要最小限の桁数で出力する\n"
            "\tRPUSH\n"
            "\tCPA\tgr1, gr0 ; if(gr1 != 0)\n"
            "\tJZE\tWB1\n"
            "\tLAD\tgr1, WBTRUE ; gr1 = \"TRUE\";\n"
            "\tJUMP\tWB2\n"

            "WB1 ; else\n"
            "\tLAD\tgr1, WBFALSE ; gr1 = \"FALSE\";\n"

            "WB2\n"
            "\tCALL\tWRITESTR ; WRITESTR();\n"
            "\tRPOP\n"
            "\tRET\n"

            "WBTRUE\tDC\t'TRUE'\n"

            "WBFALSE\tDC\t'FALSE'\n"

            "WRITELINE\n"
//            改行を出力する\n"
            "\tRPUSH\n"
            "\tLD\tgr7, OBUFSIZE\n"
            "\tLD\tgr6, NEWLINE\n"
            "\tST\tgr6, OBUF,gr7\n"
            "\tADDA\tgr7, ONE\n"
            "\tST\tgr7, OBUFSIZE\n"
            "\tOUT\tOBUF, OBUFSIZE\n"
            "\tST\tgr0, OBUFSIZE\n"
            "\tRPOP\n"
            "\tRET\n"

            "FLUSH\n"
            "\tRPUSH\n"
            "\tLD gr7, OBUFSIZE\n"
            "\tJZE FL1\n"
            "\tCALL WRITELINE\n"

            "FL1\n"
            "\tRPOP\n"
            "\tRET\n"

            "READCHAR\n"
//            gr1 が指す番地に文字一つを読み込む\n"
            "\tRPUSH\n"
            "\tLD\tgr5, RPBBUF ; if(RPBBUF != '¥0') {\n"
            "\tJZE\tRC0\n"
            "\tST\tgr5, 0,gr1 ; *gr1 = RPBBUF;\n"
            "\tST\tgr0, RPBBUF ; RPBBUF = '¥0'\n"
            "\tJUMP\tRC3 ; return; }\n"

            "RC0\n"
            "\tLD\tgr7, INP ; inp = INP;\n"
            "\tLD\tgr6, IBUFSIZE ; if(IBUFSIZE == 0) {\n"
            "\tJNZ\tRC1\n"
            "\tIN\tIBUF, IBUFSIZE ; IN();\n"
            "\tLD\tgr7, gr0 ; inp = 0;\n"
            "\t; }\n"

            "RC1\n"
            "\tCPA\tgr7, IBUFSIZE ; if(inp == IBUFSIZE) {\n"
            "\tJNZ\tRC2\n"
            "\tLD\tgr5, NEWLINE ; *gr1 = '¥n';\n"
            "\tST\tgr5, 0,gr1\n"
            "\tST\tgr0, IBUFSIZE ; IBUFSIZE = INP = 0;\n"
            "\tST\tgr0, INP\n"
            "\tJUMP\tRC3 ; }\n"

            "RC2 ; else {\n"
            "\tLD\tgr5, IBUF,gr7 ; *gr1 = *inp++;\n"
            "\tADDA\tgr7, ONE\n"
            "\tST\tgr5, 0,gr1\n"
            "\tST\tgr7, INP ; INP = inp;\n"

            "RC3 ; }\n"
            "\tRPOP\n"
            "\tRET\n"

            "READINT\n"
//            gr1 が指す番地に整数値一つを読み込む\n"
            "\tRPUSH\n"

            "RI1 ; do {\n"
            "\tCALL\tREADCHAR ; ch = READCHAR();\n"
            "\tLD\tgr7, 0,gr1\n"
            "\tCPA\tgr7, SPACE ; } while(ch==' ' || ch=='¥t' || ch=='¥n');\n"
            "\tJZE\tRI1\n"
            "\tCPA\tgr7, TAB\n"
            "\tJZE\tRI1\n"
            "\tCPA\tgr7, NEWLINE\n"
            "\tJZE\tRI1\n"
            "\tLD\tgr5, ONE ; flag = 1\n"
            "\tCPA\tgr7, MINUS ; if(ch == '-') {\n"
            "\tJNZ\tRI4\n"
            "\tLD\tgr5, gr0 ; flag = 0;\n"
            "\tCALL\tREADCHAR ; ch = READCHAR();\n"
            "\tLD\tgr7, 0,gr1\n"

            "RI4 ; }\n"
            "\tLD\tgr6, gr0 ; v = 0;\n"

            "RI2\n"
            "\tCPA\tgr7, ZERO ; while('0' <= ch && ch <= '9') {\n"
            "\tJMI\tRI3\n"
            "\tCPA\tgr7, NINE\n"
            "\tJPL\tRI3\n"
            "\tMULA\tgr6, TEN ; v = v*10+ch-'0';\n"
            "\tADDA\tgr6, gr7\n"
            "\tSUBA\tgr6, ZERO\n"
            "\tCALL\tREADCHAR ; ch = READSCHAR();\n"
            "\tLD\tgr7, 0,gr1\n"
            "\tJUMP\tRI2 ; }\n"

            "RI3\n"
            "\tST\tgr7, RPBBUF ; ReadPushBack();\n"
            "\tST\tgr6, 0,gr1 ; *gr1 = v;\n"
            "\tCPA\tgr5, gr0 ; if(flag == 0) {\n"
            "\tJNZ\tRI5\n"
            "\tSUBA\tgr5, gr6 ; *gr1 = -v;\n"
            "\tST\tgr5, 0,gr1\n"

            "RI5 ; }\n"
            "\tRPOP\n"
            "\tRET\n"

            "READLINE\n"
//            入力を改行コードまで（改行コードも含む）読み飛ばす\n"
            "\tST\tgr0, IBUFSIZE\n"
            "\tST\tgr0, INP\n"
            "\tST\tgr0, RPBBUF\n"
            "\tRET\n"

            "ONE\tDC\t1\n"
            "SIX\tDC\t6\n"
            "TEN\tDC\t10\n"
            "SPACE\tDC\t#0020 ; ' '\n"
            "MINUS\tDC\t#002D ; '-'\n"
            "TAB\tDC\t#0009 ; '¥t'\n"
            "ZERO\tDC\t#0030 ; '0'\n"
            "NINE\tDC\t#0039 ; '9'\n"
            "NEWLINE\tDC\t#000A ; '¥n'\n"
            "INTBUF\tDS\t8\n"
            "OBUFSIZE\tDC\t0\n"
            "IBUFSIZE\tDC\t0\n"
            "INP\tDC\t0\n"
            "OBUF\tDS\t257\n"
            "IBUF\tDS\t257\n"
            "RPBBUF\tDC\t0\n"

            "\tEND");
}
