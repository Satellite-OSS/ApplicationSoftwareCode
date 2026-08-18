// amf/amf_sim.c
#include "amf_sim.h"
#include "../smf/smf_sim.h" // For calling SMF functions

static amf_context_t g_amf_ctx;

static amf_ue_ngap_id_t allocate_amf_ue_ngap_id() {
    if (g_amf_ctx.next_amf_ue_ngap_id_counter == 0) {
        g_amf_ctx.next_amf_ue_ngap_id_counter = 1;
    }
    AMF_LOG_DEBUG("Allocating new AMF_UE_NGAP_ID: %u", g_amf_ctx.next_amf_ue_ngap_id_counter);
    return g_amf_ctx.next_amf_ue_ngap_id_counter++;
}

static amf_ue_context_t* find_ue_context_by_imsi(imsi_t imsi) {
    for (int i = 0; i < MAX_AMF_UES; ++i) {
        if (g_amf_ctx.ue_contexts[i].is_used && g_amf_ctx.ue_contexts[i].imsi == imsi) {
            return &g_amf_ctx.ue_contexts[i];
        }
    }
    return NULL;
}

static amf_ue_context_t* find_ue_context_by_amf_id(amf_ue_ngap_id_t amf_id) {
    if (amf_id == 0) {
        AMF_LOG_DEBUG("Attempted to find UE context with invalid AMF_UE_ID: 0");
        return NULL;
    }
    for (int i = 0; i < MAX_AMF_UES; ++i) {
        if (g_amf_ctx.ue_contexts[i].is_used && g_amf_ctx.ue_contexts[i].amf_ue_id == amf_id) {
            return &g_amf_ctx.ue_contexts[i];
        }
    }
    AMF_LOG_DEBUG("UE context not found for AMF_UE_ID: %u", amf_id);
    return NULL;
}

static amf_ue_context_t* allocate_ue_context() {
    for (int i = 0; i < MAX_AMF_UES; ++i) {
        if (!g_amf_ctx.ue_contexts[i].is_used) {
            memset(&g_amf_ctx.ue_contexts[i], 0, sizeof(amf_ue_context_t));
            g_amf_ctx.ue_contexts[i].is_used = true;
            g_amf_ctx.ue_contexts[i].state = AMF_UE_STATE_DEREGISTERED;
            g_amf_ctx.ue_contexts[i].active_pdu_session_count = 0;
            for(int j=0; j < MAX_PDU_SESSIONS_PER_UE; ++j) {
                g_amf_ctx.ue_contexts[i].pdu_sessions[j].active = false;
                g_amf_ctx.ue_contexts[i].pdu_sessions[j].id = 0;
            }
            AMF_LOG_DEBUG("Allocated new UE context slot at index: %d", i);
            return &g_amf_ctx.ue_contexts[i];
        }
    }
    AMF_LOG_ERROR("Failed to allocate new UE context, pool is full!");
    return NULL;
}

