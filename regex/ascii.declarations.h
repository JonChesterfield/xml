#ifndef ASCII_DECLARATIONS_H_INCLUDED
#define ASCII_DECLARATIONS_H_INCLUDED
#include "regex.declarations.h"

// Currently only defines the bytes, can add to that if useful
// Force the 1:1 correspondance with whatever order regex.declarations.h chose
enum ascii_grouping {

  ascii_grouping_byte_00 = regex_grouping_byte_00,
  ascii_grouping_byte_01 = regex_grouping_byte_01,
  ascii_grouping_byte_02 = regex_grouping_byte_02,
  ascii_grouping_byte_03 = regex_grouping_byte_03,
  ascii_grouping_byte_04 = regex_grouping_byte_04,
  ascii_grouping_byte_05 = regex_grouping_byte_05,
  ascii_grouping_byte_06 = regex_grouping_byte_06,
  ascii_grouping_byte_07 = regex_grouping_byte_07,
  ascii_grouping_byte_08 = regex_grouping_byte_08,
  ascii_grouping_byte_09 = regex_grouping_byte_09,
  ascii_grouping_byte_0a = regex_grouping_byte_0a,
  ascii_grouping_byte_0b = regex_grouping_byte_0b,
  ascii_grouping_byte_0c = regex_grouping_byte_0c,
  ascii_grouping_byte_0d = regex_grouping_byte_0d,
  ascii_grouping_byte_0e = regex_grouping_byte_0e,
  ascii_grouping_byte_0f = regex_grouping_byte_0f,
  ascii_grouping_byte_10 = regex_grouping_byte_10,
  ascii_grouping_byte_11 = regex_grouping_byte_11,
  ascii_grouping_byte_12 = regex_grouping_byte_12,
  ascii_grouping_byte_13 = regex_grouping_byte_13,
  ascii_grouping_byte_14 = regex_grouping_byte_14,
  ascii_grouping_byte_15 = regex_grouping_byte_15,
  ascii_grouping_byte_16 = regex_grouping_byte_16,
  ascii_grouping_byte_17 = regex_grouping_byte_17,
  ascii_grouping_byte_18 = regex_grouping_byte_18,
  ascii_grouping_byte_19 = regex_grouping_byte_19,
  ascii_grouping_byte_1a = regex_grouping_byte_1a,
  ascii_grouping_byte_1b = regex_grouping_byte_1b,
  ascii_grouping_byte_1c = regex_grouping_byte_1c,
  ascii_grouping_byte_1d = regex_grouping_byte_1d,
  ascii_grouping_byte_1e = regex_grouping_byte_1e,
  ascii_grouping_byte_1f = regex_grouping_byte_1f,
  ascii_grouping_byte_20 = regex_grouping_byte_20,
  ascii_grouping_byte_21 = regex_grouping_byte_21,
  ascii_grouping_byte_22 = regex_grouping_byte_22,
  ascii_grouping_byte_23 = regex_grouping_byte_23,
  ascii_grouping_byte_24 = regex_grouping_byte_24,
  ascii_grouping_byte_25 = regex_grouping_byte_25,
  ascii_grouping_byte_26 = regex_grouping_byte_26,
  ascii_grouping_byte_27 = regex_grouping_byte_27,
  ascii_grouping_byte_28 = regex_grouping_byte_28,
  ascii_grouping_byte_29 = regex_grouping_byte_29,
  ascii_grouping_byte_2a = regex_grouping_byte_2a,
  ascii_grouping_byte_2b = regex_grouping_byte_2b,
  ascii_grouping_byte_2c = regex_grouping_byte_2c,
  ascii_grouping_byte_2d = regex_grouping_byte_2d,
  ascii_grouping_byte_2e = regex_grouping_byte_2e,
  ascii_grouping_byte_2f = regex_grouping_byte_2f,
  ascii_grouping_byte_30 = regex_grouping_byte_30,
  ascii_grouping_byte_31 = regex_grouping_byte_31,
  ascii_grouping_byte_32 = regex_grouping_byte_32,
  ascii_grouping_byte_33 = regex_grouping_byte_33,
  ascii_grouping_byte_34 = regex_grouping_byte_34,
  ascii_grouping_byte_35 = regex_grouping_byte_35,
  ascii_grouping_byte_36 = regex_grouping_byte_36,
  ascii_grouping_byte_37 = regex_grouping_byte_37,
  ascii_grouping_byte_38 = regex_grouping_byte_38,
  ascii_grouping_byte_39 = regex_grouping_byte_39,
  ascii_grouping_byte_3a = regex_grouping_byte_3a,
  ascii_grouping_byte_3b = regex_grouping_byte_3b,
  ascii_grouping_byte_3c = regex_grouping_byte_3c,
  ascii_grouping_byte_3d = regex_grouping_byte_3d,
  ascii_grouping_byte_3e = regex_grouping_byte_3e,
  ascii_grouping_byte_3f = regex_grouping_byte_3f,
  ascii_grouping_byte_40 = regex_grouping_byte_40,
  ascii_grouping_byte_41 = regex_grouping_byte_41,
  ascii_grouping_byte_42 = regex_grouping_byte_42,
  ascii_grouping_byte_43 = regex_grouping_byte_43,
  ascii_grouping_byte_44 = regex_grouping_byte_44,
  ascii_grouping_byte_45 = regex_grouping_byte_45,
  ascii_grouping_byte_46 = regex_grouping_byte_46,
  ascii_grouping_byte_47 = regex_grouping_byte_47,
  ascii_grouping_byte_48 = regex_grouping_byte_48,
  ascii_grouping_byte_49 = regex_grouping_byte_49,
  ascii_grouping_byte_4a = regex_grouping_byte_4a,
  ascii_grouping_byte_4b = regex_grouping_byte_4b,
  ascii_grouping_byte_4c = regex_grouping_byte_4c,
  ascii_grouping_byte_4d = regex_grouping_byte_4d,
  ascii_grouping_byte_4e = regex_grouping_byte_4e,
  ascii_grouping_byte_4f = regex_grouping_byte_4f,
  ascii_grouping_byte_50 = regex_grouping_byte_50,
  ascii_grouping_byte_51 = regex_grouping_byte_51,
  ascii_grouping_byte_52 = regex_grouping_byte_52,
  ascii_grouping_byte_53 = regex_grouping_byte_53,
  ascii_grouping_byte_54 = regex_grouping_byte_54,
  ascii_grouping_byte_55 = regex_grouping_byte_55,
  ascii_grouping_byte_56 = regex_grouping_byte_56,
  ascii_grouping_byte_57 = regex_grouping_byte_57,
  ascii_grouping_byte_58 = regex_grouping_byte_58,
  ascii_grouping_byte_59 = regex_grouping_byte_59,
  ascii_grouping_byte_5a = regex_grouping_byte_5a,
  ascii_grouping_byte_5b = regex_grouping_byte_5b,
  ascii_grouping_byte_5c = regex_grouping_byte_5c,
  ascii_grouping_byte_5d = regex_grouping_byte_5d,
  ascii_grouping_byte_5e = regex_grouping_byte_5e,
  ascii_grouping_byte_5f = regex_grouping_byte_5f,
  ascii_grouping_byte_60 = regex_grouping_byte_60,
  ascii_grouping_byte_61 = regex_grouping_byte_61,
  ascii_grouping_byte_62 = regex_grouping_byte_62,
  ascii_grouping_byte_63 = regex_grouping_byte_63,
  ascii_grouping_byte_64 = regex_grouping_byte_64,
  ascii_grouping_byte_65 = regex_grouping_byte_65,
  ascii_grouping_byte_66 = regex_grouping_byte_66,
  ascii_grouping_byte_67 = regex_grouping_byte_67,
  ascii_grouping_byte_68 = regex_grouping_byte_68,
  ascii_grouping_byte_69 = regex_grouping_byte_69,
  ascii_grouping_byte_6a = regex_grouping_byte_6a,
  ascii_grouping_byte_6b = regex_grouping_byte_6b,
  ascii_grouping_byte_6c = regex_grouping_byte_6c,
  ascii_grouping_byte_6d = regex_grouping_byte_6d,
  ascii_grouping_byte_6e = regex_grouping_byte_6e,
  ascii_grouping_byte_6f = regex_grouping_byte_6f,
  ascii_grouping_byte_70 = regex_grouping_byte_70,
  ascii_grouping_byte_71 = regex_grouping_byte_71,
  ascii_grouping_byte_72 = regex_grouping_byte_72,
  ascii_grouping_byte_73 = regex_grouping_byte_73,
  ascii_grouping_byte_74 = regex_grouping_byte_74,
  ascii_grouping_byte_75 = regex_grouping_byte_75,
  ascii_grouping_byte_76 = regex_grouping_byte_76,
  ascii_grouping_byte_77 = regex_grouping_byte_77,
  ascii_grouping_byte_78 = regex_grouping_byte_78,
  ascii_grouping_byte_79 = regex_grouping_byte_79,
  ascii_grouping_byte_7a = regex_grouping_byte_7a,
  ascii_grouping_byte_7b = regex_grouping_byte_7b,
  ascii_grouping_byte_7c = regex_grouping_byte_7c,
  ascii_grouping_byte_7d = regex_grouping_byte_7d,
  ascii_grouping_byte_7e = regex_grouping_byte_7e,
  ascii_grouping_byte_7f = regex_grouping_byte_7f,
  ascii_grouping_byte_80 = regex_grouping_byte_80,
  ascii_grouping_byte_81 = regex_grouping_byte_81,
  ascii_grouping_byte_82 = regex_grouping_byte_82,
  ascii_grouping_byte_83 = regex_grouping_byte_83,
  ascii_grouping_byte_84 = regex_grouping_byte_84,
  ascii_grouping_byte_85 = regex_grouping_byte_85,
  ascii_grouping_byte_86 = regex_grouping_byte_86,
  ascii_grouping_byte_87 = regex_grouping_byte_87,
  ascii_grouping_byte_88 = regex_grouping_byte_88,
  ascii_grouping_byte_89 = regex_grouping_byte_89,
  ascii_grouping_byte_8a = regex_grouping_byte_8a,
  ascii_grouping_byte_8b = regex_grouping_byte_8b,
  ascii_grouping_byte_8c = regex_grouping_byte_8c,
  ascii_grouping_byte_8d = regex_grouping_byte_8d,
  ascii_grouping_byte_8e = regex_grouping_byte_8e,
  ascii_grouping_byte_8f = regex_grouping_byte_8f,
  ascii_grouping_byte_90 = regex_grouping_byte_90,
  ascii_grouping_byte_91 = regex_grouping_byte_91,
  ascii_grouping_byte_92 = regex_grouping_byte_92,
  ascii_grouping_byte_93 = regex_grouping_byte_93,
  ascii_grouping_byte_94 = regex_grouping_byte_94,
  ascii_grouping_byte_95 = regex_grouping_byte_95,
  ascii_grouping_byte_96 = regex_grouping_byte_96,
  ascii_grouping_byte_97 = regex_grouping_byte_97,
  ascii_grouping_byte_98 = regex_grouping_byte_98,
  ascii_grouping_byte_99 = regex_grouping_byte_99,
  ascii_grouping_byte_9a = regex_grouping_byte_9a,
  ascii_grouping_byte_9b = regex_grouping_byte_9b,
  ascii_grouping_byte_9c = regex_grouping_byte_9c,
  ascii_grouping_byte_9d = regex_grouping_byte_9d,
  ascii_grouping_byte_9e = regex_grouping_byte_9e,
  ascii_grouping_byte_9f = regex_grouping_byte_9f,
  ascii_grouping_byte_a0 = regex_grouping_byte_a0,
  ascii_grouping_byte_a1 = regex_grouping_byte_a1,
  ascii_grouping_byte_a2 = regex_grouping_byte_a2,
  ascii_grouping_byte_a3 = regex_grouping_byte_a3,
  ascii_grouping_byte_a4 = regex_grouping_byte_a4,
  ascii_grouping_byte_a5 = regex_grouping_byte_a5,
  ascii_grouping_byte_a6 = regex_grouping_byte_a6,
  ascii_grouping_byte_a7 = regex_grouping_byte_a7,
  ascii_grouping_byte_a8 = regex_grouping_byte_a8,
  ascii_grouping_byte_a9 = regex_grouping_byte_a9,
  ascii_grouping_byte_aa = regex_grouping_byte_aa,
  ascii_grouping_byte_ab = regex_grouping_byte_ab,
  ascii_grouping_byte_ac = regex_grouping_byte_ac,
  ascii_grouping_byte_ad = regex_grouping_byte_ad,
  ascii_grouping_byte_ae = regex_grouping_byte_ae,
  ascii_grouping_byte_af = regex_grouping_byte_af,
  ascii_grouping_byte_b0 = regex_grouping_byte_b0,
  ascii_grouping_byte_b1 = regex_grouping_byte_b1,
  ascii_grouping_byte_b2 = regex_grouping_byte_b2,
  ascii_grouping_byte_b3 = regex_grouping_byte_b3,
  ascii_grouping_byte_b4 = regex_grouping_byte_b4,
  ascii_grouping_byte_b5 = regex_grouping_byte_b5,
  ascii_grouping_byte_b6 = regex_grouping_byte_b6,
  ascii_grouping_byte_b7 = regex_grouping_byte_b7,
  ascii_grouping_byte_b8 = regex_grouping_byte_b8,
  ascii_grouping_byte_b9 = regex_grouping_byte_b9,
  ascii_grouping_byte_ba = regex_grouping_byte_ba,
  ascii_grouping_byte_bb = regex_grouping_byte_bb,
  ascii_grouping_byte_bc = regex_grouping_byte_bc,
  ascii_grouping_byte_bd = regex_grouping_byte_bd,
  ascii_grouping_byte_be = regex_grouping_byte_be,
  ascii_grouping_byte_bf = regex_grouping_byte_bf,
  ascii_grouping_byte_c0 = regex_grouping_byte_c0,
  ascii_grouping_byte_c1 = regex_grouping_byte_c1,
  ascii_grouping_byte_c2 = regex_grouping_byte_c2,
  ascii_grouping_byte_c3 = regex_grouping_byte_c3,
  ascii_grouping_byte_c4 = regex_grouping_byte_c4,
  ascii_grouping_byte_c5 = regex_grouping_byte_c5,
  ascii_grouping_byte_c6 = regex_grouping_byte_c6,
  ascii_grouping_byte_c7 = regex_grouping_byte_c7,
  ascii_grouping_byte_c8 = regex_grouping_byte_c8,
  ascii_grouping_byte_c9 = regex_grouping_byte_c9,
  ascii_grouping_byte_ca = regex_grouping_byte_ca,
  ascii_grouping_byte_cb = regex_grouping_byte_cb,
  ascii_grouping_byte_cc = regex_grouping_byte_cc,
  ascii_grouping_byte_cd = regex_grouping_byte_cd,
  ascii_grouping_byte_ce = regex_grouping_byte_ce,
  ascii_grouping_byte_cf = regex_grouping_byte_cf,
  ascii_grouping_byte_d0 = regex_grouping_byte_d0,
  ascii_grouping_byte_d1 = regex_grouping_byte_d1,
  ascii_grouping_byte_d2 = regex_grouping_byte_d2,
  ascii_grouping_byte_d3 = regex_grouping_byte_d3,
  ascii_grouping_byte_d4 = regex_grouping_byte_d4,
  ascii_grouping_byte_d5 = regex_grouping_byte_d5,
  ascii_grouping_byte_d6 = regex_grouping_byte_d6,
  ascii_grouping_byte_d7 = regex_grouping_byte_d7,
  ascii_grouping_byte_d8 = regex_grouping_byte_d8,
  ascii_grouping_byte_d9 = regex_grouping_byte_d9,
  ascii_grouping_byte_da = regex_grouping_byte_da,
  ascii_grouping_byte_db = regex_grouping_byte_db,
  ascii_grouping_byte_dc = regex_grouping_byte_dc,
  ascii_grouping_byte_dd = regex_grouping_byte_dd,
  ascii_grouping_byte_de = regex_grouping_byte_de,
  ascii_grouping_byte_df = regex_grouping_byte_df,
  ascii_grouping_byte_e0 = regex_grouping_byte_e0,
  ascii_grouping_byte_e1 = regex_grouping_byte_e1,
  ascii_grouping_byte_e2 = regex_grouping_byte_e2,
  ascii_grouping_byte_e3 = regex_grouping_byte_e3,
  ascii_grouping_byte_e4 = regex_grouping_byte_e4,
  ascii_grouping_byte_e5 = regex_grouping_byte_e5,
  ascii_grouping_byte_e6 = regex_grouping_byte_e6,
  ascii_grouping_byte_e7 = regex_grouping_byte_e7,
  ascii_grouping_byte_e8 = regex_grouping_byte_e8,
  ascii_grouping_byte_e9 = regex_grouping_byte_e9,
  ascii_grouping_byte_ea = regex_grouping_byte_ea,
  ascii_grouping_byte_eb = regex_grouping_byte_eb,
  ascii_grouping_byte_ec = regex_grouping_byte_ec,
  ascii_grouping_byte_ed = regex_grouping_byte_ed,
  ascii_grouping_byte_ee = regex_grouping_byte_ee,
  ascii_grouping_byte_ef = regex_grouping_byte_ef,
  ascii_grouping_byte_f0 = regex_grouping_byte_f0,
  ascii_grouping_byte_f1 = regex_grouping_byte_f1,
  ascii_grouping_byte_f2 = regex_grouping_byte_f2,
  ascii_grouping_byte_f3 = regex_grouping_byte_f3,
  ascii_grouping_byte_f4 = regex_grouping_byte_f4,
  ascii_grouping_byte_f5 = regex_grouping_byte_f5,
  ascii_grouping_byte_f6 = regex_grouping_byte_f6,
  ascii_grouping_byte_f7 = regex_grouping_byte_f7,
  ascii_grouping_byte_f8 = regex_grouping_byte_f8,
  ascii_grouping_byte_f9 = regex_grouping_byte_f9,
  ascii_grouping_byte_fa = regex_grouping_byte_fa,
  ascii_grouping_byte_fb = regex_grouping_byte_fb,
  ascii_grouping_byte_fc = regex_grouping_byte_fc,
  ascii_grouping_byte_fd = regex_grouping_byte_fd,
  ascii_grouping_byte_fe = regex_grouping_byte_fe,
  ascii_grouping_byte_ff = regex_grouping_byte_ff,
};
#endif

