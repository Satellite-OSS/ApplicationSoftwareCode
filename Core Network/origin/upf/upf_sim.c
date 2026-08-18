// upf/upf_sim.c
#include "upf_sim.h"

static upf_context_t g_upf_ctx;

static teid_t allocate_upf_n3_teid() {
    if (g_upf_ctx.next_upf_n3_teid_counter == 0) {
        g_upf_ctx.next_upf_n3_teid_counter = 20000;
    }
    UPF_LOG_DEBUG("Allocating new UPF N3 TEID: %u", g_upf_ctx.next_upf_n3_teid_counter);
    return g_upf_ctx.next_upf_n3_teid_counter++;
}

static upf_pdu_session_context_t* find_upf_pdu_context(imsi_t imsi, pdu_session_id_t pdu_id) {
    for (int i = 0; i < MAX_UPF_SESSIONS; ++i) {
        if (g_upf_ctx.pdu_sessions[i].is_used &&
            g_upf_ctx.pdu_sessions[i].imsi == imsi &&
            g_upf_ctx.pdu_sessions[i].pdu_session_id == pdu_id) {
            return &g_upf_ctx.pdu_sessions[i];
        }
    }
    return NULL;
}

static upf_pdu_session_context_t* allocate_upf_pdu_context() {
    for (int i = 0; i < MAX_UPF_SESSIONS; ++i) {
        if (!g_upf_ctx.pdu_sessions[i].is_used) {
            memset(&g_upf_ctx.pdu_sessions[i], 0, sizeof(upf_pdu_session_context_t));
            g_upf_ctx.pdu_sessions[i].is_used = true;
            g_upf_ctx.pdu_sessions[i].state = UPF_SESSION_STATE_IDLE;
            UPF_LOG_DEBUG("Allocated new UPF PDU session context at index: %d", i);
            return &g_upf_ctx.pdu_sessions[i];
        }
    }
    UPF_LOG_ERROR("Failed to allocate new UPF PDU session context, pool is full!");
    return NULL;
}

void upf_initialize() {
    UPF_LOG_INFO("Initializing UPF...");
    memset(&g_upf_ctx, 0, sizeof(upf_context_t));
    g_upf_ctx.next_upf_n3_teid_counter = 20000;

    for (int i = 0; i < MAX_UPF_SESSIONS; ++i) {
        g_upf_ctx.pdu_sessions[i].is_used = false;
    }
    UPF_LOG_INFO("UPF initialization complete.");
}

void upf_handle_session_establishment_request(
        const upf_session_establishment_request_data_t* req_data,
        upf_session_establishment_response_data_t* rsp_data)
{
    if (req_data == NULL || rsp_data == NULL) {
        UPF_LOG_ERROR("Received NULL pointer for request or response data!");
        if(rsp_data) rsp_data->success = false;
        return;
    }

    UPF_LOG_INFO("Handling Session Establishment Request from SMF: IMSI %lu, PDU_ID %u, UE_IP %s, gNB_N3_TEID %u",
                 req_data->imsi, req_data->pdu_session_id, req_data->ue_ip_address, req_data->gnb_n3_teid);

    memset(rsp_data, 0, sizeof(upf_session_establishment_response_data_t));
    rsp_data->imsi = req_data->imsi;
    rsp_data->pdu_session_id = req_data->pdu_session_id;
    rsp_data->success = false;

    upf_pdu_session_context_t* pdu_ctx = find_upf_pdu_context(req_data->imsi, req_data->pdu_session_id);
    if (pdu_ctx == NULL) {
        pdu_ctx = allocate_upf_pdu_context();
    }

    if (pdu_ctx == NULL) {
        UPF_LOG_ERROR("Failed to allocate UPF PDU session context for IMSI %lu, PDU_ID %u.",
                      req_data->imsi, req_data->pdu_session_id);
        return;
    }

    pdu_ctx->imsi = req_data->imsi;
    pdu_ctx->pdu_session_id = req_data->pdu_session_id;
    strcpy(pdu_ctx->ue_ip_address, req_data->ue_ip_address);
    pdu_ctx->gnb_n3_teid = req_data->gnb_n3_teid;
    pdu_ctx->upf_n3_teid = allocate_upf_n3_teid();
    pdu_ctx->state = UPF_SESSION_STATE_ACTIVE;

    UPF_LOG_INFO("IMSI %lu, PDU_ID %u: UPF session context configured. UE_IP: %s, gNB_N3_TEID: %u, UPF_N3_TEID: %u. State: ACTIVE",
                 pdu_ctx->imsi, pdu_ctx->pdu_session_id, pdu_ctx->ue_ip_address, pdu_ctx->gnb_n3_teid, pdu_ctx->upf_n3_teid);

    UPF_LOG_DEBUG("IMSI %lu, PDU_ID %u: (Simulated) Packet Detection and Forwarding Rules installed.",
                  pdu_ctx->imsi, pdu_ctx->pdu_session_id);

    rsp_data->success = true;
    rsp_data->upf_n3_teid = pdu_ctx->upf_n3_teid;
}

