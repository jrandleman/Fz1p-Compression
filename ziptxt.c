/* AUTHOR: JORDAN RANDLEMAN -:- ZIPTXT TXT FILE LOSSLESS COMPRESSION/ENCRYPTION */
/*********************************************************************************
* /==//  ==== /|==\\ [====] \\// [====]       //==\ //==\\ /\\  //\ /|==\\ ||^\\ *
*   //    ||  ||==//   ||    )(    ||   +===+ ||    ||  || ||\\//|| ||==// ||_// *
*  //==/ ==== ||       ||   //\\   ||         \\==/ \\==// || \/ || ||     || \\ *
*********************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define MAX_CH 1000000
#define ADD_FILENAME_EXTENSION(SEED,EXTEND,APPEND) ({strcpy(EXTEND,SEED);strcpy(&EXTEND[strlen(EXTEND)],APPEND);})
#define RMV_FILENAME_EXTENSION(FNAME,LEN) (FNAME[strlen(FNAME)-LEN]='\0')
#define HAS_EXTENSION(S,EXT) (strcmp(&S[strlen(S)-strlen(EXT)], EXT) == 0)
#define IS_CAP_CH(ch_c_inst) ((ch_c_inst) >= 'A' && (ch_c_inst) <= 'Z')
#define IS_LOW_CH(CH) ((CH) >= 'a' && (CH) <= 'z')
#define IS_ALF_CH(CH) (IS_CAP_CH((CH)) || IS_LOW_CH((CH)))
#define IS_ABBREV(X,PUNC,Y,Z) ((X) == (PUNC) && IS_ALF_CH((Y)) && (!IS_ALF_CH((Z))))
#define VALID_CAP(W,X,Y,Z) ((W) == ' ' && !(IS_PUNC((X))) && (Y) !=' ' && (IS_CAP_CH((Z)) || (!IS_ALF_CH((Z)) && IS_CAP_CH((Y)))))
#define IS_COMMA(X,Y,Z) ((X) == '-' && ((Y) == '_' || (Y) == '\'') && (Z) != ' ')
#define IS_I_OR_C_(X,Y) (((X)=='c' || (X)=='i') && ((Y)=='_' || (Y)=='\n'))
#define DUB_QUOTE(X,Y,Z) ((X) == '\'' && ((Y) == ' ' || (Z) == '_' || IS_PUNC((Z))))
#define IS_PUNC(CH) ((CH) == '.' || (CH) == '!' || (CH) == '?')
#define CEIL(N,D) (1 + (((N) - 1) / (D)))
#define SS_KEY_UCH_TOT(str_total) ((str_total) - ((str_total) / 8))
#define SS_KEY_STR_TOT(uch_total) ((uch_total) + ((uch_total) / 7))
#define myAssert(C,M) ({if(C==NULL){printf("\n=> myAssert Failed: %s, %d, %s\n%s\n",__FUNCTION__,__LINE__,#C,M);exit(0);}})
/* HIDE / SHOW FUNCTIONS */
void hide(char *, char *);
void show(char *, char *);
void err_info();
/* TEXT FILE HANDLER FUNCTIONS */
void show_txt_compressed(char *, char *);
void read_txt_engl(char *, char *);
void write_txt_compressed(char *);
/* BINARY FILE HANDLER FUNCTIONS */
void read_pass_ss_keys(char *);
void write_ss_keys_to_single_str(char [][151], int, char *);
/* COMMON WORD SUBSTITUTION FUNCTIONS */
void splice_str(char *, char *, int, int);
int delta_sub_common_words(char *, int, char [][50], char [][50]);
void print_cw_used(int);
/* SUB NUMBER OF SEQUENTIAL LETTERS & VISE-VERSA */
void sequential_letters(char *);
void sequential_numbers(char *);
/* ENCRYPTION FUNCTION */
void delta_txt_crypt(char *, char *);
/* STRING EDITING FUNCITONS */
void rm_nxt_ch(char *, int);
void modify_str(char *, int);
/* CAPITAL WORD INDEX STORAGE */
void delta_sub_capital_letters(char [], int);
typedef struct capitalized_words {
	unsigned short cap_idxs[MAX_CH], length;
} CAP_WORD;
CAP_WORD caps;
/* EDIT COMMON WORD (CW) SUBS IF KEY IN TEXT (ANOMALY) */
void hide_adjust_cw_keys(char []);
void show_adjust_cw_keys(unsigned char);
void shift_up_cw_keys(int);
void modify_key(char *, char *);
/* DE/COMPRESSION FUNCTIONS */
void s_decompress(char [][151], char *);
int s_compress(char [][151], char *);
int process_split_s(char [][151], char *, char *, char *);
/* (S)UB(S)TRING REFERENCE KEY ANALYSIS FUNCTIONS */
int is_ss_ref(int, char);
int ss_ch_index(char);
char adjust_ref_char(char);
/* (S)UB(S)TRING FUNCTIONS */
void store_ss(char [][151], char *, int);
int clean_ss(char [][151], char *, int);
void splice_ss(char [][151], int [45], int, int);
void trim_ss(char [][151], char *, int [45], int, int);
void print_ss(char [][151], int);
/* SS_KEY BIT-PACKING COMPRESSION/DECOMPRESSION FUNCTIONS */
int pack_ss_keys(char *);
void unpack_ss_keys(char *);
void bpack_wipe_garbage_mem(unsigned char [], int);
void swap_str_sign(char [], unsigned char [], int, int);
/* COMMON WORD SUBSTITUTIONS */
#define FULL_CW_LEN 255
char cw_keys[FULL_CW_LEN][50] = { /* roman numerals */
	"_2_", "_3", "_4_", "_5_", "_6_", "_7", "_8", "_9_",
	"_2\n", "_4\n", "_5\n", "_6\n", "_9\n",
	/* numbers & punctuation */
	"_jq", "_jw", "_jx", "_jy", "_jz", "_jt", "_js", "_jp", "_jn", "_jm",
	"_jb", "jc_", "_jd_", "._?-", "!_?-", "?_?-", "_?-", "--", ".-", "!-", 
	/* single letter */
	"_t_", "_o_", "_s_", "_w_", "_p_", "_d_", "_n_", "_r_", "_y_", "_f_", "_l_", "_h_", 
	"_b_", "_g_", "_m_", "_e_", "_v_", "_u_", "_j_", "_x_", "_k_", "_z_", "_q_", 
	/* two-letter 1 */
	"_te_", "_ts_", "_tt_", "_td_", "_tn_", "_tr_", "_ty_", "_tf_", "_tl_", "_tg_", "_th_", 
	"_ta_", "_tk_", "_tm_", "_tp_", "_tu_", "_tw_", "_tc_", "_ti_", "_ae_", "_ar_", "_ay_", 
	"_af_", "_al_", "_ao_", "_ag_", "_ah_", "_aa_", "_ak_", "_ap_", "_au_", "_aw_", "_ac_", 
	"_ai_", "_oe_", "_os_", "_ot_", "_od_", "_oy_", "_ol_", "_oo_", "_og_", "_ie_", "_id_", 
	"_ir_", "_iy_", "_il_", "_io_", "_ig_", "_ih_", "_se_", "_ss_", "_st_", "_sd_", "_sn_", 
	"_sr_", "_sy_", "_sf_", "_sl_", "_sg_", "_sh_", "_ws_", "_wt_", "_wd_", "_wn_", 
	"_wr_", "_wy_",
	/* two-letter 2 */
	"_wf_", "_wl_", "_wo_", "_wg_", "_wh_", "_ce_", "_cs_", "_ct_", "_cd_", "_cn_", 
	"_cr_", "_cy_", "_cf_", "_cl_", "_co_", "_cg_", "_ch_", "_bs_", "_bt_", "_bd_", 
	"_bb_", "_br_", "_bf_", "_bl_", "_bo_", "_bg_", "_pe_", "_ps_", "_pt_", "_pd_", 
	"_pb_", "_pr_", "_py_", "_pf_", "_pl_", "_po_", "_pg_", "_hs_", "_ht_", 
	"_hd_", "_hn_", "_hr_", "_hy_", "_hf_", "_hl_", "_ho_", "_hg_", "_fe_", "_fs_", 
	"_ft_", "_fd_",
	/* no-space */
	"`e", "!e", "?e", "`t", "!t", "?t", "`a", "!a", "?a", "`o", "!o", "?o", "`i", "!i", "?i", 
	"`n", "!n", "?n", "`s", "!s", "?s", "`h", "!h", "?h", "`r", "!r", "?r", "`d", "!d", "?d", 
	"`l", "!l", "?l", "`c", "!c", "?c", "`u", "!u", "?u", "`m", "!m", "?m", "`w", "!w", "?w", 
	"`f", "!f", "?f", "`g", "!g", "?g", "`y", "!y", "?y", "`p", "!p", "?p", "`b", "!b", "?b", 
	"`v", "?v", "!v", "`k", "!k", "?k", "`j", "?j", "!j", "`x", "?x", "!x", "`z", "`q", "?`",
	"!?", "?!", "!`", "`?", "`!",
};
char cw_word[FULL_CW_LEN][50] = { /* roman numerals */
	"_ii_", "_iii", "_iv_", "_v_", "_vi_", "_vii", "_iix", "_ix_",
	"_ii\n", "_iv\n", "_v\n", "_vi\n", "_ix\n",
	/* numbers & punctuation */
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
	"(", ")", "\t", ".\n", "!\n", "?\n", "\n", ":", ";", "/", 
	/* single letter */
	"_at_", "_as_", "_an_", "_be_", "_by_", "_do_", "_are_", "_in_", "_is_", "_it_", "_my_", "_of_", 
	"_on_", "_or_", "_to_", "_and_", "_the_", "_have_", "_that_", "_this_", "_with_", "_you_", "_up_",
	/* two-letter 1 */
	"_any_", "_all_", "_but_", "_can_", "_day_", "_get_", "_him_", "_his_", "_her_", "_how_", "_now_", 
	"_new_", "_out_", "_one_", "_our_", "_two_", "_use_", "_way_", "_who_", "_its_", "_say_", "_see_", 
	"_she_", "_also_", "_come_", "_from_", "_for_", "_back_", "_give_", "_good_", "_just_", "_into_", "_know_", 
	"_look_", "_like_", "_make_", "_most_", "_them_", "_over_", "_only_", "_some_", "_time_", "_take_", "_they_", 
	"_work_", "_want_", "_will_", "_year_", "_your_", "_what_", "_when_", "_then_", "_than_", "_could_", "_well_", 
	"_because_", "_even_", "_these_", "_which_", "_people_", "_about_", "_after_", "_first_", "_other_", "_think_", 
	"_there_", "_their_", 
	/* two-letter 2 */
	"_ask_", "_big_", "_bad_", "_eye_", "_few_", "_man_", "_own_", "_old_", "_try_", "_able_", 
	"_case_", "_call_", "_fact_", "_find_", "_feel_", "_hand_", "_high_", "_life_", "_last_", "_long_", 
	"_next_", "_part_", "_seem_", "_same_", "_tell_", "_week_", "_child_", "_company_", "_different_", "_early_", 
	"_government_", "_group_", "_great_", "_important_", "_leave_", "_little_", "_large_", "_number_", "_not_", 
	"_person_", "_place_", "_point_", "_problem_", "_public_", "_right_", "_small_", "_thing_", "_world_", "_woman_", 
	"_young_", "_would_", 
	/* no-space */
	"there", "tion", "then", "than", "well", "them", "some", "will", "that", "this", 
	"with", "come", "back", "nter", "for", "ear", "different", "ent", "end", "see", 
	"eye", "man", "use", "even", "able", "thing", "ing", "know", "now", "hand", 
	"and", "make", "its", "not", "other", "the", "you", "any", "call", "ally", "all", 
	"can", "most", "group", "over", "time", "take", "work", "place", "point", "right", 
	"into", "person", "high", "last", "long", "part", "tell", "she", "ask", "big", 
	"bad", "after", "ess", "but", "own", "ere", "try", "her", "how", "new", "out", 
	"one", "our", "case", "way", "who", "ate", "get", "his",
};
/* GLOBAL VARIABLES */
unsigned char CW_LEN = FULL_CW_LEN, cw_shift_up_idxs[FULL_CW_LEN]; /* store indices of rmvd cw_keys */
char ss_array_matrix[300][45][151], s_compress_storage[300][151], s_max_buffer[MAX_CH], ss_single_str[MAX_CH];
char ss_refs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01789#$<=>@[]^{|}~\0", nchar = '\n';
int cw_idxs[FULL_CW_LEN], chunk_count = 0, original_bytes = 0, compressed_bytes = 0, zip_info = 0;
int main(int argc, char *argv[]) {
	if(argc < 3 || argc > 4) err_info();
	char arg1[75], arg2[75];
	if(argc == 4) {
		strcpy(arg1, argv[2]); strcpy(arg2, argv[3]);
		(strcmp(argv[1], "-l") == 0) ? (zip_info = 1) : (printf("\n\n(!!!) WRONG INFO FLAG => -l (!!!)\n\n"));
	} else { strcpy(arg1, argv[1]); strcpy(arg2, argv[2]); }
	if(HAS_EXTENSION(arg1, ".TZ1P.txt")) { show(arg2, arg1); }
	else if(HAS_EXTENSION(arg1, ".txt")) { hide(arg2, arg1); }
	else { err_info(); }
	return 0;
}
/******************************************************************************
* HIDE / SHOW FUNCTIONS
******************************************************************************/
void hide(char *arg2, char *arg1) {
	char arg1seed[75], filename[75];
	strcpy(arg1seed, arg1);
	RMV_FILENAME_EXTENSION(arg1seed,strlen(".txt"));
	ADD_FILENAME_EXTENSION(arg1seed,filename,".TZ1P.txt");
	read_txt_engl(arg2, arg1); /* process text file in 150 char chunks */
	int ss_keys_byte = pack_ss_keys(arg2);
	write_txt_compressed(arg1); /* write compressed string to text file */
	/* output results */
	compressed_bytes += ss_keys_byte;
	float bytes_saved = 100*(1.000000 - ((float)compressed_bytes)/((float)original_bytes));
	printf("\n==============================================================================");
	printf("\n%s ==COMPRESS=ENCRYPT=> %s\n", arg1, filename);
	printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n", original_bytes, compressed_bytes, bytes_saved);
	printf("==============================================================================\n\n");
}
void show(char *arg2, char *arg1) {
	char filename[75], arg1compr[75], arg1dcmpr[75];
	strcpy(arg1compr, arg1);
	ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
	unpack_ss_keys(arg2);
	read_pass_ss_keys(arg2);
	show_txt_compressed(arg2, arg1);
	printf("\n>>> FILE REMOVED: %s\n", filename);
	ADD_FILENAME_EXTENSION(arg1,arg1compr,".TZ1P.txt");
	ADD_FILENAME_EXTENSION(arg1,arg1dcmpr,".txt");
	printf("\n==============================================================================");
	printf("\n>> %s ==DECOMPRESS=DECRYPT=> %s\n", arg1compr, arg1dcmpr);
	printf("==============================================================================\n\n");
}
void err_info() {
	printf("\n============================= INVALID EXECUTION! =============================\n");
	printf("$ gcc -o ziptxt ziptxt.c\n<ENCRYPT/COMPRESS>   $ ./ziptxt filename.txt yourpassword\n");
	printf("<DECRYPT/DECOMPRESS> $ ./ziptxt filename.TZ1P.txt yourpassword\n");
	printf("==============================================================================");
	printf("\n=> ENCRYPT/COMPRESS INFO: $ ./ziptxt -l filename.extension yourpassword\n");
	printf("==============================================================================\n\n");
	exit(0);
}
/******************************************************************************
* TEXT FILE HANDLER FUNCTIONS
******************************************************************************/
void show_txt_compressed(char *arg2, char *arg1) {
	FILE *fp;
	char s_buffer[150], filename[75];
	int s_chunk_number = 0, i;
	myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING COMPRESSED TEXT FILE (!!!)\n");
	while(fgets(s_buffer, 150, fp) > 0) { /* GET COMPRESSED TEXT */
		char *p = s_buffer;
		while(*(p + 1) != '\0') p++;
		*p = '\0'; /* remove \n from s_buffer */
		if(zip_info == 1) printf("\nENCRYPTED/COMPRESSED: %s\n", s_buffer);
		delta_txt_crypt(s_buffer, arg2); /* decrypt text */
		sequential_numbers(s_buffer);
		s_decompress(ss_array_matrix[s_chunk_number], s_buffer);
		delta_sub_common_words(s_buffer, 151, cw_keys, cw_word); /* remove keys, insert words */
		if(zip_info == 1) printf("DECRYPTED/DECOMPRESS: %s\n", s_buffer);
		strcpy(s_compress_storage[s_chunk_number], s_buffer);
		s_chunk_number++;
	}
	for(i = 0; i < s_chunk_number; i++) {
		char *p = s_max_buffer;
		if(i != 0) while(*p != '\0') p++;
		sprintf(p, "%s", s_compress_storage[i]);
	}
	fclose(fp);
	remove(arg1);
	/* PUT DECOMPRESSED TEXT */
	modify_str(s_max_buffer, 0); /* '_' => ' ' */
	RMV_FILENAME_EXTENSION(arg1,strlen(".TZ1P.txt"));
	ADD_FILENAME_EXTENSION(arg1,filename,".txt");
	myAssert((fp = fopen(filename, "w")), "\n\n(!!!) ERROR WRITING DECOMPRESSED TEXT FILE (!!!)\n");
	fwrite(s_max_buffer, sizeof(char), strlen(s_max_buffer), fp);
	fclose(fp);
	return;
}
void read_txt_engl(char *arg2, char *arg1) {
	char ss[45][151], s_chunk[151], temp_max_buff[MAX_CH];
	int ret;
	FILE *fp;
	myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING TEXT FILE (CAPITAL STORING) (!!!)\n\n"); /* FILL CAPITALS/ADJUST CW */
	fread(temp_max_buff, sizeof(char), MAX_CH, fp);
	fclose(fp);
	delta_sub_capital_letters(temp_max_buff, 1);
	hide_adjust_cw_keys(temp_max_buff);
	myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING TEXT FILE (TEXT STORING) (!!!)\n\n"); /* COMPRESS TEXT */
	while((ret = fread(s_chunk, sizeof(char), 150, fp)) > 0) {
		s_chunk[ret] = '\0';
		original_bytes += ret;
		compressed_bytes += process_split_s(ss, s_chunk, s_compress_storage[chunk_count], arg2);
		chunk_count++;
	}
	fclose(fp);
}
void write_txt_compressed(char *arg1) {
	FILE *fp;
	int i;
	char arg1seed[75], filename[75];
	strcpy(arg1seed, arg1);
	RMV_FILENAME_EXTENSION(arg1seed,strlen(".txt"));
	ADD_FILENAME_EXTENSION(arg1seed,filename,".TZ1P.txt");
	myAssert((fp = fopen(filename, "w")), "\n\n(!!!) ERROR WRITING COMPRESSED TEXT TO FILE (!!!)\n");
	for(i = 0; i < chunk_count; i++) {
		fwrite(s_compress_storage[i], sizeof(char), strlen(s_compress_storage[i]), fp);
		fwrite(&nchar, sizeof(char), 1, fp);
	}
	fclose(fp);
	remove(arg1);
}
/******************************************************************************
* BINARY FILE HANDLER FUNCTIONS
******************************************************************************/
void read_pass_ss_keys(char *arg2) {
	char ss_buffer[100], ch_buffer, filename[75];
	ADD_FILENAME_EXTENSION(arg2, filename, "_sskey.txt");
	int n = 0, m = 0; /* m - 2D ss array (sentence), n - ss (word) => WRT ss_array_matrix[m][n][] */
	FILE *fp;
	myAssert((fp = fopen(filename, "r")), "\n\n(!!!) ERROR READING TEMP SS_KEY PASSWORD TEXT FILE (!!!)\n\n");
	while(fscanf(fp, "%s%c", ss_buffer, &ch_buffer) > 0) {
		char *p = ss_buffer, *q = ss_array_matrix[m][n];
		while(*p != '\0') *q++ = *p++; /* copy ss from bin to 2d ss array in 3d matrix */
		*q = '\0';  /* terminate ss */
		if(ch_buffer == '\n') {
			ss_array_matrix[m][n+1][0] = '\0'; /* terminate ss 2d array */
			m++; /* move to next 2d ss array (1 per s_chunk) in 3d matrix */
			n = 0; /* start from row (ss) 0 */
		} else {
			n++; /* move to next row (ss) */
		}
	}
	fclose(fp);
	remove(filename); /* delete password txt file once ss keys retrieved */
}
void write_ss_keys_to_single_str(char ss[][151], int ss_total, char *arg2) {
	int i;
	for(i = 0; i < ss_total; i++) {
		sprintf(&ss_single_str[strlen(ss_single_str)], "%s", ss[i]);
		if(i < ss_total - 1) sprintf(&ss_single_str[strlen(ss_single_str)], "%c", '\t');
	}
	sprintf(&ss_single_str[strlen(ss_single_str)], "%c", '\n');
}
/******************************************************************************
* COMMON WORD SUBSTITUTION FUNCTIONS
******************************************************************************/
void splice_str(char *s, char *sub, int splice_len, int size) {
	char temp[size];
	sprintf(temp, "%s%s", sub, s + splice_len);
	strcpy(s, temp);
}
int delta_sub_common_words(char *s, int total_len, char remove[][50], char insert[][50]) {
	int count = 0, found, word_len, i, j;
	for(i = 0; i < CW_LEN; i++) {
		char *p = s;
		word_len = strlen(remove[i]);
		found = 0;
		while(*p != '\0') {
			if(*p == remove[i][0]) {
				for(j = 0; j < word_len; j++) if(*(p + j) != remove[i][j]) break;
				if(j == word_len) { /* if word in s is common word */
					splice_str(p, insert[i], word_len, total_len);
					if(zip_info == 1 && found == 0) { cw_idxs[count++] = i; found++; } /* store common words for 'info' command */
				}
			} else if(*p == '`' || *p == '!' || *p == '?') { p++; } /* avoid chaining words: themself => !uself => !?slf */
			p++;
		}
	}
	return count;
}
void print_cw_used(int sub_size) {
	printf("SUBSTITUTIONS:\n");
	int i; for(i = 0; i < sub_size; i++) printf("=> %s: %s\n", cw_keys[cw_idxs[i]], cw_word[cw_idxs[i]]);
}
/******************************************************************************
* SUB NUMBER OF SEQUENTIAL LETTERS & VISE-VERSA
******************************************************************************/
void sequential_letters(char *s) {
	char *p = s, count_s[2];
	int count;
	while(*(p + 1) != '\0') {
		if(*(p + 1) == (*p + 1) && *(p + 2) == (*p + 2)) {
			for(count = 3; (count < 7 && *(p + count) == (*p + count)); count++);
			sprintf(count_s, "%d", --count);
			splice_str(p + 1, count_s, count, 151);
		}
		p++;
	}
}
void sequential_numbers(char *s) {
	char *p = s, ch_arr[8];
	while(*p != '\0') {
		if(*p >= '2' && *p <= '6') {
			int num = (int)(*p - 48), i;
			for(i = 1; i <= num; i++) ch_arr[i - 1] = (*(p - 1) + i);
			ch_arr[i - 1] = '\0';
			splice_str(p, ch_arr, 1, 151);
		}
		p++;
	}
}
/******************************************************************************
* ENCRYPTION FUNCTION
******************************************************************************/
void delta_txt_crypt(char *char_array, char *arg2) {
	char *pass = arg2, *s = char_array;
	int key_size = CEIL(strlen(s),strlen(pass)), ch_count = 0, idx = 0;
	srand((char)pass[0]);
	while(*s != '\0') {
		if(ch_count == key_size) {
			ch_count = 0;
			idx++;
			srand((char)pass[idx]);
		}
		*s ^= (rand()%11+5);
		ch_count++;
		s++;
	}
}
/******************************************************************************
* STRING EDITING FUNCITONS
******************************************************************************/
void rm_nxt_ch(char *q, int ss_ref_idx) {
	sprintf(q, "%c%s", ss_refs[ss_ref_idx], q + 1); /* replace 1st char with symbol */
	memmove(q + 1, q + 2, strlen(q + 1)); /* remove 2nd char */
}
void modify_str(char *s, int sp_flag) {
	char *p = s, space = ' ', under_s = '_';
	if(sp_flag == 1) { /* (encrypt) */
		while(*p != '\0') {
			*p = tolower(*p); /* make lowercase */
			if(*p == space) { /* ' ' => '_' */
				*p = under_s;
			} else if(*p == ','|| *p == '_') { /* sub punctuation with dash */
				*p = '-';
			} else if(IS_ABBREV(*p, '.', *(p+1), *(p+2))) { /* handle abbreviations via dash (ie u.s. => u-s.) */
				if(!(IS_CAP_CH(*(p+4)))) *(p+2) = '-'; /* don't capitalize words post-abrev lest new sentence ('.' => '-') */
				*p = '-';
			} else if(*p == '['|| *p == '{') { /* parenthesis substitutions */
				*p = '(';
			} else if(*p == ']'|| *p == '}') { /* parenthesis substitutions */
				*p = ')';
			} else if(*p == '"') { *p = '\''; } /* double quotes => single quotes */
			p++;
		}
	} else { /* (decrypt) */
		if(IS_LOW_CH(*p)) *p ++ -= 32; /* capitalize first letter in file */
		while(*p != '\0') {
			if(*p == under_s) { /* '_' => ' ', capitalize ' I ' && ' C ' (if applicable) */
				*p = space;
				if(IS_I_OR_C_(*(p+1), *(p+2))) *(p+1) = toupper(*(p+1));
			} else if(IS_COMMA(*p, *(p+1), *(p-1))) { /* comma -_ => ,_ */
				*p = ',';
			} else if(IS_ABBREV(*p, '-', *(p+1), *(p+2))) { /* abbrev's & tag ('-' => '@') any mid-sentence */
				if(*(p+2) == '-') *(p+2) = '@';
				*p = '.';
				*(p+1) = toupper(*(p+1));
			} else if(IS_PUNC(*p) && *(p+1) == '_' && IS_LOW_CH(*(p+2))) { /* capitalize letters after . ! ? */
				*(p+2) = toupper(*(p+2));
			} else if(*p=='\n' && IS_LOW_CH(*(p+1))) { /* capitalize letters after \n */
				*(p+1) = toupper(*(p+1));
			} else if(DUB_QUOTE(*p, *(p-1), *(p+1))) { *p = '"'; }/* sub double quotes back in */
			if(p > &s[2] && *(p-2) == '@') *(p-2) = '.'; /* ('@' => '.') if mid-sentence abbreviation */
			p++;
		}
		if(s[strlen(s)-1] == '@') s[strlen(s)-1] = '.'; /* in case file ends with an abbreviation */
		delta_sub_capital_letters(s, 0);
	}
}
/******************************************************************************
* CAPITAL WORD INDEX STORAGE
******************************************************************************/
void delta_sub_capital_letters(char s[], int pack_flag) {
	char *p = s + 1;
	caps.length = 0;
	unsigned short word_idxs = 0, i = 0;
	while(*(p + 1) != '\0') {
		if(*p == ' ' || *p == '\n' || *p == '\t') { /* if is a word */
			word_idxs++;
			if(pack_flag == 1) { /* pack - get word indices of capital letters */
				if(VALID_CAP(*p,*(p-1),*(p+2),*(p+1))) caps.cap_idxs[caps.length ++] = word_idxs; /* if word capitalized */
			} else { /* unpack - capitalize */
				if(caps.cap_idxs[i] == word_idxs) { (!IS_ALF_CH(*(p+1))) ? (*(p+2) = toupper(*(p+2))) : (*(p+1) = toupper(*(p+1))); i++; }
			}
		}
		p++;
	}
}
/******************************************************************************
* EDIT COMMON WORD (CW) SUBS IF KEY IN TEXT (ANOMALY)
******************************************************************************/
void hide_adjust_cw_keys(char s[]) {
	int word_len, i, j;
	for(i = 0; i < CW_LEN; i++) {
		char *p = s, modded_key[50];
		word_len = strlen(cw_keys[i]);
		modify_key(modded_key, cw_keys[i]);
		if(modded_key[1] >= '2' && modded_key[1] <= '9') continue;
		while(*p != '\0') {
			if(*p == modded_key[0]) {
				for(j = 0; j < word_len; j++) if(*(p + j) != modded_key[j]) break;
				if(j == word_len) {
					shift_up_cw_keys(i);
					cw_shift_up_idxs[FULL_CW_LEN - (CW_LEN--)] = (unsigned char)i;
				}
			}
			p++;
		}
	}
}
void show_adjust_cw_keys(unsigned char length) {
	int i; for(i = 0; i < length; i++) { shift_up_cw_keys((int)cw_shift_up_idxs[i]); CW_LEN--; }
}
void shift_up_cw_keys(int idx) {
	int i; for(i = idx; i < (CW_LEN - 1); i++) { strcpy(cw_keys[i],cw_keys[i+1]); strcpy(cw_word[i],cw_word[i+1]);}
}
void modify_key(char *modded_key, char *key) {
	strcpy(modded_key, key);
	int i; for(i = 0; i < strlen(key); i++) if(key[i] == '_') modded_key[i] = ' ';
}
/******************************************************************************
* DE/COMPRESSION FUNCTIONS
******************************************************************************/
void s_decompress(char ss[][151], char *s) {
	char *p = s, t[151];
	while(*p != '\0') {
		if(is_ss_ref(0, *p) == 1) {
			sprintf(t, "%s%s", ss[ss_ch_index(*p)], p+1);
			sprintf(p, "%s", t);
			p = s;
		} else { p++; }
	}
}
int s_compress(char ss[][151], char *s) { /* returns # of substrings */
	modify_str(s, 1); /* lowify & ' ' => '_' */
	int sub_word_count = delta_sub_common_words(s, 151, cw_word, cw_keys); /* remove common words, insert keys */
	if(zip_info == 1) {
		printf("SUBD WORDS => LEN: %lu, STR: %s\n", strlen(s), s);
		print_cw_used(sub_word_count); /* print common words subbed and associated keys */
	}
	int ss_idx = 0, found;
	char *p = s, *r;
	while(*(p + 2) != '\0') { /* first ss instance */
		r = p + 2;
		found = 0;
		while(*(r+1) != '\0') { if(*r == *p && *(r+1) == *(p+1)) { found++; rm_nxt_ch(r, ss_idx); } r++; } /* second ss instance */
		if(found > 0) { /* if ss found, restart search for compounded ss */
			store_ss(ss, p, ss_idx);
			rm_nxt_ch(p, ss_idx);
			ss_idx++;
			p = s;
		} else { p++; }
	}
	return clean_ss(ss, s, ss_idx);
}
int process_split_s(char ss[][151], char *s, char *s_compress_storage, char *arg2) {
	if(zip_info == 1) printf("\nORIGINALLY => LEN: %lu, STR: %s\n", strlen(s), s);
	int ss_total = s_compress(ss, s);
	if(zip_info == 1) printf("COMPRESSED => LEN: %lu, STR: %s\n", strlen(s), s);
	sequential_letters(s);
	int compressed_bytes = strlen(s) + 1; /* compressed string length */
	write_ss_keys_to_single_str(ss, ss_total, arg2); /* store ss keys in bin"password"file */
	delta_txt_crypt(s, arg2); /* encrypt text */
	strcpy(s_compress_storage, s); /* store compressed string */
	if(zip_info == 1) printf("ENCRYPTION => LEN: %lu, STR: %s\n", strlen(s), s);
	if(zip_info == 1) print_ss(ss, ss_total);
	return compressed_bytes;
}
/******************************************************************************
* (S)UB(S)TRING REFERENCE KEY ANALYSIS FUNCTIONS
******************************************************************************/
int is_ss_ref(int start, char ch) {
	char *p = &ss_refs[start];
	while(*p != '\0') if(*p ++ == ch) return 1;
	return 0;
}
int ss_ch_index(char ch_c_idx) {
	if(IS_CAP_CH(ch_c_idx) == 1) { return (int)(ch_c_idx - 65); }
	else if((ch_c_idx >= '7' && ch_c_idx <= '9') || (ch_c_idx >= '<' && ch_c_idx <= '>')) { return (int)(ch_c_idx - 27); }
	char is[] = "01#$@[]^", diff[] = {22,22,4,4,28,54,50,50};
	int i; for(i = 0; i < 8; i++) if(ch_c_idx == is[i]) return (int)(ch_c_idx - diff[i]);
	return (int)(ch_c_idx - 83);
}
char adjust_ref_char(char ss_ref_ch) {
	char is[] = "07#<@][{", diff[] = {-42,6,-22,24,2,2,27,29};
	int i; for(i = 0; i < 8; i++) if(ss_ref_ch == is[i]) return (char)(ss_ref_ch - diff[i]);
	return (char)(ss_ref_ch - 1);
}
/******************************************************************************
* (S)UB(S)TRING FUNCTIONS
******************************************************************************/
void store_ss(char ss[][151], char *p, int ss_idx) { sprintf(ss[ss_idx], "%c%c", *p, *(p + 1)); }
int clean_ss(char ss[][151], char *s, int ss_total) {
	char *p;
	int nest_idx = 0, nested_ss[45], i; /* nested ss only ref'd by other ss, arr holds their idxs */
	for(i = 0; i < ss_total; i++) {
		p = s;
		while(*p != '\0') { if(*p == ss_refs[i]) { break; } p++; }
		/* if ss_refs[i] !in s, i == nested ss ref idx */
		if(*p == '\0') nested_ss[nest_idx++] = i; /* nested ss idxs stored sequentially relative to ss_refs order */
	}
	splice_ss(ss, nested_ss, nest_idx, ss_total); /* put nested ss vals in ss referencing them */
	trim_ss(ss, s, nested_ss, nest_idx, ss_total); /* adjust ss & s references as per rmvd nested ss */
	return (ss_total - nest_idx); /* actual ss total = original ss total - nested ss total */
}
void splice_ss(char ss[][151], int nested_ss[45], int nest_total, int ss_total) {
	int i, j, l;
	for(i = 0; i < nest_total; i++) { /* traverse nested ss array */
		for(j = 0; j < ss_total; j++) { /* traverse ss array */
			int curr_ss_len = strlen(ss[j]);
			for(l = 0; l < curr_ss_len; l++) { /* traverse ss */
				if(ss[j][l] == ss_refs[nested_ss[i]]) { /* if ss has nested ss reference */
					char nested_elem[151]; /* clone to hold nested ref value */
					strcpy(nested_elem, ss[nested_ss[i]]); /* clone nested ref val */
					strcpy(&nested_elem[strlen(nested_elem)], &ss[j][l + 1]); /* splice post-nested ref ss tail to end of nested ref val */
					strcpy(&ss[j][l], nested_elem); /* splice nested ref val + ss end to ss ref position */
				}
			}
		}
	}
}
void trim_ss(char ss[][151], char *s, int nested_ss[45], int nest_total, int ss_total) {
	int i, j, l;
	for(i = 0; i < nest_total; i++) { /* delete nested-ss */
		char *p = s;
		for(j = nested_ss[i]; j < (ss_total - 1); j++) {
			strcpy(ss[j], ss[j + 1]); /* shift up non-nested ss to delete nested-ss */
			int curr_ss_len = strlen(ss[j]);
			/* ss-ss refs -= 1 ss_refs val per del  */
			for(l = 0; l < curr_ss_len; l++) if(is_ss_ref((nested_ss[i] + 1), ss[j][l]) == 1) ss[j][l] = adjust_ref_char(ss[j][l]);
		}
		for (int k = i; k < nest_total; k++) nested_ss[k] = adjust_ref_char(nested_ss[k]); /* nested-ss idxs -= 1 ss_refs val per del */
		ss[j][0] = '\0';
		while(*p != '\0') { /* s ss references -= 1 ss_refs val as per del */
			if(is_ss_ref((nested_ss[i]+ 1), *p) == 1) { *p = adjust_ref_char(*p); } p++;/* if *p = nested ref */
		}
	}
}
void print_ss(char ss[][151], int ss_total) {
	printf("COMPR SUBSTRS:\n");
	int i; for(i = 0; i < ss_total; i++) printf("=> %c: %s\n", ss_refs[i], ss[i]);
}
/******************************************************************************
* SS_KEY BIT-PACKING COMPRESSION/DECOMPRESSION FUNCTIONS
******************************************************************************/
int pack_ss_keys(char *arg2) {
	/* PACK SS_KEYS */
	int passed_length = strlen(ss_single_str), char_total = SS_KEY_UCH_TOT(strlen(ss_single_str)), i, j, k;
	unsigned char cw_write_length = FULL_CW_LEN - CW_LEN, packed_str[char_total], passed_uch[passed_length];
	unsigned short char_total_uch = (unsigned short)char_total, cap_len = caps.length;
	bpack_wipe_garbage_mem(packed_str, char_total); /* init with 0's to clear garbage memory pre-bitpacking */
	swap_str_sign(ss_single_str, passed_uch, passed_length, 1); /* convert passed string into uchar str */
	for(i = 0, j = 0, k = 0; i < char_total; i++, j++, k++) { /* PACK CHARS */
		packed_str[i] = (((passed_uch[k] << (j + 1)) | (passed_uch[k + 1] >> (6 - j))) & 255);
		if((i + 1) % 7 == 0 && i != 0) { j = -1; k++; }
	}
	/* WRITE PACKED SS KEYS TO FILE */
	FILE *fp;
	char filename[75];
	ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
	myAssert((fp = fopen(filename, "wb")),"\n (!!!) ERROR WRITING PACKED SS KEYS TO BINARY FILE (!!!) \n");
	fwrite(&cw_write_length, sizeof(unsigned char), 1, fp); /* write modified cw_keys idxs length */
	fwrite(cw_shift_up_idxs, sizeof(unsigned char), cw_write_length, fp); /* write modified cw_keys idxs array */
	fwrite(&cap_len, sizeof(unsigned short), 1, fp); /* write capitals length */
	fwrite(caps.cap_idxs, sizeof(unsigned short), cap_len, fp); /* write capitals array */
	fwrite(&char_total_uch, sizeof(unsigned short), 1, fp); /* write ss_keys compr length */
	fwrite(packed_str, sizeof(unsigned char), char_total, fp); /* write ss_keys compr array */
	printf("\n>>> SIZE: %d - FILE CREATED: %s\n", (char_total + 5 + cw_write_length + (2*cap_len)), filename);
	fclose(fp);
	return (char_total + 5 + cw_write_length + (2*cap_len));
}
void unpack_ss_keys(char *arg2) {
	/* READ PACKED SS KEYS FROM FILE */
	unsigned short char_total_uch, cap_len;
	unsigned char cw_read_length;
	char filename[75], txtfilename[75], unpacked_str[MAX_CH];
	ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
	ADD_FILENAME_EXTENSION(arg2,txtfilename,"_sskey.txt");
	FILE *fp;
	myAssert((fp = fopen(filename, "rb")),"\n (!!!) ERROR READING PACKED SS KEYS FROM BINARY FILE (!!!) \n");
	fread(&cw_read_length, sizeof(unsigned char), 1, fp); /* read modified cw_keys idxs length */
	fread(cw_shift_up_idxs, sizeof(unsigned char), cw_read_length, fp); /* read modified cw_keys idxs array */
	fread(&cap_len, sizeof(unsigned short), 1, fp); /* read capitals length */
	fread(caps.cap_idxs, sizeof(unsigned short), cap_len, fp); /* read capitals array */
	fread(&char_total_uch, sizeof(unsigned short), 1, fp); /* read ss_keys compr length */
	int char_total = (int)char_total_uch, i, j, k;
	int str_total = SS_KEY_STR_TOT(char_total);
	unsigned char packed_str[char_total], unpacked_uch[str_total];
	fread(packed_str, sizeof(unsigned char), char_total, fp); /* read ss_keys compr array */
	remove(filename);
	fclose(fp);
	show_adjust_cw_keys(cw_read_length);
	bpack_wipe_garbage_mem(unpacked_uch, str_total);
	/* UNPACK SS_KEYS */
	for(i = 0, k = 0; i < str_total && k < char_total; i++, k++) { /* UNPACK PACKED CHARS */
		unpacked_uch[i] = ((packed_str[k] >> 1) & 127);
		for(j = 0; j < 6 && i < str_total && k < char_total; j++, i++, k++) {
			unpacked_uch[i + 1] = (((packed_str[k] << (6 - j)) | (packed_str[k + 1] >> (j + 2))) & 127); }
		if(i < str_total) unpacked_uch[++i] = ((packed_str[k]) & 127);
	}
	swap_str_sign(unpacked_str, unpacked_uch, str_total, 0); /* convert unpacked uchar str into char str */
	unpacked_str[str_total-1] = '\n';
	unpacked_str[str_total] = '\0';
	/* WRITE UNPACKED SS_KEYS TO TEMP TEXT FILE */
	myAssert((fp = fopen(txtfilename, "w")),"\n (!!!) ERROR WRITING PACKED SS KEYS TO TEMP TEXT FILE (!!!) \n");
	fprintf(fp, "%s", unpacked_str);
	fclose(fp);
}
void bpack_wipe_garbage_mem(unsigned char arr[], int length) { int i; for(i = 0; i < length; i++) arr[i] = 0; }
void swap_str_sign(char s[], unsigned char us[], int length, int sign_flag) { 
	if(sign_flag == 1) { int i; for(i = 0; i < length; i++) us[i] = (unsigned char)s[i]; } /* make unsigned */
	else { int i; for(i = 0; i < length; i++) s[i] = (char)us[i]; } /* make signed */
}
