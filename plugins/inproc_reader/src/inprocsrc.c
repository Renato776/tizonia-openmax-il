/**
 * Copyright (C) 2011-2017 Aratelia Limited - Juan A. Rubio
 *
 * This file is part of Tizonia
 *
 * Tizonia is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   inprocsrc.c
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tizonia - ZMQ inproc socket reader
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>

#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Types.h>

#include <tizplatform.h>

#include <tizport.h>
#include <tizscheduler.h>

#include "inprocsrcprc.h"
#include "inprocsrc.h"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.inproc_reader"
#endif

static OMX_VERSIONTYPE inproc_reader_version = { {1, 0, 0, 0} };

static OMX_PTR
instantiate_audio_port (OMX_HANDLETYPE ap_hdl)
{
  tiz_port_options_t port_opts = {
    OMX_PortDomainAudio,
    OMX_DirOutput,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_COUNT,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_SIZE,
    ARATELIA_INPROC_READER_PORT_NONCONTIGUOUS,
    ARATELIA_INPROC_READER_PORT_ALIGNMENT,
    ARATELIA_INPROC_READER_PORT_SUPPLIERPREF,
    {ARATELIA_INPROC_READER_PORT_INDEX, NULL, NULL, NULL},
    -1                          /* use -1 for now */
  };

  return factory_new (tiz_get_type (ap_hdl, "tizbinaryport"),
                      &port_opts);
}

static OMX_PTR
instantiate_video_port (OMX_HANDLETYPE ap_hdl)
{
  tiz_port_options_t port_opts = {
    OMX_PortDomainVideo,
    OMX_DirOutput,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_COUNT,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_SIZE,
    ARATELIA_INPROC_READER_PORT_NONCONTIGUOUS,
    ARATELIA_INPROC_READER_PORT_ALIGNMENT,
    ARATELIA_INPROC_READER_PORT_SUPPLIERPREF,
    {ARATELIA_INPROC_READER_PORT_INDEX, NULL, NULL, NULL},
    -1                          /* use -1 for now */
  };

  return factory_new (tiz_get_type (ap_hdl, "tizbinaryport"),
                      &port_opts);
}

static OMX_PTR
instantiate_image_port (OMX_HANDLETYPE ap_hdl)
{
  tiz_port_options_t port_opts = {
    OMX_PortDomainImage,
    OMX_DirOutput,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_COUNT,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_SIZE,
    ARATELIA_INPROC_READER_PORT_NONCONTIGUOUS,
    ARATELIA_INPROC_READER_PORT_ALIGNMENT,
    ARATELIA_INPROC_READER_PORT_SUPPLIERPREF,
    {ARATELIA_INPROC_READER_PORT_INDEX, NULL, NULL, NULL},
    -1                          /* use -1 for now */
  };

  return factory_new (tiz_get_type (ap_hdl, "tizbinaryport"),
                      &port_opts);
}

static OMX_PTR
instantiate_other_port (OMX_HANDLETYPE ap_hdl)
{
  tiz_port_options_t port_opts = {
    OMX_PortDomainOther,
    OMX_DirOutput,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_COUNT,
    ARATELIA_INPROC_READER_PORT_MIN_BUF_SIZE,
    ARATELIA_INPROC_READER_PORT_NONCONTIGUOUS,
    ARATELIA_INPROC_READER_PORT_ALIGNMENT,
    ARATELIA_INPROC_READER_PORT_SUPPLIERPREF,
    {ARATELIA_INPROC_READER_PORT_INDEX, NULL, NULL, NULL},
    -1                          /* use -1 for now */
  };

  return factory_new (tiz_get_type (ap_hdl, "tizbinaryport"),
                      &port_opts);
}

static OMX_PTR
instantiate_config_port (OMX_HANDLETYPE ap_hdl)
{
  /* Instantiate the config port */
  return factory_new (tiz_get_type (ap_hdl, "tizuricfgport"),
                      NULL,       /* this port does not take options */
                      ARATELIA_INPROC_READER_COMPONENT_NAME,
                      inproc_reader_version);
}

static OMX_PTR
instantiate_processor (OMX_HANDLETYPE ap_hdl)
{
  return factory_new (tiz_get_type (ap_hdl, "inprocsrc_prc"));
}

OMX_ERRORTYPE
OMX_ComponentInit (OMX_HANDLETYPE ap_hdl)
{
  tiz_role_factory_t audio_role;
  tiz_role_factory_t video_role;
  tiz_role_factory_t image_role;
  tiz_role_factory_t other_role;
  const tiz_role_factory_t *rf_list[] = { &audio_role, &video_role,
    &image_role, &other_role
  };
  tiz_type_factory_t inprocsrc_prc_type;
  const tiz_type_factory_t *tf_list[] = { &inprocsrc_prc_type };

  strcpy ((OMX_STRING) audio_role.role,
          ARATELIA_INPROC_READER_AUDIO_ROLE);
  audio_role.pf_cport   = instantiate_config_port;
  audio_role.pf_port[0] = instantiate_audio_port;
  audio_role.nports     = 1;
  audio_role.pf_proc    = instantiate_processor;

  strcpy ((OMX_STRING) video_role.role,
          ARATELIA_INPROC_READER_VIDEO_ROLE);
  video_role.pf_cport   = instantiate_config_port;
  video_role.pf_port[0] = instantiate_video_port;
  video_role.nports     = 1;
  video_role.pf_proc    = instantiate_processor;

  strcpy ((OMX_STRING) image_role.role,
          ARATELIA_INPROC_READER_IMAGE_ROLE);
  image_role.pf_cport   = instantiate_config_port;
  image_role.pf_port[0] = instantiate_image_port;
  image_role.nports     = 1;
  image_role.pf_proc    = instantiate_processor;

  strcpy ((OMX_STRING) other_role.role,
          ARATELIA_INPROC_READER_OTHER_ROLE);
  other_role.pf_cport   = instantiate_config_port;
  other_role.pf_port[0] = instantiate_other_port;
  other_role.nports     = 1;
  other_role.pf_proc    = instantiate_processor;

  strcpy ((OMX_STRING) inprocsrc_prc_type.class_name, "inprocsrc_prc_class");
  inprocsrc_prc_type.pf_class_init = inprocsrc_prc_class_init;
  strcpy ((OMX_STRING) inprocsrc_prc_type.object_name, "inprocsrc_prc");
  inprocsrc_prc_type.pf_object_init = inprocsrc_prc_init;

  /* Initialize the component infrastructure */
  tiz_check_omx (tiz_comp_init (ap_hdl, ARATELIA_INPROC_READER_COMPONENT_NAME));

  /* Register the "inprocsrc_prc" class */
  tiz_check_omx (tiz_comp_register_types (ap_hdl, tf_list, 1));

  /* Register the various roles */
  tiz_check_omx (tiz_comp_register_roles (ap_hdl, rf_list, 4));

  return OMX_ErrorNone;
}
