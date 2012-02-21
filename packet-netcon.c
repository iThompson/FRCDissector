#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <epan/packet.h>

#define NETCON_RBTPORT 6666
#define NETCON_DSPORT  6668

static int proto_netcon = -1;
static int hf_netcon_text = -1;

static gint ett_netcon = -1;

static void dissect_netcon(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	guint8 *msg = NULL;
	col_set_str(pinfo->cinfo, COL_PROTOCOL, "NETCON");

	msg = tvb_get_string(tvb, 0, tvb_length(tvb));
	if (msg && pinfo->destport == NETCON_RBTPORT)
	{
		col_add_fstr(pinfo->cinfo, COL_INFO, "[OUT] %s", msg);
	}
	else if (msg && pinfo->destport == NETCON_DSPORT)
	{
		col_add_fstr(pinfo->cinfo, COL_INFO, "[IN] %s", msg);
	}
	else
	{
		col_clear(pinfo->cinfo, COL_INFO);
	}

	if (tree)
	{
		proto_item *ti = NULL;
		proto_tree *netcon_tree = NULL;
		ti = proto_tree_add_item(tree, proto_netcon, tvb, 0, -1, FALSE);
		if (ti) netcon_tree = proto_item_add_subtree(ti, ett_netcon);
		if (netcon_tree) proto_tree_add_item(netcon_tree, hf_netcon_text, tvb, 0, tvb_length(tvb), FALSE);
	}
	
	if (msg)
	{
		g_free(msg);
	}
}

void proto_register_netcon(void)
{
	static hf_register_info hf[] = {
		{ &hf_netcon_text,
		{ "Text", "netcon.text",
		FT_STRING, BASE_NONE,
		NULL, 0x0,
		NULL, HFILL }
		}
	};

	static gint *ett[] = {
		&ett_netcon
	};

	proto_netcon = proto_register_protocol(
														"FRC NetConsole Protocol",
														"NETCON",
														"netcon"
													  );

	proto_register_field_array(proto_netcon, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

void proto_reg_handoff_netcon(void)
{
	static dissector_handle_t netcon_handle;

	netcon_handle = create_dissector_handle(dissect_netcon, proto_netcon);
	dissector_add_uint("udp.port", NETCON_RBTPORT, netcon_handle);
	dissector_add_uint("udp.port", NETCON_DSPORT, netcon_handle);
}