static void process_nas_registration_request(
        gnb_ue_ngap_id_t received_gnb_ue_id,
        const simplified_nas_pdu_t* nas_req_pdu)
{
    AMF_LOG_INFO("Processing NAS Registration Request from gNB_UE_ID: %u, for IMSI: %lu", received_gnb_ue_id, nas_req_pdu->imsi);

    amf_ue_context_t* current_ue_ctx = find_ue_context_by_imsi(nas_req_pdu->imsi);

    if (current_ue_ctx == NULL) {
        current_ue_ctx = allocate_ue_context();
        if (current_ue_ctx == NULL) {
            AMF_LOG_ERROR("Failed for IMSI %lu: Cannot allocate UE context during registration.", nas_req_pdu->imsi);
            return;
        }
        current_ue_ctx->imsi = nas_req_pdu->imsi;
        current_ue_ctx->amf_ue_id = allocate_amf_ue_ngap_id();
        AMF_LOG_INFO("New UE registration: IMSI %lu, AMF assigned AMF_UE_ID: %u", current_ue_ctx->imsi, current_ue_ctx->amf_ue_id);
    } else {
        AMF_LOG_INFO("Existing UE: IMSI %lu, AMF_UE_ID: %u. (Re-registration or mobility update)", current_ue_ctx->imsi, current_ue_ctx->amf_ue_id);
    }

    current_ue_ctx->gnb_ue_id = received_gnb_ue_id;
    current_ue_ctx->state = AMF_UE_STATE_REGISTERING;

    AMF_LOG_INFO("IMSI %lu: (Simulated) Authentication procedure skipped.", current_ue_ctx->imsi);
    AMF_LOG_INFO("IMSI %lu: (Simulated) NAS Security Mode Control skipped.", current_ue_ctx->imsi);

    AMF_LOG_INFO("IMSI %lu: Registration procedure ongoing. Current Context: AMF_UE_ID: %u, GNB_UE_ID: %u. State: REGISTERING.",
                 current_ue_ctx->imsi, current_ue_ctx->amf_ue_id, current_ue_ctx->gnb_ue_id);

    simplified_nas_pdu_t nas_accept_pdu;
    memset(&nas_accept_pdu, 0, sizeof(simplified_nas_pdu_t));
    nas_accept_pdu.type = NAS_MSG_TYPE_REGISTRATION_ACCEPT;
    nas_accept_pdu.imsi = current_ue_ctx->imsi;
    snprintf((char*)nas_accept_pdu.payload, MAX_NAS_PDU_LENGTH, "Registration Accepted for IMSI %lu", current_ue_ctx->imsi);
    nas_accept_pdu.payload_length = strlen((char*)nas_accept_pdu.payload);

    simplified_ngap_message_t ngap_ics_req_to_gnb;
    memset(&ngap_ics_req_to_gnb, 0, sizeof(simplified_ngap_message_t));
    ngap_ics_req_to_gnb.type = NGAP_MSG_TYPE_INITIAL_CONTEXT_SETUP_REQUEST;
    ngap_ics_req_to_gnb.amf_ue_id = current_ue_ctx->amf_ue_id;
    ngap_ics_req_to_gnb.gnb_ue_id = current_ue_ctx->gnb_ue_id;
    ngap_ics_req_to_gnb.nas_pdu = nas_accept_pdu;

    AMF_LOG_INFO("IMSI %lu: Preparing NGAP InitialContextSetupRequest for gNB (AMF_UE_ID: %u, GNB_UE_ID: %u)",
                 current_ue_ctx->imsi, ngap_ics_req_to_gnb.amf_ue_id, ngap_ics_req_to_gnb.gnb_ue_id);
    AMF_LOG_DEBUG("  NGAP InitialContextSetupRequest (simulated send): NAS_Type=%d, NAS_IMSI=%lu",
                  ngap_ics_req_to_gnb.nas_pdu.type, ngap_ics_req_to_gnb.nas_pdu.imsi);
}

