#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <epan/packet.h>
#include <epan/expert.h>
#include <epan/prefs.h>

#define FRCDS_PORT 1110

#define FRCDS_RESET_FLAG 0x80
#define FRCDS_NOTSTOP_FLAG 0x40
#define FRCDS_ENABLE_FLAG 0x20
#define FRCDS_AUTON_FLAG 0x10
#define FRCDS_FMS_FLAG 0x08
#define FRCDS_RESYNC_FLAG 0x04
#define FRCDS_CRIOCHK_FLAG 0x02
#define FRCDS_FPGACHK_FLAG 0x01

static int proto_frcds = -1;
/* BEGIN FRCCommonControlData */
static int hf_frcds_packetIndex = -1;

/* Control Byte and included bitfields */
static int hf_frcds_control = -1;
static int hf_frcds_resetflag = -1;
static int hf_frcds_notStopflag = -1;
static int hf_frcds_enableflag = -1;
static int hf_frcds_autonflag = -1;
static int hf_frcds_fmsflag = -1;
static int hf_frcds_resyncflag = -1;
static int hf_frcds_cRIOchkflag = -1;
static int hf_frcds_fpgachkflag = -1;

/* Digitals, Team Number, and DS Location */
static int hf_frcds_digIn = -1;
static int hf_frcds_teamID = -1;
static int hf_frcds_location = -1;
static int hf_frcds_alliance = -1;
static int hf_frcds_position = -1;

/* Joystick blocks */
static int hf_frcds_stick0Axis1 = -1;
static int hf_frcds_stick0Axis2 = -1;
static int hf_frcds_stick0Axis3 = -1;
static int hf_frcds_stick0Axis4 = -1;
static int hf_frcds_stick0Axis5 = -1;
static int hf_frcds_stick0Axis6 = -1;
static int hf_frcds_stick0Buttons = -1;

static int hf_frcds_stick1Axis1 = -1;
static int hf_frcds_stick1Axis2 = -1;
static int hf_frcds_stick1Axis3 = -1;
static int hf_frcds_stick1Axis4 = -1;
static int hf_frcds_stick1Axis5 = -1;
static int hf_frcds_stick1Axis6 = -1;
static int hf_frcds_stick1Buttons = -1;

static int hf_frcds_stick2Axis1 = -1;
static int hf_frcds_stick2Axis2 = -1;
static int hf_frcds_stick2Axis3 = -1;
static int hf_frcds_stick2Axis4 = -1;
static int hf_frcds_stick2Axis5 = -1;
static int hf_frcds_stick2Axis6 = -1;
static int hf_frcds_stick2Buttons = -1;

static int hf_frcds_stick3Axis1 = -1;
static int hf_frcds_stick3Axis2 = -1;
static int hf_frcds_stick3Axis3 = -1;
static int hf_frcds_stick3Axis4 = -1;
static int hf_frcds_stick3Axis5 = -1;
static int hf_frcds_stick3Axis6 = -1;
static int hf_frcds_stick3Buttons = -1;

/* Struct for referencing the fields of a joystick */
typedef struct _stickFields {
	int * const btn;
	int * const axis[6];
} stickFields;

/* Convenience array for when parsing joystick data */
static const stickFields hf_frcds_sticks[] = {
	{ &hf_frcds_stick0Buttons, {
		&hf_frcds_stick0Axis1,
		&hf_frcds_stick0Axis2,
		&hf_frcds_stick0Axis3,
		&hf_frcds_stick0Axis4,
		&hf_frcds_stick0Axis5,
		&hf_frcds_stick0Axis6}
	},
	{ &hf_frcds_stick1Buttons, {
		&hf_frcds_stick1Axis1,
		&hf_frcds_stick1Axis2,
		&hf_frcds_stick1Axis3,
		&hf_frcds_stick1Axis4,
		&hf_frcds_stick1Axis5,
		&hf_frcds_stick1Axis6}
	},
	{ &hf_frcds_stick2Buttons, {
		&hf_frcds_stick2Axis1,
		&hf_frcds_stick2Axis2,
		&hf_frcds_stick2Axis3,
		&hf_frcds_stick2Axis4,
		&hf_frcds_stick2Axis5,
		&hf_frcds_stick2Axis6}
	},
	{ &hf_frcds_stick3Buttons, {
		&hf_frcds_stick3Axis1,
		&hf_frcds_stick3Axis2,
		&hf_frcds_stick3Axis3,
		&hf_frcds_stick3Axis4,
		&hf_frcds_stick3Axis5,
		&hf_frcds_stick3Axis6}
	}
};
	

