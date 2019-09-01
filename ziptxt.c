/* AUTHOR: JORDAN RANDLEMAN -:- ZIPTXT TXT FILE LOSSLESS COMPRESSION/ENCRYPTION */
/*********************************************************************************
* /==//  ==== /|==\\ [====] \\// [====]       //==\ //==\\ /\\  //\ /|==\\ ||^\\ *
*   //    ||  ||==//   ||    )(    ||   +===+ ||    ||  || ||\\//|| ||==// ||_// *
*  //==/ ==== ||       ||   //\\   ||         \\==/ \\==// || \/ || ||     || \\ *
*********************************************************************************/
/**
 * compiles: $ gcc -o ziptxt ziptxt.c
 *   (COMPR) $ ./ziptxt yourFile.txt yourpassword
 * (DECOMPR) $ ./ziptxt yourFile.txt yourpassword
 * (DETAILS) $ ./ziptxt -l ...
 *
 * RESERVED CHARACTER SEQUENCES (for compression process):
 * (1) "qx"
 * (2) "qy" 
 * (3) "qz"
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#define MAX_CH 1000000
#define FEXIST(S) ({FILE*f;char str[]="rb";if(strcmp(&S[strlen(S-4)],".txt")==0){str[1]='\0';}((f=fopen(S,str))==NULL)?(false):({fclose(f);true;});})
#define ISNT_A_TXT_FILE(S) (strlen(S) < 5 || strcmp((S + strlen(S) - 4), ".txt") != 0)
#define ADD_FILENAME_EXTENSION(SEED,EXTEND,APPEND) ({strcpy(EXTEND,SEED);strcpy(&EXTEND[strlen(EXTEND)],APPEND);})
#define FLOOD_ZEROS(arr, len) ({int arr_i = 0;for(; arr_i < len; ++arr_i) arr[arr_i] = 0;})
#define IS_CAP_CH(ch_c_inst) ((ch_c_inst) >= 'A' && (ch_c_inst) <= 'Z')
#define CEIL(N,D) (1 + (((N) - 1) / (D)))
#define SS_KEY_UCH_TOT(str_total) ((str_total) - ((str_total) / 8))
#define SS_KEY_STR_TOT(uch_total) ((uch_total) + ((uch_total) / 7))
#define __PROGRAM__ ({char NAME[150];char*p=&__FILE__[strlen(__FILE__)-1];while(*(p-1)!='/')p--;strcpy(NAME,p);NAME;})
#define myAssert(C,M)\
  ({if(C==NULL){fprintf(stderr, "\n=> myAssert Failed: %s, program: %s, function: %s(), line: %d\n%s\n",#C,__PROGRAM__,__FUNCTION__,__LINE__,M);exit(0);}})
/* HIDE / SHOW FUNCTIONS */
void hide(char *, char *);
void show(char *, char *);
/* ERROR MESSAGES */
void check_for_reserved_qx_qy_qz_char_sequences(char *);
void confirm_valid_file(char *);
void err_info();
/* TEXT FILE HANDLER FUNCTIONS */
void show_txt_compressed(char *, char *);
void read_txt_engl(char *, char *);
void write_txt_compressed(char *);
/* BINARY FILE HANDLER FUNCTIONS */
void read_pass_ss_keys(char *);
void write_ss_keys_to_single_str(char [][151], int, char *);
/* COMMON WORD SUBSTITUTION FUNCTIONS */
void splice_str(char *, char *, int);
int delta_sub_common_words(char *, char [][50], char [][50], int);
void print_cw_used(int);
/* SUB NUMBER OF SEQUENTIAL LETTERS & VISE-VERSA */
void sequential_letters(char *);
void sequential_numbers(char *);
/* ENCRYPTION FUNCTION */
void delta_txt_crypt(char *, char *);
/* STRING EDITING FUNCITONS */
void rm_nxt_ch(char *, int);
void modify_str(char *, int);
bool is_at_substring(char *, char *);
/* NON-BITPACKABLE CHARACTER SUBSTITUTION FUNCTIONS */
void sub_out_replaceable_chars(char *);
void sub_back_in_replaceable_chars(char *);
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
void swap_str_sign(char [], unsigned char [], int, int);
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
/* NON BIT-PACKABLE SUBSTITUTED CHARS IN FZ1P COMPRESSION PROCESS */
#define TOTAL_REPLACEABLE_CHARS 65
char SUB_VALS[TOTAL_REPLACEABLE_CHARS+1] = "\"#$%&()*+,/0123456789:;<=>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`{|}~\n\t";
char SUB_KEYS[TOTAL_REPLACEABLE_CHARS][5] = { 
  "qxa","qxb","qxc","qxd","qxe","qxf","qxg","qxh","qxi","qxj","qxk","qxl","qxm",
  "qxn","qxo","qxp","qxq","qxr","qxs","qxt","qxu","qxv","qxw","qxx","qxy","qxz",
  "qya","qyb","qyc","qyd","qye","qyf","qyg","qyh","qyi","qyj","qyk","qyl","qym",
  "qyn","qyo","qyp","qyq","qyr","qys","qyt","qyu","qyv","qyw","qyx","qyy","qyz",
  "qza","qzb","qzc","qzd","qze","qzf","qzg","qzh","qzi","qzj","qzk","qzl","qzm",
};
/* GLOBAL VARIABLES */
unsigned char CW_LEN = FULL_CW_LEN, cw_shift_up_idxs[FULL_CW_LEN]; /* store indices of rmvd cw_keys */
char ss_array_matrix[300][45][151], s_compress_storage[300][151], s_max_buffer[MAX_CH], ss_single_str[MAX_CH];
char ss_refs[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01789#$<=>@[]^{|}~\0", nchar = '\n';
int cw_idxs[FULL_CW_LEN], chunk_count = 0, original_bytes = 0, compressed_bytes = 0;
bool zip_info = false;
int main(int argc, char *argv[]) {
  if(argc < 3 || argc > 4) err_info();
  char arg1[75], arg2[75], compr_arg2[75];
  if(argc == 4) {
    strcpy(arg1, argv[2]); strcpy(arg2, argv[3]);
    (strcmp(argv[1], "-l") == 0) ? (zip_info = true) : (fprintf(stderr, "\n\n(!!!) WRONG INFO FLAG => -l (!!!)\n\n"));
  } else { strcpy(arg1, argv[1]); strcpy(arg2, argv[2]); }
  ADD_FILENAME_EXTENSION(arg2,compr_arg2,"_sskey.TZ1P");
  confirm_valid_file(arg1); /* confirm text file exists, is not empty, & is less memory than MAX_CH */
  if(!FEXIST(compr_arg2)) hide(arg2, arg1); 
  else                    show(arg2, arg1); 
  return 0;
}
/******************************************************************************
* HIDE / SHOW FUNCTIONS
******************************************************************************/
void hide(char *arg2, char *arg1) {
  read_txt_engl(arg2, arg1); /* process text file in 150 char chunks */
  int ss_keys_byte = pack_ss_keys(arg2);
  write_txt_compressed(arg1); /* write compressed string to text file */
  /* output results */
  compressed_bytes += ss_keys_byte;
  float bytes_saved = 100*(1.000000 - ((float)compressed_bytes)/((float)original_bytes));
  printf("\n==============================================================================");
  printf("\n=> %s => COMPRESSED & ENCRYPTED!\n", arg1);
  printf(">> BYTES => UNCOMPRESSED: %d, COMPRESSED: %d, COMPRESSION RATE: %.2f%%\n", original_bytes, compressed_bytes, bytes_saved);
  printf("==============================================================================\n\n");
}
void show(char *arg2, char *arg1) {
  char filename[75];
  FLOOD_ZEROS(filename, 75);
  ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
  unpack_ss_keys(arg2);
  read_pass_ss_keys(arg2);
  show_txt_compressed(arg2, arg1);
  printf("\n>>> FILE REMOVED: %s\n", filename);
  printf("\n==============================================================================");
  printf("\n=> %s => DECOMPRESSED & DECRYPTED!\n", arg1);
  printf("==============================================================================\n\n");
}
/******************************************************************************
* ERROR MESSAGES
******************************************************************************/
void check_for_reserved_qx_qy_qz_char_sequences(char *str) {
  char *p = str;
  while(*(p+1) != '\0') {
    if(*p == 'q' && (*(p+1) == 'x' || *(p+1) == 'y' || *(p+1) == 'z')) {
      char temp[MAX_CH];
      FLOOD_ZEROS(temp, MAX_CH);
      strcpy(temp, p);
      int p_len = strlen(p);
      int length = p_len < 37 ? p_len : 37;
      temp[length] = '\0';
      fprintf(stderr, "\n=============================================================\n");
      fprintf(stderr, "ziptxt.c: WARNING, RESERVED CHARACTER SEQUENCE \"q%c\" DETECTED!\n", *(p+1));
      fprintf(stderr, "====== >> FOUND HERE: \"%s\"\n", temp);
      fprintf(stderr, "====== >> REMOVE TO COMPRESS FILE WITH ziptxt.c!\n");
      fprintf(stderr, "====== >> TERMINATING PROGRAM.\n");
      fprintf(stderr, "=============================================================\n\n");
      exit(EXIT_FAILURE);
    }
    ++p;
  }
}
void confirm_valid_file(char *filename) { /* confirms text file exists, non-empty, & is less memory than MAX_CH */
  struct stat buf;
  if(stat(filename, &buf)) {
    fprintf(stderr, "\n-:- ziptxt.c: WARNING, FILE \"%s\" DOES NOT EXIST! -:-\n", filename);
    fprintf(stderr, ">> Terminating Program.\n\n");
    exit(EXIT_FAILURE);
  }
  if(buf.st_size > MAX_CH || buf.st_size == 0) {
    if(buf.st_size > MAX_CH) {
      fprintf(stderr, "\n-:- ziptxt.c: WARNING, FILE \"%s\" SIZE %lld BYTES EXCEEDS %d BYTE CAP! -:- \n",filename,buf.st_size,MAX_CH); 
      fprintf(stderr, "-:- RAISE 'MAX_CH' MACRO LIMIT! -:- \n");
    } else fprintf(stderr, "\n-:- EMPTY FILES CAN'T BE COMPRESSED! -:- \n"); 
    fprintf(stderr, ">> Terminating Program.\n\n");
    exit(EXIT_FAILURE);
  }
  if(ISNT_A_TXT_FILE(filename)) { /* confirm file being compressed is a .txt */
    fprintf(stderr, "\n-:- ziptxt.c: WARNING, TEXT FILE \"%s\" IS MISSING \".txt\" EXTENSION! -:-\n", filename);
    fprintf(stderr, ">> Terminating Program.\n\n");
    exit(EXIT_FAILURE);
  }
}
void err_info() {
  fprintf(stderr, "\n============================= INVALID EXECUTION! =============================\n");
  fprintf(stderr, "$ gcc -o ziptxt ziptxt.c\n<(D)ENCRYPT / (DE)COMPRESS> $ ./ziptxt filename.txt yourpassword\n");
  fprintf(stderr, "==============================================================================");
  fprintf(stderr, "\n=> ENCRYPT / COMPRESS INFO: $ ./ziptxt -l filename.txt yourpassword\n");
  fprintf(stderr, "==============================================================================\n");
  fprintf(stderr, "=> RESERVED CHAR SEQUENCES: \"qx\", \"qy\", & \"qz\"\n");
  fprintf(stderr, "==============================================================================\n\n");
  exit(0);
}
/******************************************************************************
* TEXT FILE HANDLER FUNCTIONS
******************************************************************************/
void show_txt_compressed(char *arg2, char *arg1) {
  FILE *fp;
  char s_buffer[151];
  FLOOD_ZEROS(s_buffer, 151);
  int s_chunk_number = 0, i;
  myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING COMPRESSED TEXT FILE (!!!)\n");
  while(fgets(s_buffer, 151, fp) != NULL) { /* GET COMPRESSED TEXT */
    s_buffer[strlen(s_buffer) - 1] = '\0'; /* remove \n from s_buffer */
    if(zip_info) printf("\nENCRYPTED/COMPRESSED: %s\n", s_buffer);
    delta_txt_crypt(s_buffer, arg2); /* decrypt text */
    sequential_numbers(s_buffer);
    s_decompress(ss_array_matrix[s_chunk_number], s_buffer);
    delta_sub_common_words(s_buffer, cw_keys, cw_word, 0); /* remove keys, insert words */
    if(zip_info) printf("DECRYPTED/DECOMPRESS: %s\n", s_buffer);
    strcpy(s_compress_storage[s_chunk_number++], s_buffer);
  }
  for(i = 0; i < s_chunk_number; ++i) {
    char *p = s_max_buffer;
    if(i != 0) while(*p != '\0') ++p;
    sprintf(p, "%s", s_compress_storage[i]);
  }
  fclose(fp);
  /* PUT DECOMPRESSED TEXT */
  modify_str(s_max_buffer, 0); /* '_' => ' ' */
  sub_back_in_replaceable_chars(s_max_buffer);
  myAssert((fp = fopen(arg1, "w")), "\n\n(!!!) ERROR WRITING DECOMPRESSED TEXT FILE (!!!)\n");
  fwrite(s_max_buffer, sizeof(char), strlen(s_max_buffer), fp);
  fclose(fp);
}
void read_txt_engl(char *arg2, char *arg1) {
  char ss[45][151], s_chunk[151], temp_max_buff[MAX_CH]; int ret, i;
  FLOOD_ZEROS(s_chunk, 151);
  FILE *fp;
  /* READ FILE TO STORE CONTENTS */
  myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING TEXT FILE (COMMON WORD / RESERVED SEQUENCE SCANNING) (!!!)\n\n"); /* CHECK RESERVED SEQ'S/ADJUST CW */
  fread(temp_max_buff, sizeof(char), MAX_CH, fp);
  fclose(fp);
  original_bytes = strlen(temp_max_buff);
  check_for_reserved_qx_qy_qz_char_sequences(temp_max_buff);
  sub_out_replaceable_chars(temp_max_buff);
  hide_adjust_cw_keys(temp_max_buff);
  /* WRITE TO FILE ITS OLD CONTENTS WITH NON-BITPACKABLE CHARS NOW SUBSTITUTED OUT */
  myAssert((fp = fopen(arg1, "w")), "\n\n(!!!) ERROR WRITING TO TEXT FILE NON-BITPACKABLE CHAR SUBSTITUTIONS (!!!)\n\n"); /* SUB OUT NON-BITPACKABLE CHARS */
  fwrite(temp_max_buff, sizeof(char), strlen(temp_max_buff), fp);
  fclose(fp);
  /* COMPRESS FILE */
  myAssert((fp = fopen(arg1, "r")), "\n\n(!!!) ERROR READING TEXT FILE (TEXT STORING) (!!!)\n\n"); /* COMPRESS TEXT */
  while((ret = fread(s_chunk, sizeof(char), 150, fp)) > 0) {
    s_chunk[ret] = '\0';
    compressed_bytes += process_split_s(ss, s_chunk, s_compress_storage[chunk_count], arg2);
    ++chunk_count;
  }
  fclose(fp);
}
void write_txt_compressed(char *arg1) {
  FILE *fp; int i;
  myAssert((fp = fopen(arg1, "w")), "\n\n(!!!) ERROR WRITING COMPRESSED TEXT TO FILE (!!!)\n");
  for(i = 0; i < chunk_count; ++i) {
    fwrite(s_compress_storage[i], sizeof(char), strlen(s_compress_storage[i]), fp);
    fwrite(&nchar, sizeof(char), 1, fp);
  }
  fclose(fp);
}
/******************************************************************************
* BINARY FILE HANDLER FUNCTIONS
******************************************************************************/
void read_pass_ss_keys(char *arg2) {
  char ss_buffer[100], ch_buffer, filename[75];
  FLOOD_ZEROS(ss_buffer, 100);
  FLOOD_ZEROS(filename, 75);
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
      ++m; /* move to next 2d ss array (1 per s_chunk) in 3d matrix */
      n = 0; /* start from row (ss) 0 */
    } else { ++n; } /* move to next row (ss) */
  }
  fclose(fp);
  remove(filename); /* delete password txt file once ss keys retrieved */
}
void write_ss_keys_to_single_str(char ss[][151], int ss_total, char *arg2) {
  int i;
  for(i = 0; i < ss_total; ++i) {
    sprintf(&ss_single_str[strlen(ss_single_str)], "%s", ss[i]);
    if(i < ss_total - 1) sprintf(&ss_single_str[strlen(ss_single_str)], "%c", '\t');
  }
  sprintf(&ss_single_str[strlen(ss_single_str)], "%c", '\n');
}
/******************************************************************************
* COMMON WORD SUBSTITUTION FUNCTIONS
******************************************************************************/
void splice_str(char *s, char *sub, int splice_len) {
  char temp[MAX_CH]; 
  FLOOD_ZEROS(temp, MAX_CH);
  sprintf(temp, "%s%s", sub, s + splice_len); strcpy(s, temp);
}
int delta_sub_common_words(char *s, char remove[][50], char insert[][50], int hide_flag) {
  int count = 0, found, word_len, i, j;
  for(i = 0; i < CW_LEN; ++i) {
    char *p = s;
    word_len = strlen(remove[i]), found = 0;
    if(hide_flag == 0 && strcmp(insert[i], "\n") == 0) sprintf(insert[i], "_\n");
    while(*p != '\0') {
      if(*p == remove[i][0]) {
        for(j = 0; j < word_len; ++j) if(*(p + j) != remove[i][j]) break;
        if(j == word_len) {
          splice_str(p, insert[i], word_len);
          if(zip_info && found == 0) { cw_idxs[count++] = i; ++found; } /* store common words for 'info' command */
        }
      } else if(*p == '.' || *p == '!' || *p == '?') { ++p; } /* avoid chaining words: themself => !uself => !?slf */
      ++p;
    }
  }
  return count;
}
void print_cw_used(int sub_size) {
  printf("SUBSTITUTIONS:\n");
  int i; for(i = 0; i < sub_size; ++i) printf("=> %s: %s\n", cw_keys[cw_idxs[i]], cw_word[cw_idxs[i]]);
}
/******************************************************************************
* SUB NUMBER OF SEQUENTIAL LETTERS & VISE-VERSA
******************************************************************************/
void sequential_letters(char *s) {
  char *p = s, count_s[2]; int count;
  FLOOD_ZEROS(count_s, 2);
  while(*(p + 1) != '\0') {
    if(*(p + 1) == (*p + 1) && *(p + 2) == (*p + 2)) {
      for(count = 3; (count < 7 && *(p + count) == (*p + count)); ++count);
      sprintf(count_s, "%d", --count);
      splice_str(p + 1, count_s, count);
    }
    ++p;
  }
}
void sequential_numbers(char *s) {
  char *p = s, ch_arr[8];
  FLOOD_ZEROS(ch_arr, 8);
  while(*p != '\0') {
    if(*p >= '2' && *p <= '6') {
      int num = (int)(*p - 48), i;
      for(i = 1; i <= num; ++i) ch_arr[i - 1] = (*(p - 1) + i);
      ch_arr[i - 1] = '\0';
      splice_str(p, ch_arr, 1);
    }
    ++p;
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
      ch_count = 0, ++idx;
      srand((char)pass[idx]);
    }
    *s ^= (rand()%11+5);
    ++ch_count, ++s;
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
      if(*p == space) *p = under_s; /* ' ' => '_' */
      ++p;
    }
  } else { /* (decrypt) */
    while(*p != '\0') {
      if(*p == under_s) *p = space; /* '_' => ' ' */
      ++p;
    }
  }
}
bool is_at_substring(char *p, char *substr) {
  if(strlen(substr) > strlen(p)) return false;
  int i = 0, len = strlen(substr);
  for(; i < len; ++i) if(p[i] != substr[i]) return false;
  return true;
}
/******************************************************************************
* NON-BITPACKABLE CHARACTER SUBSTITUTION FUNCTIONS
******************************************************************************/
void sub_out_replaceable_chars(char *str) { /* Replace non-bitpackable chars with substitute keys */
  char buf[MAX_CH];
  FLOOD_ZEROS(buf, MAX_CH);
  char *p = str, *q = buf;
  int i = 0;
  while(*p != '\0') {                             /* traverse through string */
    for(i = 0; i < TOTAL_REPLACEABLE_CHARS; ++i)  /* search for replaceable characters */
      if(*p == SUB_VALS[i]) {
        strcpy(q, SUB_KEYS[i]);                   /* copy in substitute */
        q += strlen(q);
        ++p;
        break;
      }
    if(i == TOTAL_REPLACEABLE_CHARS) *q++ = *p++; /* not at replaceable char, thus copy */
  }
  *q = '\0';
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
}
void sub_back_in_replaceable_chars(char *str) { /* Resub non-bitpackable chars back into the string */
  char buf[MAX_CH];
  FLOOD_ZEROS(buf, MAX_CH);
  char *p = str, *q = buf;
  int i = 0;
  while(*p != '\0') {                             /* traverse through string */
    for(i = 0; i < TOTAL_REPLACEABLE_CHARS; ++i)  /* search for replaceable characters */
      if(is_at_substring(p, SUB_KEYS[i])) {
        *q++ = SUB_VALS[i];                       /* re-substitute in replaceable char */
        p += strlen(SUB_KEYS[i]);
        break;
      }
    if(i == TOTAL_REPLACEABLE_CHARS) *q++ = *p++; /* not at replaceable char, thus copy */
  }
  *q = '\0';
  FLOOD_ZEROS(str, MAX_CH);
  strcpy(str, buf);
}
/******************************************************************************
* EDIT COMMON WORD (CW) SUBS IF KEY IN TEXT (ANOMALY)
******************************************************************************/
void hide_adjust_cw_keys(char s[]) {
  int word_len, i, j;
  for(i = 0; i < CW_LEN; ++i) {
    char *p = s, modded_key[50];
    FLOOD_ZEROS(modded_key, 50);
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
  int i; for(i = 0; i < length; ++i) { shift_up_cw_keys((int)cw_shift_up_idxs[i]); CW_LEN--; }
}
void shift_up_cw_keys(int idx) {
  int i; for(i = idx; i < (CW_LEN - 1); ++i) { strcpy(cw_keys[i],cw_keys[i+1]); strcpy(cw_word[i],cw_word[i+1]);}
}
void modify_key(char *modded_key, char *key) {
  strcpy(modded_key, key);
  int i; for(i = 0; i < strlen(key); ++i) if(key[i] == '_') modded_key[i] = ' ';
}
/******************************************************************************
* DE/COMPRESSION FUNCTIONS
******************************************************************************/
void s_decompress(char ss[][151], char *s) {
  char *p = s, t[151];
  FLOOD_ZEROS(t, 151);
  while(*p != '\0') {
    if(is_ss_ref(0, *p) == 1) {
      sprintf(t, "%s%s", ss[ss_ch_index(*p)], p+1); sprintf(p, "%s", t);
      p = s;
    } else { ++p; }
  }
}
int s_compress(char ss[][151], char *s) { /* returns # of substrings */
  modify_str(s, 1); /* lowify & ' ' => '_' */
  int sub_word_count = delta_sub_common_words(s, cw_word, cw_keys, 1); /* remove common words, insert keys */
  if(zip_info) {
    printf("SUBD WORDS => LEN: %lu, STR: %s\n", strlen(s), s);
    print_cw_used(sub_word_count); /* print common words subbed and associated keys */
  }
  int ss_idx = 0, found;
  char *p = s, *r;
  while(*(p + 2) != '\0') { /* first ss instance */
    r = p + 2, found = 0;
    while(*(r+1) != '\0') { if(*r == *p && *(r+1) == *(p+1)) { ++found; rm_nxt_ch(r, ss_idx); } ++r; } /* second ss instance */
    if(found > 0) { /* if ss found, restart search for compounded ss */
      store_ss(ss, p, ss_idx);
      rm_nxt_ch(p, ss_idx);
      ++ss_idx, p = s;
    } else { ++p; }
  }
  return clean_ss(ss, s, ss_idx);
}
int process_split_s(char ss[][151], char *s, char *s_compress_storage, char *arg2) {
  if(zip_info) printf("\nSUB'D PUNC => LEN: %lu, STR: %s\n", strlen(s), s);
  int ss_total = s_compress(ss, s);
  if(zip_info) printf("COMPRESSED => LEN: %lu, STR: %s\n", strlen(s), s);
  sequential_letters(s);
  int compressed_bytes = strlen(s) + 1; /* compressed string length */
  write_ss_keys_to_single_str(ss, ss_total, arg2); /* store ss keys in bin"password"file */
  delta_txt_crypt(s, arg2); /* encrypt text */
  strcpy(s_compress_storage, s); /* store compressed string */
  if(zip_info) printf("ENCRYPTION => LEN: %lu, STR: %s\n", strlen(s), s);
  if(zip_info) print_ss(ss, ss_total);
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
  if(IS_CAP_CH(ch_c_idx)) { return (int)(ch_c_idx - 65); }
  else if((ch_c_idx >= '7' && ch_c_idx <= '9') || (ch_c_idx >= '<' && ch_c_idx <= '>')) { return (int)(ch_c_idx - 27); }
  char is[] = "01#$@[]^", diff[] = {22,22,4,4,28,54,50,50};
  int i; for(i = 0; i < 8; ++i) if(ch_c_idx == is[i]) return (int)(ch_c_idx - diff[i]);
  return (int)(ch_c_idx - 83);
}
char adjust_ref_char(char ss_ref_ch) {
  char is[] = "07#<@][{", diff[] = {-42,6,-22,24,2,2,27,29};
  int i; for(i = 0; i < 8; ++i) if(ss_ref_ch == is[i]) return (char)(ss_ref_ch - diff[i]);
  return (char)(ss_ref_ch - 1);
}
/******************************************************************************
* (S)UB(S)TRING FUNCTIONS
******************************************************************************/
void store_ss(char ss[][151], char *p, int ss_idx) { sprintf(ss[ss_idx], "%c%c", *p, *(p + 1)); }
int clean_ss(char ss[][151], char *s, int ss_total) {
  char *p; int nest_idx = 0, nested_ss[45], i; /* nested ss only ref'd by other ss, arr holds their idxs */
  FLOOD_ZEROS(nested_ss, 45);
  for(i = 0; i < ss_total; ++i) {
    p = s;
    while(*p != '\0') { if(*p == ss_refs[i]) { break; } ++p; }
    /* if ss_refs[i] !in s, i == nested ss ref idx */
    if(*p == '\0') nested_ss[nest_idx++] = i; /* nested ss idxs stored sequentially relative to ss_refs order */
  }
  splice_ss(ss, nested_ss, nest_idx, ss_total); /* put nested ss vals in ss referencing them */
  trim_ss(ss, s, nested_ss, nest_idx, ss_total); /* adjust ss & s references as per rmvd nested ss */
  return (ss_total - nest_idx); /* actual ss total = original ss total - nested ss total */
}
void splice_ss(char ss[][151], int nested_ss[45], int nest_total, int ss_total) {
  int i, j, l;
  for(i = 0; i < nest_total; ++i) { /* traverse nested ss array */
    for(j = 0; j < ss_total; ++j) { /* traverse ss array */
      int curr_ss_len = strlen(ss[j]);
      for(l = 0; l < curr_ss_len; ++l) { /* traverse ss */
        if(ss[j][l] == ss_refs[nested_ss[i]]) { /* if ss has nested ss reference */
          char nested_elem[151]; /* clone to hold nested ref value */
          FLOOD_ZEROS(nested_elem, 151);
          strcpy(nested_elem, ss[nested_ss[i]]); /* clone nested ref val */
          strcpy(&nested_elem[strlen(nested_elem)], &ss[j][l + 1]); /* splice post-nested ref ss tail to end of nested ref val */
          strcpy(&ss[j][l], nested_elem); /* splice nested ref val + ss end to ss ref position */
        }
      }
    }
  }
}
void trim_ss(char ss[][151], char *s, int nested_ss[45], int nest_total, int ss_total) {
  int i, j, l, k;
  for(i = 0; i < nest_total; ++i) { /* delete nested-ss */
    char *p = s;
    for(j = nested_ss[i]; j < (ss_total - 1); ++j) {
      strcpy(ss[j], ss[j + 1]); /* shift up non-nested ss to delete nested-ss */
      int curr_ss_len = strlen(ss[j]);
      /* ss-ss refs -= 1 ss_refs val per del  */
      for(l = 0; l < curr_ss_len; ++l) if(is_ss_ref((nested_ss[i] + 1), ss[j][l]) == 1) ss[j][l] = adjust_ref_char(ss[j][l]);
    }
    for (k = i; k < nest_total; ++k) nested_ss[k] = adjust_ref_char(nested_ss[k]); /* nested-ss idxs -= 1 ss_refs val per del */
    ss[j][0] = '\0';
    /* s ss references -= 1 ss_refs val as per del */
    while(*p != '\0') { if(is_ss_ref((nested_ss[i]+ 1), *p) == 1) { *p = adjust_ref_char(*p); } ++p; } /* if *p = nested ref */
  }
}
void print_ss(char ss[][151], int ss_total) {
  printf("COMPR SUBSTRS:\n");
  int i; for(i = 0; i < ss_total; ++i) printf("=> %c: %s\n", ss_refs[i], ss[i]);
}
/******************************************************************************
* SS_KEY BIT-PACKING COMPRESSION/DECOMPRESSION FUNCTIONS
******************************************************************************/
int pack_ss_keys(char *arg2) {
  /* PACK SS_KEYS */
  int passed_length = strlen(ss_single_str), char_total = SS_KEY_UCH_TOT(strlen(ss_single_str)), i, j, k;
  unsigned char cw_write_length = FULL_CW_LEN - CW_LEN, packed_str[char_total], passed_uch[passed_length];
  unsigned short char_total_uch = (unsigned short)char_total;
  FLOOD_ZEROS(packed_str, char_total);
  FLOOD_ZEROS(passed_uch, passed_length);
  swap_str_sign(ss_single_str, passed_uch, passed_length, 1); /* convert passed string into uchar str */
  for(i = 0, j = 0, k = 0; i < char_total; ++i, ++j, ++k) { /* PACK CHARS */
    packed_str[i] = (((passed_uch[k] << (j + 1)) | (passed_uch[k + 1] >> (6 - j))) & 255);
    if((i + 1) % 7 == 0 && i != 0) { j = -1; ++k; }
  }
  /* WRITE PACKED SS KEYS TO FILE */
  FILE *fp;
  char filename[75];
  FLOOD_ZEROS(filename, 75);
  ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
  myAssert((fp = fopen(filename, "wb")),"\n (!!!) ERROR WRITING PACKED SS KEYS TO BINARY FILE (!!!) \n");
  fwrite(&cw_write_length, sizeof(unsigned char), 1, fp); /* write modified cw_keys idxs length */
  fwrite(cw_shift_up_idxs, sizeof(unsigned char), cw_write_length, fp); /* write modified cw_keys idxs array */
  fwrite(&char_total_uch, sizeof(unsigned short), 1, fp); /* write ss_keys compr length */
  fwrite(packed_str, sizeof(unsigned char), char_total, fp); /* write ss_keys compr array */
  printf("\n>>> SIZE: %d - FILE CREATED: %s\n", (char_total + 3 + cw_write_length), filename);
  fclose(fp);
  return (char_total + 3 + cw_write_length);
}
void unpack_ss_keys(char *arg2) {
  /* READ PACKED SS KEYS FROM FILE */
  unsigned short char_total_uch;
  unsigned char cw_read_length;
  char filename[75], txtfilename[75], unpacked_str[MAX_CH];
  FLOOD_ZEROS(filename, 75);
  FLOOD_ZEROS(txtfilename, 75);
  FLOOD_ZEROS(unpacked_str, MAX_CH);
  ADD_FILENAME_EXTENSION(arg2,filename,"_sskey.TZ1P");
  ADD_FILENAME_EXTENSION(arg2,txtfilename,"_sskey.txt");
  FILE *fp;
  myAssert((fp = fopen(filename, "rb")),"\n (!!!) ERROR READING PACKED SS KEYS FROM BINARY FILE (!!!) \n");
  fread(&cw_read_length, sizeof(unsigned char), 1, fp); /* read modified cw_keys idxs length */
  fread(cw_shift_up_idxs, sizeof(unsigned char), cw_read_length, fp); /* read modified cw_keys idxs array */
  fread(&char_total_uch, sizeof(unsigned short), 1, fp); /* read ss_keys compr length */
  int char_total = (int)char_total_uch, i, j, k;
  int str_total = SS_KEY_STR_TOT(char_total);
  unsigned char packed_str[char_total], unpacked_uch[str_total];
  FLOOD_ZEROS(packed_str, char_total);
  FLOOD_ZEROS(unpacked_uch, str_total);
  fread(packed_str, sizeof(unsigned char), char_total, fp); /* read ss_keys compr array */
  remove(filename);
  fclose(fp);
  show_adjust_cw_keys(cw_read_length);
  FLOOD_ZEROS(unpacked_uch, str_total);
  /* UNPACK SS_KEYS */
  for(i = 0, k = 0; i < str_total && k < char_total; ++i, ++k) { /* UNPACK PACKED CHARS */
    unpacked_uch[i] = ((packed_str[k] >> 1) & 127);
    for(j = 0; j < 6 && i < str_total && k < char_total; ++j, ++i, ++k) {
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
void swap_str_sign(char s[], unsigned char us[], int length, int sign_flag) { 
  if(sign_flag == 1) { int i; for(i = 0; i < length; ++i) us[i] = (unsigned char)s[i]; } /* make unsigned */
  else { int i; for(i = 0; i < length; ++i) s[i] = (char)us[i]; } /* make signed */
}