static void process_pdu_session_establishment_request(
        amf_ue_context_t* ue_ctx,
        const simplified_nas_pdu_t* nas_req_pdu)
{
    AMF_LOG_INFO("IMSI %lu (Context AMF_UE_ID %u): Processing NAS PDU Session Estab Request for PDU_ID %u",
                 ue_ctx->imsi, ue_ctx->amf_ue_id, nas_req_pdu->pdu_session_id);

    if (ue_ctx->active_pdu_session_count >= MAX_PDU_SESSIONS_PER_UE) {
        AMF_LOG_ERROR("IMSI %lu: Cannot establish new PDU session %u. Max PDU sessions per UE (%d) reached.",
                      ue_ctx->imsi, nas_req_pdu->pdu_session_id, MAX_PDU_SESSIONS_PER_UE);
        // TODO: Send NAS PDU Session Establishment Reject
        return;
    }

    ue_ctx->state = AMF_UE_STATE_PDU_SESSION_PENDING;

    smf_create_pdu_session_request_data_t smf_req_data;
    memset(&smf_req_data, 0, sizeof(smf_create_pdu_session_request_data_t));
    smf_req_data.imsi = ue_ctx->imsi;
    smf_req_data.pdu_session_id = nas_req_pdu->pdu_session_id;
    smf_req_data.associated_amf_ue_id = ue_ctx->amf_ue_id;
    smf_req_data.associated_gnb_ue_id = ue_ctx->gnb_ue_id;

    AMF_LOG_INFO("IMSI %lu: Sending Create PDU Session Request to SMF (PDU_ID %u, Associated AMF_UE_ID %u)",
                 ue_ctx->imsi, smf_req_data.pdu_session_id, smf_req_data.associated_amf_ue_id);

    smf_create_pdu_session_response_data_t smf_rsp_data;
    smf_handle_create_pdu_session_request(&smf_req_data, &smf_rsp_data);

    simplified_nas_pdu_t nas_response_to_ue;
    memset(&nas_response_to_ue, 0, sizeof(simplified_nas_pdu_t));
    nas_response_to_ue.imsi = ue_ctx->imsi;
    nas_response_to_ue.pdu_session_id = smf_rsp_data.pdu_session_id;

    simplified_ngap_message_t ngap_pdu_setup_req_to_gnb;
    memset(&ngap_pdu_setup_req_to_gnb, 0, sizeof(simplified_ngap_message_t));
    ngap_pdu_setup_req_to_gnb.amf_ue_id = ue_ctx->amf_ue_id;
    ngap_pdu_setup_req_to_gnb.gnb_ue_id = ue_ctx->gnb_ue_id;

    if (smf_rsp_data.success) {
        AMF_LOG_INFO("IMSI %lu: SMF successfully established PDU session %u. UE IP: %s, UPF_N3_TEID: %u",
                     ue_ctx->imsi, smf_rsp_data.pdu_session_id, smf_rsp_data.ue_ip_address, smf_rsp_data.upf_n3_teid);

        bool session_slot_found = false;
        for(int i=0; i < MAX_PDU_SESSIONS_PER_UE; ++i) {
            if (!ue_ctx->pdu_sessions[i].active) {
                ue_ctx->pdu_sessions[i].id = smf_rsp_data.pdu_session_id;
                ue_ctx->pdu_sessions[i].active = true;
                ue_ctx->active_pdu_session_count++;
                session_slot_found = true;
                AMF_LOG_DEBUG("IMSI %lu: Stored PDU session %u info in AMF context at pdu_sessions[%d]",
                              ue_ctx->imsi, smf_rsp_data.pdu_session_id, i);
                break;
            }
        }
        if (!session_slot_found) {
            AMF_LOG_ERROR("IMSI %lu: Could not find slot to store PDU session %u info, though SMF reported success.",
                          ue_ctx->imsi, smf_rsp_data.pdu_session_id);
        }

        nas_response_to_ue.type = NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_ACCEPT;
        snprintf((char*)nas_response_to_ue.payload, MAX_NAS_PDU_LENGTH,
                 "PDU Session %u established. IP: %s",
                 nas_response_to_ue.pdu_session_id, smf_rsp_data.ue_ip_address);
        nas_response_to_ue.payload_length = strlen((char*)nas_response_to_ue.payload);

        ngap_pdu_setup_req_to_gnb.type = NGAP_MSG_TYPE_PDU_SESSION_RESOURCE_SETUP_REQUEST;
        ngap_pdu_setup_req_to_gnb.nas_pdu = nas_response_to_ue;
        AMF_LOG_INFO("IMSI %lu: Preparing NGAP PDUSessionResourceSetupRequest for gNB (PDU_ID %u). N2 SM Info (UPF N3 TEID: %u, UE IP: %s)",
                     ue_ctx->imsi, nas_response_to_ue.pdu_session_id, smf_rsp_data.upf_n3_teid, smf_rsp_data.ue_ip_address);
        AMF_LOG_DEBUG("  NGAP PDUSessionResourceSetupRequest (simulated send)");

    } else {
        AMF_LOG_ERROR("IMSI %lu: SMF failed to establish PDU session for PDU_ID %u.",
                      ue_ctx->imsi, nas_req_pdu->pdu_session_id);
        ue_ctx->state = AMF_UE_STATE_REGISTERED;

        nas_response_to_ue.type = NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_REJECT;
        snprintf((char*)nas_response_to_ue.payload, MAX_NAS_PDU_LENGTH,
                 "PDU Session %u establishment failed.", nas_response_to_ue.pdu_session_id);
        nas_response_to_ue.payload_length = strlen((char*)nas_response_to_ue.payload);

        ngap_pdu_setup_req_to_gnb.type = NGAP_MSG_TYPE_DOWNLINK_NAS_TRANSPORT;
        ngap_pdu_setup_req_to_gnb.nas_pdu = nas_response_to_ue;
        AMF_LOG_INFO("IMSI %lu: Preparing NGAP DownlinkNASTransport for gNB (PDU_ID %u, Reject).",
                     ue_ctx->imsi, nas_response_to_ue.pdu_session_id);
        AMF_LOG_DEBUG("  NGAP DownlinkNASTransport (simulated send)");
    }
}