/* Analog Data */
static int hf_frcds_analog1 = -1;
static int hf_frcds_analog2 = -1;
static int hf_frcds_analog3 = -1;
static int hf_frcds_analog4 = -1;

/* Array of the analog fields, allows a for loop to
 * handle all the analog data */
static int * const hf_frcds_analogs[] = {
	&hf_frcds_analog1,
	&hf_frcds_analog2,
	&hf_frcds_analog3,
	&hf_frcds_analog4
};

/* Checksum Block */
static int hf_frcds_cRIOchecksum = -1;
static int hf_frcds_FPGAchecksum1 = -1;
static int hf_frcds_FPGAchecksum2 = -1;
static int hf_frcds_FPGAchecksum3 = -1;
static int hf_frcds_FPGAchecksum4 = -1;

/* Version Data Block */
static int hf_frcds_version = -1;
/* END FRCCommonControlData */

/* BEGIN Enhanced IO */
static int hf_frcds_eio_in = -1;
static int hf_frcds_eio_out = -1;

/* BEGIN EIO Inputs */
static int hf_frcds_eio_apiver = -1;
static int hf_frcds_eio_fwver = -1;
static int hf_frcds_eio_ana1 = -1;
static int hf_frcds_eio_ana2 = -1;
static int hf_frcds_eio_ana3 = -1;
static int hf_frcds_eio_ana4 = -1;
static int hf_frcds_eio_ana5 = -1;
static int hf_frcds_eio_ana6 = -1;
static int hf_frcds_eio_ana7 = -1;
static int hf_frcds_eio_ana8 = -1;

/* Array of the analog fields, allows for easy population */
static int * const hf_frcds_eio_analogs[] = {
	&hf_frcds_eio_ana1,
	&hf_frcds_eio_ana2,
	&hf_frcds_eio_ana3,
	&hf_frcds_eio_ana4,
	&hf_frcds_eio_ana5,
	&hf_frcds_eio_ana6,
	&hf_frcds_eio_ana7,
	&hf_frcds_eio_ana8
};

static int hf_frcds_eio_digin = -1;
static int hf_frcds_eio_accel1 = -1;
static int hf_frcds_eio_accel2 = -1;
static int hf_frcds_eio_accel3 = -1;

static int * const hf_frcds_eio_accels[] = {
	&hf_frcds_eio_accel1,
	&hf_frcds_eio_accel2,
	&hf_frcds_eio_accel3
};

static int hf_frcds_eio_quad1 = -1;
static int hf_frcds_eio_quad2 = -1;
static int hf_frcds_eio_buttons = -1;
static int hf_frcds_eio_capslide = -1;
static int hf_frcds_eio_capprox = -1;
/* END EIO Inputs */

/* BEGIN EIO Outputs */
static int hf_frcds_eio_digout = -1;
static int hf_frcds_eio_digout_oe = -1;
static int hf_frcds_eio_digout_pe = -1;
static int hf_frcds_eio_pwm_compare1 = -1;
static int hf_frcds_eio_pwm_compare2 = -1;
static int hf_frcds_eio_pwm_compare3 = -1;
static int hf_frcds_eio_pwm_compare4 = -1;

static int * const hf_frcds_eio_pwm_compares[] = {
	&hf_frcds_eio_pwm_compare1,
	&hf_frcds_eio_pwm_compare2,
	&hf_frcds_eio_pwm_compare3,
	&hf_frcds_eio_pwm_compare4
};

