/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "E2SM-KPM-IEs"
 * 	found in "/home/rshacham/e2sm-kpm-v01.02.asn"
 * 	`asn1c -fcompound-names`
 */

#ifndef	_UE_Report_Type_H_
#define	_UE_Report_Type_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum UE_Report_Type {
	UE_Report_Type_oDU_Report_Per_UE	= 0,
	UE_Report_Type_oCU_CP_Report_Per_UE	= 1,
	UE_Report_Type_oCU_UP_Report_Per_UE	= 2
	/*
	 * Enumeration is extensible
	 */
} e_UE_Report_Type;

/* UE-Report-Type */
typedef long	 UE_Report_Type_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_UE_Report_Type_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_UE_Report_Type;
extern const asn_INTEGER_specifics_t asn_SPC_UE_Report_Type_specs_1;
asn_struct_free_f UE_Report_Type_free;
asn_struct_print_f UE_Report_Type_print;
asn_constr_check_f UE_Report_Type_constraint;
ber_type_decoder_f UE_Report_Type_decode_ber;
der_type_encoder_f UE_Report_Type_encode_der;
xer_type_decoder_f UE_Report_Type_decode_xer;
xer_type_encoder_f UE_Report_Type_encode_xer;
oer_type_decoder_f UE_Report_Type_decode_oer;
oer_type_encoder_f UE_Report_Type_encode_oer;
per_type_decoder_f UE_Report_Type_decode_uper;
per_type_encoder_f UE_Report_Type_encode_uper;
per_type_decoder_f UE_Report_Type_decode_aper;
per_type_encoder_f UE_Report_Type_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _UE_Report_Type_H_ */
#include <asn_internal.h>