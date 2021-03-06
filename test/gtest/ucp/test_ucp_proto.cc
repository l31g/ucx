/**
 * Copyright (C) Mellanox Technologies Ltd. 2020.  ALL RIGHTS RESERVED.
 *
 * See file LICENSE for terms.
 */

#include <common/test.h>

#include "ucp_test.h"

extern "C" {
#include <ucp/core/ucp_rkey.h>
#include <ucp/proto/proto.h>
#include <ucp/proto/proto_select.h>
#include <ucp/core/ucp_worker.inl>
}

class test_ucp_proto : public ucp_test {
public:
    static ucp_params_t get_ctx_params() {
        ucp_params_t params = ucp_test::get_ctx_params();
        params.features    |= UCP_FEATURE_TAG | UCP_FEATURE_RMA;
        return params;
    }

protected:
    virtual void init() {
        modify_config("PROTO_ENABLE", "y");
        ucp_test::init();
        sender().connect(&receiver(), get_ep_params());
    }

    ucp_worker_h worker() {
        return sender().worker();
    }
};

UCS_TEST_P(test_ucp_proto, dump_info) {
    ucp_ep_print_info(sender().ep(), stdout);
}

UCS_TEST_P(test_ucp_proto, dump_protocols) {
    ucp_proto_select_param_t select_param;
    ucs_string_buffer_t strb;

    select_param.op_id    = UCP_OP_ID_TAG_SEND;
    select_param.op_flags = 0;
    select_param.dt_class = UCP_DATATYPE_CONTIG;
    select_param.mem_type = UCS_MEMORY_TYPE_HOST;
    select_param.sys_dev  = 0;
    select_param.sg_count = 1;

    ucp_proto_select_param_str(&select_param, &strb);
    UCS_TEST_MESSAGE << ucs_string_buffer_cstr(&strb);
    ucs_string_buffer_cleanup(&strb);

    ucp_proto_select_dump_all(sender().worker(), sender().ep()->cfg_index,
                              UCP_WORKER_CFG_INDEX_NULL, &select_param, stdout);
}

UCS_TEST_P(test_ucp_proto, rkey_config) {
    ucp_rkey_config_key_t rkey_config_key;

    rkey_config_key.ep_cfg_index = 0;
    rkey_config_key.md_map       = 0;
    rkey_config_key.mem_type     = UCS_MEMORY_TYPE_HOST;
    rkey_config_key.sys_dev      = UCS_SYS_DEVICE_ID_UNKNOWN;

    ucs_status_t status;

    /* similar configurations should return same index */
    ucp_worker_cfg_index_t cfg_index1;
    status = ucp_worker_get_rkey_config(worker(), &rkey_config_key, &cfg_index1);
    ASSERT_UCS_OK(status);

    ucp_worker_cfg_index_t cfg_index2;
    status = ucp_worker_get_rkey_config(worker(), &rkey_config_key, &cfg_index2);
    ASSERT_UCS_OK(status);

    EXPECT_EQ(static_cast<int>(cfg_index1), static_cast<int>(cfg_index2));

    rkey_config_key.ep_cfg_index = 0;
    rkey_config_key.md_map       = 1;
    rkey_config_key.mem_type     = UCS_MEMORY_TYPE_HOST;
    rkey_config_key.sys_dev      = UCS_SYS_DEVICE_ID_UNKNOWN;

    /* different configuration should return different index */
    ucp_worker_cfg_index_t cfg_index3;
    status = ucp_worker_get_rkey_config(worker(), &rkey_config_key, &cfg_index3);
    ASSERT_UCS_OK(status);

    EXPECT_NE(static_cast<int>(cfg_index1), static_cast<int>(cfg_index3));
}

UCP_INSTANTIATE_TEST_CASE(test_ucp_proto)