static int hf_frcds_eio_pwm_period1 = -1;
static int hf_frcds_eio_pwm_period2 = -1;
static int hf_frcds_eio_dac1 = -1;
static int hf_frcds_eio_dac2 = -1;
static int hf_frcds_eio_leds = -1;
static int hf_frcds_eio_enables = -1;
static int hf_frcds_eio_pwm_enable = -1;
static int hf_frcds_eio_comparator_enable = -1;
static int hf_frcds_eio_quad_index_enable = -1;
static int hf_frcds_eio_fixed_digout = -1;
static int hf_frcds_eio_flags = -1;
static int hf_frcds_eio_flags_valid = -1;
static int hf_frcds_eio_flags_conf = -1;
static int hf_frcds_eio_flags_enhance = -1;

/* Subtrees */
static gint ett_frcds = -1;
static gint ett_frcds_control = -1;
static gint ett_frcds_location = -1;
static gint ett_frcds_sum = -1;
static gint ett_frcds_stick0 = -1;
static gint ett_frcds_stick1 = -1;
static gint ett_frcds_stick2 = -1;
static gint ett_frcds_stick3 = -1;
static gint ett_frcds_cio = -1;
static gint ett_frcds_eio = -1;
static gint ett_frcds_eio_out = -1;
static gint ett_frcds_eio_enables = -1;
static gint ett_frcds_eio_in = -1;
static gint ett_frcds_eio_flags = -1;

static gint * const ett_frcds_sticks[] = {
	&ett_frcds_stick0,
	&ett_frcds_stick1,
	&ett_frcds_stick2,
	&ett_frcds_stick3
};

/* Preferences */

/* Show packet summary in proto tree */
static gboolean frcds_summary_in_tree = TRUE;

/* "enum" for team Alliance Colors - makes team color a string but
 * still filterable easily */
static const value_string teamcolors[] = {
	{ 0, "Unknown" },
	{ 1, "Red" },
	{ 2, "Blue" },
	{ 0, NULL }
};

static void dissect_frcds_eio_status(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	gint offset = 0;
	guint8 length, type;
	proto_item *item = NULL;
	proto_tree *eio_tree = NULL;

	length = tvb_length(tvb);
	type = tvb_get_guint8(tvb, offset);
	offset += 1;
	
	if (length != 25 || type != 18) { /* Check protocol constants */
		item = proto_tree_add_text(tree, tvb, 0, -1, "Enhanced IO Output [ERROR]");
		expert_add_info_format(pinfo, item, PI_MALFORMED, PI_ERROR, "Invalid status block length %u and/or type %u (expected 25 and 18)", length, type);
		col_append_fstr(pinfo->cinfo, COL_INFO, " [BAD EIO STATUS BLOCK]");
		return;
	}
	
	if (tree)
	{
		item = proto_tree_add_item(tree, hf_frcds_eio_out, tvb, 0, -1, FALSE);
		if (item) eio_tree = proto_item_add_subtree(item, ett_frcds_eio_in);
		if (eio_tree)
		{
			/* Add EIO Status data here */
		}
	}
}

static void dissect_frcds_eio_control(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	gint offset = 0;
	guint8 length, type;
	proto_item *item = NULL;
	proto_tree *eio_tree = NULL;

	length = tvb_length(tvb);
	type = tvb_get_guint8(tvb, offset);
	offset += 1;

	if (length != 34 || type != 17) { /* Check protocol constants */
		item = proto_tree_add_text(tree, tvb, 0, -1, "Enhanced I/O Input [ERROR]");
		expert_add_info_format(pinfo, item, PI_MALFORMED, PI_ERROR, "Invalid control block length %u and/or type %u (expected 34 and 17)", length, type);
		col_append_fstr(pinfo->cinfo, COL_INFO, " [BAD EIO CONTROL BLOCK]");
		return;
	}
	
	if (tree)
	{
		int i;
		item = proto_tree_add_item(tree, hf_frcds_eio_in, tvb, 0, -1, FALSE);
		if (item) eio_tree = proto_item_add_subtree(item, ett_frcds_eio_out);
		if (eio_tree)
		{
			proto_tree_add_item(eio_tree, hf_frcds_eio_apiver, tvb, offset, 1, FALSE);
			offset += 1;
			proto_tree_add_item(eio_tree, hf_frcds_eio_fwver, tvb, offset, 1, FALSE);
			offset += 1;

			for (i = 0; i < 8; i++)
			{
				proto_tree_add_float(eio_tree, *hf_frcds_eio_analogs[i], tvb, offset, 2, tvb_get_ntohs(tvb, offset) / (float) 0x3FFF);
				offset += 2;
			}

			proto_tree_add_item(eio_tree, hf_frcds_eio_digin, tvb, offset, 2, FALSE);
			offset += 2;

			for (i = 0; i < 3; i++)
			{
				proto_tree_add_float(eio_tree, *hf_frcds_eio_accels[i], tvb, offset, 2, (tvb_get_ntohs(tvb, offset) - 8300) / 3300.0);
				offset += 2;
			}
		}
	}
}

