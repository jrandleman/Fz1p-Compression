/* AUTHOR: JORDAN RANDLEMAN -:- FZ1P TXT FILE LOSSLESS COMPRESSION */
/********************************************************************
* |===\ /==//  //|  /|==\\       //==\ //==\\ /\\  //\ /|==\\ ||^\\ *
* |==     //    ||  ||==// +===+ ||    ||  || ||\\//|| ||==// ||_// *
* ||     //==/ ==== ||           \\==/ \\==// || \/ || ||     || \\ *
********************************************************************/
/**
 * compiles: $ gcc -o fz1p fz1p.c
 *    (COMPR) $ ./fz1p yourFile.txt
 *  (DECOMPR) $ ./fz1p yourFile.txt.fz1p
 *  (DETAILS) $ ./fz1p -l ...
 * (!LOADBAR) $ ./fz1p -no-load ...
 *
 * RESERVED CHARACTER SEQUENCES (for compression process):
 * (1) "qx"
 * (2) "qy"
 * (3) "qz"
 *
 * STYLED TERMINAL SYNTAX SOURCES:
 * (1) GENERAL ESCAPE CODES: http://www.climagic.org/mirrors/VT100_Escape_Codes.html
 * (2) TEXT-SPECIFIC (COLORS, BOLD, ETC): http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
 *     => NOTE: for source (2) use '\033' wherever they use '\u001b' (in C, '\u' universal chars cannot be used to repn. single chars in the basic char set).
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_CH 1000000 /* 1 megabyte = max filesize */
#define MAX_PREFIX1_KEY '-' /* replaces the most common reserved 'q'-prefix */
#define MAX_PREFIX2_KEY '\'' /* replaces 2nd most common reserved 'q'-prefix */
#define MAX_UNSIGNED_8_BIT_VAL 255
#define MAX_UNSIGNED_5_BIT_VAL 31
#define COMPR_MEM(UCH_TOT) (8 + (FULL_CW_LEN - CW_LEN) + (UCH_TOT)) /* max_prefix1 + max_prefix2 + lcw l + key l & arr + uncompr l + compr arr */
#define SAVED_CW_MEM(FREQ, OLD, NEW) ((FREQ) * (strlen(OLD) - strlen(NEW)))
#define COMPR_RATIO(NEW,OLD) (100*(1.000000 - (((float)NEW)/((float)OLD))))
#define FLOOD_ZEROS(arr, len) ({int arr_i = 0;for(; arr_i < len; ++arr_i) arr[arr_i] = 0;})
#define ADD_FILENAME_EXTENSION(SEED,EXTEND,APPEND) ({strcpy(EXTEND,SEED);strcpy(&EXTEND[strlen(EXTEND)],APPEND);})
#define RMV_FILENAME_EXTENSION(OLD_FNAME,NEW_FNAME) ({strcpy(NEW_FNAME,OLD_FNAME);NEW_FNAME[strlen(OLD_FNAME)-5]='\0';})
#define HAS_EXTENSION(S,EXT) (strcmp(&S[strlen(S)-strlen(EXT)], EXT) == 0)
#define SPRINTF_HEX(LEN, UCH) ({\
  int p_idx; char *ptr = &BPACK_zip_info_BUFF[strlen(BPACK_zip_info_BUFF)];\
  for(p_idx = 0; p_idx < (LEN); ++p_idx) {\
    sprintf(ptr, "<%#04x>", UCH[p_idx]);\
    ptr += strlen(ptr);\
  }\
})
#define NEED_TO_UPDATE_ARRS(IDX) (IDX != 0 && IDX % 12 == 0)
#define ITER_NUM(N,X,Y) ((((N + X) / 2) * 3) - Y)
#define MULT_8(N) ((N) % 8 == 0)
#define M8D(N) ((N) % 8)
#define SHIFT_CHAR_TOTAL(X) ((5*(X/8))+(M8D(X)>0)+(M8D(X)>1)+(M8D(X)>3)+(M8D(X)>4)+(M8D(X)>6))
#define IS_LOW_CH(CH) ((CH) >= 'a' && (CH) <= 'z')
#define IS_CAP_CH(CH) ((CH) >= 'A' && (CH) <= 'Z')
#define TOLOWER(CH) (32 + (CH))
#define TOUPPER(CH) ((CH) - 32)
#define IS_PUNC(CH) ((CH) == '.' || (CH) == '!' || (CH) == '?')
#define myAssert(C,M) ({\
  if(C==NULL) {\
    fprintf(stderr,"\n\033[1m >> fz1p.c: \033[31mERROR:\033[0m\033[1m myAssert Failed: %s, function: %s(), line: %d\n >> %s\033[0m\n\n",\
    #C,__func__,__LINE__,M);\
    exit(EXIT_FAILURE);\
  }\
})
#define local_cw_mem_saved(STR,FREQ) ((strlen((STR)) * (FREQ)) - (strlen((STR)) + (2 * (FREQ))) - 1) /* -1 for '_' tailing pre-file local_cw word array */
#define local_cw_worth_sub(S,F) ((strlen((S))==3) ? ((F)>4) : (strlen((S))==4||strlen((S))==5) ? ((F)>2) : ((strlen((S))>5) && ((F)>1)))
#define lcw_end(CH1,CH2) ((CH1) == '_' || ((IS_PUNC((CH1)) || (CH1) == '-') && ((CH2) == '_' || ((CH2) == '\0'))))
#define EQUALx80 "================================================================================"
/* MAIN HIDE / SHOW HANDLERS */
void HIDE_HANDLER(char *, char *);
void SHOW_HANDLER(char *, char *);
/* ERROR MESSAGES */
void check_for_reserved_qx_qy_qz_char_sequences(char *);
void confirm_valid_file(char *);
void err_info();
/* HANDLE TEXT FILE */
void read_uncompr_text(char [], char *, int *);
void write_compr_text(char [], int, unsigned char []);
void read_compr_text(char *, char [], int *, unsigned char [], unsigned char []);
void delete_original_file_option(char *);
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
void modify_str(char *, bool);
bool is_at_substring(char *, char *);
/* ANIMATED "SPINNER" & LOADING BAR FUNCTIONS */
void output_load_progress();
void animate_loading_spinner(bool *, bool *, bool *, bool *);
void remove_loading_spinner();
void start_loading_spinner();
/* NON-BITPACKABLE CHARACTER SUBSTITUTION FUNCTIONS */
void sub_out_2_most_common_q_prefixes(char *);
void sub_in_2_most_common_q_prefixes(char *);
void sub_out_non_bpack_sequence(char *, int, int);
void sub_in_non_bpack_sequence(char *, char *, int);
int sub_out_non_bitpackable_chars(char *);
void sub_back_in_non_bitpackable_chars(char *);
/* COMMON WORD SUBSTITUTIONS */
int remove_duplicate_cw_idxs(int [], int, int);
void save_cw_data_for_output(int, int [], int, bool, bool, int [], char [][50], char [][50]);
void splice_str(char *, char *, int);
void delta_sub_common_words(char *, char [][50], char [][50], bool);
/* FILE-LOCAL COMMON WORD SUBSTITUTIONS */
void hide_handle_local_cws(char[]);
void show_handle_local_cws(char[]);
void delta_sub_local_common_words(char[], bool);
void update_local_word_in_struct(char[], int *);
void fill_local_word(char[], char *, int);
void keep_local_word(int);
void handle_capped_lcwIdx();
typedef struct cws {
  char cw[150], sub[150]; 
  int freq, mem_saved;
} CWS;
CWS all_local_cws[(MAX_CH/2)], local_cws[(MAX_CH/2)];
/* EDIT COMMON WORD (CW) SUBS IF KEY IN TEXT (ANOMALY) */
void hide_adjust_cw_keys(char []);
void show_adjust_cw_keys(unsigned char);
void shift_up_cw_keys(int);
void modify_key(char *, char *);
/* COMMON WORD SUBSTITUTIONS */
#define FULL_CW_LEN 222
char cw_keys[FULL_CW_LEN][50] = {
  /* single letter */
  "_t_", "_o_", "_s_", "_w_", "_p_", "_d_", "_n_", "_r_", "_y_", "_f_", "_l_", "_h_", 
  "_b_", "_g_", "_m_", "_e_", "_v_", "_u_", "_j_", "_x_", "_k_", "_z_", "_q_", 
  /* two-letter 1 */
  "_te_", "_ts_", "_tt_", "_td_", "_tn_", "_tr_", "_ty_", "_tf_", "_tl_", "_tg_", "_th_", "_ta_", "_tk_", "_tm_", 
  "_tp_", "_tu_", "_tw_", "_tc_", "_ti_", "_ae_", "_ar_", "_ay_", "_af_", "_al_", "_ao_", "_ag_", "_ah_", "_aa_", 
  "_ak_", "_ap_", "_au_", "_aw_", "_ac_", "_ai_", "_oe_", "_os_", "_ot_", "_od_", "_oy_", "_ol_", "_oo_", "_og_", 
  "_ie_", "_id_", "_ir_", "_iy_", "_il_", "_io_", "_ig_", "_ih_", "_se_", "_ss_", "_st_", "_sd_", "_sn_", 
  "_sr_", "_sy_", "_sf_", "_sl_", "_sg_", "_sh_", "_ws_", "_wt_", "_wd_", "_wn_", "_wr_", "_wy_",
  /* two-letter 2 */
  "_wf_", "_wl_", "_wo_", "_wg_", "_wh_", "_ce_", "_cs_", "_ct_", "_cd_", "_cn_", "_cr_", "_cy_", "_cf_", 
  "_cl_", "_co_", "_cg_", "_ch_", "_bs_", "_bt_", "_bd_", "_bb_", "_br_", "_bf_", "_bl_", "_bo_", "_bg_", 
  "_pe_", "_ps_", "_pt_", "_pd_", "_pb_", "_pr_", "_py_", "_pf_", "_pl_", "_po_", "_pg_", "_hs_", 
  "_ht_", "_hd_", "_hn_", "_hr_", "_hy_", "_hf_", "_hl_", "_ho_", "_hg_", "_fe_", "_fs_", "_ft_", "_fd_",
  /* no-space */
  ".e", "!e", "?e", ".t", "!t", "?t", ".a", "!a", "?a", ".o", "!o", "?o", ".i", "!i", "?i", ".n", "!n", "?n", ".s", "!s", 
  "?s", ".h", "!h", "?h", ".r", "!r", "?r", ".d", "!d", "?d", ".l", "!l", "?l", ".c", "!c", "?c", ".u", "!u", "?u", ".m", 
  "!m", "?m", ".w", "!w", "?w", ".f", "!f", "?f", ".g", "!g", "?g", ".y", "!y", "?y", ".p", "!p", "?p", ".b", "!b", "?b", 
  ".v", "?v", "!v", ".k", "!k", "?k", ".j", "?j", "!j", ".x", "?x", "!x", ".z", ".q", "?.","!?", "?!", "!.", ".?", ".!",
};
char cw_word[FULL_CW_LEN][50] = {
  /* single letter */
  "_at_", "_as_", "_an_", "_be_", "_by_", "_do_", "_are_", "_in_", "_is_", "_it_", "_my_", "_of_", 
  "_on_", "_or_", "_to_", "_and_", "_the_", "_have_", "_that_", "_this_", "_with_", "_you_", "_up_",
  /* two-letter 1 */
  "_any_", "_all_", "_but_", "_can_", "_day_", "_get_", "_him_", "_his_", "_her_", "_how_", "_now_", "_new_", "_out_", "_one_", 
  "_our_", "_two_", "_use_", "_way_", "_who_", "_its_", "_say_", "_see_", "_she_", "_also_", "_come_", "_from_", "_for_", "_back_", 
  "_give_", "_good_", "_just_", "_into_", "_know_", "_look_", "_like_", "_make_", "_most_", "_them_", "_over_", "_only_", "_some_", "_time_", 
  "_take_", "_they_", "_work_", "_want_", "_will_", "_year_", "_your_", "_what_", "_when_", "_then_", "_than_", "_could_", "_well_", 
  "_because_", "_even_", "_these_", "_which_", "_people_", "_about_", "_after_", "_first_", "_other_", "_think_", "_there_", "_their_", 
  /* two-letter 2 */
  "_ask_", "_big_", "_bad_", "_eye_", "_few_", "_man_", "_own_", "_old_", "_try_", "_able_", "_case_", "_call_", "_fact_", 
  "_find_", "_feel_", "_hand_", "_high_", "_life_", "_last_", "_long_", "_next_", "_part_", "_seem_", "_same_", "_tell_", "_week_", 
  "_child_", "_company_", "_different_", "_early_", "_government_", "_group_", "_great_", "_important_", "_leave_", "_little_", "_large_", "_number_", 
  "_not_", "_person_", "_place_", "_point_", "_problem_", "_public_", "_right_", "_small_", "_thing_", "_world_", "_woman_", "_young_", "_would_", 
  /* no-space */
  "there", "tion", "then", "than", "well", "them", "some", "will", "that", "this", "with", "come", "back", "nter", "for", 
  "ear", "different", "ent", "end", "see", "eye", "man", "use", "even", "able", "thing", "ing", "know", "now", "hand", "and", 
  "make", "its", "not", "other", "the", "you", "any", "call", "ally", "all", "can", "most", "group", "over", "time", "take", 
  "work", "place", "point", "right", "into", "person", "high", "last", "long", "part", "tell", "she", "ask", "big", "bad", "after", 
  "ess", "but", "own", "ere", "try", "her", "how", "new", "out", "one", "our", "case", "way", "who", "ate", "get", "his",
};
char local_cw_keys[FULL_CW_LEN][6] = { /* FILE-LOCAL COMMON WORD SUBSTITUTIONS */
  "bh", "bk", "bm", "bn", "bp", "bq", "bv", "bw", "bx", "bz", "cb", "ck", "cm", "cq", "cv", "cw", "cx", "cz", "db", "dd", "df", "dg", 
  "dh", "dk", "dm", "dn", "dq", "dt", "dv", "dw", "dx", "dz", "fc", "ff", "fg", "fh", "fj", "fk", "fm", "fn", "fp", "fq", "fv", "fw", 
  "fx", "fz", "gb", "gc", "gd", "gf", "gg", "gh", "gj", "gk", "gm", "gn", "gp", "gq", "gr", "gs", "gt", "gv", "gw", "gx", "gz", "hb", 
  "hc", "hh", "hj", "hk", "hm", "hp", "hv", "hx", "hz", "jb", "jc", "jd", "jf", "jg", "jh", "jj", "jm", "jn", "jp", "jq", "jt", 
  "jv", "jw", "jx", "jz", "kb", "kc", "kd", "kf", "kg", "kh", "kj", "km", "kn", "kp", "kq", "kr", "ks", "kt", "kv", "kw", "kx", 
  "kz", "mb", "mh", "mj", "mk", "mm", "mn", "mp", "mq", "mw", "mx", "mz", "nb", "nc", "nd", "nf", "ng", "nh", "nj", "nk", "nm", 
  "nn", "np", "nq", "nr", "ns", "nt", "nv", "nw", "nx", "nz", "ph", "pk", "pm", "pn", "pp", "pq", "pv", "pw", "px", "pz", "qb", 
  "qc", "qd", "qf", "qg", "qh", "qj", "qk", "qm", "qn", "qp", "qq", "qs", "qt", "qv", "qw", "qo", "qe", "rb", "rc", "rd", "rf", 
  "rg", "rh", "rj", "rk", "rm", "rn", "rp", "rq", "rr", "rs", "rt", "rv", "rw", "rx", "rz", "sb", "sc", "sk", "sm", "sp", "sq", 
  "sw", "sx", "sz", "tb", "tq", "tz", "vb", "vc", "vd", "vf", "vg", "vh", "vj", "vk", "vm", "vn", "vp", "vq", "vr", "vs", "vt", 
  "vw", "vz", "wb", "wc", "wj", "wk", "wm", "wp", "wq",
};
/* NON BIT-PACKABLE SUBSTITUTED CHARS IN FZ1P COMPRESSION PROCESS */
#define TOTAL_NON_BPACK_CHARS 67
char SUB_VALS[TOTAL_NON_BPACK_CHARS+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-\'\",/:;<=>#$%&()*+@[\\]^_`{|}~\n\t";
char SUB_KEYS[TOTAL_NON_BPACK_CHARS][5] = { 
  "qxa","qxb","qxc","qxd","qxe","qxf","qxg","qxh","qxi","qxj","qxk","qxl","qxm",
  "qxn","qxo","qxp","qxq","qxr","qxs","qxt","qxu","qxv","qxw","qxx","qxy","qxz",
  "qya","qyb","qyc","qyd","qye","qyf","qyg","qyh","qyi","qyj","qyk","qyl","qym",
  "qyn","qyo","qyp","qyq","qyr","qys","qyt","qyu","qyv","qyw","qyx","qyy","qyz",
  "qza","qzb","qzc","qzd","qze","qzf","qzg","qzh","qzi","qzj","qzk","qzl","qzm",
  "qzn","qzo",
};
#define MAX_NON_BPACK_CHAR_SEQ 10
char SUB_VAL_SEQUENCE_COUNT_KEYS[MAX_NON_BPACK_CHAR_SEQ][10] = { /* 2-11 */
  "qzp","qzq","qzr","qzs","qzt","qzu","qzv","qzw","qzx","qzy",
};
/* GLOBAL VARIABLES */
unsigned char CW_LEN = FULL_CW_LEN, cw_shift_up_idxs[FULL_CW_LEN]; /* store indices of rmvd cw_keys */
char CMPR_zip_info_BUFF[MAX_CH], BPACK_zip_info_BUFF[MAX_CH], CWS_zip_info_BUFF[MAX_CH]; /* store data to ouput at end if "-l" present */
char max_prefix1, max_prefix2; /* store which 2 of the most common reserved 'q'-prefixes were swapped out w/ '-' * '\'' */
int lcwIdx = 0; /* output compresion/decompresion progress */
bool zip_info = false, show_load = true;
int main(int argc, char *argv[]) {
  if(argc < 2 || argc > 4) err_info();
  char filename[150];
  int i;
  FLOOD_ZEROS(filename, 150); 
  FLOOD_ZEROS(CWS_zip_info_BUFF, MAX_CH); 
  FLOOD_ZEROS(CMPR_zip_info_BUFF, MAX_CH); 
  FLOOD_ZEROS(BPACK_zip_info_BUFF, MAX_CH); 
  for(i = 1; i < argc - 1; ++i) {
    if(strcmp(argv[i], "-l") == 0)
      zip_info = true;
    else if(strcmp(argv[i], "-no-load") == 0) 
      show_load = false;
    else
      fprintf(stderr, "\n\033[1mfz1p.c:%03d: \033[31mERROR:\033[0m\033[1m CMD-LINE FLAG \"%s\" WAS NOT \"-l\" NOR \"-no-load\"!\033[0m\n",__LINE__,argv[i]), 
      err_info();
  }
  confirm_valid_file(argv[argc-1]); /* confirm file exists, is not empty, & is less memory than MAX_CH */
  if(HAS_EXTENSION(argv[argc-1], ".txt")) {
    ADD_FILENAME_EXTENSION(argv[argc-1], filename, ".fz1p");
    HIDE_HANDLER(argv[argc-1], filename);
  } else if(HAS_EXTENSION(argv[argc-1], ".FZ1P") || HAS_EXTENSION(argv[argc-1], ".fz1p")) {
    RMV_FILENAME_EXTENSION(argv[argc-1], filename);
    SHOW_HANDLER(argv[argc-1], filename);
  } else err_info();
  return 0;
}
/******************************************************************************
* MAIN HIDE / SHOW HANDLERS
******************************************************************************/
void HIDE_HANDLER(char arg1[], char *filename) {
  /* START FZ1P.C TIMER */
  clock_t timer = clock();
  /* READ ARG1 FILE & PREP TEXT FOR COMPRESSION */
  char passed_str[MAX_CH];
  FLOOD_ZEROS(passed_str, MAX_CH);
  int original_length;
  read_uncompr_text(passed_str, arg1, &original_length);
  /* HASH & PACK TEXT */
  int PASSED_STR_TOTAL = strlen(passed_str);
  unsigned char packed_uch[MAX_CH];
  FLOOD_ZEROS(packed_uch, MAX_CH);
  hash_pack_handler(PASSED_STR_TOTAL, passed_str, packed_uch);
  output_load_progress();
  /* WRITE PACKED HASHED TEXT TO FILE */
  int compr_bytes = COMPR_MEM(SHIFT_CHAR_TOTAL(PASSED_STR_TOTAL));
  write_compr_text(filename, PASSED_STR_TOTAL, packed_uch);
  /* '\033' ASCII ESCAPE code removes terminal styles once loading bar complete */
  if(show_load) fprintf(stdout, "=====]\033[0m\n"), fflush(stdout);
  /* OUPUT COMPRESSION RESULTS */
  float total_saved_bytes_ratio = COMPR_RATIO(compr_bytes, original_length); 
  if(zip_info) printf("%s%s%s", CWS_zip_info_BUFF, CMPR_zip_info_BUFF, BPACK_zip_info_BUFF);
  /* END FZ1P.C TIMER */
  timer = clock() - timer;
  double fz1p_time_taken = ((double)timer) / CLOCKS_PER_SEC;
  printf("\n%s", EQUALx80);
  printf("\n%s ==COMPRESS=> %s (TIME: %.2f sec.)\n", arg1, filename, fz1p_time_taken);
  printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n", original_length, compr_bytes, total_saved_bytes_ratio);
  printf("%s\n\n", EQUALx80);
  if(show_load) delete_original_file_option(arg1);
}
void SHOW_HANDLER(char *arg1, char *filename) {
  /* START FZ1P.C TIMER */
  clock_t timer = clock();
  /* READ PACKED HASHED TEXT FROM FILE */
  char unpacked_str[MAX_CH];
  FLOOD_ZEROS(unpacked_str, MAX_CH);
  int PASSED_STR_TOTAL;
  unsigned char packed_uch[MAX_CH], unpacked_uch[MAX_CH];
  FLOOD_ZEROS(packed_uch, MAX_CH);
  FLOOD_ZEROS(unpacked_uch, MAX_CH);
  read_compr_text(arg1, unpacked_str, &PASSED_STR_TOTAL, packed_uch, unpacked_uch);
  output_load_progress();
  /* UNPACK & DEHASH TEXT */
  unpack_dehash_handler(PASSED_STR_TOTAL, unpacked_str, packed_uch, unpacked_uch);
  /* END FZ1P.C TIMER */
  timer = clock() - timer;
  double fz1p_time_taken = ((double)timer) / CLOCKS_PER_SEC;
  if(zip_info) printf("%s%s%s", BPACK_zip_info_BUFF, CMPR_zip_info_BUFF, CWS_zip_info_BUFF);
  printf("\n%s", EQUALx80);
  printf("\n>> %s ==DECOMPRESS=> %s (TIME: %.2f sec.)\n", arg1, filename, fz1p_time_taken);
  printf("%s\n\n", EQUALx80);
  /* WRITE unpacked_str[] BACK TO ARG1 TEXT FILE */
  FILE *fp;
  myAssert((fp = fopen(filename, "w")), "\n\n(!!!) ERROR WRITING DECOMPRESSED TEXT FILE (!!!)\n\n");
  fprintf(fp, "%s", unpacked_str);
  fclose(fp);
}
/******************************************************************************
* ERROR MESSAGES
******************************************************************************/
void check_for_reserved_qx_qy_qz_char_sequences(char *str) {
  char *p = str;
  while(*(p+1) != '\0') {
    if((*p == 'q' || *p == 'Q') && (*(p+1) == 'x' || *(p+1) == 'X' || *(p+1) == 'y' || *(p+1) == 'Y' || *(p+1) == 'z' || *(p+1) == 'Z')) {
      char temp[MAX_CH];
      FLOOD_ZEROS(temp, MAX_CH);
      strcpy(temp, p);
      int p_len = strlen(p);
      int length = p_len < 37 ? p_len : 37;
      temp[length] = '\0';
      fprintf(stderr, "\n===========================================================\n");
      fprintf(stderr, "\033[1mfz1p.c:%03d: \033[31mERROR:\033[0m\033[1m RESERVED CHARACTER SEQUENCE \"q%c\" DETECTED!\033[0m\n", __LINE__, *(p+1));
      fprintf(stderr, "==== >> FOUND HERE: \"%s\"\n", temp);
      fprintf(stderr, "==== >> REMOVE TO COMPRESS FILE WITH \"fz1p.c\"!\n");
      fprintf(stderr, "==== >> TERMINATING PROGRAM.\n");
      fprintf(stderr, "===========================================================\n\n");
      exit(EXIT_FAILURE);
    }
    ++p;
  }
}
void confirm_valid_file(char *filename) { /* confirms file exists, non-empty, & is less memory than MAX_CH */
  struct stat buf;
  if(stat(filename, &buf)) {
    fprintf(stderr, "\n\033[1mfz1p.c:%03d: \033[31mERROR:\033[0m\033[1m FILE \"%s\" DOES NOT EXIST!\n", __LINE__, filename);
    fprintf(stderr, ">> Terminating Program.\n\n");
    exit(EXIT_FAILURE);
  }
  if(buf.st_size > MAX_CH || buf.st_size == 0) {
    if(buf.st_size > MAX_CH) {
      fprintf(stderr, "\n\033[1mfz1p.c:%03d: \033[31mERROR:\033[0m\033[1m FILE \"%s\" SIZE %lld BYTES EXCEEDS %d BYTE CAP!\033[0m\n",
        __LINE__,filename,buf.st_size,MAX_CH); 
      fprintf(stderr, ">> RAISE 'MAX_CH' MACRO LIMIT! -:- \n");
    } else fprintf(stderr, "\n\033[1mfz1p.c:%03d: \033[31mERROR:\033[0m\033[1m EMPTY FILES CAN'T BE COMPRESSED!\033[0m\n", __LINE__);
    fprintf(stderr, ">> Terminating Program.\n\n");
    exit(EXIT_FAILURE);
  }
}
void err_info() {
  fprintf(stderr, "\n============================= \033[1m\033[31mINVALID EXECUTION!\033[0m =============================\n");
  fprintf(stderr, "$ gcc -o fz1p fz1p.c\n<COMPRESS>   $ ./fz1p filename.txt\n<DECOMPRESS> $ ./fz1p filename.txt.fz1p\n");
  fprintf(stderr, "%s\n", EQUALx80);
  fprintf(stderr, "=> COMPRESSION INFORMATION:  $ ./fz1p -l filename.extension\n");
  fprintf(stderr, "=> NO LOADING BAR ANIMATION: $ ./fz1p -no-load filename.extension\n");
  fprintf(stderr, "%s\n", EQUALx80);
  fprintf(stderr, "=> RESERVED CHAR SEQUENCES: \"qx\", \"qy\", & \"qz\"\n");
  fprintf(stderr, "%s\n\n", EQUALx80);
  exit(0);
}
/******************************************************************************
* HANDLE TEXT/BINARY FILES
******************************************************************************/
void read_uncompr_text(char s[], char *arg1, int *original_length) {
  FILE *fp;
  myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING UNCOMPRESSED TEXT FILE (!!!)\n\n");
  char buff[500];
  FLOOD_ZEROS(buff, 500);
  int count = 0, cw_tot, total_non_bitpackable_chars = 0;
  /* '\033' ASCII ESCAPE codes: reverse background & text colors, bold font (undone once loading bar terminates) */
  if(show_load) {
    fprintf(stdout, "\n\033[7m\033[1mCOMPRESSION PROGRESS: [=====                                             ]");
    fflush(stdout);
    /* moves cursor 26 character back to start of '=' to continue loading bar */
    fprintf(stdout, "\033[46D"), fflush(stdout);
  }
  /* read file */
  while(fgets(buff, 500, fp) != NULL) { 
    strcpy(&s[count], buff); 
    count += (strlen(buff)); 
    FLOOD_ZEROS(buff, 500);
  }
  fclose(fp);
  *original_length = strlen(s);
  check_for_reserved_qx_qy_qz_char_sequences(s); /* confirm file contains no "qx", "qy", or "qz" reserved char sequences */
  output_load_progress();
  total_non_bitpackable_chars = sub_out_non_bitpackable_chars(s); /* substitute out non-bitpackable chars */
  sub_out_2_most_common_q_prefixes(s); /* substitute out 2 most common reserved 'q'-prefixes w/ '-' & '\'' */
  output_load_progress();
  hide_adjust_cw_keys(s);
  output_load_progress();
  modify_str(s, true); /* prep s for compression */
  output_load_progress();
  cw_tot = *original_length - strlen(s);
  float cw_ratio = COMPR_RATIO(strlen(s), *original_length);
  if(zip_info) {
    sprintf(CMPR_zip_info_BUFF, "\n%s\n\033[1m>> Total Non-BitPackable Chars Replaced: %d\033[0m\n%s\n\n", EQUALx80, total_non_bitpackable_chars, EQUALx80);
    sprintf(&CMPR_zip_info_BUFF[strlen(CMPR_zip_info_BUFF)], 
      "\033[1m>> \033[4mCommon Words/Phrases Compressed (SAVED BYTES: %d - COMPR RATE: %.2f%%)\033[0m\033[1m:\033[0m\n%s\n\n", cw_tot, cw_ratio, s);
  }
}
void write_compr_text(char filename[], int PASSED_STR_TOTAL, unsigned char packed_uch[]) {
  unsigned int write_size = (unsigned int)PASSED_STR_TOTAL;
  unsigned char cw_write_length = FULL_CW_LEN - CW_LEN, lcw_length = (unsigned char)lcwIdx;
  int PACKED_UCH_TOTAL = SHIFT_CHAR_TOTAL(PASSED_STR_TOTAL);
  FILE *fp;
  myAssert((fp = fopen(filename, "wb")), "\n\n(!!!) ERROR WRITING COMPRESSED TEXE TO BINARY FILE (!!!)\n\n");
  fwrite(&max_prefix1, sizeof(char), 1, fp); /* write 1st most common reserved 'q' prefix (subbed w/ '-') */
  fwrite(&max_prefix2, sizeof(char), 1, fp); /* write 2nd most common reserved 'q' prefix (subbed w/ '\'') */
  fwrite(&lcw_length, sizeof(unsigned char), 1, fp); /* write local common words length */
  fwrite(&cw_write_length, sizeof(unsigned char), 1, fp); /* write modified cw_keys idxs length */
  fwrite(cw_shift_up_idxs, sizeof(unsigned char), cw_write_length, fp); /* write modified cw_keys idxs array */
  fwrite(&write_size, sizeof(unsigned int), 1, fp); /* write uncompressed/original file length */
  fwrite(packed_uch, sizeof(unsigned char), PACKED_UCH_TOTAL, fp); /* write compressed text */
  fclose(fp);
  int pack_tot = PASSED_STR_TOTAL - PACKED_UCH_TOTAL;
  float pack_ratio = COMPR_RATIO(PACKED_UCH_TOTAL, PASSED_STR_TOTAL);
  if(zip_info) {
    sprintf(BPACK_zip_info_BUFF, "\n\033[1m>> \033[4mBit-Packed (SAVED BYTES: %d - COMPR RATE: %.2f%%)\033[0m\033[1m:\033[0m\n", pack_tot, pack_ratio);
    SPRINTF_HEX(PACKED_UCH_TOTAL,packed_uch);
    sprintf(&BPACK_zip_info_BUFF[strlen(BPACK_zip_info_BUFF)], "\n");
  }
}
void read_compr_text(char *filename, char unpacked_str[], int *PASSED_STR_TOTAL, unsigned char packed_uch[], unsigned char unpacked_uch[]) {
  unsigned int read_size;
  unsigned char cw_read_length, lcw_length;
  init_decompr_arr(unpacked_str); /* init unpacked_str with 0's to clear garbage memory for bitpacking */
  init_compr_arr(unpacked_uch); /* init 0's to clear garbage memory */
  FILE *fp;
  if(show_load) {
    fprintf(stdout, "\n\033[7m\033[1mDECOMPRESSION PROGRESS: [=====                                             ]");
    fflush(stdout);
    /* moves cursor 26 character back to start of '=' to continue loading bar */
    fprintf(stdout, "\033[46D"), fflush(stdout);
  }
  /* read file */
  myAssert((fp = fopen(filename, "rb")), "\n\n(!!!) ERROR READING COMPRESSED TEXT FROM BINARY FILE (!!!)\n\n");
  fread(&max_prefix1, sizeof(char), 1, fp); /* read 1st most common reserved 'q' prefix (subbed w/ '-') */
  fread(&max_prefix2, sizeof(char), 1, fp); /* read 2nd most common reserved 'q' prefix (subbed w/ '\'') */
  fread(&lcw_length, sizeof(unsigned char), 1, fp); /* read local common words length */
  lcwIdx = (int)lcw_length;
  fread(&cw_read_length, sizeof(unsigned char), 1, fp); /* read modified cw_keys idxs length */
  fread(cw_shift_up_idxs, sizeof(unsigned char), cw_read_length, fp); /* read modified cw_keys idxs array */
  fread(&read_size, sizeof(unsigned int), 1, fp); /* read uncompressed/original file length */
  *PASSED_STR_TOTAL = (int)read_size;
  int read_char_total = SHIFT_CHAR_TOTAL(read_size);
  fread(packed_uch, sizeof(unsigned char), read_char_total, fp); /* read compressed text */
  fclose(fp);
  remove(filename);
  output_load_progress();
  show_adjust_cw_keys(cw_read_length);
  output_load_progress();
  if(zip_info) { 
    sprintf(BPACK_zip_info_BUFF, "\n\033[1m>> \033[4mBit-Packed\033[0m\033[1m:\033[0m\n");
    SPRINTF_HEX(read_char_total, packed_uch); 
    sprintf(&BPACK_zip_info_BUFF[strlen(BPACK_zip_info_BUFF)], "\n\n");
  }
}
void delete_original_file_option(char *arg1) {
  int delete_original_text_file;
  printf("DELETE ORIGINAL TEXT FILE? (enter 1 - yes, 0 - no):\n\n>>> ");
  scanf("%d", &delete_original_text_file);
  if(delete_original_text_file != 1 && delete_original_text_file != 0) {
    printf("\n -:- INVALID OPTION -:- FILE NOT DELETED -:- \n");
  } else if(delete_original_text_file) {
    printf("\n -:- FILE DELETED -:- \n");
    remove(arg1);
  }
  printf("\n");
}
/******************************************************************************
* HASH/DEHASH FUNCTIONS
******************************************************************************/
void hash_pack_handler(int PASSED_STR_TOTAL, char passed_str[], unsigned char packed_uch[]) {
  int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
  int i, j, pack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), shift_left[] = {1,0,1,1,0,1,0,1,1,0,1,1};
  unsigned char passed_uch[MAX_CH];
  FLOOD_ZEROS(passed_uch, MAX_CH);
  hash_hide(PASSED_STR_TOTAL, passed_str, passed_uch);
  for(i = 0, j = 0; i < pack_shift_iterations; ++i, ++j) { /* BIT-PACK TEXT */
    if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
    if(shift_left[j] == 1) { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] << bit_shift_nums[j]) & MAX_UNSIGNED_8_BIT_VAL); }
    else { packed_uch[pack_ch_idx[j]] |= ((passed_uch[unpack_uch_idx[j]] >> bit_shift_nums[j]) & MAX_UNSIGNED_8_BIT_VAL); }
  }
}
void unpack_dehash_handler(int PASSED_STR_TOTAL, char unpacked_str[], unsigned char packed_uch[], unsigned char unpacked_uch[]) {
  int i, j, unpack_shift_iterations = BITSHIFT_ITERATION_TOTAL(PASSED_STR_TOTAL), shift_right[] = {1,0,1,1,0,1,0,1,1,0,1,1};
  int pack_ch_idx[] = {0,0,1,1,1,2,2,3,3,3,4,4}, unpack_uch_idx[] = {0,1,1,2,3,3,4,4,5,6,6,7}, bit_shift_nums[] = {3,2,6,1,4,4,1,7,2,3,5,0};
  for(i = 0, j = 0; i < unpack_shift_iterations; ++i, ++j) { /* BIT-UNPACK TEXT */
    if(NEED_TO_UPDATE_ARRS(i)) increment_idx_shift_arrs(&j, pack_ch_idx, unpack_uch_idx); /* UPDATE CHAR ACCESS IDXS */
    if(shift_right[j] == 1) { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] >> bit_shift_nums[j]) & MAX_UNSIGNED_5_BIT_VAL)); }
    else { unpacked_uch[unpack_uch_idx[j]] |= (((packed_uch[pack_ch_idx[j]] << bit_shift_nums[j]) & MAX_UNSIGNED_5_BIT_VAL)); }
  }
  output_load_progress();
  dehash_show(PASSED_STR_TOTAL, unpacked_str, unpacked_uch);
  output_load_progress();
  if(zip_info) sprintf(CMPR_zip_info_BUFF, "\n\033[1m>> \033[4mCommon Words/Phrases Compressed\033[0m\033[1m:\033[0m\n%s\n", unpacked_str);
  modify_str(unpacked_str, false); /* revert unpacked_str back to pre-compression format */
  sub_in_2_most_common_q_prefixes(unpacked_str); /* resubstitute in 2 most common reserved 'q'-prefixes instead of '-' & '\'' */
  output_load_progress();
  sub_back_in_non_bitpackable_chars(unpacked_str); /* resubstitute in non-bitpackable chars to file */
  /* '\033' ASCII ESCAPE code removes terminal styles once loading bar complete */
  if(show_load) fprintf(stdout, "=====]\033[0m\n"), fflush(stdout);
}
void hash_hide(int len, char passed_str[], unsigned char passed_uch[]) {
  int i; for(i = 0; i < len; ++i) passed_uch[i] = hide_hash_value(passed_str[i]);
}
void dehash_show(int len, char unpacked_str[], unsigned char unpacked_uch[]) {
  int i; for(i = 0; i < len; ++i) unpacked_str[i] = (char)show_dehash_value(unpacked_uch[i]);
}
unsigned char hide_hash_value(char c) { /* lowercase . ' _ - ! ? */
  unsigned char ch = (unsigned char)c, from[] = {'.','\'','_','-','!','?'}, to[] = {26,27,28,29,30,31};
  if(IS_LOW_CH(ch)) return ch - 'a'; /* 'a' => 0, 'z' => 25 */
  int i; for(i = 0; i < 6; ++i) if(ch == from[i]) return to[i];
  return 0;
}
unsigned char show_dehash_value(unsigned char ch) {
  unsigned char from[] = {26,27,28,29,30,31}, to[] = {'.','\'','_','-','!','?'};
  if(ch >= 0 && ch <= 25) return ch + 'a'; /* 0 => 'a', 25 => 'z' */
  int i; for(i = 0; i < 6; ++i) if(ch == from[i]) return to[i];
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
void init_decompr_arr(char decompr[]) { int i; for(i = 0; i < MAX_CH; ++i) decompr[i] = 0; }
void init_compr_arr(unsigned char compr[]) { int i; for(i = 0; i < MAX_CH; ++i) compr[i] = 0; }
void increment_idx_shift_arrs(int *j, int pack_ch_idx[], int unpack_uch_idx[]) {
  int k; for(k = 0; k < 12; ++k) { pack_ch_idx[k] += 5; unpack_uch_idx[k] += 8; }
  *j = 0;
}
/******************************************************************************
* STRING EDITING FUNCITONS
******************************************************************************/
void modify_str(char *s, bool hide_flag) { 
  char *p = s, space = ' ', under_s = '_';
  if(hide_flag) { /* HIDE */
    while(*p != '\0') {
      if(*p == space) *p = under_s; /* ' ' => '_' */
      ++p;
    }
    output_load_progress();
    delta_sub_common_words(s, cw_word, cw_keys, true);
    output_load_progress();
    hide_handle_local_cws(s); /* find and append local common words in front of text */
  } else { /* SHOW */
    show_handle_local_cws(s); /* read and rmv prepended local common words */
    output_load_progress();
    delta_sub_common_words(s, cw_keys, cw_word, false);
    output_load_progress();
    while(*p != '\0') {
      if(*p == under_s) *p = space;
      ++p;
    }
  }
}
bool is_at_substring(char *p, char *substr) {
  while(*substr != '\0' && *p != '\0' && *p == *substr) ++p, ++substr;
  return (*substr == '\0');
}
/******************************************************************************
* ANIMATED "SPINNER" & LOADING BAR FUNCTIONS
******************************************************************************/
void output_load_progress() {
  if(show_load) fprintf(stdout, "====="), fflush(stdout);
}
void animate_loading_spinner(bool *upflag, bool *upleftflag, bool *sideflag, bool *downrightflag) {
  if(!show_load) return;
  if(*upflag)
    fprintf(stdout, "\b|"), *upflag = false, *upleftflag = true;
  else if(*upleftflag)
    fprintf(stdout, "\b/"), *upleftflag = false, *sideflag = true;
  else if(*sideflag)
    fprintf(stdout, "\b-"), *sideflag = false, *downrightflag = true;
  else if(*downrightflag)
    fprintf(stdout, "\b\\"), *downrightflag = false, *upflag = true;
  fflush(stdout);
}
void remove_loading_spinner() {
  if(show_load) fprintf(stdout, " \b\b \b\b"), fflush(stdout);
}
void start_loading_spinner() {
  if(show_load) fprintf(stdout, "  "), fflush(stdout);
}
/******************************************************************************
* NON-BITPACKABLE CHARACTER SUBSTITUTION FUNCTIONS
******************************************************************************/
void sub_out_2_most_common_q_prefixes(char *str) {
  int qx_tot = 0, qy_tot = 0, qz_tot = 0;
  char *p = str, buf[MAX_CH], *q;
  FLOOD_ZEROS(buf, MAX_CH);
  /* get the number of "qx", "qy", & "qz" prefixes */
  while(*p != '\0') {
    if(*p == 'q')
      switch(*(p + 1)) {
        case 'x': ++qx_tot; break;
        case 'y': ++qy_tot; break;
        case 'z': ++qz_tot;
      }
    ++p;
  }
  /* get the 2 most frequent reserved "q"-prefixes */
  (qx_tot >= qy_tot)  
    ? (qx_tot >= qz_tot)
      ? (max_prefix1 = 'x', max_prefix2 = (qy_tot >= qz_tot) ? 'y' : 'z')
      : (max_prefix1 = 'z', max_prefix2 = (qy_tot >= qx_tot) ? 'y' : 'x')
    : (qy_tot >= qz_tot)
      ? (max_prefix1 = 'y', max_prefix2 = (qx_tot >= qz_tot) ? 'x' : 'z')
      : (max_prefix1 = 'z', max_prefix2 = (qy_tot >= qx_tot) ? 'y' : 'x');
  /* replace the most common "q" prefix instances w/ "-" && the 2nd most common w/ "'" */
  p = str, q = buf;
  while(*p != '\0') {
    if(*p == 'q' && *(p + 1) == max_prefix1)
      *q++ = MAX_PREFIX1_KEY, p += 2;
    else if(*p == 'q' && *(p + 1) == max_prefix2)
      *q++ = MAX_PREFIX2_KEY, p += 2;
    else
      *q++ = *p++;
  }
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
}
void sub_in_2_most_common_q_prefixes(char *str) {
  char *p = str, buf[MAX_CH], *q;
  FLOOD_ZEROS(buf, MAX_CH);
  /* resub back in the 2 most common reserved 'q' prefixes instead of '-' && '\'' */
  q = buf;
  while(*p != '\0') {
    if(*p == MAX_PREFIX1_KEY)      *q++ = 'q', *q++ = max_prefix1, ++p;
    else if(*p == MAX_PREFIX2_KEY) *q++ = 'q', *q++ = max_prefix2, ++p;
    else *q++ = *p++;
  }
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
}
void sub_out_non_bpack_sequence(char *q, int val_key_index, int sequence_count) { 
  if(sequence_count < 2) { strcpy(q, SUB_KEYS[val_key_index]); return; }
  const int total_max_length_sequences = sequence_count / (MAX_NON_BPACK_CHAR_SEQ + 1); /* how many max length sequence keys to put */
  const int total_remaining_sequences = sequence_count % (MAX_NON_BPACK_CHAR_SEQ + 1); /* remainder of sequence that's left under max length */
  const int SUB_VAL_SEQ_COUNT_IDX = total_remaining_sequences - 2;
  const int MAX_SEQ_COUNT_IDX = MAX_NON_BPACK_CHAR_SEQ - 1;
  int i;
  for(i = 0; i < total_max_length_sequences; ++i) { /* add the needed number of sequence replacements */
    sprintf(q, "%s%c%c", SUB_VAL_SEQUENCE_COUNT_KEYS[MAX_SEQ_COUNT_IDX], SUB_KEYS[val_key_index][1], SUB_KEYS[val_key_index][2]);
    q += strlen(q);
  }
  if(total_remaining_sequences == 1)
    strcpy(q, SUB_KEYS[val_key_index]);
  else if(total_remaining_sequences > 1)
    sprintf(q, "%s%c%c", SUB_VAL_SEQUENCE_COUNT_KEYS[SUB_VAL_SEQ_COUNT_IDX], SUB_KEYS[val_key_index][1], SUB_KEYS[val_key_index][2]);
}
void sub_in_non_bpack_sequence(char *q, char *str, int seq_count_key_index) { 
  const int nonbpack_char_sequence_length = seq_count_key_index + 2;
  char sub_key[5];
  FLOOD_ZEROS(sub_key, 5);
  sprintf(sub_key, "q%c%c", *(str + 3), *(str + 4));
  int val_key_index = 0, i;
  while(val_key_index < TOTAL_NON_BPACK_CHARS && !is_at_substring(sub_key, SUB_KEYS[val_key_index]))
    ++val_key_index;
  for(i = 0; i < nonbpack_char_sequence_length; ++i)
    *q++ = SUB_VALS[val_key_index];
}
int sub_out_non_bitpackable_chars(char *str) { /* Replace non-bitpackable chars with substitute keys */
  char buf[MAX_CH], capitals_sequence[MAX_CH];
  FLOOD_ZEROS(buf, MAX_CH);
  char *p = str, *q = buf, *scout;
  int i = 0, total_non_bitpackable_chars = 0, non_bpack_char_seq_length, cap_seq_idx;
  bool upflag = true, upleftflag = false, sideflag = false, downrightflag = false;
  start_loading_spinner();
  while(*p != '\0') {                           /* traverse through string */
    animate_loading_spinner(&upflag, &upleftflag, &sideflag, &downrightflag);
    for(i = 0; i < TOTAL_NON_BPACK_CHARS; ++i)  /* search for non bitpackable characters */
      if(*p == SUB_VALS[i]) {
        /* at least 3 capitals needed to make reserved-char compression worth it */
        if(IS_CAP_CH(*p) && IS_CAP_CH(*(p+1)) && IS_CAP_CH(*(p+2))) {
          cap_seq_idx = 0;
          FLOOD_ZEROS(capitals_sequence, MAX_CH);
          capitals_sequence[cap_seq_idx++] = TOLOWER(*p);
          scout = p + 1;
          while(*scout != '\0' && IS_CAP_CH(*scout)) /* copy sequence of capital letters > length 3 */
            capitals_sequence[cap_seq_idx++] = (TOLOWER(*scout)), ++scout;
          capitals_sequence[cap_seq_idx] = '\0';
          scout = capitals_sequence;
          while(*scout != '\0' && *scout == capitals_sequence[0]) ++scout;
          if(*scout != '\0') { /* sequence of capitals isn't just the same letter (more effectively compressed as a sequence if so) */
            sprintf(q, "qzz%sqz", capitals_sequence); /* sequence of 3+ capital letters always stored btwn "qzz" & "qz" */
            q += strlen(q);
            p += cap_seq_idx;
            break;
          }
        }
        /* not a sequence of 3+ capitals thus check convert/compress sequence of non-bitpackable chars */
        scout = p + 1;
        non_bpack_char_seq_length = 1;
        while(*scout != '\0' && *scout == SUB_VALS[i]) ++scout, ++non_bpack_char_seq_length;
        sub_out_non_bpack_sequence(q, i, non_bpack_char_seq_length); /* copy in substitute */
        q += strlen(q);
        p += non_bpack_char_seq_length;
        total_non_bitpackable_chars += non_bpack_char_seq_length; /* for "-l" flag */
        break;
      }
    if(i == TOTAL_NON_BPACK_CHARS) *q++ = *p++; /* not at non bitpackable char, thus copy */
  }
  *q = '\0';
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
  remove_loading_spinner();
  return total_non_bitpackable_chars; /* for "-l" flag */
}
void sub_back_in_non_bitpackable_chars(char *str) { /* Resub non-bitpackable chars back into the string */
  char buf[MAX_CH], capitals_sequence[MAX_CH], *scout;
  FLOOD_ZEROS(buf, MAX_CH);
  char *p = str, *q = buf;
  int i = 0, j = 0, cap_seq_idx;
  bool upflag = true, upleftflag = false, sideflag = false, downrightflag = false;
  start_loading_spinner();
  while(*p != '\0') {               /* traverse through string */
    animate_loading_spinner(&upflag, &upleftflag, &sideflag, &downrightflag);
    if(is_at_substring(p, "qzz")) { /* check for sequences of capital letters */
      cap_seq_idx = 0;
      FLOOD_ZEROS(capitals_sequence, MAX_CH);
      scout = p + 3; /* at start of capitals sequence */
      while(*scout != '\0' && !is_at_substring(scout, "qz"))
        capitals_sequence[cap_seq_idx++] = TOUPPER(*scout), ++scout;
      capitals_sequence[cap_seq_idx] = '\0';
      strcpy(q, capitals_sequence);
      q += strlen(q);
      p += cap_seq_idx + 5; /* capitals sequence length + "qzz" + "qzz" */
    }
    for(j = 0; j < MAX_NON_BPACK_CHAR_SEQ; ++j) {
      if(is_at_substring(p, SUB_VAL_SEQUENCE_COUNT_KEYS[j])) {
        sub_in_non_bpack_sequence(q, p, j);
        q += strlen(q);
        p += 5; /* length of "SUB_VAL_SEQUENCE_COUNT_KEYS[j]" + the sequence's 2 tailing "key_type" indicators */
        break;
      }
    }
    for(i = 0; i < TOTAL_NON_BPACK_CHARS; ++i)  /* search for non bitpackable characters */
      if(is_at_substring(p, SUB_KEYS[i])) {
        *q++ = SUB_VALS[i];                       /* re-substitute in non bitpackable char */
        p += strlen(SUB_KEYS[i]);
        break;
      }
    if(i == TOTAL_NON_BPACK_CHARS && j == MAX_NON_BPACK_CHAR_SEQ) 
      *q++ = *p++; /* not at non bitpackable char, thus copy */
  }
  *q = '\0';
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
  remove_loading_spinner();
}
/******************************************************************************
* COMMON WORD SUBSTITUTION FUNCTIONS
******************************************************************************/
int remove_duplicate_cw_idxs(int idxs_of_found_cw[], int found_cw_count, int original_size) {
  int idxs_of_unique_found_cw[FULL_CW_LEN], unique_cw_count = 0, i, j;
  FLOOD_ZEROS(idxs_of_unique_found_cw, FULL_CW_LEN);
  for(i = 0; i < found_cw_count; ++i) { /* for each found cw index */
    for(j = 0; j < unique_cw_count && idxs_of_unique_found_cw[j] != idxs_of_found_cw[i]; ++j); /* for each unique cw index */
    if(j == unique_cw_count) /* add idx if not found in "idxs_of_unique_found_cw" */
      idxs_of_unique_found_cw[unique_cw_count++] = idxs_of_found_cw[i];
  }
  FLOOD_ZEROS(idxs_of_found_cw, original_size); /* empty old idxs & refill w/ only unique instances of each idx */
  for(i = 0; i < unique_cw_count; ++i) 
    idxs_of_found_cw[i] = idxs_of_unique_found_cw[i];
  return unique_cw_count;
}
void save_cw_data_for_output(int cw_count,int cw_idxs[],int idxs_len,bool hide_flag,bool local_cw,int cw_freq_hashtbl[],char rmv[][50],char add[][50]) {
  /* only invoked if "-l" zip_info present: saves common word (local & general) info for ouput as program finishes */
  int total_cw_instances = cw_count, i, saved_mem = 0;
  cw_count = remove_duplicate_cw_idxs(cw_idxs, cw_count, idxs_len);
  char *p = &CWS_zip_info_BUFF[strlen(CWS_zip_info_BUFF)], local_header[7], col1[6], col2[6], bytes_sign;
  (local_cw) ? strcpy(local_header, "LOCAL ") : strcpy(local_header, ""); /* put "LOCAL" in header if relevant */
  sprintf(p, "\n%s\n\033[1m\033[4m%sCOMMON WORDS FOUND & REPLACED (%d INSTANCES)\033[0m\033[1m:\033[0m\n", EQUALx80, local_header, total_cw_instances);
  p += strlen(p);
  if(hide_flag) strcpy(col1, "INSRT"), strcpy(col2, "RMVD"), bytes_sign = '-';
  else          strcpy(col1, "RMVD"), strcpy(col2, "INSRT"), bytes_sign = '+';
  sprintf(p, "\t\033[1m\033[4mFREQ\033[0m\t\033[1m\033[4mBYTES\033[0m\t\033[1m\033[4m%s\033[0m\t\033[1m\033[4m%s\033[0m\n", col1, col2);
  p += strlen(p);
  for(i = 0; i < cw_count; ++i) {
    saved_mem = (local_cw)  ? SAVED_CW_MEM(cw_freq_hashtbl[cw_idxs[i]], local_cws[cw_idxs[i]].cw, local_cws[cw_idxs[i]].sub)
              : (hide_flag) ? SAVED_CW_MEM(cw_freq_hashtbl[cw_idxs[i]], rmv[cw_idxs[i]], add[cw_idxs[i]])
                            : SAVED_CW_MEM(cw_freq_hashtbl[cw_idxs[i]], add[cw_idxs[i]], rmv[cw_idxs[i]]);
    if(local_cw)
      sprintf(p, "%02d)\tx%02d\t%c%02d\t%s\t%s\n", i+1, cw_freq_hashtbl[cw_idxs[i]], bytes_sign, saved_mem, local_cws[cw_idxs[i]].sub, local_cws[cw_idxs[i]].cw);
    else if(hide_flag)
      sprintf(p, "%02d)\tx%02d\t%c%02d\t%s\t%s\n", i+1, cw_freq_hashtbl[cw_idxs[i]], bytes_sign, saved_mem, add[cw_idxs[i]], rmv[cw_idxs[i]]);
     else
      sprintf(p, "%02d)\tx%02d\t%c%02d\t%s\t%s\n", i+1, cw_freq_hashtbl[cw_idxs[i]], bytes_sign, saved_mem, rmv[cw_idxs[i]], add[cw_idxs[i]]);
    p += strlen(p);
  }
}
void delta_sub_common_words(char *s, char remove[][50], char insert[][50], bool hide_flag) {
  int word_len, i, j, condition;
  int idxs_of_found_cw[MAX_CH], found_cw_count = 0, total_cw_instances; /* for "-l" flag */
  int cw_frequency_hash_table[FULL_CW_LEN]; /* for "-l" flag */
  char *p;
  FLOOD_ZEROS(idxs_of_found_cw, MAX_CH);
  FLOOD_ZEROS(cw_frequency_hash_table, FULL_CW_LEN);
  (hide_flag) ? (i = 0) : (i = CW_LEN - 1); /* traverse the cw matrix forwards on hide & backwards on show */
  bool upflag = true, upleftflag = false, sideflag = false, downrightflag = false;
  start_loading_spinner();
  while(1) {
    animate_loading_spinner(&upflag, &upleftflag, &sideflag, &downrightflag);
    (hide_flag) ? (condition = (i < CW_LEN)) : (condition = (i >= 0));
    if(!condition) break;
    p = s;
    word_len = strlen(remove[i]);
    while(*p != '\0') {
      if(*p == remove[i][0]) {
        for(j = 0; j < word_len; ++j) if(*(p + j) != remove[i][j]) break;
        if(j == word_len) {
          if(zip_info) {  /* for "-l" flag */
            idxs_of_found_cw[found_cw_count++] = i;
            ++ cw_frequency_hash_table[i];
          }
          splice_str(p, insert[i], word_len); /* if word in s is common word */
        }
      } else if(IS_PUNC(*p)) { ++p; } /* avoid chaining word subs */
      ++p;
    }
    (hide_flag) ? (++i) : (--i);
  }
  if(zip_info) save_cw_data_for_output(found_cw_count, idxs_of_found_cw, MAX_CH, hide_flag, false, cw_frequency_hash_table, remove, insert);
  remove_loading_spinner();
}
void splice_str(char *s, char *sub, int splice_len) {
  char temp[MAX_CH]; 
  FLOOD_ZEROS(temp, MAX_CH); 
  sprintf(temp, "%s%s", sub, s + splice_len); strcpy(s, temp);
}
/******************************************************************************
* FILE-LOCAL COMMON WORD SUBSTITUTIONS
******************************************************************************/
void hide_handle_local_cws(char s[]) {
  int i, j, count, files_first_word = 0, all_lcwIdx = 0, buff_idx = 0;
  char *p = s, *q, local_word[150], lcw_write_array[MAX_CH], lcw_write_buffer[MAX_CH]; /* 'local_word' == any substring btwn "_"'s */
  FLOOD_ZEROS(local_word, 150);
  FLOOD_ZEROS(lcw_write_array, MAX_CH);
  FLOOD_ZEROS(lcw_write_buffer, MAX_CH);
  for(i = 0; i < (MAX_CH/2); ++i) all_local_cws[i].freq = 0; /* init frequencies */
  while(*p != '\0') {
    if(*p == '_' || files_first_word == 0) { /* if could be valid local common word */
      q = p + files_first_word, count = 0;
      while(!lcw_end(*q, *(q+1)) && *q != '\0') { ++q; ++count; } /* get local word's length */
      fill_local_word(local_word, p + 1, count); /* p + 1 to not include prefix '_' */
      update_local_word_in_struct(local_word, &all_lcwIdx); /* increment freq or add word to struct */
      files_first_word = 1, p = q; /* move p to the next word */
    } else { ++p; }
  }
  output_load_progress();
  for(j = 0; j < all_lcwIdx; ++j) if(local_cw_worth_sub(all_local_cws[j].cw, all_local_cws[j].freq)) keep_local_word(j); /* eliminate ineffective lcw's */
  if(lcwIdx >= FULL_CW_LEN) handle_capped_lcwIdx(); /* trim least-memory-saving-lcws if cap of 222 passed => anomaly, but just in case */
  for(j=0; j<lcwIdx; ++j) {sprintf(&lcw_write_array[buff_idx],"%s_",local_cws[j].cw); buff_idx+=(strlen(local_cws[j].cw)+1);} /* prepend cws to file txt */
  lcw_write_array[buff_idx] = '\0';
  delta_sub_local_common_words(s, true); /* replace lcws with their associated lcw key */
  strcpy(lcw_write_buffer, lcw_write_array); strcpy(&lcw_write_buffer[strlen(lcw_write_array)], s); strcpy(s, lcw_write_buffer); /* rmv prepended lcws */
}
void show_handle_local_cws(char s[]) {
  int under_s_count = 0, word_len;
  char *p = s, *q, local_word[MAX_CH], file_text_buffer[MAX_CH];
  FLOOD_ZEROS(local_word, MAX_CH);
  FLOOD_ZEROS(file_text_buffer, MAX_CH);
  while(under_s_count < lcwIdx && *p != '\0') { /* scrape lcw's from front (prepend) of file */
    word_len = 0, q = p + 1; while(*q != '_' && *q != '\0') { ++q; ++word_len; }
    strcpy(local_word, p); local_word[word_len + 1] = '\0';
    strcpy(local_cws[under_s_count].cw, local_word); /* add local common word to struct */
    strcpy(local_cws[under_s_count].sub, local_cw_keys[under_s_count]); /* add substitution key */
    under_s_count++, p = q + 1;
  }
  strcpy(file_text_buffer, p); strcpy(s, file_text_buffer); /* splice out prepended lcws */
  delta_sub_local_common_words(s, false); /* replace lcw keys with lcws */
}
void delta_sub_local_common_words(char s[], bool hide_flag) {
  int word_len, i, j;
  int idxs_of_found_cw[MAX_CH/2], found_cw_count = 0, total_cw_instances; /* for "-l" flag */
  int cw_frequency_hash_table[FULL_CW_LEN]; /* for "-l" flag */
  char empty1[1][50], empty2[1][50]; /* for "-l" flag: required args for "save_cw_data_for_output" but do nothing in this case */
  bool upflag = true, upleftflag = false, sideflag = false, downrightflag = false;
  start_loading_spinner();
  FLOOD_ZEROS(idxs_of_found_cw, MAX_CH/2);
  FLOOD_ZEROS(cw_frequency_hash_table, FULL_CW_LEN);
  char *p, rmv[150], add[150];
  for(i = 0; i < lcwIdx; ++i) {
    p = s;
    FLOOD_ZEROS(rmv, 150);
    FLOOD_ZEROS(add, 150);
    if(hide_flag) { strcpy(rmv, local_cws[i].cw); strcpy(add, local_cws[i].sub); } 
    else { strcpy(rmv, local_cws[i].sub); strcpy(add, local_cws[i].cw); }
    word_len = strlen(rmv);
    animate_loading_spinner(&upflag, &upleftflag, &sideflag, &downrightflag);
    while(*(p + word_len + 1) != '\0') {
      if(*p == '_' && lcw_end(*(p+word_len+1),*(p+word_len+2)) && *(p + 1) == rmv[0]) {
        ++p; for(j = 0; j < word_len; ++j) if(*(p + j) != rmv[j]) break;
        if(j == word_len) {
          if(zip_info) { /* for "-l" flag */
            idxs_of_found_cw[found_cw_count++] = i; 
            ++ cw_frequency_hash_table[i];
          }
          splice_str(p, add, word_len); /* if word in s is local common word */
        }
      }
      ++p;
    }
  }
  if(zip_info) save_cw_data_for_output(found_cw_count, idxs_of_found_cw, MAX_CH/2, hide_flag, true, cw_frequency_hash_table, empty1, empty2);
  remove_loading_spinner();
}
void update_local_word_in_struct(char s[], int *all_lcwIdx) {
  int i; for(i = 0; i < (*all_lcwIdx); ++i) if(strcmp(all_local_cws[i].cw, s) == 0) { ++ all_local_cws[i].freq; return; }
  strcpy(all_local_cws[(*all_lcwIdx)].cw, s);
  all_local_cws[(*all_lcwIdx)++].freq = 1;
}
void fill_local_word(char local_word[], char *start, int end) { /* local_word = [start, end) */
  char *p = start;
  int i; for(i = 0; i < end; ++i) local_word[i] = *(p + i);
  local_word[end] = '\0';
}
void keep_local_word(int j) {
  strcpy(local_cws[lcwIdx].cw, all_local_cws[j].cw); 
  (lcwIdx < FULL_CW_LEN) ? (strcpy(local_cws[lcwIdx].sub, local_cw_keys[lcwIdx])) : ((strcpy(local_cws[lcwIdx].sub, "lcwIdx_cap_overflow")));
  local_cws[lcwIdx].freq = all_local_cws[j].freq;
  local_cws[lcwIdx++].mem_saved = local_cw_mem_saved(all_local_cws[j].cw, all_local_cws[j].freq);
}
void handle_capped_lcwIdx() {
  int i, j, rmv_amount = lcwIdx - FULL_CW_LEN + 1;
  for(i = 0; i < rmv_amount; ++i) { /* find rmv_amount # of least-memory-saving lcws & rmv them */
    int min = 0; for(j = 0; j < lcwIdx; ++j) if(local_cws[j].mem_saved < local_cws[min].mem_saved) min = j; /* find least-memory-saving lcw */
    --lcwIdx; for(j = min; j < lcwIdx; ++j) local_cws[j] = local_cws[j + 1]; /* rmv least-memory-saving lcw from local_cws struct */
  }
  for(j = 0; j < FULL_CW_LEN; ++j) strcpy(local_cws[j].sub, local_cw_keys[j]); /* re-assign local_cw_keys to lcws relative to their new position */
}
/******************************************************************************
* EDIT COMMON WORD (CW) SUBS IF KEY IN TEXT (ANOMALY)
******************************************************************************/
void hide_adjust_cw_keys(char s[]) {
  int word_len, i, j;
  for(i = 0; i < CW_LEN; ++i) {
    char *p = s, modded_key[50];
    word_len = strlen(cw_keys[i]);
    modify_key(modded_key, cw_keys[i]);
    if(modded_key[1] >= '2' && modded_key[1] <= '9') continue;
    while(*p != '\0') {
      if(*p == modded_key[0]) {
        for(j = 0; j < word_len; ++j) if(*(p + j) != modded_key[j]) break;
        if(j == word_len) { shift_up_cw_keys(i); cw_shift_up_idxs[FULL_CW_LEN - (CW_LEN--)] = (unsigned char)i; }
      }
      ++p;
    }
  }
}
void show_adjust_cw_keys(unsigned char length) {
  int i; for(i = 0; i < length; ++i) { shift_up_cw_keys((int)cw_shift_up_idxs[i]); --CW_LEN; }
}
void shift_up_cw_keys(int idx) {
  int i; for(i = idx; i < (CW_LEN - 1); ++i) { strcpy(cw_keys[i],cw_keys[i+1]); strcpy(cw_word[i],cw_word[i+1]);}
}
void modify_key(char *modded_key, char *key) {
  strcpy(modded_key, key);
  int i, len = strlen(key); for(i = 0; i < len; ++i) if(key[i] == '_') modded_key[i] = ' ';
}