void upf_receive_uplink_data(teid_t upf_n3_teid_of_tunnel, const char* ue_ip_src, const char* data, uint16_t data_len) {
    UPF_LOG_INFO("Received UPLINK data on UPF_N3_TEID: %u (from UE_IP: %s), Data: '%.*s', Length: %u",
                 upf_n3_teid_of_tunnel, ue_ip_src, data_len, data, data_len);

    bool session_found = false;
    for (int i = 0; i < MAX_UPF_SESSIONS; ++i) {
        if (g_upf_ctx.pdu_sessions[i].is_used && g_upf_ctx.pdu_sessions[i].upf_n3_teid == upf_n3_teid_of_tunnel) {
            UPF_LOG_DEBUG("  Matching session for UL data: IMSI %lu, PDU_ID %u, UE_IP %s",
                          g_upf_ctx.pdu_sessions[i].imsi, g_upf_ctx.pdu_sessions[i].pdu_session_id, g_upf_ctx.pdu_sessions[i].ue_ip_address);
            session_found = true;
            break;
        }
    }
    if (session_found) {
        UPF_LOG_INFO("  (Simulated) Forwarding UL data from UE %s to Data Network (N6)...", ue_ip_src);
    } else {
        UPF_LOG_ERROR("  No active session found for UL data on UPF_N3_TEID: %u", upf_n3_teid_of_tunnel);
    }
}

void upf_receive_downlink_data(const char* ue_ip_dest, const char* data, uint16_t data_len) {
    UPF_LOG_INFO("Received DOWNLINK data from N6 (Internet) for UE_IP: %s, Data: '%.*s', Length: %u",
                 ue_ip_dest, data_len, data, data_len);

    bool session_found = false;
    for (int i = 0; i < MAX_UPF_SESSIONS; ++i) {
        if (g_upf_ctx.pdu_sessions[i].is_used && strcmp(g_upf_ctx.pdu_sessions[i].ue_ip_address, ue_ip_dest) == 0) {
            UPF_LOG_DEBUG("  Matching session for DL data: IMSI %lu, PDU_ID %u, gNB_N3_TEID %u, UPF_N3_TEID %u",
                          g_upf_ctx.pdu_sessions[i].imsi, g_upf_ctx.pdu_sessions[i].pdu_session_id,
                          g_upf_ctx.pdu_sessions[i].gnb_n3_teid, g_upf_ctx.pdu_sessions[i].upf_n3_teid);
            session_found = true;
            UPF_LOG_INFO("  (Simulated) Encapsulating DL data with GTP-U (UPF_TEID %u -> gNB_TEID %u) and sending to gNB via N3...",
                         g_upf_ctx.pdu_sessions[i].upf_n3_teid, g_upf_ctx.pdu_sessions[i].gnb_n3_teid);
            break;
        }
    }
    if (!session_found) {
        UPF_LOG_ERROR("  No active session found for DL data to UE_IP: %s", ue_ip_dest);
    }
}

