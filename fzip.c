/* AUTHOR: JORDAN RANDLEMAN -:- FZIP TXT FILE LOSSLESS COMPRESSION */
/********************************************************************
* |===\ /==//  ==== /|==\\       //==\ //==\\ /\\  //\ /|==\\ ||^\\ *
* |==     //    ||  ||==// +===+ ||    ||  || ||\\//|| ||==// ||_// *
* ||     //==/ ==== ||           \\==/ \\==// || \/ || ||     || \\ *
********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#define MAX_CH 1000000
#define COMPR_MEM(UCH_TOT,CAP_SIZE) (5 + (FULL_CW_LEN - CW_LEN) + (UCH_TOT) + (2 * (CAP_SIZE))) /* key l + key arr + cap l + cap arr + uncompr l + compr arr */
#define COMPR_RATIO(NEW,OLD) (100*(1.000000 - (((float)NEW)/((float)OLD))))
#define ADD_FILENAME_EXTENSION(SEED,EXTEND,APPEND) ({strcpy(EXTEND,SEED);strcpy(&EXTEND[strlen(EXTEND)-4],APPEND);})
#define NEED_TO_UPDATE_ARRS(IDX) (IDX != 0 && IDX % 12 == 0)
#define ITER_NUM(N,X,Y) ((((N + X) / 2) * 3) - Y)
#define MULT_8(N) ((N) % 8 == 0)
#define M8D(N) ((N) % 8)
#define SHIFT_CHAR_TOTAL(X) ((5*(X/8))+(M8D(X)>0)+(M8D(X)>1)+(M8D(X)>3)+(M8D(X)>4)+(M8D(X)>6))
#define IS_LOW_CH(CH) ((CH) >= 'a' && (CH) <= 'z')
#define IS_CAP_CH(CH) ((CH) >= 'A' && (CH) <= 'Z')
#define IS_ALF_CH(CH) (IS_CAP_CH((CH)) || IS_LOW_CH((CH)))
#define IS_PUNC(CH) ((CH) == '.' || (CH) == '!' || (CH) == '?')
#define VALID_CAP(W,X,Y,Z) ((W) == ' ' && !(IS_PUNC((X))) && (Y) !=' ' && (IS_CAP_CH((Z)) || (!IS_ALF_CH((Z)) && IS_CAP_CH((Y)))))
#define IS_ABBREV(X,PUNC,Y,Z) ((X) == (PUNC) && IS_ALF_CH((Y)) && (!IS_ALF_CH((Z))))
#define IS_COMMA(X,Y,Z) ((X) == '-' && ((Y) == '_' || (Y) == '\'') && (Z) != ' ')
#define IS_I_OR_C_(X,Y) (((X)=='c' || (X)=='i') && ((Y)=='_' || (Y)=='\n'))
#define DUB_QUOTE(X,Y,Z) ((X) == '\'' && ((Y) == ' ' || (Z) == '_' || IS_PUNC((Z))))
/* CUSTOM ASSERT FUNCTIONS */
void myAssert(void *condition, char message[]){if(condition==NULL){printf("%s",message);exit(0);}}
/* MAIN HIDE / SHOW HANDLERS */
void HIDE_HANDLER(char *, char *);
void SHOW_HANDLER(char *, char *);
/* HANDLE TEXT FILE */
void read_passed_str(char [], char *, int *);
void write_compr_str(char [], int, int, unsigned char []);
void read_compr_text(char *, char [], int *, unsigned char [], unsigned char []);
void delete_original_file_option(char *);
void err_info();
/* HASH/DEHASH FUNCTIONS */
void hash_pack_handler(int, char [], unsigned char []);
void unpack_dehash_handler(int, char [], unsigned char [], unsigned char []);
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
void modify_str(char *, int);
void lowify_str(char []);
/* COMMON WORD SUBSTITUTIONS */
void splice_str(char *, char *, int);
void delta_sub_common_words(char *, char [][50], char [][50]);
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
/* COMMON WORD SUBSTITUTIONS */
#define FULL_CW_LEN 255
char cw_keys[FULL_CW_LEN][50] = { /* roman numerals */
	"_2_", "_3", "_4_", "_5_", "_6_", "_7", "_8", "_9_",
	"_2\n", "_4\n", "_5\n", "_6\n", "_9\n",
	/* numbers & punctuation */
	"_jq", "_jw", "_jx", "_jy", "_jz", "_jt", "_js", "_jp", "_jn", "_jm",
	"_jb_", "_jc_", "_jd_", "._?-", "!_?-", "?_?-", "_?-", "--", ".-", "!-", 
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
	".e", "!e", "?e", ".t", "!t", "?t", ".a", "!a", "?a", ".o", "!o", "?o", ".i", "!i", "?i", 
	".n", "!n", "?n", ".s", "!s", "?s", ".h", "!h", "?h", ".r", "!r", "?r", ".d", "!d", "?d", 
	".l", "!l", "?l", ".c", "!c", "?c", ".u", "!u", "?u", ".m", "!m", "?m", ".w", "!w", "?w", 
	".f", "!f", "?f", ".g", "!g", "?g", ".y", "!y", "?y", ".p", "!p", "?p", ".b", "!b", "?b", 
	".v", "?v", "!v", ".k", "!k", "?k", ".j", "?j", "!j", ".x", "?x", "!x", ".z", ".q", "?.",
	"!?", "?!", "!.", ".?", ".!",
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
unsigned char CW_LEN = FULL_CW_LEN, cw_shift_up_idxs[FULL_CW_LEN]; /* store indices of rmvd cw_keys */
int zip_info = 0; /* output compresion/decompresion progress */
int main(int argc, char *argv[]) {
	if(argc < 3) err_info();
	char filename[75];
	lowify_str(argv[2]);
	ADD_FILENAME_EXTENSION(argv[1], filename, "_fzipped.bin");
	if(argc == 4) {
		lowify_str(argv[3]);
		if(strcmp(argv[3], "info") == 0) zip_info = 1;
	}
	if(strcmp(argv[2], "hide") == 0) {
		HIDE_HANDLER(argv[1], filename);
	} else if(strcmp(argv[2], "show") == 0) {
		SHOW_HANDLER(argv[1], filename);
	} else { err_info(); }
	return 0;
}
/******************************************************************************
* MAIN HIDE / SHOW HANDLERS
******************************************************************************/
void HIDE_HANDLER(char arg1[], char *filename) {
	/* READ ARG1 FILE & PREP TEXT FOR COMPRESSION */
	char passed_str[MAX_CH];
	int original_length;
	read_passed_str(passed_str, arg1, &original_length);
	/* HASH & PACK TEXT */
	int PASSED_STR_TOTAL = strlen(passed_str);
	unsigned char packed_uch[MAX_CH];
	hash_pack_handler(PASSED_STR_TOTAL, passed_str, packed_uch);
	/* WRITE PACKED HASHED TEXT TO FILE */
	int compr_bytes = COMPR_MEM(SHIFT_CHAR_TOTAL(PASSED_STR_TOTAL), (caps.length));
	write_compr_str(filename, PASSED_STR_TOTAL, compr_bytes, packed_uch);
	/* OUPUT COMPRESSION RESULTS */
	float total_saved_bytes_ratio = COMPR_RATIO(compr_bytes, original_length); 
	printf("\n==============================================================================");
	printf("\n%s ==COMPRESS=> %s\n", arg1, filename);
	printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n", original_length, compr_bytes, total_saved_bytes_ratio);
	printf("==============================================================================\n\n");
	delete_original_file_option(arg1);
}
void SHOW_HANDLER(char *arg1, char *filename) {
	/* READ PACKED HASHED TEXT FROM FILE */
	char unpacked_str[MAX_CH];
	int PASSED_STR_TOTAL;
	unsigned char packed_uch[MAX_CH], unpacked_uch[MAX_CH];
	read_compr_text(filename, unpacked_str, &PASSED_STR_TOTAL, packed_uch, unpacked_uch);
	/* UNPACK & DEHASH TEXT */
	unpack_dehash_handler(PASSED_STR_TOTAL, unpacked_str, packed_uch, unpacked_uch);
	printf("\n>>> FILE REMOVED: %s\n", filename);
	printf("\n==============================================================================");
	printf("\n>> %s ==DECOMPRESS=> %s\n", filename, arg1);
	printf("==============================================================================\n\n");
	/* WRITE unpacked_str[] BACK TO ARG1 TEXT FILE */
	FILE *fp;
	myAssert((fp = fopen(arg1, "w")), "\n\n(!!!) ERROR WRITING DECOMPRESSED TEXT FILE (!!!)\n\n");
	fprintf(fp, "%s", unpacked_str);
	fclose(fp);
}
/******************************************************************************
* HANDLE TEXT/BINARY FILES
******************************************************************************/
void read_passed_str(char s[], char *arg1, int *original_length) {
	FILE *fp;
	myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING UNCOMPRESSED TEXT FILE (!!!)\n\n");
	char buff[500];
	int count = 0, cw_tot;
	while(fgets(buff, 500, fp) != NULL) { strcpy(&s[count], buff); count += (strlen(buff)); }
	fclose(fp);
	*original_length = strlen(s);
	hide_adjust_cw_keys(s);
	modify_str(s, 1); /* prep s for compression */
	cw_tot = *original_length - strlen(s);
	float cw_ratio = COMPR_RATIO(strlen(s), *original_length);
	if(zip_info == 1) printf("\nCommon Words/Phrases Compressed (SAVED BYTES: %d - COMPR RATE: %.2f%%):\n%s\n", cw_tot, cw_ratio, s);
}
void write_compr_str(char filename[], int PASSED_STR_TOTAL, int compr_bytes, unsigned char packed_uch[]) {
	unsigned short write_size = (unsigned short)PASSED_STR_TOTAL, cap_len = caps.length;
	unsigned char cw_write_length = FULL_CW_LEN - CW_LEN;
	int PACKED_UCH_TOTAL = SHIFT_CHAR_TOTAL(PASSED_STR_TOTAL);
	FILE *fp;
	myAssert((fp = fopen(filename, "wb")), "\n\n(!!!) ERROR WRITING COMPRESSED TEXE TO BINARY FILE (!!!)\n\n");
	fwrite(&cw_write_length, sizeof(unsigned char), 1, fp); /* write modified cw_keys idxs length */
	fwrite(cw_shift_up_idxs, sizeof(unsigned char), cw_write_length, fp); /* write modified cw_keys idxs array */
	fwrite(&cap_len, sizeof(unsigned short), 1, fp); /* write capitals length */
	fwrite(caps.cap_idxs, sizeof(unsigned short), cap_len, fp); /* write capitals array */
	fwrite(&write_size, sizeof(unsigned short), 1, fp); /* write uncompressed/original file length */
	fwrite(packed_uch, sizeof(unsigned char), PACKED_UCH_TOTAL, fp); /* write compressed text */
	fclose(fp);
	int pack_tot = PASSED_STR_TOTAL - PACKED_UCH_TOTAL;
	float pack_ratio = COMPR_RATIO(PACKED_UCH_TOTAL, PASSED_STR_TOTAL);
	if(zip_info == 1) printf("\nBit-Packed (SAVED BYTES: %d - COMPR RATE: %.2f%%):\n%s\n", pack_tot, pack_ratio, packed_uch);
	printf("\n>>> SIZE: %d - FILE CREATED: %s\n", compr_bytes, filename);
}
void read_compr_text(char *filename, char unpacked_str[], int *PASSED_STR_TOTAL, unsigned char packed_uch[], unsigned char unpacked_uch[]) {
	unsigned short read_size, cap_len;
	unsigned char cw_read_length;
	init_decompr_arr(unpacked_str); /* init unpacked_str with 0's to clear garbage memory for bitpacking */
	init_compr_arr(unpacked_uch); /* init 0's to clear garbage memory */
	FILE *fp;
	myAssert((fp = fopen(filename, "rb")), "\n\n(!!!) ERROR READING COMPRESSED TEXT FROM BINARY FILE (!!!)\n\n");
	fread(&cw_read_length, sizeof(unsigned char), 1, fp); /* read modified cw_keys idxs length */
	fread(cw_shift_up_idxs, sizeof(unsigned char), cw_read_length, fp); /* read modified cw_keys idxs array */
	fread(&cap_len, sizeof(unsigned short), 1, fp); /* read capitals length */
	fread(caps.cap_idxs, sizeof(unsigned short), cap_len, fp); /* read capitals array */
	fread(&read_size, sizeof(unsigned short), 1, fp); /* read uncompressed/original file length */
	*PASSED_STR_TOTAL = (int)read_size;
	int read_char_total = SHIFT_CHAR_TOTAL(read_size);
	fread(packed_uch, sizeof(unsigned char), read_char_total, fp); /* read compressed text */
	fclose(fp);
	remove(filename);
	show_adjust_cw_keys(cw_read_length);
	if(zip_info == 1) printf("\nBit-Packed:\n%s\n", packed_uch);
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
	printf("==============================================================================\n");
	printf("=> COMPRESSION INFORMATION: $ ... hide/show info\n");
	printf("==============================================================================\n\n");
	exit(0);
}
/******************************************************************************
* HASH/DEHASH FUNCTIONS
******************************************************************************/
void hash_pack_handler(int PASSED_STR_TOTAL, char passed_str[], unsigned char packed_uch[]) {
	int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
	int pack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), shift_left[] = {1,0,1,1,0,1,0,1,1,0,1,1};
	unsigned char passed_uch[MAX_CH];
	init_compr_arr(packed_uch); /* init packed_uch with 0's to clear garbage memory for bitpacking */
	hash_hide(PASSED_STR_TOTAL, passed_str, passed_uch);
	for(int i = 0, j = 0; i < pack_shift_iterations; i++, j++) { /* BIT-PACK TEXT */
		if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
		if(shift_left[j] == 1) { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] << bit_shift_nums[j]) & 255); }
		else { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] >> bit_shift_nums[j]) & 255); }
	}
}
void unpack_dehash_handler(int PASSED_STR_TOTAL, char unpacked_str[], unsigned char packed_uch[], unsigned char unpacked_uch[]) {
	int unpack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), shift_right[] = {1,0,1,1,0,1,0,1,1,0,1,1};
	int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
	for(int i = 0, j = 0; i < unpack_shift_iterations; i++, j++) { /* BIT-UNPACK TEXT */
		if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
		if(shift_right[j] == 1) { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] >> bit_shift_nums[j]) & 31)); }
		else { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] << bit_shift_nums[j]) & 31)); }
	}
	dehash_show(PASSED_STR_TOTAL, unpacked_str, unpacked_uch);
	if(zip_info == 1) printf("\nCommon Words/Phrases Compressed:\n%s\n", unpacked_str);
	modify_str(unpacked_str, 0); /* revert unpacked_str back to pre-compression format */
}
void hash_hide(int len, char passed_str[], unsigned char passed_uch[]) {
	for(int i = 0; i < len; i++) passed_uch[i] = hide_hash_value(passed_str[i]);
}
void dehash_show(int len, char unpacked_str[], unsigned char unpacked_uch[]) {
	for(int i = 0; i < len; i++) unpacked_str[i] = (char)show_dehash_value(unpacked_uch[i]);
}
unsigned char hide_hash_value(char c) { /* lowercase . ' _ - ! ? */
	unsigned char ch = (unsigned char)c, from[] = {'.','\'','_','-','!','?'}, to[] = {26,27,28,29,30,31};
	if(IS_LOW_CH(ch)) return ch - 'a'; /* 'a' => 0, 'z' => 25 */
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
void init_decompr_arr(char decompr[]) { for(int i = 0; i < MAX_CH; i++) decompr[i] = 0; }
void init_compr_arr(unsigned char compr[]) { for(int i = 0; i < MAX_CH; i++) compr[i] = 0; }
void increment_idx_shift_arrs(int *j, int pack_ch_idx[], int unpack_uch_idx[]) {
	for(int k = 0; k < 12; k++) { pack_ch_idx[k] += 5; unpack_uch_idx[k] += 8; }
	*j = 0;
}
/******************************************************************************
* STRING EDITING FUNCITONS
******************************************************************************/
void modify_str(char *s, int hide_flag) { 
	char *p = s, space = ' ', under_s = '_';
	if(hide_flag == 1) { /* HIDE */
		delta_sub_capital_letters(s, 1);
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
		delta_sub_common_words(s, cw_word, cw_keys);
	} else { /* SHOW */
		delta_sub_common_words(s, cw_keys, cw_word);
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
			} else if(DUB_QUOTE(*p, *(p-1), *(p+1))) { /* sub double quotes back in */
				*p = '"';
			} else if(p > &s[2] && *(p-2) == '@') { *(p-2) = '.'; } /* ('@' => '.') if mid-sentence abbreviation */
			p++;
		}
		if(s[strlen(s)-1] == '@') s[strlen(s)-1] = '.'; /* in case file ends with an abbreviation */
		delta_sub_capital_letters(s, 0);
	}
}
void lowify_str(char s[]) {
	char *p = s;
	while(*p != '\0') { *p = tolower(*p); p++; }
}
/******************************************************************************
* COMMON WORD SUBSTITUTION FUNCTIONS
******************************************************************************/
void delta_sub_common_words(char *s, char remove[][50], char insert[][50]) {
	int word_len, i, j;
	for(i = 0; i < CW_LEN; i++) {
		char *p = s;
		word_len = strlen(remove[i]);
		while(*p != '\0') {
			if(*p == remove[i][0]) {
				for(j = 0; j < word_len; j++) if(*(p + j) != remove[i][j]) break;
				if(j == word_len) splice_str(p, insert[i], word_len); /* if word in s is common word */
			} else if(IS_PUNC(*p)) { p++; } /* avoid chaining word subs */
			p++;
		}
	}
}
void splice_str(char *s, char *sub, int splice_len) {
	char temp[MAX_CH];
	sprintf(temp, "%s%s", sub, s + splice_len);
	strcpy(s, temp);
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
	for(int i = 0; i < length; i++) { shift_up_cw_keys((int)cw_shift_up_idxs[i]); CW_LEN--; }
}
void shift_up_cw_keys(int idx) {
	for(int i = idx; i < (CW_LEN - 1); i++) { strcpy(cw_keys[i],cw_keys[i+1]); strcpy(cw_word[i],cw_word[i+1]);}
}
void modify_key(char *modded_key, char *key) {
	strcpy(modded_key, key);
	for(int i = 0; i < strlen(key); i++) if(key[i] == '_') modded_key[i] = ' ';
}
