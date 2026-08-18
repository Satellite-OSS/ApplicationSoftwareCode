// common/common_defs.h
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef uint64_t imsi_t;
typedef uint32_t gnb_ue_ngap_id_t;
typedef uint32_t amf_ue_ngap_id_t;
typedef uint8_t  pdu_session_id_t;
typedef uint32_t teid_t;


#define MAX_AMF_UES 10
#define MAX_PDU_SESSIONS_PER_UE 2
#define MAX_SMF_PDU_SESSIONS (MAX_AMF_UES * MAX_PDU_SESSIONS_PER_UE)
#define MAX_UPF_SESSIONS MAX_SMF_PDU_SESSIONS


#define MAX_NAS_PDU_LENGTH 256
#define MAX_IMSI_LENGTH    16
#define MAX_UE_IP_ADDR_LEN 16


#define LOG_PREFIX_AMF   "[AMF_SIM]"
#define LOG_PREFIX_GNB   "[GNB_SIM]"
#define LOG_PREFIX_UE    "[UE_SIM]"
#define LOG_PREFIX_SMF   "[SMF_SIM]"
#define LOG_PREFIX_UPF   "[UPF_SIM]"
#define LOG_PREFIX_MAIN  "[MAIN_SIM]"

#define PRINT_LOG(prefix, level, fmt, ...) \
    do { \
        printf("%s[%s] " fmt "\n", prefix, level, ##__VA_ARGS__); \
        fflush(stdout); \
    } while (0)


#define AMF_LOG_INFO(fmt, ...)  PRINT_LOG(LOG_PREFIX_AMF, "INFO", fmt, ##__VA_ARGS__)
#define AMF_LOG_DEBUG(fmt, ...) PRINT_LOG(LOG_PREFIX_AMF, "DEBUG", fmt, ##__VA_ARGS__)
#define AMF_LOG_ERROR(fmt, ...) PRINT_LOG(LOG_PREFIX_AMF, "ERROR", fmt, ##__VA_ARGS__)


#define SMF_LOG_INFO(fmt, ...)  PRINT_LOG(LOG_PREFIX_SMF, "INFO", fmt, ##__VA_ARGS__)
#define SMF_LOG_DEBUG(fmt, ...) PRINT_LOG(LOG_PREFIX_SMF, "DEBUG", fmt, ##__VA_ARGS__)
#define SMF_LOG_WARN(fmt, ...)  PRINT_LOG(LOG_PREFIX_SMF, "WARN", fmt, ##__VA_ARGS__) // ***** 添加 SMF_LOG_WARN *****
#define SMF_LOG_ERROR(fmt, ...) PRINT_LOG(LOG_PREFIX_SMF, "ERROR", fmt, ##__VA_ARGS__)


#define UPF_LOG_INFO(fmt, ...)  PRINT_LOG(LOG_PREFIX_UPF, "INFO", fmt, ##__VA_ARGS__)
#define UPF_LOG_DEBUG(fmt, ...) PRINT_LOG(LOG_PREFIX_UPF, "DEBUG", fmt, ##__VA_ARGS__)
#define UPF_LOG_ERROR(fmt, ...) PRINT_LOG(LOG_PREFIX_UPF, "ERROR", fmt, ##__VA_ARGS__)



typedef enum {
    NAS_MSG_TYPE_UNKNOWN = 0,
    NAS_MSG_TYPE_REGISTRATION_REQUEST,
    NAS_MSG_TYPE_REGISTRATION_ACCEPT,
    NAS_MSG_TYPE_REGISTRATION_REJECT,
    NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_REQUEST,
    NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_ACCEPT,
    NAS_MSG_TYPE_PDU_SESSION_ESTABLISHMENT_REJECT,
} nas_message_type_t;


typedef struct {
    nas_message_type_t type;
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    uint8_t payload[MAX_NAS_PDU_LENGTH];
    uint16_t payload_length;
} simplified_nas_pdu_t;


typedef enum {
    NGAP_MSG_TYPE_UNKNOWN = 0,
    NGAP_MSG_TYPE_INITIAL_UE_MESSAGE,
    NGAP_MSG_TYPE_DOWNLINK_NAS_TRANSPORT,
    NGAP_MSG_TYPE_UPLINK_NAS_TRANSPORT,
    NGAP_MSG_TYPE_INITIAL_CONTEXT_SETUP_REQUEST,
    NGAP_MSG_TYPE_INITIAL_CONTEXT_SETUP_RESPONSE,
    NGAP_MSG_TYPE_PDU_SESSION_RESOURCE_SETUP_REQUEST,
    NGAP_MSG_TYPE_PDU_SESSION_RESOURCE_SETUP_RESPONSE,
} ngap_message_type_t;


typedef struct {
    ngap_message_type_t type;
    gnb_ue_ngap_id_t gnb_ue_id;
    amf_ue_ngap_id_t amf_ue_id;
    simplified_nas_pdu_t nas_pdu;
} simplified_ngap_message_t;



typedef struct {
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    amf_ue_ngap_id_t associated_amf_ue_id;
    gnb_ue_ngap_id_t associated_gnb_ue_id;
} smf_create_pdu_session_request_data_t;

typedef struct {
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    bool success;
    char ue_ip_address[MAX_UE_IP_ADDR_LEN];
    teid_t upf_n3_teid;
} smf_create_pdu_session_response_data_t;



typedef struct {
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    char ue_ip_address[MAX_UE_IP_ADDR_LEN];
    teid_t gnb_n3_teid;
} upf_session_establishment_request_data_t;

typedef struct {
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    bool success;
    teid_t upf_n3_teid;
} upf_session_establishment_response_data_t;



typedef enum {
    SMF_PDU_STATE_IDLE,
    SMF_PDU_STATE_CREATING,
    SMF_PDU_STATE_ACTIVE,
    SMF_PDU_STATE_MODIFYING,
    SMF_PDU_STATE_RELEASING
} smf_pdu_session_state_t;


typedef struct {
    bool is_used;
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    smf_pdu_session_state_t state;
    char ue_ip_address[MAX_UE_IP_ADDR_LEN];
    teid_t upf_n3_teid;
    teid_t gnb_n3_teid;
    amf_ue_ngap_id_t associated_amf_ue_id;
} smf_pdu_context_t;


typedef enum {
    UPF_SESSION_STATE_IDLE,
    UPF_SESSION_STATE_ACTIVE
} upf_user_session_state_t;


typedef struct {
    bool is_used;
    imsi_t imsi;
    pdu_session_id_t pdu_session_id;
    upf_user_session_state_t state;
    char ue_ip_address[MAX_UE_IP_ADDR_LEN];
    teid_t gnb_n3_teid;
    teid_t upf_n3_teid;
} upf_pdu_session_context_t;

#endif // COMMON_DEFS_H
