// upf/upf_sim.h
#ifndef UPF_SIM_H
#define UPF_SIM_H

#include "../common/common_defs.h"




typedef struct {
    upf_pdu_session_context_t pdu_sessions[MAX_UPF_SESSIONS];
    teid_t next_upf_n3_teid_counter;
} upf_context_t;


void upf_initialize();
void upf_handle_session_establishment_request(
        const upf_session_establishment_request_data_t* req_data,
        upf_session_establishment_response_data_t* rsp_data
);
void upf_receive_uplink_data(teid_t upf_n3_teid_of_tunnel, const char* ue_ip_src, const char* data, uint16_t data_len);
void upf_receive_downlink_data(const char* ue_ip_dest, const char* data, uint16_t data_len);

#endif // UPF_SIM_H

