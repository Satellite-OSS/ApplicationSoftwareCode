// smf/smf_sim.c
#include "smf_sim.h"


static smf_context_t g_smf_ctx;

static void allocate_ue_ip_address(char* ip_buffer) {
    if (g_smf_ctx.next_ue_ip_octet == 0 || g_smf_ctx.next_ue_ip_octet > 254) {
        g_smf_ctx.next_ue_ip_octet = 1;
    }
    snprintf(ip_buffer, MAX_UE_IP_ADDR_LEN, "10.0.0.%u", g_smf_ctx.next_ue_ip_octet++);
}

static smf_pdu_context_t* find_pdu_context(imsi_t imsi, pdu_session_id_t pdu_id) {
    for (int i = 0; i < MAX_SMF_PDU_SESSIONS; ++i) {
        if (g_smf_ctx.pdu_contexts[i].is_used &&
            g_smf_ctx.pdu_contexts[i].imsi == imsi &&
            g_smf_ctx.pdu_contexts[i].pdu_session_id == pdu_id) {
            return &g_smf_ctx.pdu_contexts[i];
        }
    }
    return NULL;
}

static smf_pdu_context_t* allocate_pdu_context() {
    for (int i = 0; i < MAX_SMF_PDU_SESSIONS; ++i) {
        if (!g_smf_ctx.pdu_contexts[i].is_used) {
            memset(&g_smf_ctx.pdu_contexts[i], 0, sizeof(smf_pdu_context_t));
            g_smf_ctx.pdu_contexts[i].is_used = true;
            g_smf_ctx.pdu_contexts[i].state = SMF_PDU_STATE_IDLE;
            SMF_LOG_DEBUG("Allocated new PDU session context at index: %d", i);
            return &g_smf_ctx.pdu_contexts[i];
        }
    }
    SMF_LOG_ERROR("Failed to allocate new PDU session context, pool is full!");
    return NULL;
}


static void smf_send_session_establishment_to_upf(
        smf_pdu_context_t* pdu_ctx,
        upf_session_establishment_response_data_t* upf_rsp)
{
    upf_session_establishment_request_data_t upf_req_data;
    memset(&upf_req_data, 0, sizeof(upf_session_establishment_request_data_t));

    upf_req_data.imsi = pdu_ctx->imsi;
    upf_req_data.pdu_session_id = pdu_ctx->pdu_session_id;
    strcpy(upf_req_data.ue_ip_address, pdu_ctx->ue_ip_address);
    upf_req_data.gnb_n3_teid = pdu_ctx->gnb_n3_teid;

    SMF_LOG_INFO("Sending Session Establishment Request to UPF for IMSI %lu, PDU ID %u, UE IP %s, gNB_N3_TEID %u",
                 upf_req_data.imsi, upf_req_data.pdu_session_id, upf_req_data.ue_ip_address, upf_req_data.gnb_n3_teid);

    upf_handle_session_establishment_request(&upf_req_data, upf_rsp);

    SMF_LOG_INFO("Received Session Establishment Response from UPF for IMSI %lu, PDU_ID %u. Success: %d, UPF_N3_TEID: %u",
                 upf_rsp->imsi, upf_rsp->pdu_session_id, upf_rsp->success, upf_rsp->upf_n3_teid);
}

void smf_initialize() {
    SMF_LOG_INFO("Initializing SMF...");
    memset(&g_smf_ctx, 0, sizeof(smf_context_t));
    g_smf_ctx.next_ue_ip_octet = 1;

    for (int i = 0; i < MAX_SMF_PDU_SESSIONS; ++i) {
        g_smf_ctx.pdu_contexts[i].is_used = false;
    }
    SMF_LOG_INFO("SMF initialization complete.");
}

