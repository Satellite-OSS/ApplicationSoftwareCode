// Hello, sky
#include "common/common_defs.h"
#include "amf/amf_sim.h"
#include "smf/smf_sim.h"
#include "upf/upf_sim.h" // Include UPF header

// Simulate gNB sending InitialUEMessage to AMF
// ue_imsi: UE's IMSI
// gnb_ue_id_for_procedure: ID assigned by gNB for this specific UE access/NGAP signaling connection
void gnb_sim_send_initial_ue_message_to_amf(imsi_t ue_imsi, gnb_ue_ngap_id_t gnb_ue_id_for_procedure) {
    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "UE (IMSI: %lu) initiating access, gNB uses its GNB_UE_NGAP_ID: %u for this procedure", ue_imsi, gnb_ue_id_for_procedure);
    simplified_ngap_message_t ngap_msg;
    memset(&ngap_msg, 0, sizeof(simplified_ngap_message_t));

    ngap_msg.type = NGAP_MSG_TYPE_INITIAL_UE_MESSAGE;
    ngap_msg.gnb_ue_id = gnb_ue_id_for_procedure; // Use the ID gNB assigned for this procedure
    ngap_msg.amf_ue_id = 0;                      // AMF ID is assigned by AMF at this stage

    ngap_msg.nas_pdu.type = NAS_MSG_TYPE_REGISTRATION_REQUEST;
    ngap_msg.nas_pdu.imsi = ue_imsi;
    snprintf((char*)ngap_msg.nas_pdu.payload, MAX_NAS_PDU_LENGTH, "NAS Registration Request from IMSI %lu", ue_imsi);
    ngap_msg.nas_pdu.payload_length = strlen((char*)ngap_msg.nas_pdu.payload);

    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "Sending InitialUEMessage to AMF (GNB_UE_ID in msg: %u, NAS IMSI: %lu)", ngap_msg.gnb_ue_id, ngap_msg.nas_pdu.imsi);
    amf_handle_ngap_message_from_gnb(&ngap_msg);
}

// Simulate gNB sending InitialContextSetupResponse to AMF
// target_amf_ue_id: AMF UE NGAP ID provided by AMF in InitialContextSetupRequest
// gnb_ue_id_for_procedure: GNB UE NGAP ID used by gNB for this procedure
void gnb_sim_send_initial_context_setup_response_to_amf(amf_ue_ngap_id_t target_amf_ue_id, gnb_ue_ngap_id_t gnb_ue_id_for_procedure, bool success) {
    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "Preparing InitialContextSetupResponse for AMF (Target AMF_UE_ID:%u, Own GNB_UE_ID:%u, Success:%d)",
              target_amf_ue_id, gnb_ue_id_for_procedure, success);
    simplified_ngap_message_t ngap_msg;
    memset(&ngap_msg, 0, sizeof(simplified_ngap_message_t));

    ngap_msg.type = NGAP_MSG_TYPE_INITIAL_CONTEXT_SETUP_RESPONSE;
    ngap_msg.amf_ue_id = target_amf_ue_id;
    ngap_msg.gnb_ue_id = gnb_ue_id_for_procedure;

    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "Sending InitialContextSetupResponse to AMF (AMF_UE_ID:%u, GNB_UE_ID:%u)", ngap_msg.amf_ue_id, ngap_msg.gnb_ue_id);
    amf_handle_ngap_message_from_gnb(&ngap_msg);
}

// Simulate gNB sending PDUSessionResourceSetupResponse to AMF
void gnb_sim_send_pdu_session_resource_setup_response_to_amf(amf_ue_ngap_id_t target_amf_ue_id, gnb_ue_ngap_id_t gnb_ue_id_for_procedure, pdu_session_id_t pdu_id, bool success) {
    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "Preparing PDUSessionResourceSetupResponse for AMF (Target AMF_UE_ID:%u, Own GNB_UE_ID:%u, PDU_ID:%u, Success:%d)",
              target_amf_ue_id, gnb_ue_id_for_procedure, pdu_id, success);
    simplified_ngap_message_t ngap_msg;
    memset(&ngap_msg, 0, sizeof(simplified_ngap_message_t));

    ngap_msg.type = NGAP_MSG_TYPE_PDU_SESSION_RESOURCE_SETUP_RESPONSE;
    ngap_msg.amf_ue_id = target_amf_ue_id;
    ngap_msg.gnb_ue_id = gnb_ue_id_for_procedure;
    ngap_msg.nas_pdu.pdu_session_id = pdu_id;

    PRINT_LOG(LOG_PREFIX_GNB, "INFO", "Sending PDUSessionResourceSetupResponse to AMF (AMF_UE_ID:%u, GNB_UE_ID:%u, PDU_ID:%u)",
              ngap_msg.amf_ue_id, ngap_msg.gnb_ue_id, ngap_msg.nas_pdu.pdu_session_id);
    amf_handle_ngap_message_from_gnb(&ngap_msg);
}


