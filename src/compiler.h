/* token-list.h  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRSIZE 1024
#define MAXNUM 32767

#define NORMAL 1
#define ERROR -1

/* Token */
#define	TNAME		1	/* Name : Alphabet { Alphabet | Digit } */
#define	TPROGRAM	2	/* program : Keyword */
#define	TVAR		3	/* var : Keyword */
#define	TARRAY		4	/* array : Keyword */
#define	TOF		5	/* of : Keyword */
#define	TBEGIN		6	/* begin : Keyword */
#define	TEND		7  	/* end : Keyword */
#define	TIF		8  	/* if : Keyword */
#define	TTHEN		9	/* then : Keyword */
#define	TELSE		10	/* else : Keyword */
#define	TPROCEDURE	11	/* procedure : Keyword */
#define	TRETURN		12	/* return : Keyword */
#define	TCALL		13	/* call : Keyword */
#define	TWHILE		14	/* while : Keyword */
#define	TDO		15 	/* do : Keyword */
#define	TNOT		16	/* not : Keyword */
#define	TOR		17	/* or : Keyword */
#define	TDIV		18 	/* div : Keyword */
#define	TAND		19 	/* and : Keyword */
#define	TCHAR		20	/* char : Keyword */
#define	TINTEGER	21	/* integer : Keyword */
#define	TBOOLEAN	22 	/* boolean : Keyword */
#define	TREADLN		23	/* readln : Keyword */
#define	TWRITELN	24	/* writeln : Keyword */
#define	TTRUE		25	/* true : Keyword */
#define	TFALSE		26	/* false : Keyword */
#define	TNUMBER		27	/* unsigned integer */
#define	TSTRING		28	/* String */
#define	TPLUS		29	/* + : symbol */
#define	TMINUS		30 	/* - : symbol */
#define	TSTAR		31 	/* * : symbol */
#define	TEQUAL		32 	/* = : symbol */
#define	TNOTEQ		33 	/* <> : symbol */
#define	TLE		34 	/* < : symbol */
#define	TLEEQ		35 	/* <= : symbol */
#define	TGR		36	/* > : symbol */
#define	TGREQ		37	/* >= : symbol */
#define	TLPAREN		38 	/* ( : symbol */
#define	TRPAREN		39 	/* ) : symbol */
#define	TLSQPAREN	40	/* [ : symbol */
#define	TRSQPAREN	41 	/* ] : symbol */
#define	TASSIGN		42	/* := : symbol */
#define	TDOT		43 	/* . : symbol */
#define	TCOMMA		44	/* , : symbol */
#define	TCOLON		45	/* : : symbol */
#define	TSEMI		46	/* ; : symbol */
#define	TREAD		47	/* read : Keyword */
#define	TWRITE		48	/* write : Keyword */
#define	TBREAK		49	/* break : Keyword */

#define NUMOFTOKEN	49

/*Type*/
#define TPINT 1
#define TPCHAR 2
#define TPBOOL 3
#define TPARRAY 4
#define TPARRAYINT 5
#define TPARRAYCHAR 6
#define TPARRAYBOOL 7
#define TPPROC 8

#define NUMOFTYPE 8

/* token-list.c */
#define KEYWORDSIZE	28

extern struct KEY {
	char * keyword;
	int keytoken;
} key[KEYWORDSIZE];

extern void error(char *mes);
extern void print_info(char *mes);

/*** scan.c *******************************/
extern int init_scan(char *filename);
extern int scan(void);
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int get_linenum(void);
extern void end_scan(void);
extern FILE *fp;
extern int cbuf;

/*** id-list.c *******************************/
enum{
	global=0,local
};

enum{
	var=0,formal
};

/* namelist */
extern void init_namelist();
extern struct NAMELIST *add_namelist(char *_name,int _defline,struct NAMELIST *nl);
extern int insert_namelist(char *_name,int _defline);
extern int search_namelist(char *_name);
extern void print_namelist();
extern void free_namelist();
extern char * get_names();
/* ID list */
extern void init_globalidtab();
extern void init_localidtab();
extern struct ID *add_idlist(char *_name,
                      char *_procname,
                      int _type,
                      int _ispara,
                      int _defline,
                      int arraysize,
                      struct ID *il);
extern int insert_idlist(char *_procname,
                  int _type,
                  int _ispara,
                  int scope,
                  int array_size);
extern int is_overload(int _scope);
extern int id_count(char *_name,int _scope,int _line);
/* ID list for Proc */
extern void add_proc(char *_procname,int _line);
extern void regist_proctype();
extern int is_recursion(char *_name);
extern int check_proc_type(int exp_type);
extern int is_null_proc_type();
extern void copy_locallist();
/* ID list for print */
extern void print_idlist(int _scope);
extern void sort_list();
extern void print_sortedlist();
extern void delete_list(struct ID *p);
extern void free_lists();
/* ID list else */
extern int get_array_size();

/***parser.c*******************************/
extern int parse_program();
extern int block();
extern int variable_declaration();
extern int variable_names();
extern int type();
extern int standard_type();
extern int array_type();

extern int subprogram_declaration();
extern int formal_parameters();
extern int compound_statement();
extern int statement();
extern int condition_statement();
extern int iteration_statement();
extern int exit_statement();
extern int call_statement();
extern int expressions();
extern int return_statement();
extern int assignment_statement();

extern int variable();
extern int expression();
extern int simple_expression();
extern int term();
extern int factor();
extern int constant();
extern int multiplicative_operator();
extern int addictive_operator();
extern int relational_operator();

extern int input_statement();
extern int output_statement();
extern int output_format();

/* output_util.c */

enum{
    FINT = 1,
    FCHAR,
    FBOOL,
    FSTR
};

extern FILE *fp_out;
extern FILE * init_output(char *filename);
extern void out_commmon_library();
extern void out_def_names(int exp_type, char *procname);
extern void out_def_label(char *_name, char *_procname);
extern void out_val_names(int _scope);
extern void out_val_label(char *_name, char *_procname);
extern void init_strlist();
extern struct STRLIST *add_strlist(char *_str,int _label_num);
extern void insert_strlist(char *_str,int _label_num);
extern void out_call_WRITE(char *_str, int _label_num,int format,int length);
extern void out_str_labels();
extern void out_call_READ(char *_name,int _is_para,int format);
extern char * get_label_name(char *_name,int _scope);
extern int get_is_para(char *_name,int _scope);

/* Else function*/
extern int error_parse(char *mes);
extern int error_variable(char *mes);
extern void reset_flags();
extern void reset_array();

