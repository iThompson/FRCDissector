#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <epan/packet.h>
#include <epan/expert.h>

#include <string.h>

#define CRRVISION_DSTPORT 6639

static int proto_crrvision = -1;

static int hf_crr_dist_high = -1;
static int hf_crr_angle_high = -1;
static int hf_crr_dist_right = -1;
static int hf_crr_angle_right = -1;
static int hf_crr_dist_left = -1;
static int hf_crr_angle_left = -1;
static int hf_crr_dist_low = -1;
static int hf_crr_angle_low = -1;

static gint ett_crrvision = -1;


static void dissect_crrvision(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	gint offset = 0;
	proto_item *item = NULL;
	proto_tree *crrvision_tree = NULL;

	guint8 *head = NULL;
	col_set_str(pinfo->cinfo, COL_PROTOCOL, "CRRVISION");

	// Check that the packet is the proper length
	if (tvb_length(tvb) != 20)
	{
		if (tree) item = proto_tree_add_text(tree, tvb, 0, -1, "Team 639 Vision Protocol [ERROR]");
		expert_add_info_format(pinfo, item, PI_MALFORMED, PI_ERROR, "Data is not 20 bytes");
		col_set_str(pinfo->cinfo, COL_INFO, "[IMPROPERLY SIZED VISION PACKET]");
		return;
	}

	head = tvb_get_string(tvb, 0, 4);
	if (!head || strcmp(head, "0639") != 0)
	{
		if (tree) item = proto_tree_add_text(tree, tvb, 0, -1, "Team 639 Vision Protocol [ERROR]");
		expert_add_info_format(pinfo, item, PI_MALFORMED, PI_ERROR, "Packet header is invalid");
		col_set_str(pinfo->cinfo, COL_INFO, "[INVALID PACKET HEADER]");
		return;
	}

	col_set_str(pinfo->cinfo, COL_INFO, "Team 639 Vision Protocol");

	if (tree)
	{ // We need to build the data tree
		item = proto_tree_add_item(tree, proto_crrvision, tvb, 0, -1, FALSE);
		if (item) crrvision_tree = proto_item_add_subtree(item, ett_crrvision);
		if (crrvision_tree)
		{
			offset = 4;
			proto_tree_add_item(crrvision_tree, hf_crr_dist_high, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_angle_high, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_dist_right, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_angle_right, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_dist_left, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_angle_left, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_dist_low, tvb, offset, 2, FALSE);
			offset += 2;
			proto_tree_add_item(crrvision_tree, hf_crr_angle_low, tvb, offset, 2, FALSE);
			}
	}

	if (head)
	{
		g_free(head);
	}
}

void proto_register_crrvision(void)
{
	static hf_register_info hf[] = {
		{ &hf_crr_dist_high,
		{ "High Distance", "crrvision.high.dist",
		FT_UINT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_angle_high,
		{ "High Angle", "crrvision.high.angle",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_dist_right,
		{ "Right Distance", "crrvision.right.dist",
		FT_UINT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_angle_right,
		{ "Right Angle", "crrvision.right.angle",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_dist_left,
		{ "Left Distance", "crrvision.left.dist",
		FT_UINT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_angle_left,
		{ "Left Angle", "crrvision.left.angle",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_dist_low,
		{ "Low Distance", "crrvision.low.dist",
		FT_UINT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		},
		{ &hf_crr_angle_low,
		{ "Low Angle", "crrvision.low.angle",
		FT_INT16, BASE_DEC,
		NULL, 0x0,
		NULL, HFILL }
		}
	};

	static gint *ett[] = {
		&ett_crrvision
	};

	proto_crrvision = proto_register_protocol(
														"Team 639 Vision Protocol",
														"CRRVISION",
														"crrvision"
													  );

	proto_register_field_array(proto_crrvision, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_crrvision(void)
{
	static dissector_handle_t crrvision_handle;

	crrvision_handle = create_dissector_handle(dissect_crrvision, proto_crrvision);
	dissector_add_uint("udp.port", CRRVISION_DSTPORT, crrvision_handle);
}