int main() {
    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Starting 5G Core NF Simulation (AMF, SMF, UPF) ---");

    amf_initialize();
    smf_initialize();
    upf_initialize(); // Initialize UPF

    // --- UE 1 ---
    imsi_t ue1_imsi = 100000000000001ULL;
    gnb_ue_ngap_id_t ue1_gnb_id = 101;
    amf_ue_ngap_id_t ue1_amf_id_expected = 1;
    pdu_session_id_t ue1_pdu_id = 5;
    char ue1_ip_assigned[MAX_UE_IP_ADDR_LEN] = {0};
    teid_t ue1_upf_n3_teid_assigned = 0;
    teid_t ue1_gnb_n3_teid_simulated_by_smf = 30000 + ue1_gnb_id; // Matches SMF's simulation logic

    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating UE1 (IMSI: %lu) Registration ---", ue1_imsi);
    gnb_sim_send_initial_ue_message_to_amf(ue1_imsi, ue1_gnb_id);
    gnb_sim_send_initial_context_setup_response_to_amf(ue1_amf_id_expected, ue1_gnb_id, true);

    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating UE1 (IMSI: %lu) PDU Session Establishment Request (PDU_ID: %u) ---", ue1_imsi, ue1_pdu_id);
    amf_trigger_ue_pdu_session_request(ue1_imsi, ue1_pdu_id);
    gnb_sim_send_pdu_session_resource_setup_response_to_amf(ue1_amf_id_expected, ue1_gnb_id, ue1_pdu_id, true);

    // For data plane simulation, we need the UE IP and TEIDs.
    // In a real system, AMF would get N2 SM Info (containing UE IP, UPF N3 TEID) from SMF
    // and pass it to gNB in PDUSessionResourceSetupRequest.
    // For this simulation, we'll "know" them based on SMF/UPF logic for logging.
    // Based on smf_sim.c: IP will be 10.0.0.1
    // Based on upf_sim.c: UPF N3 TEID will be 20000
    strcpy(ue1_ip_assigned, "10.0.0.1");
    ue1_upf_n3_teid_assigned = 20000; // First UPF N3 TEID allocated

    if (strlen(ue1_ip_assigned) > 0 && ue1_upf_n3_teid_assigned != 0) {
        PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating Data Flow for UE1 (IP: %s, UPF_N3_TEID: %u, GNB_N3_TEID_for_UPF: %u) ---",
                  ue1_ip_assigned, ue1_upf_n3_teid_assigned, ue1_gnb_n3_teid_simulated_by_smf);
        // Uplink: gNB sends to UPF's N3 TEID
        upf_receive_uplink_data(ue1_upf_n3_teid_assigned, ue1_ip_assigned, "Hello from UE1 to Internet!", 27);
        // Downlink: Data Network sends to UE's IP, UPF receives and forwards to gNB's N3 TEID
        upf_receive_downlink_data(ue1_ip_assigned, "Welcome to Internet from Server!", 30);
    }


    // --- UE 2 ---
    imsi_t ue2_imsi = 100000000000002ULL;
    gnb_ue_ngap_id_t ue2_gnb_id = 102;
    amf_ue_ngap_id_t ue2_amf_id_expected = 2;
    pdu_session_id_t ue2_pdu_id = 7;
    char ue2_ip_assigned[MAX_UE_IP_ADDR_LEN] = {0};
    teid_t ue2_upf_n3_teid_assigned = 0;
    teid_t ue2_gnb_n3_teid_simulated_by_smf = 30000 + ue2_gnb_id;

    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating UE2 (IMSI: %lu) Registration ---", ue2_imsi);
    gnb_sim_send_initial_ue_message_to_amf(ue2_imsi, ue2_gnb_id);
    gnb_sim_send_initial_context_setup_response_to_amf(ue2_amf_id_expected, ue2_gnb_id, true);

    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating UE2 (IMSI: %lu) PDU Session Establishment Request (PDU_ID: %u) ---", ue2_imsi, ue2_pdu_id);
    amf_trigger_ue_pdu_session_request(ue2_imsi, ue2_pdu_id);
    gnb_sim_send_pdu_session_resource_setup_response_to_amf(ue2_amf_id_expected, ue2_gnb_id, ue2_pdu_id, true);

    strcpy(ue2_ip_assigned, "10.0.0.2"); // Second IP
    ue2_upf_n3_teid_assigned = 20001; // Second UPF N3 TEID

    if (strlen(ue2_ip_assigned) > 0 && ue2_upf_n3_teid_assigned != 0) {
        PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- Simulating Data Flow for UE2 (IP: %s, UPF_N3_TEID: %u, GNB_N3_TEID_for_UPF: %u) ---",
                  ue2_ip_assigned, ue2_upf_n3_teid_assigned, ue2_gnb_n3_teid_simulated_by_smf);
        upf_receive_uplink_data(ue2_upf_n3_teid_assigned, ue2_ip_assigned, "Another packet from UE2!", 24);
        upf_receive_downlink_data(ue2_ip_assigned, "Server says hi to UE2!", 22);
    }

    PRINT_LOG(LOG_PREFIX_MAIN, "INFO", "--- 5G Core NF Simulation Finished ---");
    return 0;
}
