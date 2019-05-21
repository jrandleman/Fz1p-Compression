/* AUTHOR: JORDAN RANDLEMAN */
/*******************************************************************
* |===\ /==//  ==== /|==\\       //==\ //==\\ /\\  //\ /|==\\ ||\\ *
* |==     //    ||  ||==// +===+ ||    ||  || ||\\//|| ||==// ||// *
* ||     //==/ ==== ||           \\==/ \\==// || \/ || ||     ||\\ *
*******************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define ADD_FILENAME_EXTENSION(SEED,EXTEND,APPEND) ({strcpy(EXTEND,SEED);strcpy(&EXTEND[strlen(EXTEND)],APPEND);})
#define RM_NXT_CH(S) ({memmove(S + 1, S + 2, strlen(S + 1));})
#define NEED_TO_UPDATE_ARRS(IDX) (IDX != 0 && IDX % 12 == 0)
#define ITER_NUM(N,X,Y) ((((N + X) / 2) * 3) - Y)
#define MULT_8(N) (N % 8 == 0)
#define M8D(N) (N % 8)
#define SHIFT_CHAR_TOTAL(X) ((5*(X/8))+(M8D(X)>0)+(M8D(X)>1)+(M8D(X)>3)+(M8D(X)>4)+(M8D(X)>6))
#define NUM(CH) ((CH) >= '0' && (CH) <= '9')
#define IS_LOW_CH(ch_c_inst) ((ch_c_inst) >= 'a' && (ch_c_inst) <= 'z')
#define DASH_PUNC(C_CH) (C_CH==':'||C_CH==';'||C_CH=='/'||C_CH==','||C_CH=='['||C_CH==']'||C_CH=='{'||C_CH=='}'||C_CH=='('||C_CH==')'||C_CH=='_')
/* CUSTOM ASSERT FUNCTIONS */
void myAssert(void *condition, char message[]){if(condition==NULL){printf("%s",message);exit(0);}}
/* MAIN HIDE / SHOW HANDLERS */
void HIDE_HASH_PACK_HANDLER(char *, char *);
void SHOW_DEHASH_UNPACK_HANDLER(char *, char *);
/* HANDLE TEXT FROM ARG1 FILE */
void read_passed_str(char [], char *);
void delete_original_file_option(char *);
void err_info();
/* HASH/DEHASH FUNCTIONS */
void hash_hide(int, char [], unsigned char []);
void dehash_show(int, char [], unsigned char unpacked_uch[]);
unsigned char hide_hash_value(char);
unsigned char show_dehash_value(unsigned char);
/* GET BITSHIFT LOOP ITERATION TOTAL & TOTAL TXT LEN WRT NUMBER SUBSTITUTION */
int BITSHIFT_ITERATION_TOTAL(int);
/* ARRAY INTIALIZATION AND INCREMENTING FUNCTIONS */
void init_decompr_arr(char []);
void init_compr_arr(unsigned char []);
void increment_idx_shift_arrs(int *, int [], int []);
/* STRING EDITING FUNCITONS */
void char_after_here(char *, char);
void modify_str(char *, int);
void lowify_str(char []);
/* COMMON WORD SUBSTITUTIONS */
void splice_str(char *, char *, int);
void delta_sub_words(char *, char [][50], char [][50]);
/* COMMON WORD SUBSTITUTIONS */
int cw_idxs[224];
char cw_keys[222][50] = { /* single letter */
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
	/* NO-SPACE */
	".e", "!e", "?e", ".t", "!t", "?t", ".a", "!a", "?a", ".o", "!o", "?o", ".i", "!i", "?i", 
	".n", "!n", "?n", ".s", "!s", "?s", ".h", "!h", "?h", ".r", "!r", "?r", ".d", "!d", "?d", 
	".l", "!l", "?l", ".c", "!c", "?c", ".u", "!u", "?u", ".m", "!m", "?m", ".w", "!w", "?w", 
	".f", "!f", "?f", ".g", "!g", "?g", ".y", "!y", "?y", ".p", "!p", "?p", ".b", "!b", "?b", 
	".v", "?v", "!v", ".k", "!k", "?k", ".j", "?j", "!j", ".x", "?x", "!x", ".z", ".q", "?.", 
	"!?", "?!", "!.", ".?", ".!", 
};
char cw_word[222][50] = { /* single letter */ 
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
	/* NO-SPACE */
	"there", "tion", "then", "than", "well", "them", "some", "will", "that", "this", 
	"with", "come", "back", "nter", "for", "ear", "different", "ent", "end", "see", 
	"eye", "man", "use", "even", "able", "thing", "ing", "know", "now", "hand", 
	"and", "make", "its", "not", "other", "the", "you", "any", "call", "all", "but", 
	"can", "most", "group", "over", "time", "take", "work", "place", "point", "right", 
	"into", "person", "high", "last", "long", "part", "tell", "she", "ask", "big", 
	"bad", "after", "fact", "feel", "own", "old", "try", "her", "how", "new", "out", 
	"one", "our", "case", "way", "who", "day", "get", "his",
};
int main(int argc, char *argv[]) { 
	if(argc < 3) err_info();
	char *action = argv[2], filename[75];
	lowify_str(action);
	ADD_FILENAME_EXTENSION(argv[1], filename, "_fzipped.bin");
	if(strcmp(action, "hide") == 0) {
		HIDE_HASH_PACK_HANDLER(argv[1], filename);
	} else if(strcmp(action, "show") == 0) {
		SHOW_DEHASH_UNPACK_HANDLER(argv[1], filename);
	} else { err_info(); }
	return 0;
}
/******************************************************************************
* MAIN HIDE / SHOW HANDLERS
******************************************************************************/
void HIDE_HASH_PACK_HANDLER(char arg1[], char *filename) {
	/* READ STR FROM ARG1 FILE ARG */
	char passed_str[30000];
	read_passed_str(passed_str, arg1);
	/* HASH & PACK SSKEY */
	int original_length = strlen(passed_str);
	modify_str(passed_str, 1); /* prep passed_str for compression */
	int PASSED_STR_TOTAL = strlen(passed_str), j_true[] = {1,0,1,1,0,1,0,1,1,0,1,1};
	int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
	int pack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), PACKED_UCH_TOTAL = SHIFT_CHAR_TOTAL(PASSED_STR_TOTAL);
	unsigned char packed_uch[30000], passed_uch[30000];
	init_compr_arr(packed_uch); /* init packed_uch with 0's to clear garbage memory for bitpacking */
	hash_hide(PASSED_STR_TOTAL, passed_str, passed_uch);
	for(int i = 0, j = 0; i < pack_shift_iterations; i++, j++) {
		if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
		if(j_true[j] == 1) { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] << bit_shift_nums[j]) & 255); }
		else { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] >> bit_shift_nums[j]) & 255); }
	}
	/* WRITE PACKED_UCH TO TRIMMED ARRAY */
	unsigned char trimmed_uch[PACKED_UCH_TOTAL + 1]; /* only as large as needed for packing */
	memcpy(trimmed_uch, packed_uch, sizeof(unsigned char)*(PACKED_UCH_TOTAL + 1));
	trimmed_uch[PACKED_UCH_TOTAL] = '\0';
	/* WRITE PACKED HASHED SSKEYS TO FILE */
	unsigned short write_size = (unsigned short)PASSED_STR_TOTAL;
	FILE *fp;
	myAssert((fp = fopen(filename, "wb")), "\n\n(!!!) ERROR WRITING COMPRESSED TEXE TO BINARY FILE (!!!)\n\n");
	fwrite(&write_size, sizeof(unsigned short), 1, fp);
	fwrite(trimmed_uch, sizeof(unsigned char), PACKED_UCH_TOTAL, fp);
	fclose(fp);
	printf("\nSIZE: %d - FILE CREATED: %s\n", (PACKED_UCH_TOTAL + 2), filename);
	/* OUPUT COMPRESSION RESULTS */
	int original_bytes = original_length, compr_bytes = PACKED_UCH_TOTAL;
	float total_saved_bytes_ratio = 100*(1.000000 - ((float)(2 + compr_bytes)/(float)original_bytes)); /* 2+ for compression length ushort in front */
	printf("\n==============================================================================");
	printf("\n%s ==COMPRESS=> %s\n", arg1, filename);
	printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n", original_bytes, (2 + compr_bytes), total_saved_bytes_ratio);
	printf("==============================================================================\n\n");
	delete_original_file_option(arg1);
}
void SHOW_DEHASH_UNPACK_HANDLER(char *arg1, char *filename) {
	/* READ PACKED HASHED SSKEYS FROM FILE */
	char unpacked_str[30000];
	init_decompr_arr(unpacked_str);
	int PASSED_STR_TOTAL, read_char_total;
	unsigned char packed_uch[30000], unpacked_uch[30000];
	init_compr_arr(unpacked_uch); /* init unpacked_uch with 0's to clear garbage memory for bitpacking */
	unsigned short read_size;
	FILE *fp;
	myAssert((fp = fopen(filename, "rb")), "\n\n(!!!) ERROR READING COMPRESSED TEXT FROM BINARY FILE (!!!)\n\n");
	fread(&read_size, sizeof(unsigned short), 1, fp);
	PASSED_STR_TOTAL = (int)read_size;
	read_char_total = SHIFT_CHAR_TOTAL(read_size);
	fread(packed_uch, sizeof(unsigned char), read_char_total, fp);
	fclose(fp);
	printf("\nFILE REMOVED: %s\n", filename);
	remove(filename);
	/* UNPACK & DEHASH SSKEY */
	int unpack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), j_true[] = {1,0,1,1,0,1,0,1,1,0,1,1};
	int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
	for(int i = 0, j = 0; i < unpack_shift_iterations; i++, j++) { /* UNPACK SSKEY */
		if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
		if(j_true[j] == 1) { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] >> bit_shift_nums[j]) & 31)); }
		else { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] << bit_shift_nums[j]) & 31)); }
	}
	dehash_show(PASSED_STR_TOTAL, unpacked_str, unpacked_uch);
	modify_str(unpacked_str, 0); /* revert unpacked_str back to pre-compression format */
	printf("\n==============================================================================");
	printf("\n>> %s ==DECOMPRESS=> %s\n", filename, arg1);
	printf("==============================================================================\n\n");
	/* WRITE unpacked_str[] BACK TO ARG1 TEXT FILE */
	myAssert((fp = fopen(arg1, "w")), "\n\n(!!!) ERROR WRITING DECOMPRESSED TEXT FILE (!!!)\n\n");
	fprintf(fp, "%s", unpacked_str);
	fclose(fp);
}
/******************************************************************************
* HANDLE TEXT FROM ARG1 FILE
******************************************************************************/
void read_passed_str(char s[], char *arg1) {
	FILE *fp; /* READ UNCOMPRESSED TEXT PASSED AS ARG1 FOR THE FIRST TIME */
	myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING UNCOMPRESSED TEXT FILE (!!!)\n\n");
	char buff[300];
	int ret, count = 0;
	while((ret = fscanf(fp, "%s ", buff)) > 0) {
		sprintf(&s[count], "%s ", buff);
		count += (strlen(buff) + 1);
	}
	s[strlen(s) - 1] = '\0';
	fclose(fp);
}
void delete_original_file_option(char *arg1) {
	int delete_original_text_file;
	printf("DELETE ORIGINAL TEXT FILE? (enter 1 - yes, 0 - no):\n\n>>> ");
	scanf("%d", &delete_original_text_file);
	if(delete_original_text_file != 1 && delete_original_text_file != 0) {
		printf("\n -:- INVALID OPTION -:- FILE NOT DELTED -:- \n");
	} else if(delete_original_text_file) {
		printf("\n -:- FILE DELTED -:- \n");
		remove(arg1);
	}
	printf("\n");
}
void err_info() {
	printf("\n============================= INVALID EXECUTION! =============================\n");
	printf("$ gcc -o fzip fzip.c\n$ ./fzip textfile.txt hide/show\n");
	printf("==============================================================================");
	printf("\n=> FILE.TXT FORMAT: AVOID NUMBERS!\n");
	printf("==============================================================================\n\n");
	exit(0);
}
/******************************************************************************
* HASH/DEHASH FUNCTIONS
******************************************************************************/
void hash_hide(int len, char passed_str[], unsigned char passed_uch[]) {
	for(int i = 0; i < len; i++) passed_uch[i] = hide_hash_value(passed_str[i]);
}
void dehash_show(int len, char unpacked_str[], unsigned char unpacked_uch[]) {
	for(int i = 0; i < len; i++) unpacked_str[i] = (char)show_dehash_value(unpacked_uch[i]);
}
unsigned char hide_hash_value(char c) { /* lowercase . ' _ - ! ? */
	unsigned char ch = (unsigned char)c, from[] = {'.','\'','_','-','!','?'}, to[] = {26,27,28,29,30,31};
	if(ch >= 'a' && ch <= 'z') return ch - 'a'; /* 'a' => 0, 'z' => 25 */
	for(int i = 0; i < 6; i++) if(ch == from[i]) return to[i];
	return 0;
}
unsigned char show_dehash_value(unsigned char ch) {
	unsigned char from[] = {26,27,28,29,30,31}, to[] = {'.','\'','_','-','!','?'};
	if(ch >= 0 && ch <= 25) return ch + 'a'; /* 0 => 'a', 25 => 'z' */
	for(int i = 0; i < 6; i++) if(ch == from[i]) return to[i];
	return 0;
}
/******************************************************************************
* GET BITSHIFT LOOP ITERATION TOTAL & TOTAL TXT LEN WRT NUMBER SUBSTITUTION
******************************************************************************/
int BITSHIFT_ITERATION_TOTAL(int PASSED_STR_TOTAL) {
	if((PASSED_STR_TOTAL & 1) == 0) return ITER_NUM(PASSED_STR_TOTAL, 0, 0);
	if(MULT_8((PASSED_STR_TOTAL + 7))) return ITER_NUM(PASSED_STR_TOTAL, -1, -1);
	if(MULT_8((PASSED_STR_TOTAL + 1))) return ITER_NUM(PASSED_STR_TOTAL, 1, 1);
	if(MULT_8((PASSED_STR_TOTAL + 3))) return ITER_NUM((PASSED_STR_TOTAL + 3), 0, 4);
	return ITER_NUM((PASSED_STR_TOTAL + 5), 0, 8); /* if(MULT_8((PASSED_STR_TOTAL + 5))) */
}
/******************************************************************************
* ARRAY INTIALIZATION AND INCREMENTING FUNCTION
******************************************************************************/
void init_decompr_arr(char decompr[]) { for(int i = 0; i < 30000; i++) decompr[i] = 0; }
void init_compr_arr(unsigned char compr[]) { for(int i = 0; i < 30000; i++) compr[i] = 0; }
void increment_idx_shift_arrs(int *j, int pack_ch_idx[], int unpack_uch_idx[]) {
	for(int k = 0; k < 12; k++) { pack_ch_idx[k] += 5; unpack_uch_idx[k] += 8; }
	*j = 0;
}
/******************************************************************************
* STRING EDITING FUNCITONS
******************************************************************************/
void char_after_here(char *s, char c) {
	char t[30000];
	sprintf(t, "%c%s", c, s + 1);
	sprintf(s + 1, "%s", t);
}
void modify_str(char *s, int hide_flag) { 
	char post_q_num_ref[] = "opqrtvwxyz", *p = s, space = ' ', under_s = '_';
	if(hide_flag == 1) { /* HIDE - lowercase + underscores for spaces + punc subs */
		while(*p != '\0') {
			*p = tolower(*p); /* make lowercase */
			if(*p == space) { /* ' ' => '_' */
				*p = under_s;
			} else if(DASH_PUNC(*p)) { /* replace certain punctuation with a dash */
				*p = '-';
			} else if(*p == '"'){ /* double quotes => single quotes (easier compr if u know what I'm talking aboot) */
				*p = '\'';
			} else if(NUM(*p)) { /* if a number character */
				char_after_here(p, post_q_num_ref[(*p - 48)]); /* hash number character to a corresponding post-q representative */
				*p ++ = 'q';
			} else if(*p == '\t' || *p == '\n') { /* replace tabs and newlines with '_' */
				*p = under_s;
			}
			p++;
		}
		delta_sub_words(s, cw_word, cw_keys);
	} else { /* SHOW - spaces for underscores + punc resubs + uppercases */
		delta_sub_words(s, cw_keys, cw_word);
		if(IS_LOW_CH(*p)) *p ++ -= 32; /* capitalize first letter in file */
		while(*p != '\0') {
			if(*p == under_s) { /* '_' => ' ' */
				*p = space;
				if((*(p+1)=='c' || *(p+1)=='i') && *(p+2)==under_s) *(p+1) -= 32; /* capitalize ' I ' && ' C ' */
			} else if(*p == '-' && *(p + 1) == '_' && *(p - 1) != space) { /* comma -_ => ,_ */
				*p = ',';
			} else if(*p=='.' && *(p+1) == '_' && IS_LOW_CH(*(p+2))) {
				*(p+2) -= 32; /* capitalize letters after periods */
			} else if((*p == 'q' || *p == 'Q') && *(p + 1) != 'u') { /* handle potential number re-substitutions */
				int i = 0;
				for(; i < 10; i++) if(post_q_num_ref[i] == *(p + 1)) break;
				if(i != 10) { *p = (i + 48); RM_NXT_CH(p); }
			}
			p++;
		}
	}
}
void lowify_str(char s[]) {
	char *p = s;
	while(*p != '\0') { *p = tolower(*p); p++; }
}
/******************************************************************************
* COMMON WORD SUBSTITUTION FUNCTIONS
******************************************************************************/
void splice_str(char *s, char *sub, int splice_len) {
	char temp[strlen(sub) + strlen(s + splice_len) + 1];
	sprintf(temp, "%s%s", sub, s + splice_len);
	strcpy(s, temp);
}
void delta_sub_words(char *s, char remove[][50], char insert[][50]) {
	int count = 0, found, word_len, i, j;
	for(i = 0; i < 224; i++) {
		char *p = s;
		word_len = strlen(remove[i]);
		found = 0;
		while(*p != '\0') {
			if(*p == remove[i][0]) {
				for(j = 0; j < word_len; j++) if(*(p + j) != remove[i][j]) break;
				if(j == word_len) splice_str(p, insert[i], word_len); /* if word in s is common word */
			} else if(*p == '.' || *p == '!' || *p == '?') { p++; } /* avoid chaining word subs */
			p++;
		}
	}
}
