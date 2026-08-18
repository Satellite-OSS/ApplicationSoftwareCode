// amf/amf_sim.h
#ifndef AMF_SIM_H
#define AMF_SIM_H

#include "../common/common_defs.h"



typedef enum {
    AMF_UE_STATE_DEREGISTERED,
    AMF_UE_STATE_REGISTERING,
    AMF_UE_STATE_REGISTERED,
    AMF_UE_STATE_PDU_SESSION_PENDING,
    AMF_UE_STATE_PDU_SESSION_ACTIVE
} amf_ue_state_t;


typedef struct {
    bool is_used;
    imsi_t imsi;
    amf_ue_state_t state;
    amf_ue_ngap_id_t amf_ue_id;
    gnb_ue_ngap_id_t gnb_ue_id;

    struct {
        pdu_session_id_t id;
        bool active;
        // teid_t n2_sm_info_upf_teid; 
        // char n2_sm_info_ue_ip[MAX_UE_IP_ADDR_LEN];
    } pdu_sessions[MAX_PDU_SESSIONS_PER_UE];
    int active_pdu_session_count;

} amf_ue_context_t;


typedef struct {
    amf_ue_context_t ue_contexts[MAX_AMF_UES];
    amf_ue_ngap_id_t next_amf_ue_ngap_id_counter;
} amf_context_t;


void amf_initialize();
void amf_handle_ngap_message_from_gnb(const simplified_ngap_message_t* ngap_msg);
void amf_trigger_ue_pdu_session_request(imsi_t imsi, pdu_session_id_t pdu_id_req);

#endif // AMF_SIM_H