void smf_handle_create_pdu_session_request(
        const smf_create_pdu_session_request_data_t* req_data,
        smf_create_pdu_session_response_data_t* rsp_data)
{
    if (req_data == NULL || rsp_data == NULL) {
        SMF_LOG_ERROR("Received NULL pointer for request or response data!");
        if(rsp_data) rsp_data->success = false;
        return;
    }

    SMF_LOG_INFO("Handling Create PDU Session Request from AMF: IMSI %lu, PDU_ID %u, Associated AMF_UE_ID %u, Associated GNB_UE_ID %u",
                 req_data->imsi, req_data->pdu_session_id, req_data->associated_amf_ue_id, req_data->associated_gnb_ue_id);

    memset(rsp_data, 0, sizeof(smf_create_pdu_session_response_data_t));
    rsp_data->imsi = req_data->imsi;
    rsp_data->pdu_session_id = req_data->pdu_session_id;
    rsp_data->success = false;

    smf_pdu_context_t* pdu_ctx = find_pdu_context(req_data->imsi, req_data->pdu_session_id);
    if (pdu_ctx != NULL && pdu_ctx->state == SMF_PDU_STATE_ACTIVE) {
        SMF_LOG_INFO("PDU Session (IMSI %lu, PDU_ID %u) already active. Re-using existing context.",
                     req_data->imsi, req_data->pdu_session_id);
        rsp_data->success = true;
        strcpy(rsp_data->ue_ip_address, pdu_ctx->ue_ip_address);
        rsp_data->upf_n3_teid = pdu_ctx->upf_n3_teid;
        return;
    }

    if (pdu_ctx == NULL) {
        pdu_ctx = allocate_pdu_context();
    }

    if (pdu_ctx == NULL) {
        SMF_LOG_ERROR("Failed to allocate PDU session context for IMSI %lu, PDU_ID %u.",
                      req_data->imsi, req_data->pdu_session_id);
        return;
    }

    pdu_ctx->imsi = req_data->imsi;
    pdu_ctx->pdu_session_id = req_data->pdu_session_id;
    pdu_ctx->state = SMF_PDU_STATE_CREATING;
    pdu_ctx->associated_amf_ue_id = req_data->associated_amf_ue_id;


    if (req_data->associated_gnb_ue_id != 0) {
        pdu_ctx->gnb_n3_teid = 30000 + req_data->associated_gnb_ue_id;
        SMF_LOG_DEBUG("IMSI %lu, PDU_ID %u: Simulated gNB_N3_TEID based on GNB_UE_ID %u from AMF: %u",
                      pdu_ctx->imsi, pdu_ctx->pdu_session_id, req_data->associated_gnb_ue_id, pdu_ctx->gnb_n3_teid);
    } else {
        pdu_ctx->gnb_n3_teid = (teid_t)(rand() % 1000) + 31000;
        SMF_LOG_WARN("IMSI %lu, PDU_ID %u: GNB_UE_ID not available from AMF request, using random gNB_N3_TEID: %u",
                     pdu_ctx->imsi, pdu_ctx->pdu_session_id, pdu_ctx->gnb_n3_teid);
    }


    SMF_LOG_DEBUG("IMSI %lu, PDU_ID %u: (Simulated) UPF selection complete (assuming one UPF).", pdu_ctx->imsi, pdu_ctx->pdu_session_id);

    allocate_ue_ip_address(pdu_ctx->ue_ip_address);
    SMF_LOG_INFO("IMSI %lu, PDU_ID %u: Assigned UE IP Address: %s",
                 pdu_ctx->imsi, pdu_ctx->pdu_session_id, pdu_ctx->ue_ip_address);

    upf_session_establishment_response_data_t upf_rsp_data;
    smf_send_session_establishment_to_upf(pdu_ctx, &upf_rsp_data);

    if (upf_rsp_data.success) {
        pdu_ctx->upf_n3_teid = upf_rsp_data.upf_n3_teid;
        pdu_ctx->state = SMF_PDU_STATE_ACTIVE;
        SMF_LOG_INFO("IMSI %lu, PDU_ID %u: User plane setup successful with UPF. UPF_N3_TEID: %u. State: ACTIVE",
                     pdu_ctx->imsi, pdu_ctx->pdu_session_id, pdu_ctx->upf_n3_teid);

        rsp_data->success = true;
        strcpy(rsp_data->ue_ip_address, pdu_ctx->ue_ip_address);
        rsp_data->upf_n3_teid = pdu_ctx->upf_n3_teid;
    } else {
        SMF_LOG_ERROR("IMSI %lu, PDU_ID %u: UPF failed to establish session. Releasing PDU context.",
                      pdu_ctx->imsi, pdu_ctx->pdu_session_id);
        pdu_ctx->is_used = false;
        pdu_ctx->state = SMF_PDU_STATE_IDLE;
    }
}
