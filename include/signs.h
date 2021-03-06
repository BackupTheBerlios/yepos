/*yepos the PalmOS dictionary: application-wide constants
Copyright (C) 2008, 2009 D.T.Ineiev<ineiev@users.berlios.de>, super V 93
This program is under the GNU GPL v3+*/
/*this creator id was officially reserved for Ineiev (Nov 2008),
  so there should be no conflicts with other PalmOS software */
#define CREATOR_STRING "yEpo"
#define DB_TYPE_STRING "Dict"/*dictionary databases type*/
#define PALM_ID_FROM_STRING(a) (((((long)a[0])<<24)|\
 (((long)a[1])<<16))|((((long)a[2])<<8)|a[3]))
#define CREATOR	PALM_ID_FROM_STRING(CREATOR_STRING)
#define DB_TYPE	PALM_ID_FROM_STRING(DB_TYPE_STRING)
enum feature_bits
{compression_bit=1<<0,
 sort_table_bit=1<<1,
 implemented_features_bits=compression_bit|sort_table_bit
};