void amf_initialize() {
    AMF_LOG_INFO("Initializing AMF...");
    memset(&g_amf_ctx, 0, sizeof(amf_context_t));
    g_amf_ctx.next_amf_ue_ngap_id_counter = 1;

    for (int i = 0; i < MAX_AMF_UES; ++i) {
        g_amf_ctx.ue_contexts[i].is_used = false;
        g_amf_ctx.ue_contexts[i].active_pdu_session_count = 0;
        for(int j=0; j < MAX_PDU_SESSIONS_PER_UE; ++j) {
            g_amf_ctx.ue_contexts[i].pdu_sessions[j].active = false;
            g_amf_ctx.ue_contexts[i].pdu_sessions[j].id = 0;
        }
    }
    AMF_LOG_INFO("AMF initialization complete.");
}

void amf_handle_ngap_message_from_gnb(const simplified_ngap_message_t* ngap_msg) {
    if (ngap_msg == NULL) {
        AMF_LOG_ERROR("Received null NGAP message pointer!");
        return;
    }

    AMF_LOG_INFO("Received NGAP message from gNB: Type=%d, GNB_UE_ID=%u (from msg_header), AMF_UE_ID=%u (from msg_header, if present)",
                 ngap_msg->type, ngap_msg->gnb_ue_id, ngap_msg->amf_ue_id);

    amf_ue_context_t* ue_ctx = NULL;
    if (ngap_msg->type != NGAP_MSG_TYPE_INITIAL_UE_MESSAGE) {
        ue_ctx = find_ue_context_by_amf_id(ngap_msg->amf_ue_id);
        if (!ue_ctx) {
            AMF_LOG_ERROR("No matching UE context found for NGAP message type %d using AMF_UE_ID: %u from message",
                          ngap_msg->type, ngap_msg->amf_ue_id);
            return;
        }
        if (ue_ctx->gnb_ue_id != ngap_msg->gnb_ue_id && ngap_msg->gnb_ue_id != 0 ) {
            AMF_LOG_DEBUG("Note: GNB_UE_ID in message (%u) differs from context (%u) for AMF_UE_ID %u. This could be normal in some scenarios (e.g. UE moved). Context gNB_UE_ID might need update based on procedure.",
                          ngap_msg->gnb_ue_id, ue_ctx->gnb_ue_id, ue_ctx->amf_ue_id);
        }
    }

    switch (ngap_msg->type) {
        case NGAP_MSG_TYPE_INITIAL_UE_MESSAGE:
            AMF_LOG_DEBUG("NGAP message is InitialUEMessage.");
            if (ngap_msg->nas_pdu.type == NAS_MSG_TYPE_REGISTRATION_REQUEST) {
                AMF_LOG_DEBUG("  InitialUEMessage contains NAS Registration Request.");
                process_nas_registration_request(ngap_msg->gnb_ue_id, &ngap_msg->nas_pdu);
            } else {
                AMF_LOG_ERROR("  InitialUEMessage contains unhandled NAS PDU type: %d", ngap_msg->nas_pdu.type);
            }
            break;

        case NGAP_MSG_TYPE_UPLINK_NAS_TRANSPORT:
            AMF_LOG_DEBUG("NGAP message is UplinkNASTransport.");
            if (ue_ctx) {
                if (ngap_msg->nas_pdu.type == NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_REQUEST) {
                    AMF_LOG_INFO("IMSI %lu (Context AMF_UE_ID %u): Received PDU Session Establishment Request via Uplink NAS Transport.", ue_ctx->imsi, ue_ctx->amf_ue_id);
                    process_pdu_session_establishment_request(ue_ctx, &ngap_msg->nas_pdu);
                } else {
                    AMF_LOG_ERROR("  UplinkNASTransport (IMSI %lu) contains unhandled NAS PDU type: %d", ue_ctx->imsi, ngap_msg->nas_pdu.type);
                }
            }
            break;

        case NGAP_MSG_TYPE_INITIAL_CONTEXT_SETUP_RESPONSE:
            AMF_LOG_DEBUG("NGAP message is InitialContextSetupResponse.");
            if (ue_ctx) {
                AMF_LOG_INFO("IMSI %lu (Context AMF_UE_ID %u): gNB confirmed InitialContextSetup success. Context GNB_UE_ID: %u, Msg GNB_UE_ID: %u.",
                             ue_ctx->imsi, ue_ctx->amf_ue_id, ue_ctx->gnb_ue_id, ngap_msg->gnb_ue_id);
                if(ue_ctx->state == AMF_UE_STATE_REGISTERING){
                    ue_ctx->state = AMF_UE_STATE_REGISTERED;
                    AMF_LOG_DEBUG("IMSI %lu: State updated to REGISTERED after InitialContextSetupResponse.", ue_ctx->imsi);
                }
            }
            break;

        case NGAP_MSG_TYPE_PDU_SESSION_RESOURCE_SETUP_RESPONSE:
            AMF_LOG_DEBUG("NGAP message is PDUSessionResourceSetupResponse.");
            if (ue_ctx) {
                pdu_session_id_t resp_pdu_id = ngap_msg->nas_pdu.pdu_session_id;
                AMF_LOG_INFO("IMSI %lu (Context AMF_UE_ID %u): gNB confirmed PDUSessionResourceSetup success for PDU_ID %u.",
                             ue_ctx->imsi, ue_ctx->amf_ue_id, resp_pdu_id);

                bool session_active_in_context = false;
                for(int i=0; i < MAX_PDU_SESSIONS_PER_UE; ++i) {
                    if (ue_ctx->pdu_sessions[i].active && ue_ctx->pdu_sessions[i].id == resp_pdu_id) {
                        session_active_in_context = true;
                        break;
                    }
                }
                if (!session_active_in_context) {
                    AMF_LOG_DEBUG("IMSI %lu: PDU_ID %u from PDUSessionResourceSetupResponse was not marked active in AMF's pdu_sessions list, but gNB confirmed setup.",
                                  ue_ctx->imsi, resp_pdu_id);
                }

                if (ue_ctx->state == AMF_UE_STATE_PDU_SESSION_PENDING) {
                    ue_ctx->state = AMF_UE_STATE_PDU_SESSION_ACTIVE;
                    AMF_LOG_DEBUG("IMSI %lu, PDU_ID %u: UE overall state updated to PDU_SESSION_ACTIVE.",
                                  ue_ctx->imsi, resp_pdu_id);
                } else {
                    AMF_LOG_DEBUG("IMSI %lu, PDU_ID %u: UE state (%d) not PDU_SESSION_PENDING upon gNB PDU setup response. Current state preserved.",
                                  ue_ctx->imsi, resp_pdu_id, ue_ctx->state);
                }
            }
            break;

        default:
            AMF_LOG_ERROR("Received unhandled NGAP message type: %d", ngap_msg->type);
            break;
    }
}

