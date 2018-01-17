#include "compiler.h"
/*todo
 * 副プログラム宣言内でグローバル宣言を使っている
 * クロスリファレンサに対して変なところでえんbしている
 * */

/* keyword list */
struct KEY key[KEYWORDSIZE] = {
		{"and", 	TAND	},
		{"array",	TARRAY	},
		{"begin",	TBEGIN	},
		{"boolean",	TBOOLEAN},
		{"break",	TBREAK  },
		{"call",	TCALL	},
		{"char",	TCHAR	},
		{"div",		TDIV	},
		{"do",		TDO	},
		{"else",	TELSE	},
		{"end",		TEND	},
		{"false",	TFALSE	},
		{"if",		TIF	},
		{"integer",	TINTEGER},
		{"not",		TNOT	},
		{"of",		TOF	},
		{"or",		TOR	},
		{"procedure", TPROCEDURE},
		{"program",	TPROGRAM},
		{"read",	TREAD	},
		{"readln",	TREADLN },
		{"return", 	TRETURN },
		{"then",	TTHEN	},
		{"true",	TTRUE	},
		{"var",		TVAR	},
		{"while",	TWHILE	},
		{"write",	TWRITE  },
		{"writeln",	TWRITELN}
};


int main(int nc, char *np[]) {
	if(nc < 2) {
		printf("[ERROR]File name id not given.\n");
		return 0;
	}
	if(init_scan(np[1]) < 0) {
		printf("[ERROR]File %s can not open.\n", np[1]);
		return 0;
	}
    if((fp_out = init_output(np[1])) == NULL) {
        printf("[ERROR]File %s can not make.\n", np[1]);
        return 0;
    }

	parse_program();
    out_str_labels();
	out_commmon_library();

	printf("[Result]Search end of line:%d.\n",get_linenum());
	end_scan();
	return 0;
}


void error(char *mes) {
	printf("\n[SCAN_ERROR]%s\n\n", mes);
}