static void dissect_frcds(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	gint offset = 0;
	proto_item *item = NULL;
	proto_tree *frcds_tree = NULL;

	guint16 teamID;
	guint8 control;
	
	/* Set our protocol name */
	col_set_str(pinfo->cinfo, COL_PROTOCOL, "FRCDS");

	/* Check that this is the right length for a FRCDS packet */
	if (tvb_length(tvb) != 1024)
	{
		if (tree) item = proto_tree_add_text(tree, tvb, 0, -1, "FRC DS->Robot Protocol [ERROR]");
		expert_add_info_format(pinfo, item, PI_MALFORMED, PI_ERROR, "Data is not 1024 bytes");
		col_set_str(pinfo->cinfo, COL_INFO, "[IMPROPERLY SIZED FRCDS PACKET]");
		return;
	}

	teamID = tvb_get_ntohs(tvb, 4);
	control = tvb_get_guint8(tvb, 2);
	
	/* Write general data into the Info column */
	col_add_fstr(pinfo->cinfo, COL_INFO, "Team %u%s %s %s", teamID, control & FRCDS_FMS_FLAG ? " [FMS]" : "", control & FRCDS_NOTSTOP_FLAG ? control & FRCDS_ENABLE_FLAG ? "[ENABLED]" : "[DISABLED]" : "[E-STOP]", control & FRCDS_AUTON_FLAG ? "[AUTON]" : "[TELEOP]");

	if (tree) { /* We need to build the data tree */
		guint16 data_word = 0;
		int i = 0;
		int j = 0;
		proto_item *ti = NULL;
		proto_tree *cio_tree = NULL;
		
		if (frcds_summary_in_tree)
		{
			ti = proto_tree_add_protocol_format(tree, proto_frcds, tvb, 0, -1, "FRC DS->Robot Protocol, Team %u%s %s %s", teamID, control & FRCDS_FMS_FLAG ? " [FMS]" : "", control & FRCDS_NOTSTOP_FLAG ? control & FRCDS_ENABLE_FLAG ? "[ENABLED]" : "[DISABLED]" : "[E-STOP]", control & FRCDS_AUTON_FLAG ? "[AUTON]" : "[TELEOP]");
		} else {
			ti = proto_tree_add_item(tree, proto_frcds, tvb, 0, -1, FALSE);
		}
		
		if (ti) frcds_tree = proto_item_add_subtree(ti, ett_frcds);
		if (frcds_tree) {
			/* Packet Index */
			proto_tree_add_item(frcds_tree, hf_frcds_packetIndex, tvb, offset, 2, FALSE);
			offset += 2;
			
			{ /* Control Byte Block */
				proto_item *control_item = NULL;
				proto_item *control_tree = NULL;

				item = proto_tree_add_item(frcds_tree, hf_frcds_control, tvb, offset, 1, FALSE);
				if (item) control_tree = proto_item_add_subtree(item, ett_frcds_control);
				if (control_tree) { /* Display the bitfields as well */
					proto_tree_add_item(control_tree, hf_frcds_resetflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_notStopflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_enableflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_autonflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_fmsflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_resyncflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_cRIOchkflag, tvb, offset, 1, FALSE);
					proto_tree_add_item(control_tree, hf_frcds_fpgachkflag, tvb, offset, 1, FALSE);
				}
				offset += 1;
			}
			
			/* DS Digitals */
			{
				item = proto_tree_add_text(frcds_tree, tvb, offset, 1, "Compatible IO");
				if (item) cio_tree = proto_item_add_subtree(item, ett_frcds_cio);
				if (cio_tree) proto_tree_add_item(cio_tree, hf_frcds_digIn, tvb, offset, 1, FALSE);
				offset += 1;
			}

			/* Team Number */
			proto_tree_add_item(frcds_tree, hf_frcds_teamID, tvb, offset, 2, FALSE);
			offset += 2;
			
			{ /* Alliance Station Block */
				proto_item *location_tree = NULL;

				item = proto_tree_add_item(frcds_tree, hf_frcds_location, tvb, offset, 2, FALSE);
				if (item) location_tree = proto_item_add_subtree(item, ett_frcds_location);
				if (location_tree) {
					guint8 data_byte = 0;
					data_byte = tvb_get_guint8(tvb, offset); /* Get the byte containing Alliance */
					if (data_byte == 0x52) { /* Alliance and Position are encoded as a string, use ASCII to decode */
						proto_tree_add_uint(location_tree, hf_frcds_alliance, tvb, offset, 1, 1); /* Red Alliance */
					} else if (data_byte == 0x42) {
						proto_tree_add_uint(location_tree, hf_frcds_alliance, tvb, offset, 1, 2); /* Blue Alliance */
					} else {
						proto_tree_add_uint(location_tree, hf_frcds_alliance, tvb, offset, 1, 0); /* Unknown Alliance - Bad packet? */
					}
					offset += 1;

					data_byte = tvb_get_guint8(tvb, offset); /* Get the byte containing Position */
					if (data_byte >= 0x31 && data_byte <= 0x33) { /* Position is encoded as ASCII char containing 1, 2, or 3. WHY???????? */
						proto_tree_add_uint(location_tree, hf_frcds_position, tvb, offset, 1, (data_byte - 0x30)); /* Convert ASCII to numerical */
					} else {
						proto_tree_add_uint(location_tree, hf_frcds_position, tvb, offset, 1, 0); /* Bad data, 0 is error position */
					}
					offset += 1;
				} else {
					offset += 2; /* Couldn't make subtree, make sure offset is correct */
				}
			}

			for (i = 0; i <= 3; i++) { /* Iterate through Joysticks, fill fields */
				proto_tree *stick_tree;
				gint8			data_int8;

				item = proto_tree_add_text(frcds_tree, tvb, offset, 8, "Joystick %u", i);
				if (item) stick_tree = proto_item_add_subtree(item, *ett_frcds_sticks[i]);
				if (stick_tree) {
					for (j = 0; j < 6; j++) {
						data_int8 = (gint8) tvb_get_guint8(tvb, offset);
						proto_tree_add_float(stick_tree, *hf_frcds_sticks[i].axis[j], tvb, offset, 1, data_int8 >= 0 ? (float) data_int8 / 127 : (float) data_int8 / 128);
						offset += 1;
					}
					proto_tree_add_item(stick_tree, *hf_frcds_sticks[i].btn, tvb, offset, 2, FALSE);
					offset += 2;
				} else {
					offset += 8; /* Couldn't make subtree, make sure offset stays correct */
				}
			}

			/* DS Analog */
			for (i = 0; i <= 3; i++) { /* Iterate through analog inputs */
				data_word = tvb_get_ntohs(tvb, offset);
				/* Analogs are stored as 10 bit integers. Convert to float */
				if (cio_tree) proto_tree_add_float(cio_tree, *hf_frcds_analogs[i], tvb, offset, 2, ((float) data_word / 1023 * 5.0f));
				offset += 2;
			}

			{ /* Checksum Block */
				proto_item *sum_tree;
				item = proto_tree_add_text(frcds_tree, tvb, offset, 24, "Robot Checksum Block");
				if (item) sum_tree = proto_item_add_subtree(item, ett_frcds_sum);
				if (sum_tree) {
					proto_tree_add_item(sum_tree, hf_frcds_cRIOchecksum, tvb, offset, 8, FALSE);
					offset += 8;
					proto_tree_add_item(sum_tree, hf_frcds_FPGAchecksum1, tvb, offset, 4, FALSE);
					offset += 4;
					proto_tree_add_item(sum_tree, hf_frcds_FPGAchecksum2, tvb, offset, 4, FALSE);
					offset += 4;
					proto_tree_add_item(sum_tree, hf_frcds_FPGAchecksum3, tvb, offset, 4, FALSE);
					offset += 4;
					proto_tree_add_item(sum_tree, hf_frcds_FPGAchecksum4, tvb, offset, 4, FALSE);
					offset += 4;
				} else {
					offset += 24; /* Couldn't make subtree, compensate in offset */
				}
			}

			{ /* Version Data Block */
				guint8 *version1 = NULL;
				guint8 *version2 = NULL;
				guint8 *version3 = NULL;
				guint8 *version4 = NULL;
				guint8 version_string[20]; /* Version is expected to be 12 characters (+ NULL), so this should be plenty */

				/* Split version string into 4 strings */
				version1 = tvb_get_string(tvb, offset, 2);
				version2 = tvb_get_string(tvb, offset + 2, 2);
				version3 = tvb_get_string(tvb, offset + 4, 2);
				version4 = tvb_get_string(tvb, offset + 6, 2);

				/* Join the strings with '.' in between */
				g_snprintf(version_string, 20, "%s.%s.%s.%s", version1, version2, version3, version4);

				/* Display the interpreted version string */
				proto_tree_add_string(frcds_tree, hf_frcds_version, tvb, offset, 8, version_string);

				/* Clean up */
				g_free(version1);
				g_free(version2);
				g_free(version3);
				g_free(version4);
				offset += 8;
			}

		}
	}

	/* Dynamic Blocks
	 * We may be adding expert info, so we need to be outside the tree
	 * check */
	for (; offset < 0x03f8;)  /* 0x03f8 is the beginning of the checksum */
	{
		guint8 size = 0;
		guint8 type = 0;
		
		size			 = tvb_get_guint8(tvb, offset++);
		if (size > 0x00 && (0x03f8 - offset) >= size)
		{
			tvbuff_t	 *block_tvb;
			guint8	 type;
			
			type		 = tvb_get_guint8(tvb, offset);
			block_tvb = tvb_new_subset(tvb, offset, size, size);
			switch (type)
			{
				case 17:
					dissect_frcds_eio_control(block_tvb, pinfo, frcds_tree);
					break;
				case 18:
					dissect_frcds_eio_status(block_tvb, pinfo, frcds_tree);
					break;
				default:
					item = proto_tree_add_text(frcds_tree, tvb, offset, size, "Unknown type block %u", type);
					expert_add_info_format(pinfo, item, PI_PROTOCOL, PI_WARN, "Unknown block type %u (size %u)", type, size);
					col_append_fstr(pinfo->cinfo, COL_INFO, " [BAD DYNAMIC BLOCK %u]", type);
					break;
			}
			offset += size;
		}
	}
}