void amf_trigger_ue_pdu_session_request(imsi_t imsi, pdu_session_id_t pdu_id_req) {
    AMF_LOG_INFO("Attempting to trigger PDU Session Request for IMSI %lu, PDU_ID %u", imsi, pdu_id_req);
    amf_ue_context_t* ue_ctx = find_ue_context_by_imsi(imsi);

    if (ue_ctx && ue_ctx->state == AMF_UE_STATE_REGISTERED) {
        simplified_nas_pdu_t nas_req;
        memset(&nas_req, 0, sizeof(simplified_nas_pdu_t));
        nas_req.type = NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_REQUEST;
        // nas_req.imsi = imsi;
        nas_req.pdu_session_id = pdu_id_req;
        snprintf((char*)nas_req.payload, MAX_NAS_PDU_LENGTH, "NAS PDU Session Establishment Request for PDU ID %u", pdu_id_req);
        nas_req.payload_length = strlen((char*)nas_req.payload);

        process_pdu_session_establishment_request(ue_ctx, &nas_req);
    } else if (ue_ctx) {
        AMF_LOG_ERROR("Cannot trigger PDU Session Request for IMSI %lu: UE not in suitable state (current state: %d). Expected REGISTERED.",
                      imsi, ue_ctx->state);
    } else {
        AMF_LOG_ERROR("Cannot trigger PDU Session Request for IMSI %lu: UE context not found.", imsi);
    }
}
