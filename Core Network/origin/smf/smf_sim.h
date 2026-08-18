// smf/smf_sim.h
#ifndef SMF_SIM_H
#define SMF_SIM_H

#include "../common/common_defs.h"
#include "../upf/upf_sim.h"




typedef struct {
    smf_pdu_context_t pdu_contexts[MAX_SMF_PDU_SESSIONS];
    uint32_t next_ue_ip_octet;
} smf_context_t;


void smf_initialize();
void smf_handle_create_pdu_session_request(
        const smf_create_pdu_session_request_data_t* req_data,
        smf_create_pdu_session_response_data_t* rsp_data
);

#endif // SMF_SIM_H
