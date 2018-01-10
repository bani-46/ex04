#include "compiler.h"

//#define DEBUG

int cbuf;
int line_num = 1;
int is_first_scan = 0;
int is_end_loop = 0;

char string_attr[MAXSTRSIZE];
int num_attr;

FILE *fp;

int init_scan(char *filename){
	fp = fopen(filename,"r");
	if(fp == NULL) {
		return -1;
	} else {
		//Prefetching 1 token
		cbuf = fgetc(fp);
		if(cbuf == EOF){
			error("File could open but cbuf is EOF.\n");
			return -1;
		}
		printf("[INFO]Open File.\n");
		return 0;
	}
}

//scan token
int scan(void){
	int i=0;
	memset(string_attr,'\0',sizeof(string_attr));

	if(is_first_scan == 0)is_first_scan = 1;

	/*Jump read elements*/
	while((cbuf == '\r') || (cbuf == '\n') || (cbuf == ' ') || (cbuf == '\t') ||(cbuf == '{') ||(cbuf == '/')){
		switch(cbuf){
		/*End of Line*/
		case '\r':
			cbuf = fgetc(fp);
			if(cbuf =='\n')cbuf = fgetc(fp);
			line_num++;
			break;
		case '\n':
			cbuf = fgetc(fp);
			if(cbuf =='\r')cbuf = fgetc(fp);
			line_num++;
			break;
			/*SP or Tab*/
		case ' ':
		case '\t':
			cbuf = fgetc(fp);
			break;
			/*Comment*/
		case '{':
			i=0;
			while(cbuf != '}'){
				if(cbuf == EOF){
					error("Found EOF Comment searcing.");
					return -1;
				}
				string_attr[i] = cbuf;
				i++;
				cbuf = fgetc(fp);
				/*count up end of line*/
				if(cbuf == '\r'){
					while(cbuf == '\r'){
						string_attr[i] = cbuf;
						i++;
						cbuf = fgetc(fp);
						if(cbuf =='\n'){
							string_attr[i] = cbuf;
							i++;
							cbuf = fgetc(fp);
						}
						line_num++;
					}
				}
				else if(cbuf == '\n'){
					while(cbuf == '\n'){
						string_attr[i] = cbuf;
						i++;
						cbuf = fgetc(fp);
						if(cbuf =='\r'){
							string_attr[i] = cbuf;
							i++;
							cbuf = fgetc(fp);
						}
						line_num++;
					}
				}
				/*find can't display character*/
				if(!(cbuf >= ' ' && cbuf <= '~') && !(cbuf == '\t') && !(cbuf == '\r') && !(cbuf == '\n')){
					error("Found \'can`t display character\' until Comment searching.");
					return -1;
				}
			}
			string_attr[i] = cbuf;
#ifdef DEBUG
			printf("Comment[%s]\n",string_attr);
#endif
			memset(string_attr,'\0',sizeof(string_attr));
			i=0;
			cbuf = fgetc(fp);
			break;
		case '/':
			cbuf = fgetc(fp);
			is_end_loop = 0;
			if(cbuf == '*'){
				i=0;
				string_attr[i] = '/';
				i++;
				while(1){
					if(is_end_loop)break;
					if(cbuf == EOF){
						error("Found EOF Comment searcing.");
						return -1;
					}
					string_attr[i] = cbuf;
					i++;
					cbuf = fgetc(fp);
					while(cbuf == '\r' || cbuf == '\n' || cbuf == '*'){
						if(cbuf == '\r'){
							while(cbuf == '\r'){
								string_attr[i] = cbuf;
								i++;
								cbuf = fgetc(fp);
								if(cbuf =='\n'){
									string_attr[i] = cbuf;
									i++;
									cbuf = fgetc(fp);
								}
								line_num++;
							}
						}
						if(cbuf == '\n'){
							while(cbuf == '\n'){
								string_attr[i] = cbuf;
								i++;
								cbuf = fgetc(fp);
								if(cbuf =='\r'){
									string_attr[i] = cbuf;
									i++;
									cbuf = fgetc(fp);
								}
								line_num++;
							}
						}
						if(cbuf == '*'){
							string_attr[i] = cbuf;
							i++;
							cbuf = fgetc(fp);
							if(cbuf == '/'){
								string_attr[i] = cbuf;
								is_end_loop = 1;
								break;
							}
						}
					}
					/*find can't display character*/
					if(!(cbuf >= ' ' && cbuf <= '~') && !(cbuf == '\t') && !(cbuf == '\r') && !(cbuf == '\n')){
						error("Found \'can`t display character\' until Comment searching.");
						return -1;
					}
				}
				string_attr[i] = cbuf;
#ifdef DEBUG
				printf("Comment[%s]\n",string_attr);
#endif
				memset(string_attr,'\0',sizeof(string_attr));
				i=0;
				cbuf = fgetc(fp);
			}
			else{
				error("Found single '/'.");
				return -1;
			}
			break;
		}
	}

	/*name*/
	if((cbuf >= 'A' && cbuf <= 'Z') || (cbuf >= 'a' && cbuf <= 'z')){
		i=0;
		while((cbuf >= '0' && cbuf <= '9') || (cbuf >= 'A' && cbuf <= 'Z') || (cbuf >= 'a' && cbuf <= 'z')){
			string_attr[i] = cbuf;
			i++;
			if(i > MAXSTRSIZE){
				error("String_size reached MAXSTRSIZE.\n");
				return -1;
			}
			cbuf = fgetc(fp);
			if(cbuf == EOF){
				break;
			}
		}
		/*judge keyword*/
		for(i = 0;i < KEYWORDSIZE;i++){
			if(strcmp(string_attr,key[i].keyword) == 0){
#ifdef DEBUG
				printf("Key[%s]\n",string_attr);
#endif
				return key[i].keytoken;
			}
		}
#ifdef DEBUG
		printf("Name[%s]\n",string_attr);
#endif
		return TNAME;
	}
	/*numbers*/
	else if(cbuf >= '0' && cbuf <= '9'){
		i=0;
		while(cbuf >= '0' && cbuf <= '9'){
			string_attr[i] = cbuf;
			i++;
			cbuf = fgetc(fp);
			if(cbuf == EOF){
				break;
			}
		}
		num_attr = atoi(string_attr);
		if(num_attr > MAXNUM){
			error("Input_Number is too big.");
			return -1;
		}
#ifdef DEBUG
		printf("Number[%d]\n",num_attr);
#endif
		return TNUMBER;
	}
	/*string*/
	else if(cbuf == 39){//39=single quart
		i=0;
		while(1){
			string_attr[i] = cbuf;
			i++;
			if(i > MAXSTRSIZE){
				error("String_size reached MAXSTRSIZE.\n");
				return -1;
			}
			cbuf = fgetc(fp);

			/*End of Line*/
			if(cbuf == '\n' || cbuf == '\r' ){
				error("Found End of Line until String searching.");
				return -1;
			}

			/*single quart*/
			if(cbuf == 39){
				string_attr[i] = cbuf;
				i++;
				cbuf= fgetc(fp);
				if(cbuf != 39)break;
			}

			if(cbuf == EOF){
				error("Found EOF until String searcing.");
				return -1;
			}
		}
#ifdef DEBUG
		printf("String[%s]\n",string_attr);
#endif
		return TSTRING;
	}
	/*symbols*/
	else{
		switch(cbuf){
		case '+':cbuf=fgetc(fp);return TPLUS;
		case '-':cbuf=fgetc(fp);return TMINUS;
		case '*':cbuf=fgetc(fp);return TSTAR;
		case '=':cbuf=fgetc(fp);return TEQUAL;
		case '<':
			cbuf=fgetc(fp);
			if(cbuf == '>'){
				cbuf=fgetc(fp);
				return TNOTEQ;
			}
			else if(cbuf == '='){
				cbuf=fgetc(fp);
				return TLEEQ;
			}
			else
				return TLE;
		case '>':
			cbuf=fgetc(fp);
			if(cbuf == '='){
				cbuf=fgetc(fp);
				return TGREQ;
			}
			else
				return TGR;
		case '(':cbuf=fgetc(fp);return TLPAREN;
		case ')':cbuf=fgetc(fp);return TRPAREN;
		case '[':cbuf=fgetc(fp);return TLSQPAREN;
		case ']':cbuf=fgetc(fp);return TRSQPAREN;
		case '.':cbuf=fgetc(fp);return TDOT;
		case ',':cbuf=fgetc(fp);return TCOMMA;
		case ':':
			cbuf=fgetc(fp);
			if(cbuf == '='){
				cbuf=fgetc(fp);
				return TASSIGN;
			}
			else {
				return TCOLON;
			}
		case ';':cbuf=fgetc(fp);return TSEMI;
		case EOF:
#ifdef DEBUG
			printf("At %d line,",get_linenum());
			printf("Reached EOF\n");
#endif
			return -1;
		}
	}
	printf("\n[ERROR][SCAN]Never Reach.\n");
	return -1;
}

int get_linenum(void){
	if(is_first_scan)return line_num;
	else return 0;
}

void end_scan(void){
	fclose(fp);
	printf("[INFO]Close File.\n");
}
