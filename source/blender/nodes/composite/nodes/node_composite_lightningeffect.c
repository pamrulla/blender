/*
* ***** BEGIN GPL LICENSE BLOCK *****
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* The Original Code is Copyright (C) 2006 Blender Foundation.
* All rights reserved.
*
* The Original Code is: all of this file.
*
* Contributor(s): none yet.
*
* ***** END GPL LICENSE BLOCK *****
*/

/** \file blender/nodes/composite/nodes/node_composite_boxmask.c
*  \ingroup cmpnodes
*/


#include "../node_composite_util.h"

/* **************** SCALAR MATH ******************** */
static bNodeSocketTemplate cmp_node_lightningeffect_in[] = {
	{ SOCK_FLOAT, 1, N_("Mask"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0, SOCK_HIDDEN },
	{ SOCK_FLOAT, 1, N_("Start X"), 100.0f, 0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, PROP_PIXEL,  },
	{ SOCK_FLOAT, 1, N_("Start Y"), 400.0f, 0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, PROP_PIXEL, },
	{ SOCK_FLOAT, 1, N_("End X"), 150.0f, 0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, PROP_PIXEL, },
	{ SOCK_FLOAT, 1, N_("End Y"), 50.0f, 0.0f, 0.0f, 0.0f, -10000.0f, 10000.0f, PROP_PIXEL, },
	{ -1, 0, "" }
};

static bNodeSocketTemplate cmp_node_lightningeffect_out[] = {
	{ SOCK_RGBA, 0, N_("Image")},
	{ -1, 0, "" }
};

static void node_composit_init_lightningeffect(bNodeTree *UNUSED(ntree), bNode *node)
{
	NodeLightningEffect *data = MEM_callocN(sizeof(NodeLightningEffect), "NodeLightningEffect");
	data->x1 = 100;
	data->y1 = 400;
	data->x2 = 150;
	data->y2 = 50;
	data->iterations = 0.3;
	data->offsetAmount = 0.2;
	data->branches = 2;
	data->rotation = 0.1;
	data->length = 0.4;
	data->twigs = 1;
	node->storage = data;
}

void register_node_type_cmp_lightningeffect(void)
{
	static bNodeType ntype;

	cmp_node_type_base(&ntype, CMP_NODE_EFFECT_LIGHTNING, "Lightning Effect", NODE_CLASS_EFFECT, 0);
	node_type_socket_templates(&ntype, cmp_node_lightningeffect_in, cmp_node_lightningeffect_out);
	node_type_init(&ntype, node_composit_init_lightningeffect);
	node_type_storage(&ntype, "NodeLightningEffect", node_free_standard_storage, node_copy_standard_storage);
	node_type_size(&ntype, 200, 200, 300);

	nodeRegisterType(&ntype);
}