void proto_register_frcds(void)
{
	module_t *frcds_module;
	
	static hf_register_info hf[] = {
		{ &hf_frcds_packetIndex,
		{ "Packet Index", "frcds.index",
		FT_UINT16, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_control,
		{ "Control Byte", "frcds.control",
		FT_UINT8, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_resetflag,
		{ "Reset Flag", "frcds.control.reset",
		FT_BOOLEAN, 8,
		NULL, FRCDS_RESET_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_notStopflag,
		{ "Not E-Stop Flag", "frcds.control.notstop",
		FT_BOOLEAN, 8,
		NULL, FRCDS_NOTSTOP_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_enableflag,
		{ "Enable Flag", "frcds.control.enable",
		FT_BOOLEAN, 8,
		NULL, FRCDS_ENABLE_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_autonflag,
		{ "Autonomous Flag", "frcds.control.auton",
		FT_BOOLEAN, 8,
		NULL, FRCDS_AUTON_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_fmsflag,
		{ "FMS Connected Flag", "frcds.control.fms",
		FT_BOOLEAN, 8,
		NULL, FRCDS_FMS_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_resyncflag,
		{ "Resync Flag", "frcds.control.resync",
		FT_BOOLEAN, 8,
		NULL, FRCDS_RESYNC_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_cRIOchkflag,
		{ "cRIO Checksum Flag", "frcds.control.criochk",
		FT_BOOLEAN, 8,
		NULL, FRCDS_CRIOCHK_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_fpgachkflag,
		{ "FPGA Checksum Flag", "frcds.control.fpgachk",
		FT_BOOLEAN, 8,
		NULL, FRCDS_FPGACHK_FLAG,
		NULL, HFILL }
		},
		{ &hf_frcds_digIn,
		{ "Digital In", "frcds.cio.digin",
		FT_UINT8, BASE_HEX,
		NULL, 0xff,
		NULL, HFILL }
		},
		{ &hf_frcds_teamID,
		{ "Team Number", "frcds.teamid",
		FT_UINT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_location,
		{ "Location", "frcds.loc",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_alliance,
		{ "Alliance", "frcds.loc.alliance",
		FT_UINT8, BASE_DEC,
		VALS(teamcolors), 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_position,
		{ "Position", "frcds.loc.position",
		FT_UINT8, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_stick0Buttons,
		{ "Buttons", "frcds.joy0.btn",
		FT_UINT16, BASE_HEX,
		NULL, 0xffff,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis1,
		{ "Axis 1", "frcds.joy0.ax1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis2,
		{ "Axis 2", "frcds.joy0.ax2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis3,
		{ "Axis 3", "frcds.joy0.ax3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis4,
		{ "Axis 4", "frcds.joy0.ax4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis5,
		{ "Axis 5", "frcds.joy0.ax5",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick0Axis6,
		{ "Axis 6", "frcds.joy0.ax6",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Buttons,
		{ "Buttons", "frcds.joy1.btn",
		FT_UINT16, BASE_HEX,
		NULL, 0xffff,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis1,
		{ "Axis 1", "frcds.joy1.ax1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis2,
		{ "Axis 2", "frcds.joy1.ax2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis3,
		{ "Axis 3", "frcds.joy1.ax3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis4,
		{ "Axis 4", "frcds.joy1.ax4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis5,
		{ "Axis 5", "frcds.joy1.ax5",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick1Axis6,
		{ "Axis 6", "frcds.joy1.ax6",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Buttons,
		{ "Buttons", "frcds.joy2.btn",
		FT_UINT16, BASE_HEX,
		NULL, 0xffff,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis1,
		{ "Axis 1", "frcds.joy2.ax1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis2,
		{ "Axis 2", "frcds.joy2.ax2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis3,
		{ "Axis 3", "frcds.joy2.ax3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis4,
		{ "Axis 4", "frcds.joy2.ax4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis5,
		{ "Axis 5", "frcds.joy2.ax5",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick2Axis6,
		{ "Axis 6", "frcds.joy2.ax6",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Buttons,
		{ "Buttons", "frcds.joy3.btn",
		FT_UINT16, BASE_HEX,
		NULL, 0xffff,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis1,
		{ "Axis 1", "frcds.joy3.ax1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis2,
		{ "Axis 2", "frcds.joy3.ax2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis3,
		{ "Axis 3", "frcds.joy3.ax3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis4,
		{ "Axis 4", "frcds.joy3.ax4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis5,
		{ "Axis 5", "frcds.joy3.ax5",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_stick3Axis6,
		{ "Axis 6", "frcds.joy3.ax6",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_analog1,
		{ "Analog 1", "frcds.cio.ana1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_analog2,
		{ "Analog 2", "frcds.cio.ana2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_analog3,
		{ "Analog 3", "frcds.cio.ana3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_analog4,
		{ "Analog 4", "frcds.cio.ana4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_frcds_cRIOchecksum,
		{ "cRIO Checksum", "frcds.criosum",
		FT_UINT64, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_FPGAchecksum1,
		{ "FPGA Checksum 1", "frcds.fpgasum1",
		FT_UINT32, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_FPGAchecksum2,
		{ "FPGA Checksum 2", "frcds.fpgasum2",
		FT_UINT32, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_FPGAchecksum3,
		{ "FPGA Checksum 3", "frcds.fpgasum3",
		FT_UINT32, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_FPGAchecksum4,
		{ "FPGA Checksum 4", "frcds.fpgasum4",
		FT_UINT32, BASE_HEX,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_version,
		{ "DS Software Version", "frcds.version",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_in,
		{ "Enhanced I/O Input", "frcds.eio.in",
		FT_NONE, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_out,
		{ "Enhanced I/O Output", "frcds.eio.out",
		FT_NONE, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_apiver,
		{ "API Version", "frcds.eio.in.apiver",
		FT_UINT8, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_fwver,
		{ "Firmware Version", "frcds.eio.in.fwver",
		FT_UINT8, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana1,
		{ "Analog 1", "frcds.eio.in.ana1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana2,
		{ "Analog 2", "frcds.eio.in.ana2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana3,
		{ "Analog 3", "frcds.eio.in.ana3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana4,
		{ "Analog 4", "frcds.eio.in.ana4",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana5,
		{ "Analog 5", "frcds.eio.in.ana5",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana6,
		{ "Analog 6", "frcds.eio.in.ana6",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana7,
		{ "Analog 7", "frcds.eio.in.ana7",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_ana8,
		{ "Analog 8", "frcds.eio.in.ana8",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_digin,
		{ "Digital", "frcds.eio.in.dig",
		FT_UINT16, BASE_HEX,
		NULL, 0xffff,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_accel1,
		{ "Accelerator 1", "frcds.eio.in.accel1",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_accel2,
		{ "Accelerator 2", "frcds.eio.in.accel2",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_accel3,
		{ "Accelerator 3", "frcds.eio.in.accel3",
		FT_FLOAT, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_quad1,
		{ "Encoder 1", "frcds.eio.in.quad1",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_quad2,
		{ "Encoder 2", "frcds.eio.in.quad2",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_buttons,
		{ "Buttons", "frcds.eio.in.btn",
		FT_UINT8, BASE_HEX,
		NULL, 0xff,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_capslide,
		{ "Capsense Slider", "frcds.eio.in.capslide",
		FT_UINT8, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		},
		{ &hf_frcds_eio_capprox,
		{ "Capsense Proximity", "frcds.eio.in.capprox",
		FT_UINT8, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL}
		}
	};

	static gint *ett[] = {
		&ett_frcds,
		&ett_frcds_control,
		&ett_frcds_location,
		&ett_frcds_sum,
		&ett_frcds_stick0,
		&ett_frcds_stick1,
		&ett_frcds_stick2,
		&ett_frcds_stick3,
		&ett_frcds_cio,
		&ett_frcds_eio,
		&ett_frcds_eio_out,
		&ett_frcds_eio_enables,
		&ett_frcds_eio_in,
		&ett_frcds_eio_flags
	};
	
	proto_frcds = proto_register_protocol(
													"FRC DS->Robot Protocol",
													"FRCDS",
													"frcds"
												  );

	proto_register_field_array(proto_frcds, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

	frcds_module = prefs_register_protocol(proto_frcds, NULL);
	prefs_register_bool_preference(frcds_module, "summary_in_tree",
											 "Show packet summary in protocol tree",
											 "Whether the FRCDS summary line should be shown in the protocol tree",
											 &frcds_summary_in_tree);
}

void proto_reg_handoff_frcds(void)
{
	static dissector_handle_t frcds_handle;

	frcds_handle = create_dissector_handle(dissect_frcds, proto_frcds);
	dissector_add_uint("udp.port", FRCDS_PORT, frcds_handle);
}