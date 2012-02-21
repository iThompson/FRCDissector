/*
 * Do not modify this file.
 *
 * It is created automatically by Makefile or Makefile.nmake.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gmodule.h>

#include "moduleinfo.h"

#ifndef ENABLE_STATIC
G_MODULE_EXPORT const gchar version[] = VERSION;

/* Start the functions we need for the plugin stuff */

G_MODULE_EXPORT void
plugin_register (void)
{
  {extern void proto_register_crrvision (void); proto_register_crrvision ();}
  {extern void proto_register_frcds (void); proto_register_frcds ();}
  {extern void proto_register_netcon (void); proto_register_netcon ();}
}

G_MODULE_EXPORT void
plugin_reg_handoff(void)
{
  {extern void proto_reg_handoff_crrvision (void); proto_reg_handoff_crrvision ();}
  {extern void proto_reg_handoff_frcds (void); proto_reg_handoff_frcds ();}
  {extern void proto_reg_handoff_netcon (void); proto_reg_handoff_netcon ();}
}
#endif
